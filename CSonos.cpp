#include "Sonos.h"

CSonos::CSonos()
{

}

CSonos::~CSonos()
{
    curl_easy_cleanup(m_pCurl);
}

int CSonos::Start()
{
    int iDef, iRet = 0;

    iDef = ReadConfig();
    if(!(iDef & 0b0001))
    {
        cout << "Key nicht definiert in Sonos.config" << endl;
        return iRet;
    }
    if(!(iDef & 0b0010))
    {    
        cout << "Secret nicht definiert in Sonos.config" << endl;
        return iRet;
    }
    iRet = getAuthCode();
    if(iRet)
    {   
        iRet = createToken();
        if(iRet)
        {
            WriteToSonosConfig();
            cout << "Sie finden die Konfiguration in ProWo.sonos" << endl;
        }
    }   
    return iRet;
}

//
//  ReadConfig

int CSonos::ReadConfig()
{
    int iRet = 0;
    char buf[256];

    m_pReadFile = new CReadFile;
    m_pReadFile->OpenRead(pProgramPath, 1); // Sonos.config
    for (;;)
    {
        if (m_pReadFile->ReadLine())
        {
            m_pReadFile->ReadBuf(buf, ':');
            if (strncmp(buf, "Key", 3) == 0 && strlen(buf) == 3)
            {
                m_strKEY = m_pReadFile->ReadText();
                iRet |= 1;
            }
            else if (strncmp(buf, "Secret", 6) == 0 && strlen(buf) == 6)
            {
                m_strSECRET= m_pReadFile->ReadText();
                iRet |= 2;
            }                  
        }
        else
            break;
    }
    m_pReadFile->Close();
    delete m_pReadFile;
    m_pReadFile = NULL;

    return iRet;
}

// https://docs.sonos.com/reference/create-authorization-code
int CSonos::getAuthCode()
{
    string str;

    str = "<!DOCTYPE html>";
    str += "<html xmlns=\"http://www.w3.org/1999/xhtml\">";
    str + "<head>";
    str += "<title>Sonos</title>";
    str += "</head>";
    str += "<body data-init=\"\" data-bright-tag=\"false\">";
    str += "<button onclick=\"location.href=";
    m_strConnect = "'https://api.sonos.com/login/v3/oauth";
    m_strConnect += "?client_id=" + m_strKEY;
    m_strConnect += "&response_type=code";
    m_strConnect += "&state=Your_Test_State";
    m_strConnect += "&scope=playback-control-all";
    m_strConnect += "&redirect_uri=https%3A%2F%2Flocalhost'\" ";
    str += m_strConnect;
    str += "type=\"button\">";
    str += "Sign in with Sonos";
    str += "</button>";
    str += "</body>";
    str += "</html>";

    m_pReadFile = new CReadFile;
    m_pReadFile->OpenWrite(pProgramPath, 2, 0); // sonos.html
    m_pReadFile->WriteLine(str.c_str());
    m_pReadFile->Close();
    delete m_pReadFile;
    m_pReadFile = NULL;
    system("open sonos.html");
    cout << "Bitte geben Sie den erhaltenen Code ein:";
    cin >> m_strAuthorization;
    cout << "Code = " << m_strAuthorization << endl;
    if(m_strAuthorization.length())
        return 1;
    else
        return 0;
}
// https://docs.sonos.com/reference/createtoken
int CSonos::createToken()
{
    string strReadBuffer, str;
    bool bSuccess;
    CJson *pJson = new CJson;
    CJsonNodeValue JsonNodeValue;
    int iRet = 0, iErr;

    m_pCurl = curl_easy_init();
    // init
    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, 0L); 
    // Timeout auf 10 Sekunden
    curl_easy_setopt(m_pCurl, CURLOPT_CONNECTTIMEOUT, 10L);   
    curl_easy_setopt(m_pCurl, CURLOPT_ERRORBUFFER, m_curlErrorBuffer);   
    
    // Authentication
    curl_easy_setopt(m_pCurl, CURLOPT_USERNAME, m_strKEY.c_str());
    curl_easy_setopt(m_pCurl, CURLOPT_PASSWORD, m_strSECRET.c_str()); 
    curl_easy_setopt(m_pCurl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);   
    
    // URL   
    m_strConnect = "https://api.sonos.com/login/v3/oauth/access";
    curl_easy_setopt(m_pCurl, CURLOPT_URL, m_strConnect.c_str());

    // Post fields
    curl_easy_setopt(m_pCurl, CURLOPT_POST, 1L);
    str = "grant_type=authorization_code&code=" + m_strAuthorization + "&redirect_uri=https%3A%2F%2Flocalhost";
    curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, str.c_str());      
    // Empfang
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, &CSonos::write_data);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, (void *)&strReadBuffer); 

    // Header
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);   

    CURLcode ret = curl_easy_perform(m_pCurl);
    if(ret != CURLE_OK)
        cout << strReadBuffer << endl;
    else
    {    
        bSuccess = pJson->parse((char *)strReadBuffer.c_str(), strReadBuffer.length(), &iErr);
        if(bSuccess)
        {
            pJson->get((char *)"error", &JsonNodeValue);
            if(JsonNodeValue.m_iTyp)
            {
                cout << strReadBuffer << endl;
            }
            else
            {
                pJson->get((char *)"refresh_token", &JsonNodeValue);
                if(JsonNodeValue.m_iTyp) 
                {
                    m_strTOKEN = JsonNodeValue.asString();
                    iRet = 1;
                }               
            }
        }   
        else
            cout << strReadBuffer << endl;
    } 
    return iRet;
}    

size_t CSonos::write_data(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

string CSonos::ReplaceBackSlash(string str)
{
    string newStr;
    int i, len;
    char ch;

    len = str.length();
    for(i=0; i < len; i++)
    {
        ch = str.at(i);
        if(ch != 0x5c)
            newStr += ch;
    }
    return newStr;
}
int CSonos::WriteToSonosConfig()
{
    int iRet = 0;
    string strWrite;

    m_pReadFile = new CReadFile;
    m_pReadFile->OpenWrite(pProgramPath, 3); // Prowo.sonos
    strWrite = "' Definition erstellt von dem Sonos-Program\n";
    m_pReadFile->WriteLine(strWrite.c_str());    
    strWrite = "Key:" + m_strKEY + "\n";
    m_pReadFile->WriteLine(strWrite.c_str());
    strWrite = "Secret:" + m_strSECRET + "\n";
    m_pReadFile->WriteLine(strWrite.c_str());
    strWrite = "Refreshtoken:" + m_strTOKEN + "\n";
    m_pReadFile->WriteLine(strWrite.c_str());  
    strWrite = "Houshold:1 ' Normalerweise ist dieser Wert 1\n";
    m_pReadFile->WriteLine(strWrite.c_str()); 
    strWrite = "Group: ' Name der Gruppe angeben\n";
    m_pReadFile->WriteLine(strWrite.c_str());     
    m_pReadFile->Close();
    delete m_pReadFile;
    m_pReadFile = NULL; 
    return iRet;
}

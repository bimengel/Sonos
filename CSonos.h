/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CSonos.h
 * Author: josefengel
 *
 * Created on 07/10/2025
 */

#ifndef CSONOS_H
#define CSONOS_H

class CSonos{
public:
    CSonos();
    ~CSonos();
    int Start();
    
private:
    CReadFile *m_pReadFile;
    CURL *m_pCurl;
    string m_strURL;
    string m_strKEY;
    string m_strSECRET;
    string m_strTOKEN;
    string m_strUSER;
    string m_strPWD;
    string m_strConnect;
    string m_strAuthorization;
    string m_strSessionID;
    char m_curlErrorBuffer[CURL_ERROR_SIZE];

    static size_t write_data(void *contents, size_t size, size_t nmemb, void *stream);  
    int ReadConfig();
    int getAuthCode();
    int createToken();
//    int ActivateToken();
    string ReplaceBackSlash(string str);
    int WriteToSonosConfig();
};

#endif /* CSONOS_H */
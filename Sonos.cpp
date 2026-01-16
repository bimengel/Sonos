#include "Sonos.h"

char *pProgramPath;

int main(int argc, char *argv[])
{   
    int j;

//    std::filesystem::path cwd = std::filesystem::current_path();
//    string str = cwd.string() + "/";
//    j = str.length();
//    strcpy(pProgramPath, str.c_str());
    j = strlen(argv[0]);
    pProgramPath = new char[j+2];    
    strcpy(pProgramPath, argv[0]);
    for( ; j > 0; j-- )
    {
        if(pProgramPath[j-1] == '/')
        {
            pProgramPath[j] = 0;
            break;
        }
    } 
        
    CSonos *pTahoma = new CSonos;
    pTahoma->Start();
    return 01;
}
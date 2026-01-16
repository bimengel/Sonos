/* 
 * File:   CJson.h
 * Author: josefengel
 *
 * Created on February 4, 2022, 10:14 AM
 */

#ifndef CJSON_H
#define CJSON_H

class CJsonNode;

class CJsonNodeValue 
{
public:
    CJsonNodeValue();
    CJsonNodeValue(int iTyp);
    ~CJsonNodeValue();
    string asString();
    int asInt();
    double asDouble();
    bool asBool();
    
public:
    short m_iTyp; // 0 = not defined 1 = Object, 2 = Array, 3 = value, 4 = string 5 = bool 6 = null
    string m_strValue;
    double m_dValue;
    CJsonNode *m_pJsonNode;   
};

class CJsonNode
{
public:
    CJsonNode(CJsonNode *pAktJsonNode);
    int size();
public:
    CJsonNode * get(char *pName, CJsonNodeValue *pJsonNodeValue);
    CJsonNode * get(int iIdx, CJsonNodeValue *pJsonNodeValue);
    map <std::string, CJsonNodeValue *> m_mapNode;
    CJsonNode *m_pJsonNodeBefore;
    int m_iArray; 
    void WriteFile();
};

class CJson {
public:
    CJson();
    ~CJson();
    bool parse(char * pPtr, int len, int * iErr);
    CJsonNode * get(char * pName, CJsonNodeValue *pJsonNodeValue);
    void WriteToFile(string strName, int iTyp, bool bErease);
    
private:
    CJsonNode *m_pJsonNode;
    void purge(CJsonNode * pJsonNode);
    CReadFile *m_pReadFile;
    void NodeWriteToFile(CJsonNode *pJsonNode, int iLevel, int iTyp); 
    bool IsSpaceOrEnd(char ch);  
};

#endif /* CJSON_H */
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CJson.cpp
 * Author: josefengel
 * 
 * Created on February 4, 2022, 10:14 AM
 */

#include "Sonos.h"

CJson::CJson() {

    m_pJsonNode = NULL;
}

CJson::~CJson() {
    
    if(m_pJsonNode != NULL)
    {
        purge(m_pJsonNode);
    }
}

void CJson::purge(CJsonNode * pJsonNode)
{
    map<std::string, CJsonNodeValue *>::iterator it;
       
    if(!pJsonNode->m_mapNode.empty())
    {
        it = pJsonNode->m_mapNode.begin();
        while(it != pJsonNode->m_mapNode.end())
        {
                if(it->second->m_pJsonNode)
                    purge(it->second->m_pJsonNode);
                delete it->second;
                it++;
        }  
        pJsonNode->m_mapNode.clear();
        delete pJsonNode;
    }  
}

CJsonNode* CJson::get(char * pName, CJsonNodeValue *pJsonNodeValue)
{
    pJsonNodeValue->m_iTyp = 0;
    return m_pJsonNode->get(pName, pJsonNodeValue);
}

void CJson::WriteToFile(string strName, int iTyp, bool bErease)
{
    m_pReadFile = new CReadFile;
    string str;

    if(bErease)
        m_pReadFile->OpenWrite(pProgramPath, iTyp);
    else
        m_pReadFile->OpenAppend(pProgramPath, iTyp);
    
    str = strName + " {\n";
    m_pReadFile->WriteLine(str.c_str());

    NodeWriteToFile(m_pJsonNode, 1, 0);

    m_pReadFile->Close();
    delete m_pReadFile;   
}

void CJson::NodeWriteToFile(CJsonNode *pJsonNode, int iLevel, int iTyp)
{
    int i;
    string str;

    map<std::string, CJsonNodeValue *>::iterator it;
    CJsonNodeValue JsonNodeValue;

    if(pJsonNode == NULL)
        i = 1;

    if(!pJsonNode->m_mapNode.empty())
    {
        it = pJsonNode->m_mapNode.begin();
        while(it != pJsonNode->m_mapNode.end())
        {   
            str = string(iLevel*4, ' ') + "\"" + it->first + "\" : ";
            i = it->second->m_iTyp;
            JsonNodeValue = *it->second;
            it++;
            switch(i) {
            case 1: // Object
                str += "{\n";
                m_pReadFile->WriteLine(str.c_str());
                str = "";
                if(it == pJsonNode->m_mapNode.end())
                    i = 11;
                else
                    i = 1;
                NodeWriteToFile(JsonNodeValue.m_pJsonNode, iLevel+1, i);
                break;
            case 2: // Array
                str += "[\n";
                m_pReadFile->WriteLine(str.c_str());
                if(it == pJsonNode->m_mapNode.end())
                    i = 12;
                else
                    i = 2;                
                NodeWriteToFile(JsonNodeValue.m_pJsonNode, iLevel+1, i);
                str = "";
                break;
            case 3: // numeric value
                str += to_string(JsonNodeValue.m_dValue);
                break;
            case 4: // string
                str += "\"" + JsonNodeValue.m_strValue + "\"";
                break; 
            case 5: // boolean
                if(JsonNodeValue.m_dValue)
                    str += "true";
                else
                    str += "false";
                break;
            case 6: // null
                str += "null";
                break;
            default:
                str += "undefined\n";
                break;  
            }  
            if(str.length()) 
            {
                if(it != pJsonNode->m_mapNode.end() && (i == 3 || i == 4 || i == 5))
                    str += ",\n";
                else
                    str += "\n";
                m_pReadFile->WriteLine(str.c_str());                
            }
        }
    }
    switch(iTyp) {
    case 1:
        str = string(iLevel*4, ' ') + "},\n";
        break;
    case 2:
        str = string(iLevel*4, ' ') + "],\n";    
        break;
    case 0:        
    case 11:
        str = string(iLevel*4, ' ') + "}\n";    
        break;  
    case 12:
        str = string(iLevel*4, ' ') + "]\n";
        break;                  
    default:
        str = ""; 
        break;
    }
    if(str.length())
        m_pReadFile->WriteLine(str.c_str());       
}

bool CJson::parse(char* pPtr, int len, int* iErr)
{
    int iPos, iLevel, iTyp, iVal=0;
    bool bRet = false, bContinue = true; 
    int iState = 0;
    char ch;
    string strName, strValue;
    CJsonNodeValue *pJsonNodeValue;
    *iErr = 0;
    
    CJsonNode *pJsonNode = NULL;
    
    for(iPos = 0, iLevel = 0; iPos < len && bContinue; iPos++)
    {
        ch = *(pPtr + iPos);
        
        switch(iState) {
        case 0: // es muss { oder whitespace  
            if(IsSpaceOrEnd(ch))
                break;
            else if(ch == '{')
            {
                pJsonNode = new CJsonNode(pJsonNode);
                if(iLevel == 0)
                    m_pJsonNode = pJsonNode;
                iLevel++;
                iState = 1;
            }
            else if(ch == '}' || ch == ']')
            {
                iLevel--;
                if(iLevel < 0)
                    bContinue = false;
                pJsonNode = pJsonNode->m_pJsonNodeBefore;
            }
            else if(ch == ',')
                iState = 1;
            else if(ch == '[') 
            {   // neuer Knoten mit einem array als Inhalt
                
                if(pJsonNode == NULL)
                    pJsonNode = new CJsonNode(pJsonNode);
                pJsonNodeValue = new CJsonNodeValue(2);
                pJsonNodeValue->m_pJsonNode = new CJsonNode(pJsonNode);
                strName = to_string(pJsonNode->m_iArray++);
                pJsonNode->m_mapNode.insert({strName, pJsonNodeValue});
                pJsonNode = pJsonNodeValue->m_pJsonNode;
                if(iLevel == 0)
                    m_pJsonNode = pJsonNode;
                iLevel++;  
                iState = 7;
                strValue = "";
                pJsonNode->m_iArray = 1;
            }                
            else 
                bContinue = false;
            break;
        case 1: // es muss " für Namen oder whitespace
            if(IsSpaceOrEnd(ch))
                break;
            if(ch == '"')
            {
                iState = 2;
                strName = "";
            }
            else if(ch == '{')
            {
                iLevel++;
                pJsonNodeValue = new CJsonNodeValue(1);
                pJsonNodeValue->m_pJsonNode = new CJsonNode(pJsonNode);
                strName = to_string(pJsonNode->m_iArray++);
                pJsonNode->m_mapNode.insert({strName, pJsonNodeValue});  
                pJsonNode = pJsonNodeValue->m_pJsonNode;
            }
            else if(ch == '}') // wenn leere json
            {
                iLevel--;
                if(iLevel < 0)
                    bContinue = false;
                iState = 0;
                pJsonNode = pJsonNode->m_pJsonNodeBefore;
            }   
            else if(ch == '[') 
            {
                iLevel++;                    
                pJsonNodeValue = new CJsonNodeValue(2);
                strName = to_string(pJsonNode->m_iArray++);
                pJsonNodeValue->m_pJsonNode = new CJsonNode(pJsonNode);
                pJsonNode->m_mapNode.insert({strName, pJsonNodeValue});
                pJsonNode = pJsonNodeValue->m_pJsonNode;
                iState = 7;
                strValue = "";
                pJsonNode->m_iArray = 1;
            }  
            else if(isdigit(ch))  
            {
                if(pJsonNode->m_iArray)
                {
                    strValue = ch;
                    iState = 8;
                }
                else
                    bContinue = false;
            }      
            else
                bContinue = false;
            break;
        case 2:   // Name einlesen
            if(ch != '"')
                strName += ch;
            else
                iState = 3;
            break;
        case 3:     // warten auf : oder whitespace oder
            if(IsSpaceOrEnd(ch))
                break;
            if(ch == ':')
                iState = 4;
            else 
                bContinue = false;
            break;
        case 4:     // warten auf die Value (
            if(IsSpaceOrEnd(ch))
                break;
            else if(ch == '"') // es folgt eine Zeichenkette
            {
                iState = 6;
                strValue = "";
            }
            else if(ch == '[') // es folgt ein array
            { 

                iLevel++;
                pJsonNodeValue = new CJsonNodeValue(2);               
                pJsonNodeValue->m_pJsonNode = new CJsonNode(pJsonNode);
                pJsonNode->m_mapNode.insert({strName, pJsonNodeValue});
                pJsonNode = pJsonNodeValue->m_pJsonNode;
                iState = 7;
                strValue = "";
                pJsonNode->m_iArray = 1;
            }
            else if(isdigit(ch)) // numerisch
            {
                iState = 8;
                strValue = ch;
            }
            else if(ch == 'f' || ch == 't' || ch == 'n' ) // es folgt false, true oder null
            {
                strValue = ch;
                iState = 5;
            }
            else if(ch == '{')
            {
                iLevel++;
                pJsonNodeValue = new CJsonNodeValue(1);
                pJsonNodeValue->m_pJsonNode = new CJsonNode(pJsonNode);
                pJsonNode->m_mapNode.insert({strName, pJsonNodeValue});  
                pJsonNode = pJsonNodeValue->m_pJsonNode;
                iState = 1;                
            }
            else         
                bContinue = false;
            break;
        case 5:  // false oder true wird eingelesen
            if(IsSpaceOrEnd(ch))
                break;
            else if(ch == ',' || ch == '}')
            {
                iTyp = 0;
                if(strValue == "false")
                {   iTyp = 5; // boolean
                    iVal = 0;
                }
                if(strValue == "true")
                {   iTyp = 5; // boolean
                    iVal = 1;
                }
                if(strValue == "null")
                    iTyp = 6; // null
                if(!iTyp) // strValue ist nicht false, true oder null!!!
                    bContinue = false;
                else
                {
                    if(pJsonNode->m_iArray)
                    {
                        pJsonNodeValue = new CJsonNodeValue(2);
                        strName = to_string(pJsonNode->m_iArray++);
                    }
                    else
                    {   
                        pJsonNodeValue = new CJsonNodeValue(iTyp);
                        if(iTyp == 5)
                            pJsonNodeValue->m_dValue = (double)iVal;
                    }
                    pJsonNode->m_mapNode.insert({strName, pJsonNodeValue});
                    iState = 0;
                    iPos--; 
                }               
            }
            else
                strValue += ch;
            break;
        case 6:     // einlesen einer Zeichenkette
            if(ch == '"')
            {
                if(pJsonNode->m_iArray)
                {
                    strName = to_string(pJsonNode->m_iArray++);
                    iState = 9;
                }
                else
                    iState = 0;
                pJsonNodeValue = new CJsonNodeValue(4);
                pJsonNodeValue->m_strValue = strValue;
                pJsonNode->m_mapNode.insert({strName, pJsonNodeValue});
            }
            else
                strValue += ch;
            break;
        case 7: // es folgt ein Array
            if(IsSpaceOrEnd(ch))
                break;
            else if (ch == ']') // es ist ein leeres array
            {
                iLevel--;
                if(iLevel < 0)
                    bContinue = false;
                pJsonNode = pJsonNode->m_pJsonNodeBefore; 
                iState = 0;               
            }
            else if(ch == '"')
                iState = 6;
            else if(isdigit(ch))
            {
                strValue += ch;            
                iState = 8;
            }
            else if(ch == '{') 
            {
                iLevel++;
                pJsonNodeValue = new CJsonNodeValue(1);
                pJsonNodeValue->m_pJsonNode = new CJsonNode(pJsonNode);
                strName = to_string(pJsonNode->m_iArray++);
                pJsonNode->m_mapNode.insert({strName, pJsonNodeValue});  
                pJsonNode = pJsonNodeValue->m_pJsonNode;
                iState = 1;
            }
            else if(ch == '[') 
            {
                iLevel++;
                pJsonNodeValue = new CJsonNodeValue(2);
                strName = to_string(pJsonNode->m_iArray++);
                pJsonNodeValue->m_pJsonNode = new CJsonNode(pJsonNode);
                pJsonNode->m_mapNode.insert({strName, pJsonNodeValue});
                pJsonNode = pJsonNodeValue->m_pJsonNode;
                iState = 7;
                strValue = "";
                pJsonNode->m_iArray = 1;                
            }
            else
                bContinue = false;
            break;
        case 8: // numerischer Wert
            if(isdigit(ch) || ch == '.') 
                strValue += ch;
            else if(IsSpaceOrEnd(ch))
                break;
            else if(ch == ',' || ch == '}' || ch == ']') 
            {
                iState = 0;
                if(pJsonNode->m_iArray)
                    strName = to_string(pJsonNode->m_iArray++);
                pJsonNodeValue = new CJsonNodeValue(3);
                pJsonNodeValue->m_dValue = strtod(strValue.c_str(), NULL);
                pJsonNode->m_mapNode.insert({strName, pJsonNodeValue});
                iPos--;
            }  
            else
                bContinue = false;  
            break;
        case 9: // ein array Wert wurde eingelesen, es muss , oder ] folgen
            if(IsSpaceOrEnd(ch))
                break;
            else if(ch == ',')
                iState = 4; // strValue einlesen
            else if(ch == ']')
            {
                iLevel--;
                if(iLevel < 0)
                    bContinue = false;
                pJsonNode = pJsonNode->m_pJsonNodeBefore;
                iState = 0;
            }
            else
                bContinue = false;
            break;
        default:
            bContinue = false;
            break;
        }
    }
    if(!iLevel && bContinue)
        bRet = true;
    else
        *iErr = iPos;
    
    return bRet;
}

bool CJson::IsSpaceOrEnd(char ch)
{
    if(ch == ' ' || ch == 0x0a || ch == 0x0d)
        return true;
    else
        return false;
}
CJsonNode::CJsonNode(CJsonNode *pJsonNode)
{
    m_pJsonNodeBefore = pJsonNode;
    m_iArray = 0;
}

int CJsonNode::size()
{
    return m_mapNode.size();
}

CJsonNode* CJsonNode::get(char * pName, CJsonNodeValue *pJsonNodeValue)
{
    CJsonNode *pJsonNode = this;
  
    auto search = m_mapNode.find(pName);
    if(search != m_mapNode.end())
    {
        *pJsonNodeValue = *(CJsonNodeValue *)search->second;
        if(pJsonNodeValue->m_pJsonNode != NULL)
            pJsonNode = pJsonNodeValue->m_pJsonNode;
    }
        
    return pJsonNode;
}
CJsonNode * CJsonNode::get(int iIdx, CJsonNodeValue *pJsonNodeValue)
{
    CJsonNode *pJsonNode = this;
    string str = to_string(iIdx + 1);
    
    auto search = m_mapNode.find(str);
    if(search != m_mapNode.end())
    {
        *pJsonNodeValue = *(CJsonNodeValue *)search->second;
        if(pJsonNodeValue->m_pJsonNode != NULL)
            pJsonNode = pJsonNodeValue->m_pJsonNode;
    }
        
    return pJsonNode;    
}
CJsonNodeValue::CJsonNodeValue()
{
    m_strValue = "";
    m_dValue = 0;
    m_pJsonNode = NULL; 
    m_iTyp = 0;
}

CJsonNodeValue::CJsonNodeValue(int iTyp)
{
    m_iTyp = iTyp; // 0 = not defined 1 = Object, 2 = Array, 3 = value, 4 = string
    m_strValue = "";
    m_dValue = 0;
    m_pJsonNode = NULL;
}

CJsonNodeValue::~CJsonNodeValue()
{
    
}

string CJsonNodeValue::asString()
{
    string strRet;

    switch(m_iTyp) {
        case 0:
            strRet = "not defined";
            break;
        case 1:
            strRet = "object";
            break;
        case 2:
            strRet = "array";
            break;
        case 3:
            strRet = to_string(m_dValue);
            break;
        case 4:
            strRet = m_strValue;
            break;
        case 5:
            if(m_dValue)
                strRet = "true";
            else
                strRet = "false";
            break;
        case 6:
            strRet = "null";
            break;
        default:
            strRet = "undefined";
            break;
    }

    return strRet;
}

int CJsonNodeValue::asInt()
{
    return((int)m_dValue);
}

double CJsonNodeValue::asDouble()
{
    return m_dValue;
}

bool CJsonNodeValue::asBool()
{
    bool bRet = false;
    if(m_dValue)
        bRet = true;
    return bRet;
}

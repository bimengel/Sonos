/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * ProWo
 * Copyright (C) root 2014 <root@raspberrypi>
 * 
ProWo is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ProWo is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CREADFILE_H_
#define _CREADFILE_H_

class CReadFile
{
public:
    CReadFile();
    int OpenRead(char *pPath, int type, int nr=0, int iTest=0);
    int OpenWrite(char *pPath, int type, int nr=0);
    int OpenWriteExisting(char *pPath, int type, int nr=0);    
    int OpenAppend(char *pPath, int type, int nr=0);
    bool OpenAppendBinary(char *pPath, int type, int nr=0);   
    void Close();
    void Error(int error);
    int FindText(char *searchText);
    int ReadBuf(char *buf, int len);
    int ReadBuf(char *buf, char ch);
    int ReadLine(); 
    int WriteLine(const char *pBuf);
    int WriteLine(const char *pBuf, int len);
    int ReadNumber();   // nur Zahlen, ohne Vorzeichen
    int ReadZahl();	    // Zahlen, auch mit Vorzeichen
    string ReadAlpha(); // nur alphabetische Zeichen
    string ReadText();  
    string ReadText(char chTrennzeichen);
    bool ReadEqual();
    bool ReadSeparator();
    char GetChar();
    char GetChar(int iIdx);
    void ResetLine();
    static string GetName(char *pPath, int type, int nr=0);    
    string GetFileName(char *pPath, int type, int nr);
    void ControlPath(string str);
    // Speziell für das Menu einlesen
    int CountSpace();
    bool ReadBinary(char *pBuf, int len);
	long GetFilePos();
    bool SetFilePos(long lPos);
    string GetError();
    bool IsDigit();
    int GetLine();
    
protected:
    int m_iType;
    int m_iLen;
    int m_iPos;
    FILE *m_fp;
    char m_cBuf[256];
    int m_iZeile;
    string m_strFileName;
    string m_strError;
private:

};

#endif // _CREADFILE_H_

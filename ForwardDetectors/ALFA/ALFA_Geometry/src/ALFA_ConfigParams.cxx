/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// DefaultParams.cpp: implementation of the ALFA_ConfigParams class.
//
//////////////////////////////////////////////////////////////////////

#include "ALFA_Geometry/ALFA_ConfigParams.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void ALFA_ConfigParams::UnInitialize()
{
	m_bIsValid=false;
	m_strSection="invalid";
	m_mapParams.clear();
}

int ALFA_ConfigParams::Init(const char *szFile, const char *szSection)
{
	bool bRes=false;
	FILE* pfile=nullptr;
	char szbuff[256],szsec[256];
	char* ppv;

	UnInitialize();

	if((pfile=fopen(szFile,"r"))==nullptr) return false;
	
	//read section
	memset(szsec,0,sizeof(szsec));
	if(szSection) strncpy(szsec,szSection,sizeof(szsec)-1);
	//strlwr(szsec);
	m_strSection=szsec;

	while(!feof(pfile)){
		fgets(szbuff,sizeof(szbuff),pfile);
		if(*(szbuff+strlen(szbuff)-1)=='\n') *(szbuff+strlen(szbuff)-1)=0;
		if(strcmp(szbuff,szsec) != 0) continue; //if(strcmp(strlwr(szbuff),szsec)) continue;
		else{ bRes=true; break;	}
	}
	
	if(!bRes){
		fclose(pfile);
		return 0;
	}

	while(!feof(pfile)){
		fgets(szbuff,sizeof(szbuff),pfile);
		if(*szbuff==0 || *szbuff==' ' || *szbuff=='\n' || *szbuff==';') continue;
		if(*szbuff=='[') break;
		if((ppv=strchr(szbuff,'='))==nullptr) continue;

		if(*(szbuff+strlen(szbuff)-1)=='\n') *(szbuff+strlen(szbuff)-1)=0;
		*ppv=0; ppv++;
		m_mapParams.insert(StringStringMap::value_type(szbuff,ppv)); 
	}

	fclose(pfile);
	if(!m_mapParams.empty()) m_bIsValid=true;

	return m_mapParams.size();
}

const char* ALFA_ConfigParams::GetParameter(const char *szKey) const
{
	if(!m_bIsValid) return nullptr;
	
	char szbuff[256];
	memset(szbuff,0,sizeof(szbuff));
	if(szKey) strncpy(szbuff,szKey,sizeof(szbuff)-1);
	//strlwr(szbuff);
	
	StringStringMap::const_iterator iter;
	if((iter=m_mapParams.find(szbuff))!=m_mapParams.end()){
		return (*iter).second.c_str();
	}
	else return nullptr;
}

bool ALFA_ConfigParams::IsKey(const char *szKey) const
{
	if(!m_bIsValid) return false;

	char szbuff[256];
	memset(szbuff,0,sizeof(szbuff));
	if(szKey) strncpy(szbuff,szKey,sizeof(szbuff)-1);
	//strlwr(szbuff);
	
	StringStringMap::const_iterator iter;
	return (iter=m_mapParams.find(szbuff))!=m_mapParams.end();
}


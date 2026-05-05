/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AlfaConfigParams_h
#define AlfaConfigParams_h 1

#include <string>
#include <map>

////////////////////////////////////////////////////////////////////////////////////////////////////


class ALFA_ConfigParams
{
	public:
    using StringStringMap = std::map<std::string, std::string, std::less<> >;
	private:
		bool m_bIsValid{};
		std::string m_strSection{"invalid"};
		StringStringMap m_mapParams;

	public:
		bool IsKey(const char* szKey) const;
		const char* GetParameter(const char* szKey) const;
		int Init(const char* szFile, const char* szSection);
		void UnInitialize();
};

#endif // AlfaConfigParams_h

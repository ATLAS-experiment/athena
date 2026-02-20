/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "PersistentDataModelTPCnv/DataHeader_p3.h"

DataHeaderElement_p3::DataHeaderElement_p3() : m_clids(),
	m_token(),
	m_alias(),
	m_technology(0U),
	m_oid1(0U),
	m_oid2(0U),
	m_dbGuidIdx(0U),
	m_classIdIdx(0U),
	m_prefixIdx(0U),
	m_keyPos(0U),
	m_hashes() {}


const std::vector<unsigned int>& DataHeaderElement_p3::clids() const {
   return(m_clids);
}

const std::string& DataHeaderElement_p3::token() const {
   return(m_token);
}

const std::vector<std::string>& DataHeaderElement_p3::alias() const {
   return(m_alias);
}

const std::string& DataHeaderElement_p3::key() const {
   return(m_alias.front());
}

unsigned int DataHeaderElement_p3::pClid() const {
   return(m_clids.front());
}

unsigned int DataHeaderElement_p3::oid1() const {
   return(m_oid1);
}

unsigned int DataHeaderElement_p3::oid2() const {
   return(m_oid2);
}


DataHeader_p3::DataHeader_p3()
	: m_DataHeader(), m_InputDataHeader(), m_GuidMap() {}

const std::vector<DataHeaderElement_p3>& DataHeader_p3::elements() const {
   return(m_DataHeader);
}

const std::vector<DataHeaderElement_p3>& DataHeader_p3::inputElements() const {
   return(m_InputDataHeader);
}

const std::vector<std::string>& DataHeader_p3::GuidMap() const {
   return(m_GuidMap);
}

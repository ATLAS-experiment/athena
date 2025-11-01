/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "PersistentDataModelTPCnv/DataHeader_p4.h"

    static_assert(std::is_nothrow_move_constructible<DataHeaderElement_p4>::value);
    static_assert(std::is_nothrow_move_constructible<DataHeader_p4>::value);

DataHeaderElement_p4::DataHeaderElement_p4() : m_clids(),
	m_token(),
	m_alias(),
	m_technology(0U),
	m_oid1(0U),
	m_oid2(0U),
	m_dbGuidIdx(0U),
	m_classIdIdx(0U),
	m_prefixIdx(0U),
	m_keyPos(0U),
	m_hashes() {

   }

unsigned int DataHeaderElement_p4::pClid() const {
   return(m_clids.front());
}

const std::vector<unsigned int>& DataHeaderElement_p4::clids() const {
   return(m_clids);
}

const std::string& DataHeaderElement_p4::key() const {
   return(m_alias.front());
}

const std::vector<std::string>& DataHeaderElement_p4::alias() const {
   return(m_alias);
}

const std::string& DataHeaderElement_p4::token() const {
   return(m_token);
}

unsigned int DataHeaderElement_p4::oid1() const {
   return(m_oid1);
}

unsigned int DataHeaderElement_p4::oid2() const {
   return(m_oid2);
}


DataHeader_p4::DataHeader_p4() : m_dataHeader(), m_provSize(0U), m_guidMap() {}

const std::vector<DataHeaderElement_p4>& DataHeader_p4::elements() const {
   return(m_dataHeader);
}

unsigned int DataHeader_p4::provenanceSize() const {
   return(m_provSize);
}

const std::vector<std::string>& DataHeader_p4::guidMap() const {
   return(m_guidMap);
}

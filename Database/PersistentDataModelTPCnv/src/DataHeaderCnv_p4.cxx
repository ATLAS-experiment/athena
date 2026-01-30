/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file DataHeaderCnv_p4.cxx
 *  @brief This file contains the implementation for the DataHeaderCnv_p4 class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 *  $Id: DataHeaderCnv_p4.cxx,v 1.3 2009-05-15 01:01:57 gemmeren Exp $
 **/

#include "PersistentDataModel/DataHeader.h"
#include "PersistentDataModelTPCnv/DataHeaderCnv_p4.h"

#include <cstdio>
#include <sstream>
#include <algorithm>

DataHeaderElementCnv_p4::DataHeaderElementCnv_p4() {}

//______________________________________________________________________________
void DataHeaderElementCnv_p4::persToTrans(const DataHeaderElement_p4* pers,
	DataHeaderElement* trans,
	const std::vector<std::string>& map) {
   std::vector<unsigned int>::const_iterator intIter = pers->m_clids.begin();
   const std::vector<unsigned int>::const_iterator intLast = pers->m_clids.end();
   trans->m_pClid = *intIter; ++intIter;
   trans->m_clids.assign (intIter, intLast);
   std::vector<std::string>::const_iterator strIter = pers->m_alias.begin();
   trans->m_key = *strIter; ++strIter;
   trans->m_alias.assign (strIter, pers->m_alias.end());
   if (!std::ranges::is_sorted (trans->m_alias)) {
     // Should really be sorted, but just in case...
     std::ranges::sort (trans->m_alias);
     auto ret = std::ranges::unique (trans->m_alias);
     trans->m_alias.erase (ret.begin(), ret.end());
   }
   trans->m_hashes.clear();
   trans->m_hashes.reserve(pers->m_hashes.size());
   for (std::vector<uint64_t>::const_iterator iter = pers->m_hashes.begin(),
		   last = pers->m_hashes.end(); iter != last; ++iter) {
      trans->m_hashes.push_back((unsigned int)(*iter));
   }
// Translate PoolToken
   std::string tokenStr;
   if (map.empty()) {
      tokenStr = pers->m_token;
   } else {
      unsigned int tokenSize = 129 + pers->m_token.size();
      if (pers->m_prefixIdx > 0) {
         tokenSize += map[pers->m_prefixIdx].size();
      }
      if (pers->m_keyPos > 0) {
         tokenSize += trans->m_key.size();
      }
      tokenStr.reserve(tokenSize);
// Append DbGuid
      tokenStr = "[DB=";
      tokenStr.append(map[pers->m_dbGuidIdx]);
// Still optimize
      tokenStr.append("][CNT=");
      std::string cntName;
      if (pers->m_prefixIdx > 0) {
         tokenStr.append(map[pers->m_prefixIdx]);
      }
// insert key
      if (pers->m_keyPos > 0) {
         tokenStr.append(pers->m_token.substr(0, pers->m_keyPos - 1));
         tokenStr.append(trans->m_key);
         tokenStr.append(pers->m_token.substr(pers->m_keyPos - 1));
      } else {
         tokenStr.append(pers->m_token);
      }
// Append ClassId
      tokenStr.append("][CLID=");
      tokenStr.append(map[pers->m_classIdIdx]);
// Add Technology and Offsets
      const unsigned int length = 40;
      char text[length];
      snprintf(text, length, "][TECH=%08X][OID=%08X-%08X]", pers->m_technology, pers->m_oid1, pers->m_oid2);
      tokenStr.append(text);
   }
   trans->m_token.fromString(tokenStr);
}
//______________________________________________________________________________
void DataHeaderElementCnv_p4::transToPers(const DataHeaderElement* /*trans*/,
	DataHeaderElement_p4* /*pers*/,
	std::vector<std::string>& /*map*/) {
// Moved to next _p version
}
//______________________________________________________________________________
//______________________________________________________________________________
DataHeaderCnv_p4::DataHeaderCnv_p4() {}
//______________________________________________________________________________
void DataHeaderCnv_p4::persToTrans(const DataHeader_p4* pers, DataHeader* trans) {
   trans->m_inputDataHeader.resize(pers->m_provSize);
   std::vector<DataHeaderElement>::iterator it = trans->m_inputDataHeader.begin();
   std::vector<DataHeaderElement_p4>::const_iterator pit = pers->m_dataHeader.begin();
   for (unsigned int i = 0U; i != pers->m_provSize; ++i, ++it, ++pit) {
      m_elemCnv.persToTrans(&(*pit), &(*it), pers->m_guidMap);
   }
   trans->m_dataHeader.resize(pers->m_dataHeader.size() - pers->m_provSize);
   it = trans->m_dataHeader.begin();
   for (std::vector<DataHeaderElement_p4>::const_iterator last = pers->m_dataHeader.end();
		   pit != last; ++it, ++pit) {
      m_elemCnv.persToTrans(&(*pit), &(*it), pers->m_guidMap);
   }
}
//______________________________________________________________________________
void DataHeaderCnv_p4::transToPers(const DataHeader* /*trans*/, DataHeader_p4* /*pers*/) {
// Moved to next _p version
}
//______________________________________________________________________________
void DataHeaderCnv_p4::insertDHRef(DataHeader_p4* /*pers*/,
	const std::string& /*key*/,
	const std::string& /*token*/) {
// Moved to next _p version
}

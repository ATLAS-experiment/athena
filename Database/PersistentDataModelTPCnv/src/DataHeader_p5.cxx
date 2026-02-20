/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "PersistentDataModelTPCnv/DataHeader_p5.h"

#include "CxxUtils/MD5.h"

#include <uuid/uuid.h>
#include<type_traits>
#include <sstream>


static_assert(std::is_nothrow_move_constructible<DataHeaderElement_p5>::value);
static_assert(std::is_nothrow_move_constructible<DataHeaderForm_p5>::value);
static_assert(std::is_nothrow_move_constructible<DataHeader_p5>::value);

DataHeaderElement_p5::DataHeaderElement_p5() : m_token(), m_oid2(0U) {}

const std::string& DataHeaderElement_p5::token() const {
   return(m_token);
}

long long int DataHeaderElement_p5::oid2() const {
   return(m_oid2);
}


DataHeaderForm_p5::DataHeaderForm_p5() : m_map(), m_uints() {}

const std::vector<std::string>& DataHeaderForm_p5::map() const {
   return(m_map);
}

void DataHeaderForm_p5::insertMap(const std::string& element) {
   m_map.push_back(element);
}

const std::vector<unsigned int>& DataHeaderForm_p5::params(unsigned int entry) const {
   return(m_uints[entry - 1]);
}

void DataHeaderForm_p5::insertParam(unsigned int param, unsigned int entry) {
   m_uints[entry - 1].push_back(param);
}

unsigned int DataHeaderForm_p5::size() const {
   return(m_uints.size());
}

void DataHeaderForm_p5::resize(unsigned int size) {
   m_uints.resize(size);
}


DataHeader_p5::DataHeader_p5() : m_dataHeader(), m_dhFormToken(), m_dhFormMdx() {}

const std::vector<DataHeaderElement_p5>& DataHeader_p5::elements() const {
   return(m_dataHeader);
}

const std::string& DataHeader_p5::dhFormToken() const {
   return(m_dhFormToken);
}

void DataHeader_p5::setDhFormToken(const std::string& formToken,
                                   const DataHeaderForm_p5& dhForm)
{
  m_dhFormToken = formToken;
  std::ostringstream stream;
  for (const std::string& s : dhForm.map()) {
    stream << s << "\n";
  }
  for (unsigned int entry = 1; entry <= dhForm.size(); ++entry) {
    for (unsigned int x : dhForm.params(entry)) {
      stream << x << ",";
    }
    stream << "\n";
  }
  MD5 checkSum((unsigned char*)stream.str().c_str(), stream.str().size());
  uuid_t checkSumUuid;
  checkSum.raw_digest((unsigned char*)(&checkSumUuid));
  char text[37];
  uuid_unparse_upper(checkSumUuid, text);
  m_dhFormMdx = text;
}
const std::string& DataHeader_p5::dhFormMdx() const {
   return(m_dhFormMdx);
}

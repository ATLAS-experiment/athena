/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file DataHeader.cxx
 *  @brief This file contains the implementation for the DataHeader and DataHeaderElement classes.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "PersistentDataModel/DataHeader.h"
#include "PersistentDataModel/TokenAddress.h"

#include "SGTools/TransientAddress.h"
#include "SGTools/DataProxy.h"
#include "AthenaKernel/IStringPool.h"

//______________________________________________________________________________
DataHeaderElement::DataHeaderElement() : m_pClid(0), m_clids(), m_key(), m_alias(), m_hashes() {
}
//______________________________________________________________________________
DataHeaderElement::DataHeaderElement(const DataHeaderElement& rhs) : m_pClid(rhs.m_pClid),
	m_clids(rhs.m_clids),
	m_key(rhs.m_key),
	m_alias(rhs.m_alias),
        m_token(rhs.getToken()),
	m_hashes(rhs.m_hashes) {
}
//______________________________________________________________________________
DataHeaderElement::DataHeaderElement(const SG::TransientAddress* sgAddress, IOpaqueAddress* tokAddress,
                                     const std::string& pTag)
  : DataHeaderElement (sgAddress->clID(),
                       sgAddress->name(),
                       sgAddress->transientID(),
                       SG::DataProxy::AliasCont_t (sgAddress->alias()),
                       sgAddress->address(),
                       tokAddress, pTag)
{
}
//______________________________________________________________________________
DataHeaderElement::DataHeaderElement(const SG::DataProxy* proxy, IOpaqueAddress* tokAddress,
                                     const std::string& pTag)
  : DataHeaderElement (proxy->clID(),
                       proxy->name(),
                       proxy->transientID(),
                       proxy->alias(),
                       proxy->address(),
                       tokAddress, pTag)
{
}
//______________________________________________________________________________
DataHeaderElement::DataHeaderElement(CLID clid,
                                     const std::string& name,
                                     const std::vector<CLID>& tClids,
                                     std::set<std::string>&& alias,
                                     IOpaqueAddress* tadAddress,
                                     IOpaqueAddress* tokAddress,
                                     const std::string& pTag)
  : m_pClid(clid),
    m_clids(tClids),
    m_key((pTag.empty()) ? name : pTag),
    m_alias(std::move(alias)),
    m_hashes()
{
  std::ranges::sort (m_clids);
  const auto ret = std::ranges::unique (m_clids);
  m_clids.erase (ret.begin(), ret.end());
  std::erase (m_clids, m_pClid);
  TokenAddress* tokAddr = dynamic_cast<TokenAddress*>(tokAddress);
  if (tokAddr != 0 && tokAddr->getToken() != 0) {
    tokAddr->getToken()->setData (&m_token);
  } else {
    tokAddr = dynamic_cast<TokenAddress*>(tadAddress);
    if (tokAddr != 0 && tokAddr->getToken() != 0) {
      tokAddr->getToken()->setData (&m_token);
    } else if (tokAddress != 0) {
      m_token.fromString(*(tokAddress->par()));
    } else if (tadAddress != 0) {
      m_token.fromString(*(tadAddress->par()));
    }
  }
}
//______________________________________________________________________________
DataHeaderElement::DataHeaderElement(const CLID classID,
	const std::string& key, Token&& token)
	: m_pClid(classID), m_clids(), m_key(key), m_alias(), m_token(std::move(token)), m_hashes() {
}
//______________________________________________________________________________
DataHeaderElement::~DataHeaderElement() {
}
//______________________________________________________________________________
DataHeaderElement& DataHeaderElement::operator=(const DataHeaderElement& rhs) {
   if (this != &rhs) {
      m_pClid = rhs.m_pClid;
      m_clids = rhs.m_clids;
      m_key = rhs.m_key;
      m_alias = rhs.m_alias;
      rhs.getToken()->setData(&m_token);
      m_hashes = rhs.m_hashes;
   }
   return(*this);
}
//______________________________________________________________________________
CLID DataHeaderElement::getPrimaryClassID() const {
   if (m_pClid > 0) {
      return(m_pClid);
   }
   return(m_clids.front());
}
//______________________________________________________________________________
const std::set<CLID> DataHeaderElement::getClassIDs() const {
  std::set<CLID> allClids (m_clids.begin(), m_clids.end());
  allClids.insert(m_pClid);
  return(allClids);
}
//______________________________________________________________________________
const std::string& DataHeaderElement::getKey() const {
   return(m_key);
}
//______________________________________________________________________________
const std::set<std::string>& DataHeaderElement::getAlias() const {
   return(m_alias);
}
//_____________________________________________________________________________
const Token* DataHeaderElement::getToken() const {
   return(&m_token);
}
//_____________________________________________________________________________
long DataHeaderElement::getStorageType() const {
   return(POOL_StorageType);
}
//_____________________________________________________________________________
const std::vector<DataHeaderElement::sgkey_t>&
DataHeaderElement::getHashes() const {
   return(m_hashes);
}
//______________________________________________________________________________
void DataHeaderElement::addHash(IStringPool* pool) {
   if (!m_hashes.empty()) {
      return;
   }
   std::set<CLID> clids = getClassIDs();
   for (std::set<CLID>::iterator iter = clids.begin(), last = clids.end(); iter != last; ++iter) {
      m_hashes.push_back(pool->stringToKey(m_key, *iter));
   }
}
//______________________________________________________________________________
SG::TransientAddress* DataHeaderElement::getAddress(unsigned long contextId) const {
   return(getAddress(m_key, contextId));
}
//______________________________________________________________________________
SG::TransientAddress* DataHeaderElement::getAddress(const std::string& key,
	unsigned long contextId) const {
   CLID primaryClID = getPrimaryClassID();
   TokenAddress* tokAdd = new TokenAddress(this->getStorageType(), primaryClID, "", m_key, contextId , &m_token);
   SG::TransientAddress* sgAddress = new SG::TransientAddress(primaryClID, key, tokAdd, m_clids);
   sgAddress->setAlias(m_alias);
   return(sgAddress);
}
//______________________________________________________________________________
void DataHeaderElement::dump(std::ostream& ostr) const
{
   using namespace std;
   ostr << "SGKey: " << getKey() << endl;
   ostr << "CLID: " << getPrimaryClassID();
   if( getClassIDs().size() > 1 ) {
      ostr << " ||";
      for( auto& c : getClassIDs() ) ostr << " " << c;
   }
   ostr << std::endl;
   if( getAlias().size() > 0 ) {
      ostr << "Alias: ";
      for( auto& a : getAlias() ) ostr << " " << a;
      ostr << endl;
   }
   ostr << "Token: " << m_token.toString() << endl;
   if( m_hashes.size() ) {
      ostr << "Hashes:";
      for( auto h : m_hashes ) ostr <<  " " << h;
      ostr << endl;
   }
}

//______________________________________________________________________________
//______________________________________________________________________________
DataHeader::DataHeader() : m_dataHeader(),
	m_inputDataHeader(),
	m_status(statusFlag(Other)),
	m_processTag(),
	m_attrList(0),
	m_evtRefTokenStr() {
}
//______________________________________________________________________________
DataHeader::DataHeader(const DataHeader& rhs) :
        DataObject (rhs),
        m_dataHeader(rhs.m_dataHeader),
	m_inputDataHeader(rhs.m_inputDataHeader),
	m_status(rhs.m_status),
	m_processTag(rhs.m_processTag),
	m_attrList(rhs.m_attrList),
	m_evtRefTokenStr() {
}
//______________________________________________________________________________
DataHeader::~DataHeader() {
}
//______________________________________________________________________________
DataHeader& DataHeader::operator=(const DataHeader& rhs) {
   if (this != &rhs) {
      m_dataHeader = rhs.m_dataHeader;
      m_inputDataHeader = rhs.m_inputDataHeader;
      m_status = rhs.m_status;
      m_processTag = rhs.m_processTag;
      m_attrList = rhs.m_attrList;
   }
   return(*this);
}
//______________________________________________________________________________
void DataHeader::setStatus(statusFlag status) {
   m_status = status;
}
//______________________________________________________________________________
bool DataHeader::isInput() const {
   return(m_status == statusFlag(Input));
}
//______________________________________________________________________________
void DataHeader::setProcessTag(const std::string& processTag) {
   m_processTag = processTag;
}
//______________________________________________________________________________
const std::string& DataHeader::getProcessTag() const {
   return(m_processTag);
}
//______________________________________________________________________________
int DataHeader::size() const {
   return(m_dataHeader.size());
}
//______________________________________________________________________________
std::vector<DataHeaderElement>::const_iterator DataHeader::begin() const {
   return(m_dataHeader.begin());
}
//______________________________________________________________________________
std::vector<DataHeaderElement>::const_iterator DataHeader::end() const {
   return(m_dataHeader.end());
}
//______________________________________________________________________________
const std::vector<DataHeaderElement>& DataHeader::elements() const {
   return m_dataHeader;
}
//______________________________________________________________________________
void DataHeader::insert(const SG::TransientAddress* sgAddress, IOpaqueAddress* tokAddress, const std::string& pTag) {
   if (sgAddress != 0) {
      m_dataHeader.emplace_back(sgAddress, tokAddress, pTag);
   }
}
//______________________________________________________________________________
void DataHeader::insert(const SG::DataProxy* proxy, IOpaqueAddress* tokAddress, const std::string& pTag) {
   m_dataHeader.emplace_back(proxy, tokAddress, pTag);
}
//______________________________________________________________________________
void DataHeader::insert(const DataHeaderElement& dhe) {
   m_dataHeader.push_back(dhe);
}
//______________________________________________________________________________
int DataHeader::sizeProvenance() const {
   return(m_inputDataHeader.size());
}
//______________________________________________________________________________
std::vector<DataHeaderElement>::const_iterator DataHeader::beginProvenance() const {
   return(m_inputDataHeader.begin());
}
//______________________________________________________________________________
std::vector<DataHeaderElement>::const_iterator DataHeader::endProvenance() const {
   return(m_inputDataHeader.end());
}
//______________________________________________________________________________
void DataHeader::insertProvenance(const DataHeaderElement& dhe) {
   m_inputDataHeader.push_back(dhe);
}
//______________________________________________________________________________
void DataHeader::addHash(IStringPool* pool) {
   for (std::vector<DataHeaderElement>::iterator iter = m_dataHeader.begin(),
		   last = m_dataHeader.end(); iter != last; ++iter) {
      iter->addHash(pool);
   }
   for (std::vector<DataHeaderElement>::iterator iter = m_inputDataHeader.begin(),
		   last = m_inputDataHeader.end(); iter != last; ++iter) {
      iter->addHash(pool);
   }
}
//______________________________________________________________________________
const std::string& DataHeader::dhFormToken() const
{
  return m_dhFormToken;
}
//______________________________________________________________________________
void DataHeader::setDhFormToken(const std::string& formToken)
{
  m_dhFormToken = formToken;
}
//______________________________________________________________________________
void DataHeader::setAttributeList(const coral::AttributeList* attrList) {
   m_attrList = attrList;
}
//______________________________________________________________________________
const coral::AttributeList* DataHeader::getAttributeList() {
   return(m_attrList);
}
//______________________________________________________________________________
void DataHeader::setEvtRefTokenStr(const std::string& tokenStr) {
   m_evtRefTokenStr = tokenStr;
}
//______________________________________________________________________________
const std::string& DataHeader::getEvtRefTokenStr() {
   return(m_evtRefTokenStr);
}
//______________________________________________________________________________
#include "CoralBase/AttributeList.h"
#include "CoralBase/Attribute.h"

void DataHeader::dump(std::ostream& ostr) const
{
   using namespace std;
   ostr << "--- DataHeader Dump ---" << endl;
   for( auto& el : m_dataHeader ) {
      el.dump(ostr);
   }
   for( auto& el : m_inputDataHeader ) {
      el.dump(ostr);
   }
   ostr << "Status: " << m_status << endl;
   ostr << "Proc tag: " << m_processTag << endl;
   ostr << "evtRef: " << m_evtRefTokenStr << endl;
   ostr << "attrListPtr: " << m_attrList << endl;
   if( m_attrList ) ostr << "attrListSize: " << m_attrList->size() << endl;
   ostr << "--- DataHeader End ---" << endl;   
}
//______________________________________________________________________________
void DataHeader::recycle()
{
}

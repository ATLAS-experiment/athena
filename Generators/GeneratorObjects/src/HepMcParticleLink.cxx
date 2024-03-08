/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/



// Don't use the StoreGateSvc interface here, so that this code
// will work in root too.

#include "GeneratorObjects/HepMcParticleLink.h"
#include "GeneratorObjects/McEventCollection.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenEvent.h"
#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "SGTools/CurrentEventStore.h"
#include "SGTools/DataProxy.h"
#include <sstream>
#include <cassert>


namespace {


/**
 * @brief StoreGate keys to try
 */
constexpr int NKEYS = 5;
const
std::string s_keys[NKEYS] = {"TruthEvent","G4Truth","GEN_AOD","GEN_EVENT","Bkg_TruthEvent"};


/**
 * @brief Hint about where to start searching.
 *
 * On a successful SG lookup of a McEventCollection, we store here the index
 * in the s_keys array of the key that worked.  We can that start the
 * search at that position the next time.
 */
std::atomic<unsigned> s_hint = NKEYS;


const unsigned short CPTRMAXMSGCOUNT = 100;


} // anonymous namespace


//**************************************************************************
// ExtendedBarCode
//


/**
 * @brief Dump in textual format to a stream.
 */
void HepMcParticleLink::ExtendedBarCode::print (std::ostream& os) const
{
  os << "Event index " ;
  index_type index, position;
  eventIndex (index, position);
  if (position != UNDEFINED) {
    os << position << " (position in collection) ";
  }
  else {
    os << index;
  }
  os << ", Barcode " << m_BC
     << ", McEventCollection "
     << HepMcParticleLink::getLastEventCollectionName();
}


/**
 * @brief Dump in textual format to a MsgStream.
 */
void HepMcParticleLink::ExtendedBarCode::print (MsgStream& os) const
{
  std::ostringstream ss;
  print (ss);
  os << ss.str();
}


//**************************************************************************
// HepMcParticleLink
//


/**
 * @brief Constructor.
 * @param p Particle to reference.
 * @param eventIndex Identifies the target GenEvent in a McEventCollection,
 *        as either the event number if @c isIndexEventPosition is IS_EVENTNUM,
 *        or the position in the container
 *        if isIndexEventPosition is IS_POSITION.
 *        0 always means the first event in the collection.
 * @param positionFlag: See @c eventIndex.
 * @param sg Optional specification of a specific store to reference.
 */
HepMcParticleLink::HepMcParticleLink (const HepMC::ConstGenParticlePtr& part,
                                      uint32_t eventIndex,
                                      PositionFlag positionFlag /*= IS_EVENTNUM*/,
                                      IProxyDict* sg /*= SG::CurrentEventStore::store()*/)
  : m_store (sg),
    m_ptr (part),
    m_extBarcode((nullptr != part) ? HepMC::barcode(part) : 0, eventIndex, positionFlag, HepMcParticleLink::IS_BARCODE)
{
  assert(part);

  if (part != nullptr && positionFlag == IS_POSITION) {
    if (const McEventCollection* pEvtColl = retrieveMcEventCollection(sg)) {
      const HepMC::GenEvent *pEvt = pEvtColl->at (eventIndex);
      m_extBarcode.makeIndex (pEvt->event_number(), eventIndex);
    }
    else {
      MsgStream log (Athena::getMessageSvc(), "HepMcParticleLink");
      log << MSG::WARNING << "cptr: McEventCollection not found" << endmsg;
    }
  }
}


/**
 * @brief Dereference.
 */
HepMC::ConstGenParticlePtr HepMcParticleLink::cptr() const
{
  // dummy link
  const bool is_valid = m_ptr.isValid();
  if (!is_valid && !m_store) {
    return nullptr;
  }
  if (is_valid) return *m_ptr.ptr();
    if (0 == barcode()) {
#if 0
      MsgStream log (Athena::getMessageSvc(), "HepMcParticleLink");
      log << MSG::DEBUG
             << "cptr: no truth particle associated with this hit (barcode==0)."
             << " Probably this is a noise hit" << endmsg;
#endif
      return nullptr;
    }
    IProxyDict* sg = m_store;
    if (!sg) {
      sg = SG::CurrentEventStore::store();
    }
    if (const McEventCollection* pEvtColl = retrieveMcEventCollection(sg)) {
      const HepMC::GenEvent *pEvt = nullptr;
      index_type index, position;
      m_extBarcode.eventIndex (index, position);
      if (index == 0) {
        pEvt = pEvtColl->at(0);
      }
      else if (position != ExtendedBarCode::UNDEFINED) {
        if (position < pEvtColl->size()) {
          pEvt = pEvtColl->at (position);
        }
        else {
#if 0
          MsgStream log (Athena::getMessageSvc(), "HepMcParticleLink");
          log << MSG::WARNING << "cptr: position = " << position << ", McEventCollection size = "<< pEvtColl->size() << endmsg;
#endif
          return nullptr;
        }
      }
      else {
        pEvt = pEvtColl->find (index);
      }

      if (nullptr != pEvt) {
        // Be sure to update m_extBarcode before m_ptrs;
        // otherwise, the logic in eventIndex() won't work correctly.
        if (position != ExtendedBarCode::UNDEFINED) {
          m_extBarcode.makeIndex (pEvt->event_number(), position);
        }
        if (barcode() != 0) {
         const HepMC::ConstGenParticlePtr p = HepMC::barcode_to_particle(pEvt,barcode());
          if (p) {
            m_ptr.set (p);
            return p;
          }
        }
      } else {
        MsgStream log (Athena::getMessageSvc(), "HepMcParticleLink");
        if (position != ExtendedBarCode::UNDEFINED) {
          log << MSG::WARNING
            << "cptr: Mc Truth not stored for event at " << position
            << endmsg;
        } else {
          log << MSG::WARNING
            << "cptr: Mc Truth not stored for event with event number " << index
            << endmsg;
        }
      }
    } else {
      MsgStream log (Athena::getMessageSvc(), "HepMcParticleLink");
      log << MSG::WARNING << "cptr: McEventCollection not found" << endmsg;
    }
  return nullptr;
}


/**
 * @brief Return the event number of the referenced GenEvent.
 *        0 means the first GenEvent in the collection.
 */
HepMcParticleLink::index_type HepMcParticleLink::eventIndex() const
{
  // dummy link
  if (!m_ptr.isValid() && !m_store) {
    return ExtendedBarCode::UNDEFINED;
  }

  index_type event_number, event_position;
  m_extBarcode.eventIndex (event_number, event_position);
  if (event_number == ExtendedBarCode::UNDEFINED) {
    const HepMC::GenEvent* pEvt{};
    if (const McEventCollection* coll = retrieveMcEventCollection (m_store)) {
      if (event_position < coll->size()) {
        pEvt = coll->at (event_position);
      }
      if (pEvt) {
        const int genEventNumber = pEvt->event_number();
        // Be sure to update m_extBarcode before m_ptr.
        // Otherwise, if two threads run this method simultaneously,
        // one thread could see event_number == UNDEFINED, but where m_ptr
        // is already updated so we get nullptr back for sg.
        if (genEventNumber > -1) {
          event_number = static_cast<index_type>(genEventNumber);
          m_extBarcode.makeIndex (event_number, event_position);
          return event_number;
        }
        if (barcode() != 0) {
          HepMC::ConstGenParticlePtr pp = HepMC::barcode_to_particle(pEvt, barcode());
          if (pp) {
            m_ptr.set (pp);
          }
        }
      }
    }
  }
  // Don't trip the assertion for a null link.
  if (barcode() == 0 ) {  // || barcode() == 0x7fffffff)
    if (event_number != ExtendedBarCode::UNDEFINED) {
      return event_number;
    }
    return 0;
  }
  cptr();
  m_extBarcode.eventIndex (event_number, event_position);
  assert (event_number != ExtendedBarCode::UNDEFINED);
  return event_number;
}


/**
 * @brief Return the position in the McEventCollection of the
 *        (first) GenEvent with a given event number
 */
HepMcParticleLink::index_type
HepMcParticleLink::getEventPositionInCollection (const IProxyDict* sg) const
{
  index_type index, position;
  m_extBarcode.eventIndex (index, position);
  if (position != ExtendedBarCode::UNDEFINED) {
    return position;
  }
  if (index == 0) {
    return 0;
  }

  std::vector<index_type> positions = getEventPositionInCollection(index, sg);
  return positions[0];
}


/**
 * @brief Return the position in the McEventCollection of the
 *        (first) GenEvent with a given event number
 */
std::vector<HepMcParticleLink::index_type>
HepMcParticleLink::getEventPositionInCollection (index_type index, const IProxyDict* sg)
{
  std::vector<index_type> positions; positions.reserve(1);
  const int intIndex = static_cast<int>(index);
  if (const McEventCollection* coll = retrieveMcEventCollection (sg)) {
    size_t sz = coll->size();
    for (size_t i = 0; i < sz; i++) {
      if ((*coll)[i]->event_number() == intIndex) {
        positions.push_back(i);
      }
    }
  }
  if (positions.empty() ) {
    positions.push_back(ExtendedBarCode::UNDEFINED);
  }
  return positions;
}


/**
 * @brief Return the event number of the GenEvent at the specified
 *        position in the McEventCollection.
 */
int HepMcParticleLink::getEventNumberAtPosition (index_type position, const IProxyDict* sg)
{
  if (const McEventCollection* coll = retrieveMcEventCollection (sg)) {
    if (position < coll->size()) {
      return coll->at (position)->event_number();
    }
  }
#if 0
  MsgStream log (Athena::getMessageSvc(), "HepMcParticleLink");
  log << MSG::WARNING << "getEventNumberAtPosition: position = " << position << ", McEventCollection size = "<< coll->size() << endmsg;
#endif
  return -999;
}


/**
 * @brief Alter the persistent part of the link.
 */
void HepMcParticleLink::setExtendedBarCode (const ExtendedBarCode& extBarcode)
{
  m_extBarcode = extBarcode;
  m_store = SG::CurrentEventStore::store();
  m_ptr.reset();
}


/**
 * @brief Look up the event collection we're targeting.
 * @param sg Target event store.
 * May return nullptr if the collection is not found.
 */
const McEventCollection*
HepMcParticleLink::retrieveMcEventCollection (const IProxyDict* sg)
{
  const McEventCollection* pEvtColl = nullptr;
  SG::DataProxy* proxy = find_proxy (sg);
  if (proxy) {
    pEvtColl = SG::DataProxy_cast<McEventCollection> (proxy);
    if (!pEvtColl) {
      MsgStream log (Athena::getMessageSvc(), "HepMcParticleLink");
      log << MSG::WARNING << "cptr: McEventCollection not found" << endmsg;
    }
  }
  return pEvtColl;
}

/**
 * @brief Find the proxy for the target event collection.
 * @param sg Target event store.
 * May return nullptr if the collection is not found.
 */
SG::DataProxy* HepMcParticleLink::find_proxy (const IProxyDict* sg)
{
  const CLID clid = ClassID_traits<McEventCollection>::ID();
  unsigned int hint_orig = s_hint;
  if (hint_orig >= NKEYS) hint_orig = 0;
  unsigned int hint = hint_orig;
  do {
    SG::DataProxy* proxy = sg->proxy (clid, s_keys[hint]);
    if (proxy) {
      if (hint != s_hint) {
        s_hint = hint;
      }
      static std::atomic<unsigned> findCount {0};
      if(++findCount == 1) {
        MsgStream log (Athena::getMessageSvc(), "HepMcParticleLink");
        log << MSG::INFO << "find_proxy: Using " << s_keys[hint]
            <<" as McEventCollection key for this job " << endmsg;
      }
      return proxy;
    }
    ++hint;
    if (hint >= NKEYS) hint = 0;
  } while (hint != hint_orig);

  MsgStream log (Athena::getMessageSvc(), "HepMcParticleLink");
  static std::atomic<unsigned long> msgCount {0};
  unsigned int count = ++msgCount;
  if (count <= CPTRMAXMSGCOUNT) {
    log << MSG::WARNING << "find_proxy: No Valid MC event Collection found "
        <<   endmsg;
  }
  if (count == CPTRMAXMSGCOUNT) {
    log << MSG::WARNING <<"find_proxy: suppressing further messages about valid MC event Collection. Use \n"
        << "  msgSvc.setVerbose += [HepMcParticleLink]\n"
        << "to see all messages" << endmsg;
  }
  if (count > CPTRMAXMSGCOUNT) {
    log << MSG::VERBOSE << "find_proxy: No Valid MC event Collection found "
        << endmsg;
  }
  return nullptr;
}


/**
 * @brief Return the most recent SG key used for a particular collection type.
 */
std::string HepMcParticleLink::getLastEventCollectionName ()
{
  static const std::string unset =  "CollectionNotSet";
  unsigned idx = s_hint;
  if (idx < NKEYS) {
    return s_keys[idx];
  }
  return unset;
}


/**
 * @brief Output operator.
 * @param os Stream to which to output.
 * @param link Link to dump.
 */
std::ostream&
operator<< (std::ostream& os, const HepMcParticleLink& link)
{
  link.m_extBarcode.print(os);
  return os;
}


/**
 * @brief Output operator.
 * @param os MsgStream to which to output.
 * @param link Link to dump.
 */
MsgStream&
operator<< (MsgStream& os, const HepMcParticleLink& link)
{
  link.m_extBarcode.print(os);
  return os;
}

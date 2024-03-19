/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/



// Don't use the StoreGateSvc interface here, so that this code
// will work in root too.

#include "GeneratorObjects/HepMcParticleLink.h"
#include "GeneratorObjects/McEventCollection.h"
#include "TruthUtils/MagicNumbers.h"
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
  index_type event_number, position;
  eventIndex (event_number, position);
  if (position != UNDEFINED) {
    os << position << " (position in collection) ";
  }
  else {
    os << event_number << " (event number) ";
  }
  os << ", Unique ID " ;
  barcode_type particle_id, particle_barcode;
  uniqueID (particle_id, particle_barcode);
  if (particle_barcode == 0 && particle_id == 0) {
    os << " 0 (id/barcode) ";
  }
  else if (particle_barcode != UNDEFINEDBC) {
    os << particle_barcode << " (barcode) ";
  }
  else {
    os << particle_id << " (id) ";
  }
  os << ", McEventCollection "
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
    m_extBarcode((nullptr != part) ? HepMC::uniqueID(part) : 0, eventIndex, positionFlag, IS_ID)
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
  if (is_valid) {
    return *m_ptr.ptr();
  }
  if (m_extBarcode.linkIsNull()) {
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
    index_type event_number, position;
    m_extBarcode.eventIndex (event_number, position);
    if (event_number == 0) {
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
      pEvt = pEvtColl->find (event_number);
    }

    if (nullptr != pEvt) {
      // Be sure to update m_extBarcode before m_ptrs;
      // otherwise, the logic in eventIndex() won't work correctly.
      if (position != ExtendedBarCode::UNDEFINED) {
        m_extBarcode.makeIndex (pEvt->event_number(), position);
      }
      if (event_number == 0) {
        m_extBarcode.makeIndex (pEvt->event_number(), position);
      }
      if ( !m_extBarcode.linkIsNull() ) { // Check that either the ID or Barcode is non-zero or undefined
        barcode_type particle_id, particle_barcode;
        m_extBarcode.uniqueID (particle_id, particle_barcode);
        if (particle_id == ExtendedBarCode::UNDEFINEDBC) {
          // barcode to GenParticle
          const HepMC::ConstGenParticlePtr p = HepMC::barcode_to_particle(pEvt,int(particle_barcode));
          if (p) {
            int genParticleID = HepMC::uniqueID(p);
            if (genParticleID > -1) {
              particle_id = static_cast<barcode_type>(genParticleID);
              m_extBarcode.makeID (particle_id, particle_barcode);
            }
            m_ptr.set (p);
            return p;
          }
        }
        else {
          // id to GenParticle
#ifdef HEPMC3
          const auto &particles = pEvt->particles();
          if (particle_id-1 < particles.size()) {
            const HepMC::ConstGenParticlePtr p = particles[particle_id-1];
            if (p) {
              m_ptr.set (p);
              return p;
            }
          }
#else
          const HepMC::ConstGenParticlePtr p = HepMC::barcode_to_particle(pEvt,int(particle_id)); // For HepMC2 "id" == barcode
          if (p) {
            m_ptr.set (p);
            return p;
          }
#endif
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
            << "cptr: Mc Truth not stored for event with event number " << event_number
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
 * @brief Return the id of the referenced GenParticle.
 */
int HepMcParticleLink::id() const
{
  // if m_BC is zero (delta rays), then just return that same for barcode and id
  if (m_extBarcode.uid()) {
    // dummy link
    if (!m_ptr.isValid() && !m_store) {
      return 0; // TODO Decide if this is a good default - constant from MagicNumbers.h instead?
    }

    barcode_type particle_id, particle_barcode;
    m_extBarcode.uniqueID (particle_id, particle_barcode);
    if (particle_id == ExtendedBarCode::UNDEFINEDBC) {
      (void) cptr(); // FIXME be careful to avoid an infinite loop of calls here
      m_extBarcode.uniqueID (particle_id, particle_barcode);
    }
    if (particle_id != ExtendedBarCode::UNDEFINEDBC) {
      return particle_id;
    }
  }
  return 0;
}


/**
 * @brief Return the barcode of the target particle.  0 for a null link.
 * FIXME: return type.
 */
int HepMcParticleLink::barcode() const
{
  // if m_BC is zero (delta rays), then just return that same for barcode and id
  if (m_extBarcode.uid()) {
    barcode_type particle_id, particle_barcode;
    m_extBarcode.uniqueID (particle_id, particle_barcode);
    if (particle_barcode != ExtendedBarCode::UNDEFINEDBC) {
      return int(particle_barcode);
    }
    // dummy link
    if (!m_ptr.isValid() && !m_store) {
      return 0; // TODO Decide if this is a good default - constant from MagicNumbers.h instead?
    }
    // we will need to look up the barcode from the GenParticle
    (void) eventIndex(); // FIXME be careful to avoid an infinite loop of calls here
    if (cptr()) {
      return HepMC::barcode(cptr());
    }
  }
  return 0;
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
  if ( m_extBarcode.linkIsNull() )
    {
      return (event_number != ExtendedBarCode::UNDEFINED) ? event_number : 0;
    }
  // Attempt to find the GenParticle
  cptr();
  // Check if event_number is valid once more
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
  index_type event_number, position;
  m_extBarcode.eventIndex (event_number, position);
  if (position != ExtendedBarCode::UNDEFINED) {
    return position;
  }
  if (event_number == 0) {
    return 0;
  }

  std::vector<index_type> positions = getEventPositionInCollection(event_number, sg);
  return positions[0];
}


/**
 * @brief Return the position in the McEventCollection of the
 *        (first) GenEvent with a given event number
 */
std::vector<HepMcParticleLink::index_type>
HepMcParticleLink::getEventPositionInCollection (index_type event_number, const IProxyDict* sg)
{
  std::vector<index_type> positions; positions.reserve(1);
  const int int_event_number = static_cast<int>(event_number);
  if (const McEventCollection* coll = retrieveMcEventCollection (sg)) {
    size_t sz = coll->size();
    for (size_t i = 0; i < sz; i++) {
      if ((*coll)[i]->event_number() == int_event_number) {
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

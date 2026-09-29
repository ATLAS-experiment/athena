/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/ConcurrencyFlags.h"
#include "AthenaKernel/CloneService.h"
#include "AthenaKernel/errorcheck.h"
#include "AthenaKernel/StoreID.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/tools/SGImplSvc.h"
#include "StoreGate/tools/SGMsgStreamMacros.h"
#include "StoreGate/SGHiveMgrSvc.h"

using namespace SG;

__thread HiveEventSlot* s_current(0);

HiveMgrSvc::HiveMgrSvc(const std::string& name, 
                       ISvcLocator* svc) : base_class(name, svc),
                                           m_hiveStore(StoreID::storeName(StoreID::EVENT_STORE), name),
                                           m_nSlots(1)
{
  declareProperty("HiveStoreSvc", m_hiveStore);
  declareProperty("NSlots", m_nSlots, "number of event slots");
}


/** Set number of concurrent processes
 *
 * This can only be called by "friends" of this class. Its sole purpose
 * is to have a common entry point within ATLAS to call the private
 * methods of Gaudi::ConcurrencyFlags.
 *
 * @param numProcs  [IN]   Number of concurrent processes
 */
void HiveMgrSvc::setNumProcs(size_t numProcs)
{
  Gaudi::Concurrency::ConcurrencyFlags::setNumProcs(numProcs);
}

/** Activate an given 'slot' for all subsequent calls within the
 * same thread id.
 *
 * @param  slot     [IN]     Slot number (event slot)   *
 * @return Status code indicating failure or success.
 */
StatusCode HiveMgrSvc::selectStore(size_t slotIndex) {
  s_current = &m_slots[slotIndex];
  StoreGateSvc::setSlot(s_current);
  return StatusCode::SUCCESS;
}

/** Clear a given 'slot'.
 *
 * @param  slot     [IN]     Slot number (event slot)   *
 * @return Status code indicating failure or success.
 */
StatusCode HiveMgrSvc::clearStore(size_t slotIndex) {
  StatusCode rc(StatusCode::FAILURE);
  if (slotIndex < m_nSlots) {
    rc=m_slots[slotIndex].pEvtStore->clearStore();
    if (rc.isSuccess()) {
      SG_MSG_DEBUG ("cleared store {}", slotIndex);
    }
  }    
  if (!rc.isSuccess()) {
    SG_MSG_ERROR ("could not clear store {}", slotIndex);
  }
  return rc;
}
  
/** Set the number of 'slots'.
 *
 * @param  slot     [IN]     Slot number (event slot)   *
 * @return Status code indicating failure or success.
 */
StatusCode HiveMgrSvc::setNumberOfStores(size_t slots) {
  //FIXME what if running?
  if(FSMState() ==  Gaudi::StateMachine::INITIALIZED) {
    SG_MSG_FATAL ("Too late to change the number of slots!");
    return StatusCode::FAILURE;
  } else {
    m_slots.resize(slots);
    m_nSlots = slots;
    m_freeSlots.store(slots);
    Gaudi::Concurrency::ConcurrencyFlags::setNumConcEvents( slots );
    return StatusCode::SUCCESS;
  }
}

size_t HiveMgrSvc::getNumberOfStores() const {
  return m_nSlots;
}


  
/** Allocate a store slot for new event
 *
 * @param     evtnumber     [IN]     Event number
 * @param     slot     [OUT]    Returned slot or slot number
 * @return Slot number (npos to indicate an error).
 */
size_t HiveMgrSvc::allocateStore( int evtNumber ) {
  if (m_freeSlots == 0) {
    SG_MSG_ERROR ("No slots available for event number {}", evtNumber);
    return std::string::npos;
  }
  std::scoped_lock lock{m_mutex};
  for (size_t index=0; index<m_nSlots; ++index) {
    if( m_slots[index].eventNumber == evtNumber) {
      SG_MSG_ERROR ("Attempt to allocate an event slot for an event that is still active: event number {}", evtNumber);
      return std::string::npos;
    } else if (m_slots[index].eventNumber == -1) {
      m_slots[index].eventNumber = evtNumber;
      SG_MSG_DEBUG ("Slot {} allocated to event number {}", index, evtNumber);
      m_freeSlots--;
      return index;
    }
  }
  SG_MSG_ERROR ("No slots available for event number {}", evtNumber);
  return std::string::npos;
}
  
/** Free a store slot
 *
 * @param     slot     [IN]     Slot number
 * @return Status code indicating failure or success.
 */
StatusCode HiveMgrSvc::freeStore( size_t slotIndex ) {
  if (slotIndex < m_nSlots) {
    std::scoped_lock lock{m_mutex};
    if (m_slots[slotIndex].eventNumber == -1) {
      SG_MSG_DEBUG ("Slot {} is already free", slotIndex);
    }
    else {
      m_slots[slotIndex].eventNumber = -1;
      m_freeSlots++;
      SG_MSG_DEBUG ("Freed slot {}", slotIndex);
    }
    return StatusCode::SUCCESS;
  } else {
    SG_MSG_ERROR ("no slot at {}", slotIndex);
    return StatusCode::FAILURE;
  }
}
  
  
/** Get the slot number corresponding to a given event
 *
 * @param     evtNumber     [IN]     Event number
 * @return    slot number (npos to indicate an error).
 */
size_t HiveMgrSvc::getPartitionNumber(int evtNumber) const {
  std::scoped_lock lock{m_mutex};
  for (size_t index=0; index<m_nSlots; ++index) {
    if( m_slots[index].eventNumber == evtNumber) return index;
  }
  return std::string::npos;
}

size_t HiveMgrSvc::freeSlots() {
  return m_freeSlots;
}

bool HiveMgrSvc::exists( const DataObjID& id) {
  // this should only get called in error situations, so we
  // don't care if it's slow
  std::string key = id.key();
  key.erase(0,key.find('+')+1);

  if (id.clid() == 0) {
    // this is an ugly hack in case the DataObjID gets munged
    // upstream, and we have to re-separate it into (class,key)
    // from "class/key"
    std::string cl = id.fullKey();
    cl.erase(cl.find('/'),cl.length());

    DataObjID d2(std::move(cl),key);
    return m_hiveStore->transientContains(d2.clid(), key);
  } else {    
    return m_hiveStore->transientContains(id.clid(), key);
  }
 
} 

StatusCode HiveMgrSvc::initialize() {
  SG_MSG_VERBOSE ("Initializing {}", name());

  if ( !(Service::initialize().isSuccess()) )  {
    SG_MSG_FATAL ("Unable to initialize base class");
    return StatusCode::FAILURE;
  }
  //this sets the hiveStore pointer to StoreGateSvc.defaultStore
  if (!(m_hiveStore.retrieve()).isSuccess()) {
    SG_MSG_FATAL ("Unable to get hive event store");
    return StatusCode::FAILURE;
  }

  //use hiveStore default impl store as prototype
  Service* child(0);
  SGImplSvc* pSG(0);

  for( size_t i = 0; i< m_nSlots; ++i) {
    std::ostringstream oss;
    std::print (oss, "{}_{}", i, m_hiveStore->currentStore()->name());
    if (CloneService::clone(m_hiveStore->currentStore(), oss.str(), child).isSuccess() &&
        child->initialize().isSuccess() &&
        0 != (pSG = dynamic_cast<SGImplSvc*>(child)) )
      {
        pSG->setSlotNumber (i, m_nSlots);
        m_slots.push_back(SG::HiveEventSlot(pSG));
      } else {
      SG_MSG_FATAL ("Unable to clone event store {}", oss.str());
      return StatusCode::FAILURE;
    }
  }
  
  m_freeSlots.store( m_nSlots );
  Gaudi::Concurrency::ConcurrencyFlags::setNumConcEvents( m_nSlots );

  return selectStore(0);
}

StatusCode HiveMgrSvc::finalize() {
  SG_MSG_INFO ("Finalizing {}", name());

  for (SG::HiveEventSlot& s : m_slots) {
    // The impl services are not set to active, so ServiceManager
    // won't finalize them.
    CHECK( s.pEvtStore->finalize() );
    s.pEvtStore->release();
  }

  return StatusCode::SUCCESS;
}


StatusCode HiveMgrSvc::start()
{
  // On a start transition, merge the string pool from the default store
  // into that of each store, so that they will know about any explicit
  // registrations that were done during initialize().
  // See ATEAM-846.
  for (SG::HiveEventSlot& slot : m_slots) {
    slot.pEvtStore->mergeStringPool (*m_hiveStore->currentStore());
  }
  return StatusCode::SUCCESS;
}

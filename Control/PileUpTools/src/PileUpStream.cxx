/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PileUpTools/PileUpStream.h"

#include <stdexcept>
#include <string>

#include "GaudiKernel/Service.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ISvcManager.h"
#include "AthenaKernel/CloneService.h"
#include "AthenaKernel/IAddressProvider.h"
#include "StoreGate/StoreGateSvc.h"

#include "SGTools/DataProxy.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"


class IOpaqueAddress;

/// Structors
PileUpStream::PileUpStream():
  AthMessaging ("PileUpStream"),
  m_name("INVALID"),
  m_mergeSvc("PileUpMergeSvc", m_name)
{
}

PileUpStream::PileUpStream(PileUpStream&& rhs):
  PileUpStream()
{
  *this = std::move(rhs);
}

PileUpStream&
PileUpStream::operator=(PileUpStream&& rhs)
{
  if (this != &rhs) {
    m_name=rhs.m_name;
    p_svcLoc=rhs.p_svcLoc;
    m_sel=rhs.m_sel;
    m_SG=rhs.m_SG;
    p_iter=rhs.p_iter;
    m_mergeSvc = rhs.m_mergeSvc;
    m_ownEvtIterator=rhs.m_ownEvtIterator;
    m_neverLoaded=rhs.m_neverLoaded;
    m_ownStore=rhs.m_ownStore;
    m_used=rhs.m_used;
    m_hasRing=rhs.m_hasRing;
    m_iOriginalRing=rhs.m_iOriginalRing;
    //transferred ownership
    rhs.m_ownEvtIterator=false;
    rhs.m_ownStore=false;
  }
  return *this;
}

PileUpStream::PileUpStream(const std::string& name,
			   ISvcLocator* svcLoc,
			   IEvtSelector* sel):
  AthMessaging (name),
  m_name(name),
  p_svcLoc(svcLoc),
  m_sel(sel),
  m_mergeSvc("PileUpMergeSvc", name)
{
  if( !( m_sel->createContext(p_iter).isSuccess() && m_mergeSvc.isValid() ) ) {
    const std::string errMsg("PileUpStream:: can not create stream");
    ATH_MSG_ERROR ( errMsg );
    throw std::runtime_error(errMsg);
  } else  m_ownEvtIterator=true;
}

PileUpStream::PileUpStream(const std::string& name, 
			   ISvcLocator* svcLoc,
			   const std::string& selecName):
  AthMessaging (name),
  m_name(name),
  p_svcLoc(svcLoc),
  m_mergeSvc("PileUpMergeSvc", name)
{
  m_sel = serviceLocator()->service<IEvtSelector>(selecName);
  if ( !(m_sel.isValid() && m_mergeSvc.isValid() &&
         m_sel->createContext(p_iter).isSuccess()) ) {
    const std::string errMsg("PileUpStream: can not create stream");
    ATH_MSG_ERROR ( errMsg );
    throw std::runtime_error(errMsg);
  } else  m_ownEvtIterator=true;
}

PileUpStream::~PileUpStream() 
{ 
}

bool PileUpStream::setupStore() 
{
  bool rc(true);
  std::string storeName(name() + "_SG");

  //start by looking for the store directly: in overlay jobs it may already be there
  m_SG = serviceLocator()->service<StoreGateSvc>(storeName, /*createIf*/false);
  if (m_SG) {
    m_ownStore = false;
  } else {
    //not there, create one cloning the master store
    Service *child;
    //if the parent store is not there barf
    //remember the clone function also initializes the service if needed
    SmartIF<StoreGateSvc> pIS(serviceLocator()->service("StoreGateSvc"));
    rc = (pIS.isValid() &&
          CloneService::clone(pIS, storeName, child).isSuccess() &&
          (m_SG = SmartIF<StoreGateSvc>(child)).isValid());
    if ( rc )  {
      m_ownStore = true;
      // further initialization of the cloned service 
      rc = (m_SG->sysInitialize()).isSuccess();
      m_SG->setStoreID(StoreID::PILEUP_STORE); //needed by ProxyProviderSvc
    } //clones
  }
  if (rc) {
    //if the selector is an address provider like the AthenaPool one, 
    //create a dedicated ProxyProviderSvc and associate it to the store
    SmartIF<IAddressProvider> pIAP(m_sel);
    if (pIAP.isValid()) {
      const std::string PPSName(name() + "_PPS");
      SmartIF<IProxyProviderSvc> pPPSvc(serviceLocator()->service(PPSName));
      SmartIF<ISvcManager> pISM(serviceLocator());
      if ( pISM.isValid() &&
           pISM->declareSvcType(PPSName, "ProxyProviderSvc").isSuccess() &&
           pPPSvc.isValid() ) {

        pPPSvc->addProvider(pIAP);
        SmartIF<IAddressProvider> pAthPoolAddProv(serviceLocator()->service("AthenaPoolAddressProviderSvc"));
        if (pAthPoolAddProv.isValid()) {
          pPPSvc->addProvider(pAthPoolAddProv);
        } else {
          ATH_MSG_WARNING ( "could not add AthenaPoolAddressProviderSvc as AddresssProvider for "<< PPSName );
        }
        SmartIF<IAddressProvider> pAddrRemap(serviceLocator()->service("AddressRemappingSvc"));
        if (pAddrRemap.isValid()) {
          pPPSvc->addProvider(pAddrRemap);
        } else {
          ATH_MSG_WARNING ( "could not add AddressRemappingSvc as AddresssProvider for "<< PPSName );
        }
        m_SG->setProxyProviderSvc(pPPSvc);
      } 
    } //valid address provider
  }
  return rc;
}

void PileUpStream::setActiveStore()
{
  store().makeCurrent();
}

StatusCode PileUpStream::nextRecordPre_Passive()
{
  this->setActiveStore();
  // Clear the store, move to next event
  return (this->store().clearStore().isSuccess() && 
	  this->selector().next(iterator()).isSuccess() ) ?
    StatusCode::SUCCESS :
    StatusCode::FAILURE;
}

StatusCode PileUpStream::nextRecordPre()
{
  // Clear the store, move to next event and load the store
  return (this->nextRecordPre_Passive().isSuccess() && 
	  this->loadStore()) ?
    StatusCode::SUCCESS :
    StatusCode::FAILURE;
}

bool PileUpStream::loadStore()
{
  m_neverLoaded=false;

  IOpaqueAddress* paddr(0);
  bool rc = (this->selector().createAddress(iterator(), paddr)).isSuccess();
  if ( 0 != paddr) rc &= this->store().recordAddress(paddr).isSuccess();

  // load store proxies
  rc &= this->store().loadEventProxies().isSuccess();
  return rc;
}

//return next event, load store with next event
const xAOD::EventInfo* PileUpStream::nextEventPre(bool readRecord)
{
   if (m_neverLoaded) readRecord=true;
   else if (readRecord) {
      //do not reset these the first time we call nextEventPre
      this->resetUsed();
      m_hasRing=false;
   }
   //  if (isNotEmpty()) {
   if (readRecord && this->nextRecordPre().isFailure()) {
      ATH_MSG_INFO ( "nextEventPre(): end of the loop. No more events in the selection" );
      return nullptr;
   }

   const xAOD::EventInfo* xAODEventInfo = m_mergeSvc->getPileUpEvent( m_SG, "" );
   if (readRecord and xAODEventInfo) {
      ATH_MSG_DEBUG ( "nextEventPre(): read new event " 
		      <<  xAODEventInfo->eventNumber() 
		      << " run " << xAODEventInfo->runNumber()
		      << " into store " << this->store().name() );
   }

   return xAODEventInfo;
}

bool PileUpStream::nextEventPre_Passive(bool readRecord) {
  if (m_neverLoaded) readRecord=true;
  else if (readRecord) {
    //do not reset these the first time we call nextEventPre_Passive
    this->resetUsed();
    m_hasRing=false;
  }
  if (readRecord && this->nextRecordPre_Passive().isFailure()) {
    ATH_MSG_INFO ( "nextEventPre_Passive(): end of the loop. No more events in the selection" );
    return false;
  } 
  return true;
}    

StatusCode PileUpStream::finalize() {
  StatusCode sc(StatusCode::SUCCESS);
  if (m_ownEvtIterator) delete p_iter;
  //we own and manage our cloned SG instance
#ifdef GAUDIKERNEL_STATEMACHINE_H_
  if (m_ownStore && Gaudi::StateMachine::INITIALIZED == store().FSMState()) {
    sc = this->store().sysFinalize();
  }
#else
  if (m_ownStore && this->store().state() == IService::INITIALIZED) sc = this->store().sysFinalize();
#endif
  this->store().release();
  return sc;
}

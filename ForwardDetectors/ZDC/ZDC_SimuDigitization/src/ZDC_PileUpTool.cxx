/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "ZDC_SimuDigitization/ZDC_PileUpTool.h"
#include "xAODForward/ZdcModuleToString.h"
#include "ZdcIdentifier/ZdcID.h"

#include "ZdcUtils/ZDCWaveformFermiExp.h"
#include "ZdcUtils/ZDCWaveformLTLinStep.h"
#include "ZdcUtils/ZDCWaveformSampler.h"

#include "PileUpTools/PileUpMergeSvc.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "AthContainers/Accessor.h"
#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGaussQ.h"
#include "CLHEP/Random/RandPoissonQ.h"
#include <algorithm>

ZDC_PileUpTool::ZDC_PileUpTool(const std::string& type,
             const std::string& name,
             const IInterface* parent) :
  PileUpToolBase(type, name, parent)
{
}

StatusCode ZDC_PileUpTool::initialize() {

  ATH_MSG_DEBUG ( "ZDC_PileUpTool::initialize() called" );

  const ZdcID* zdcId = nullptr;
  if (detStore()->retrieve( zdcId ).isFailure() ) {
    ATH_MSG_ERROR("execute: Could not retrieve ZdcID object from the detector store");
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG("execute: retrieved ZdcID");
  }
  m_ZdcID = zdcId;

  ATH_CHECK(m_randomSvc.retrieve());
  ATH_MSG_DEBUG ( "Retrieved RandomNumber Service" );

  ATH_CHECK(m_SimFiberHitCollectionKey.initialize());
  ATH_CHECK(m_ZdcModuleContainerName.initialize());
  ATH_CHECK(m_ZdcSumsContainerName.initialize());

  ATH_MSG_INFO("ZDC_PileUpTool configuration = " << m_configuration);

  if(m_configuration == "PbPb2015"){
    initializePbPb2015();
  }else if(m_configuration == "LHCf2022"){
    initializeLHCf2022();
  }else if(m_configuration == "PbPb2023"){
    initializePbPb2023();
  }

  m_mergedFiberHitList = new ZDC_SimFiberHit_Collection("mergedFiberHitList");

  return StatusCode::SUCCESS;
}

void ZDC_PileUpTool::initializePbPb2015(){

  m_Pedestal = 100;
  m_numTimeBins = 7;
  m_freqMHz = 80;
  m_delayChannels = true;
  m_LTQuadStepFilt = true;
  m_doRPD = false;
  m_doBRAN = false;
  m_zdct0 = 28;
  m_qsfRiseTime =  0.5;
  m_qsfFallTime =  11;
  m_qsfFilter =  15;
}

void ZDC_PileUpTool::initializeLHCf2022(){

  m_Pedestal = 100;
  m_numTimeBins = 24;
  m_freqMHz = 320;
  m_delayChannels = false;
  m_LTQuadStepFilt = false;
  m_doRPD = false;
  m_doBRAN = false;
  m_zdct0 = 28;
  m_rpdt0 = 28;
  m_zdcRiseTime =  1;
  m_zdcFallTime =  5.5;
  m_zdcAdcPerPhoton = 0.000498;
}

void ZDC_PileUpTool::initializePbPb2023(){

  m_Pedestal = 100;
  m_numTimeBins = 24;
  m_freqMHz = 320;
  m_delayChannels = false;
  m_LTQuadStepFilt = false;
  m_doRPD = true;
  m_doBRAN = true;
  m_zdct0 = 31.2;
  m_rpdt0 = 27.9;
  m_zdcRiseTime =  1.1;
  m_zdcFallTime =  4.5;
  m_rpdRiseTime =  0.8;
  m_rpdFallTime =  15;
  m_zdcAdcPerPhoton = 0.000498;
  m_rpdAdcPerPhoton = {5.25 , 1.7};
}

StatusCode ZDC_PileUpTool::processAllSubEvents(const EventContext& ctx){

  ATH_MSG_DEBUG ( "ZDC_PileUpTool::processAllSubEvents()" );

  ATHRNG::RNGWrapper* rngWrapper = m_randomSvc->getEngine(this, m_randomStreamName);
  rngWrapper->setSeed( m_randomStreamName, ctx );
  CLHEP::HepRandomEngine* rngEngine = rngWrapper->getEngine(ctx);

  /******************************************
   * retrieve the hit collection list (input)
  ******************************************/

  SG::ReadHandle<ZDC_SimFiberHit_Collection> hitCollection(m_SimFiberHitCollectionKey, ctx);
  if (!hitCollection.isValid()) {
    ATH_MSG_ERROR("Could not get ZDC_SimFiberHit_Collection container " << hitCollection.name() << " from store " << hitCollection.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("ZDC_SimFiberHitHitCollection found with " << hitCollection->size() << " hits");

  /******************************************
   * Do light guide efficiency cuts on the ZDCs
  ******************************************/
  TimedHitCollection<ZDC_SimFiberHit> thpcZDC_Fiber = doZDClightGuideCuts(hitCollection.cptr());

  /******************************************
   * Create the output container
  ******************************************/
  std::unique_ptr<xAOD::ZdcModuleContainer> moduleContainer( new xAOD::ZdcModuleContainer());
  std::unique_ptr<xAOD::ZdcModuleAuxContainer> moduleAuxContainer( new xAOD::ZdcModuleAuxContainer() );
  moduleContainer->setStore( moduleAuxContainer.get() );

  SG::AuxElement::Accessor<std::vector<uint16_t>> g0acc("g0data");
  SG::AuxElement::Accessor<std::vector<uint16_t>> g1acc("g1data");
  if(m_delayChannels){
    SG::AuxElement::Accessor<std::vector<uint16_t>> g0d1acc("g0d1data");
    SG::AuxElement::Accessor<std::vector<uint16_t>> g1d1acc("g1d1data");
  }

  /******************************************
   * Create the waveforms
  ******************************************/
  fillContainer(thpcZDC_Fiber, rngEngine, moduleContainer.get());


  /******************************************
   * Create the zdcSums container
  ******************************************/
  std::unique_ptr<xAOD::ZdcModuleContainer> sumsContainer( new xAOD::ZdcModuleContainer());
  std::unique_ptr<xAOD::ZdcModuleAuxContainer> sumsAuxContainer( new xAOD::ZdcModuleAuxContainer() );
  sumsContainer->setStore( sumsAuxContainer.get() );

  static const SG::Accessor<uint16_t> LucrodTriggerSideAmpAcc ("LucrodTriggerSideAmp");
  for (int iside : {-1, 1}){
    xAOD::ZdcModule* new_sum = new xAOD::ZdcModule();
    sumsContainer->push_back(xAOD::ZdcModuleContainer::unique_type(new_sum));
    new_sum->setZdcSide(iside);
    LucrodTriggerSideAmpAcc (*new_sum) = 42;
  }

  /******************************************
   * Write the output
  ******************************************/

  //Print the module contents
  ATH_MSG_DEBUG( ZdcModuleToString(*moduleContainer) );

  auto moduleContainerH = SG::makeHandle( m_ZdcModuleContainerName, ctx );
  ATH_CHECK( moduleContainerH.record (std::move(moduleContainer), std::move(moduleAuxContainer)) );

  auto sumsContainerH = SG::makeHandle( m_ZdcSumsContainerName, ctx );
  ATH_CHECK( sumsContainerH.record (std::move(sumsContainer), std::move(sumsAuxContainer)) );

  return StatusCode::SUCCESS;
}

StatusCode ZDC_PileUpTool::prepareEvent(const EventContext& /*ctx*/,const unsigned int nInputEvents){

  ATH_MSG_DEBUG ( "ZDC_PileUpTool::prepareEvent() called for " << nInputEvents << " input events" );

  m_ZdcModuleContainer = std::make_unique<xAOD::ZdcModuleContainer>();
  m_ZdcModuleAuxContainer = std::make_unique<xAOD::ZdcModuleAuxContainer>();
  m_ZdcModuleContainer->setStore( m_ZdcModuleAuxContainer.get() );

  SG::AuxElement::Accessor<std::vector<uint16_t>> g0acc("g0data");
  SG::AuxElement::Accessor<std::vector<uint16_t>> g1acc("g1data");
  SG::AuxElement::Accessor<std::vector<uint16_t>> g0d1acc("g0d1data");
  SG::AuxElement::Accessor<std::vector<uint16_t>> g1d1acc("g1d1data");

  m_mergedFiberHitList->clear();

  return StatusCode::SUCCESS;
}


StatusCode ZDC_PileUpTool::processBunchXing(int bunchXing,
                                            SubEventIterator bSubEvents,
                                            SubEventIterator eSubEvents){
  ATH_MSG_DEBUG ( "ZDC_PileUpTool::processBunchXing() " << bunchXing );
  SubEventIterator iEvt = bSubEvents;
  for (; iEvt!=eSubEvents; ++iEvt) {
    StoreGateSvc& seStore = *iEvt->ptr()->evtStore();
    ATH_MSG_VERBOSE("SubEvt StoreGate " << seStore.name() << " :"
                    << " bunch crossing : " << bunchXing
                    << " time offset : " << iEvt->time()
                    << " event number : " << iEvt->ptr()->eventNumber()
                    << " run number : " << iEvt->ptr()->runNumber());

    const ZDC_SimFiberHit_Collection* tmpColl = 0;

    if (!seStore.retrieve(tmpColl, m_HitCollectionName).isSuccess()) {
      ATH_MSG_ERROR ( "SubEvent ZDC_SimFiberHit_Collection not found in StoreGate " << seStore.name() );
      return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG ( "ZDC_SimFiberHit_Collection found with " << tmpColl->size() << " hits" );

    ZDC_SimFiberHit_Collection::const_iterator it = tmpColl->begin();
    ZDC_SimFiberHit_Collection::const_iterator end = tmpColl->end();

    for (; it!=end; ++it) m_mergedFiberHitList->push_back(*it);
  }

  return StatusCode::SUCCESS;
}


StatusCode ZDC_PileUpTool::mergeEvent(const EventContext& ctx){

  ATHRNG::RNGWrapper* rngWrapper = m_randomSvc->getEngine(this, m_randomStreamName);
  rngWrapper->setSeed( m_randomStreamName, ctx );
  CLHEP::HepRandomEngine* rngEngine = rngWrapper->getEngine(ctx);
  fillContainer(m_mergedFiberHitList, rngEngine, m_ZdcModuleContainer.get());

  SG::WriteHandle<xAOD::ZdcModuleContainer> moduleContainerH (m_ZdcModuleContainerName, ctx);
  ATH_CHECK( moduleContainerH.record (std::move(m_ZdcModuleContainer), std::move(m_ZdcModuleAuxContainer)) );

  return StatusCode::SUCCESS;
}

void ZDC_PileUpTool::fillContainer(TimedHitCollection<ZDC_SimFiberHit>& thpczdc, CLHEP::HepRandomEngine* rndEngine, xAOD::ZdcModuleContainer *zdcModuleContainer){
  TimedHitCollection<ZDC_SimFiberHit> thpc = thpczdc;
  TimedHitCollection<ZDC_SimFiberHit>::const_iterator i, e, it;
  while (thpc.nextDetectorElement(i, e)) for (it = i; it != e; ++it) {
    createAndStoreWaveform(*(*it), rndEngine, zdcModuleContainer);
  }
  addEmptyWaveforms(zdcModuleContainer, rndEngine);
}


void ZDC_PileUpTool::fillContainer(const ZDC_SimFiberHit_Collection* ZDC_SimFiberHit_Collection, CLHEP::HepRandomEngine* rndEngine, xAOD::ZdcModuleContainer *zdcModuleContainer){
  ZDC_SimFiberHit_ConstIterator it    = ZDC_SimFiberHit_Collection->begin();
  ZDC_SimFiberHit_ConstIterator itend = ZDC_SimFiberHit_Collection->end();

  for (; it != itend; ++it) {
    createAndStoreWaveform(*it, rndEngine, zdcModuleContainer);
  }
  addEmptyWaveforms(zdcModuleContainer, rndEngine);
}

void ZDC_PileUpTool::addEmptyWaveforms(xAOD::ZdcModuleContainer *zdcModuleContainer, CLHEP::HepRandomEngine* rndEngine){
  //Initialize bools to keep track of what modules/channels we have seen
  bool zdcFound[2][4] =  {{}};
  bool rpdFound[2][16] = {{}};
  bool branFound[2] = {};
  for(auto module : *zdcModuleContainer){
    int iside = (module->zdcSide() == -1) ? 0 : 1;
    int mod = module->zdcModule();
    if(mod < 4){ //ZDC
      zdcFound[iside][mod] = true;
      ATH_MSG_DEBUG("ZDC_PileUpTool::addEmptyWaveforms Found ZDC side " << module->zdcSide() << " module " << mod);
    }else if(mod == 4){ //RPD
      rpdFound[iside][module->zdcChannel()] = true;
      ATH_MSG_DEBUG("ZDC_PileUpTool::addEmptyWaveforms Found RPD side " << module->zdcSide() << " channel " << module->zdcChannel());
    }else{//BRAN
      branFound[iside] = true;
      ATH_MSG_DEBUG("ZDC_PileUpTool::addEmptyWaveforms Found BRAN side " << module->zdcSide() );
    }
  }

  for(int iside : {0,1}){
    int side = (iside == 0) ? -1 : 1;
    for(int mod = 0; mod < 4; mod++){
      if(!zdcFound[iside][mod]){
        ATH_MSG_DEBUG("ZDC_PileUpTool::addEmptyWaveforms Creating empty waveform for ZDC side " << side << " module " << mod);
        createAndStoreWaveform(ZDC_SimFiberHit( m_ZdcID->channel_id(side, mod, ZdcIDType::SINGLECHANNEL, 0), 0, 0), rndEngine, zdcModuleContainer);
      }
    }
    for(int channel = 0; channel < 16; channel++){
      if(!rpdFound[iside][channel] && m_doRPD){
        ATH_MSG_DEBUG("ZDC_PileUpTool::addEmptyWaveforms Creating empty waveform for RPD side " << side << " channel " << channel);
        createAndStoreWaveform(ZDC_SimFiberHit( m_ZdcID->channel_id(side, 4, ZdcIDType::MULTICHANNEL, channel), 0, 0), rndEngine, zdcModuleContainer);
      }
    }
    if(!branFound[iside] && m_doBRAN){
      ATH_MSG_DEBUG("ZDC_PileUpTool::addEmptyWaveforms Creating empty waveform for BRAN side " << side );
      createAndStoreWaveform(ZDC_SimFiberHit( m_ZdcID->channel_id(side, 5, ZdcIDType::SINGLECHANNEL, 0), 0, 0), rndEngine, zdcModuleContainer);
    }
  }
}

TimedHitCollection<ZDC_SimFiberHit> ZDC_PileUpTool::doZDClightGuideCuts(const ZDC_SimFiberHit_Collection* hitCollection){

  ZDC_SimFiberHit* newHits[2][4] = {{nullptr}};
  ZDC_SimFiberHit_Collection* newCollection = new ZDC_SimFiberHit_Collection("ZDC_SimFiberHit_Collection_Temp");

  for(ZDC_SimFiberHit hit : *hitCollection){
    Identifier id = hit.getID();
    //Translate side from -1,1 to 0,1 to index ZDC hits
    int side = (m_ZdcID->side( id ) < 0 ) ? 0 : 1;
    int module = m_ZdcID->module( id );
    
    //ZDCs are module 0-3, just insert this hit unmodified and move on
    if(module > 3){ 
      newCollection->Insert(hit);
      continue;
    }

    //TODO: Implement a method to get the efficiency of this location to multiply hit.getNphotons() and hit.getEdep()
    //based on the channel retrieved by m_ZdcID->channel( id ) and some efficiency map
    float efficiency = 1.0; //For now we will just use 100% efficiency

    if(!newHits[side][module]){
      //The module hasn't been seen yet, create a new hit with values modified by the efficiency factor
      //The ID is modified because we initially used channel to denote the position of the rods for these efficiency cuts
      //Now that that's done, we make sure the type is SINGLECHANNEL and the channel is 0
      newHits[side][module] = new ZDC_SimFiberHit(m_ZdcID->channel_id(m_ZdcID->side(id), module, ZdcIDType::SINGLECHANNEL, 0), efficiency * hit.getNPhotons(), efficiency * hit.getEdep());
    }else{
      //The module has been seen, add the photons and energy from this new hit to it
      newHits[side][module]->Add( efficiency*hit.getNPhotons(), efficiency*hit.getEdep());
    }
  }//end loop over hits

  for(int side : {0,1}){
    for(int module = 0; module < 4; module++){
      //Make sure the hit exists before attempting to insert it
      //If it doesn't we will take care of this module in addEmptyWaveforms
      if(newHits[side][module]){
        newCollection->Insert(*(newHits[side][module]));
      }
    }
  }

  //Now insert one hit per detector in the new collection
  TimedHitCollection<ZDC_SimFiberHit> newTimedCollection;
  newTimedCollection.insert(0.0, newCollection);

  return newTimedCollection;
}


void ZDC_PileUpTool::createAndStoreWaveform(const ZDC_SimFiberHit &hit, CLHEP::HepRandomEngine* rndEngine, xAOD::ZdcModuleContainer *zdcModuleContainer){
  Identifier id = hit.getID( );
  int side = m_ZdcID->side(id);
  int module = m_ZdcID->module(id);
  int channel = m_ZdcID->channel(id);

  if(module == 4 && !m_doRPD) return;
  if(module == 5 && !m_doBRAN) return;

  //Here we have to switch the type of the RPD from ACTIVE to MULTICHANNEL
  //TODO: Either change the channel numbering in the geometry, or the analysis
  if(module == 4) id = m_ZdcID->channel_id(side,module,ZdcIDType::MULTICHANNEL,channel);

  //Create a new ZdcModule to store the waveforms
  xAOD::ZdcModule* zdc = new xAOD::ZdcModule();
  zdcModuleContainer->push_back(xAOD::ZdcModuleContainer::unique_type(zdc));
  zdcModuleContainer->back()->setZdcId(id.get_identifier32().get_compact());
  zdcModuleContainer->back()->setZdcSide(side);
  zdcModuleContainer->back()->setZdcModule(module);
  zdcModuleContainer->back()->setZdcType(m_ZdcID->type(id));
  zdcModuleContainer->back()->setZdcChannel(channel);

  ATH_MSG_DEBUG( "Digitizing Side " <<  side << 
                " Module " << module << 
                                " Channel " << channel << 
                ", whith " << hit.getNPhotons() << " photons" );
  
  uint iSide = (side == -1) ? 0 : 1;
  float amplitude = 0, t0 = 0;
  bool doHighGain = true;
  std::shared_ptr<ZDCWaveformBase> waveformPtr;
  std::shared_ptr<ZDCWaveformSampler> wfSampler;
  
  if(module < 4){//It's a ZDC channel
    amplitude = CLHEP::RandPoissonQ::shoot(rndEngine, hit.getNPhotons())*m_zdcAdcPerPhoton;

    if(m_LTQuadStepFilt){ 
      waveformPtr = std::make_shared<ZDCWaveformLTLinStep>("zdc",m_qsfRiseTime,m_qsfFallTime,m_qsfFilter);
    }else{
      waveformPtr = std::make_shared<ZDCWaveformFermiExp>("zdc",m_zdcRiseTime,m_zdcFallTime);
    }
    wfSampler = std::make_shared<ZDCWaveformSampler>(m_freqMHz, 0, m_numTimeBins, 12, m_Pedestal, waveformPtr);
    t0 = m_zdct0;

  }else{ //It's an RPD channel
    // ATH_MSG_INFO("Digitizing RPD " << side << ", channel " << channel << " with " << hit.getNPhotons() << " and " << m_rpdAdcPerPhoton[iSide] << " amplification");
    amplitude = CLHEP::RandPoissonQ::shoot(rndEngine, hit.getNPhotons())*m_rpdAdcPerPhoton[iSide];
    waveformPtr = std::make_shared<ZDCWaveformFermiExp>("rpd",m_rpdRiseTime,m_rpdFallTime);
    wfSampler = std::make_shared<ZDCWaveformSampler>(m_freqMHz, 0, m_numTimeBins, 12, m_Pedestal, waveformPtr);
    t0 = m_rpdt0;
    doHighGain = false;
  }

  //Record the number of photons that lead to this waveform
  SG::Accessor<unsigned int> nPhotonsAcc ("nPhotons");
  nPhotonsAcc (*zdc) = hit.getNPhotons();

  //Generate in time waveforms
  zdc->setWaveform("g0data", generateWaveform(wfSampler, amplitude, t0));
  if(doHighGain) zdc->setWaveform("g1data", generateWaveform(wfSampler, 10*amplitude, t0));

  //Generate delayed waveforms
  if(m_delayChannels){
    float timeBinWidth = 1000.0/m_freqMHz;
    zdc->setWaveform("g0d1data", generateWaveform(wfSampler, 10*amplitude, t0+timeBinWidth/2));
    if(doHighGain) zdc->setWaveform("g1d1data", generateWaveform(wfSampler, 10*amplitude, t0+timeBinWidth/2));
  }

}

std::vector<short unsigned int> ZDC_PileUpTool::generateWaveform(std::shared_ptr<ZDCWaveformSampler> wfSampler, float amplitude, float t0){
  std::vector<unsigned int> wf = wfSampler->Generate(amplitude, t0);
  std::vector<short unsigned int> retVal;
  for(uint sample = 0; sample < wf.size(); sample++){
    retVal.push_back(wf[sample]);
  }
  return retVal;
}

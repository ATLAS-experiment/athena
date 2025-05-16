/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////////////////////
//
// sTgcDigitizationTool
// ------------
// Authors:  Nectarios Benekos  <nectarios.benekos@cern.ch>
//           Jiaming Yu  <jiaming.yu@cern.ch>
////////////////////////////////////////////////////////////////////////////////

//sTGC digitization includes
#include "sTGC_Digitization/sTgcDigitizationTool.h"

#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuonSimData/MuonSimDataCollection.h"
#include "MuonSimData/MuonSimData.h"

//Outputs
#include "MuonDigitContainer/sTgcDigitContainer.h"


//Geometry
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "MuonSimEvent/sTgcHitIdHelper.h"
#include "MuonSimEvent/sTgcSimIdToOfflineId.h"
#include "TrkDetDescrUtils/GeometryStatics.h"
#include "TrkEventPrimitives/LocalDirection.h"
#include "TrkSurfaces/Surface.h"
#include "CLHEP/Random/RandGaussZiggurat.h"

//Truth
#include "GeneratorObjects/HepMcParticleLink.h"
#include "AtlasHepMC/GenParticle.h"

#include "AthenaKernel/RNGWrapper.h"

#include <sstream>
#include <iostream>
#include <fstream>

#include <memory>


using namespace MuonGM;
using sTgcSimDigitVec = sTgcDigitizationTool::sTgcSimDigitVec;


/*******************************************************************************/
sTgcDigitizationTool::sTgcDigitizationTool(const std::string& type, const std::string& name, const IInterface* parent) :
    PileUpToolBase(type, name, parent) {}

/*******************************************************************************/
// member function implementation
//--------------------------------------------
StatusCode sTgcDigitizationTool::initialize() {

  ATH_MSG_INFO (" sTgcDigitizationTool  retrieved");
  ATH_MSG_INFO ( "Configuration  sTgcDigitizationTool" );
  ATH_MSG_INFO ( "doSmearing             "<< m_doSmearing);
  ATH_MSG_INFO ( "RndmSvc                " << m_rndmSvc             );
  ATH_MSG_INFO ( "RndmEngine             " << m_rndmEngineName      );
  ATH_MSG_INFO ( "InputObjectName        " << m_hitsContainerKey.key());
  ATH_MSG_INFO ( "OutputObjectName       " << m_outputDigitCollectionKey.key());
  ATH_MSG_INFO ( "OutputSDOName          " << m_outputSDO_CollectionKey.key());
  ATH_MSG_INFO ( "HV                     " << m_runVoltage);
  ATH_MSG_INFO ( "threshold              " << m_chargeThreshold);
  ATH_MSG_INFO ( "useCondThresholds      " << m_useCondThresholds);

  if (m_hitsContainerKey.key().empty()) {
    ATH_MSG_FATAL("Property InputObjectName not set !");
    return StatusCode::FAILURE;
  }

  if (m_onlyUseContainerName) m_inputObjectName = m_hitsContainerKey.key();
  ATH_MSG_DEBUG("Input objects in container: '" << m_inputObjectName << "'");

  // Pile-up merge service
  if (m_onlyUseContainerName) {
    ATH_CHECK(m_mergeSvc.retrieve());
  }

  // retrieve MuonDetctorManager from DetectorStore
  ATH_CHECK(m_detMgrKey.initialize());
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_effiKey.initialize(m_doEfficiencyCorrection));

 
  // calibration tool
  ATH_CHECK(m_calibTool.retrieve());
  // initialize ReadCondHandleKey
  ATH_CHECK(m_condThrshldsKey.initialize(m_useCondThresholds));
  // Initialize ReadHandleKey
  ATH_CHECK(m_hitsContainerKey.initialize());

  //initialize the output WriteHandleKeys
  ATH_CHECK(m_outputDigitCollectionKey.initialize());
  ATH_CHECK(m_outputSDO_CollectionKey.initialize());

  // initialize sTgcDigitMaker class to digitize hits
  // meanGasGain is the mean value of the polya gas gain function describing the
  // avalanche of electrons caused by the electric field
  // Parameterization is obtained from ATL-MUON-PUB-2014-001 and the corrected
  // fit to data to parameterize gain vs HV in kV
  // m_runVoltage MUST BE in kV!
  if (m_runVoltage < 2.3 || m_runVoltage > 3.2){
    ATH_MSG_ERROR("STGC run voltage must be in kV and within fit domain of 2.3 kV to 3.2 kV");
    return StatusCode::FAILURE;
  }
  double meanGasGain = 2.15 * 1E-4 * std::exp(6.88*m_runVoltage);
  m_digitizer = std::make_unique<sTgcDigitMaker>(m_idHelperSvc.get(), m_doChannelTypes, meanGasGain, m_doPadSharing, m_stripChargeScale, m_applyAsBuiltBLines);
  m_digitizer->setLevel(static_cast<MSG::Level>(msgLevel()));
  ATH_CHECK(m_digitizer->initialize());

  ATH_CHECK(m_rndmSvc.retrieve());
  // getting our random numbers stream
  ATH_MSG_DEBUG("Getting random number engine : <" << m_rndmEngineName << ">");

  return StatusCode::SUCCESS;
}
/*******************************************************************************/
StatusCode sTgcDigitizationTool::prepareEvent(const EventContext& /*ctx*/, unsigned int nInputEvents) {

  ATH_MSG_DEBUG("sTgcDigitizationTool::prepareEvent() called for " << nInputEvents << " input events" );
  m_STGCHitCollList.clear();

  return StatusCode::SUCCESS;
}
/*******************************************************************************/

StatusCode sTgcDigitizationTool::processBunchXing(int bunchXing,
              SubEventIterator bSubEvents,
              SubEventIterator eSubEvents) {
  ATH_MSG_DEBUG ( "sTgcDigitizationTool::in processBunchXing()" );
  if (m_thpcsTGC == nullptr) {
    m_thpcsTGC = std::make_unique<TimedHitCollection<sTGCSimHit>>();
  }
  using TimedHitCollList = PileUpMergeSvc::TimedList<sTGCSimHitCollection>::type;
  TimedHitCollList hitCollList;

  if (!(m_mergeSvc->retrieveSubSetEvtData(m_inputObjectName, hitCollList, bunchXing,
                                          bSubEvents, eSubEvents).isSuccess()) &&
      hitCollList.empty()) {
    ATH_MSG_ERROR("Could not fill TimedHitCollList");
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_VERBOSE(hitCollList.size() << " sTGCSimHitCollection with key " <<
                    m_inputObjectName << " found");
  }

  TimedHitCollList::iterator iColl(hitCollList.begin());
  TimedHitCollList::iterator endColl(hitCollList.end());

  // Iterating over the list of collections
  for( ; iColl != endColl; ++iColl){

    auto hitCollPtr = std::make_unique<sTGCSimHitCollection>(*iColl->second);
    PileUpTimeEventIndex timeIndex(iColl->first);

    ATH_MSG_DEBUG("sTGCSimHitCollection found with " << hitCollPtr->size() << " hits");
    ATH_MSG_VERBOSE("time index info. time: " << timeIndex.time()
                    << " index: " << timeIndex.index()
                    << " type: " << timeIndex.type());

    m_thpcsTGC->insert(timeIndex, hitCollPtr.get());
    m_STGCHitCollList.push_back(std::move(hitCollPtr));
  }
  return StatusCode::SUCCESS;
}
/*******************************************************************************/
StatusCode sTgcDigitizationTool::getNextEvent(const EventContext& ctx) {

  ATH_MSG_DEBUG ( "sTgcDigitizationTool::getNextEvent()" );

  //  get the container(s)
  using TimedHitCollList = PileUpMergeSvc::TimedList<sTGCSimHitCollection>::type;

  // In case of single hits container just load the collection using read handles
  if (!m_onlyUseContainerName) {
    SG::ReadHandle<sTGCSimHitCollection> hitCollection(m_hitsContainerKey, ctx);
    if (!hitCollection.isValid()) {
      ATH_MSG_ERROR("Could not get sTGCSimHitCollection container " << hitCollection.name() << " from store " << hitCollection.store());
      return StatusCode::FAILURE;
    }

    // create a new hits collection
    m_thpcsTGC = std::make_unique<TimedHitCollection<sTGCSimHit>>(1);
    m_thpcsTGC->insert(0, hitCollection.cptr());
    ATH_MSG_DEBUG("sTGCSimHitCollection found with " << hitCollection->size() << " hits");
    return StatusCode::SUCCESS;
  }

  //this is a list<info<time_t, DataLink<sTGCSimHitCollection> > >
  TimedHitCollList hitCollList;

  if (!(m_mergeSvc->retrieveSubEvtsData(m_inputObjectName, hitCollList).isSuccess()) ) {
    ATH_MSG_ERROR ( "Could not fill TimedHitCollList" );
    return StatusCode::FAILURE;
  }
  if (hitCollList.empty()) {
    ATH_MSG_ERROR ( "TimedHitCollList has size 0" );
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG ( hitCollList.size() << " sTGC SimHitCollections with key " << m_inputObjectName << " found" );
  }

  //Perform null check on m_thpcsTGC. If pointer is not null throw error
  if (!m_thpcsTGC) {
    m_thpcsTGC = std::make_unique<TimedHitCollection<sTGCSimHit>>();
  }else{
  ATH_MSG_ERROR ( "m_thpcsTGC is not null" );
  return StatusCode::FAILURE;
  }

  //now merge all collections into one
  TimedHitCollList::iterator iColl(hitCollList.begin());
  TimedHitCollList::iterator endColl(hitCollList.end());
  while (iColl != endColl) {
    const sTGCSimHitCollection* p_collection(iColl->second);
    m_thpcsTGC->insert(iColl->first, p_collection);
    ATH_MSG_DEBUG ( "sTGC SimHitCollection found with " << p_collection->size() << " hits"  );
    ++iColl;
  }

  return StatusCode::SUCCESS;
}

/*******************************************************************************/
StatusCode sTgcDigitizationTool::mergeEvent(const EventContext& ctx) {
  ATH_MSG_DEBUG ( "sTgcDigitizationTool::in mergeEvent()" );
  ATH_CHECK(doDigitization(ctx));
  // reset the pointer
  m_thpcsTGC.reset();
  m_STGCHitCollList.clear();

  return StatusCode::SUCCESS;
}
/*******************************************************************************/
StatusCode sTgcDigitizationTool::digitize(const EventContext& ctx) {
  return this->processAllSubEvents(ctx);
}
/*******************************************************************************/
StatusCode sTgcDigitizationTool::processAllSubEvents(const EventContext& ctx) {
  ATH_MSG_DEBUG (" sTgcDigitizationTool::processAllSubEvents()" );
  //merging of the hit collection in getNextEvent method
  if (!m_thpcsTGC ) {
     ATH_CHECK(getNextEvent(ctx));
  }
  ATH_CHECK(doDigitization(ctx));
  // reset the pointer
  m_thpcsTGC.reset();

  return StatusCode::SUCCESS;
}

template <class CondType> StatusCode sTgcDigitizationTool::retrieveCondData(const EventContext& ctx,
                                                                            SG::ReadCondHandleKey<CondType>& key,
                                                                            const CondType* & condPtr) const{
    if (key.empty()) {
       ATH_MSG_DEBUG("No key has been configured for object "<<typeid(CondType).name()<<". Clear pointer");
       condPtr = nullptr;
       return StatusCode::SUCCESS;
    }
    SG::ReadCondHandle<CondType> readHandle{key, ctx};
    if (!readHandle.isValid()){
        ATH_MSG_FATAL("Failed to load conditions object "<<key.fullKey()<<".");
        return StatusCode::FAILURE;
    }
    condPtr = readHandle.cptr();
    return StatusCode::SUCCESS;
}

/*******************************************************************************/
StatusCode sTgcDigitizationTool::doDigitization(const EventContext& ctx) {

  ATH_MSG_DEBUG ("sTgcDigitizationTool::doDigitization()" );
  const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
  
  sTgcDigitMaker::DigiConditions digitCond{};
  digitCond.rndmEngine = getRandomEngine(m_rndmEngineName, ctx);
  ATH_CHECK(retrieveCondData(ctx, m_detMgrKey, digitCond.detMgr));
  ATH_CHECK(retrieveCondData(ctx, m_effiKey, digitCond.efficiencies));
  ATH_CHECK(retrieveCondData(ctx, m_condThrshldsKey , digitCond.thresholdData));
  

  // create and record the Digit container in StoreGate
  SG::WriteHandle<sTgcDigitContainer> digitContainer(m_outputDigitCollectionKey, ctx);
  ATH_CHECK(digitContainer.record(std::make_unique<sTgcDigitContainer>(idHelper.module_hash_max())));
  ATH_MSG_DEBUG ( "sTgcDigitContainer recorded in StoreGate." );

  // Create and record the SDO container in StoreGate
  SG::WriteHandle<MuonSimDataCollection> sdoContainer(m_outputSDO_CollectionKey, ctx);
  ATH_CHECK(sdoContainer.record(std::make_unique<MuonSimDataCollection>()));
  ATH_MSG_DEBUG( "sTgcSDOCollection recorded in StoreGate." );


  TimedHitCollection<sTGCSimHit>::const_iterator i, e;

  // Collections of digits by digit type associated with a detector element
  sTgcSimDigitCont unmergedPadDigits{}, unmergedStripDigits{}, unmergedWireDigits{};
  sTgcDigtCont outputDigits{};
  
  ATH_MSG_DEBUG("create Digit container of size " << idHelper.module_hash_max());

  double earliestEventTime = 9999;

  // --nextDetectorElement>sets an iterator range with the hits of current detector element , returns a bool when done
  while(m_thpcsTGC->nextDetectorElement(i, e)) {
    int nhits = 0;
    ATH_MSG_VERBOSE("Next Detector Element");
    while(i != e){ //loop through the hits on this Detector Element
      ATH_MSG_VERBOSE("Looping over hit " << nhits+1 << " on this Detector Element." );

      ++nhits;
      TimedHitPtr<sTGCSimHit> phit = *i++;
      const sTGCSimHit& hit = *phit;
      ATH_MSG_VERBOSE("Hit Particle ID : " << hit.particleEncoding() );
      double eventTime = phit.eventTime();
      if(eventTime < earliestEventTime) earliestEventTime = eventTime;
      // Cut on energy deposit of the particle
      if(hit.depositEnergy() < m_energyDepositThreshold) {
        ATH_MSG_VERBOSE("Hit with Energy Deposit of " << hit.depositEnergy()
                        << " less than " << m_energyDepositThreshold << ". Skip this hit." );
        continue;
      }

      // Old HITS format doesn't have kinetic energy (i.e it is set to -1).
      double hit_kineticEnergy = hit.kineticEnergy();

      // Skip digitizing some problematic hits, if processing compatible HITS format
      if (hit_kineticEnergy > 0.) {
        // Skip electron with low kinetic energy, since electrons are mainly secondary particles.
        if ((std::abs(hit.particleEncoding()) == 11) && (hit_kineticEnergy < m_limitElectronKineticEnergy)) {
          ATH_MSG_DEBUG("Skip electron hit with kinetic energy " << hit_kineticEnergy
                      << ", which is less than the lower limit of " << m_limitElectronKineticEnergy);
          continue;
        }

       // No support for particles with direction perpendicular to the beam line, since such particles
       // can deposit energy on a lot of strips and pads of the gas gap. So a good model of charge
       // spreading should be implemented. Also, these particles are rare, and most of them are
       // secondary particles suh as electrons.
       if (std::abs(hit.globalPosition().z() - hit.globalPrePosition().z()) < 0.00001) {
         ATH_MSG_VERBOSE("Skip hit with a direction perpendicular to the beam line, ie z-component is less than 0.00001 mm.");
         continue;
       }
      }

      if(eventTime != 0){
         ATH_MSG_DEBUG("Updated hit global time to include off set of " << eventTime << " ns from OOT bunch.");
      }
      else {
          ATH_MSG_DEBUG("This hit came from the in time bunch.");
      }
      sTgcSimIdToOfflineId simToOffline(&idHelper);
      const int idHit = hit.sTGCId();
      ATH_MSG_VERBOSE("Hit ID " << idHit );
      Identifier layid = simToOffline.convert(idHit);
      int eventId = phit.eventId();
      
      /// apply the smearing tool to decide if the hit has to be digitized or not
      /// based on layer efficiency
      if (m_doSmearing) {
        bool acceptHit = true;
        ATH_CHECK(m_smearingTool->isAccepted(layid, acceptHit, digitCond.rndmEngine));
        if ( !acceptHit ) {
          ATH_MSG_DEBUG("Dropping the hit - smearing tool");
          continue;
        }
      }

      const MuonGM::sTgcReadoutElement* detEL = digitCond.detMgr->getsTgcReadoutElement(layid);  //retreiving the sTGC this hit is located in
      if(!detEL) {
        ATH_MSG_WARNING("Failed to retrieve detector element for " 
                      << m_idHelperSvc->toStringDetEl(layid));
        continue;
      }

      // project the hit position to wire surface (along the incident angle)
      ATH_MSG_VERBOSE("Projecting hit to Wire Surface" );
      const Amg::Vector3D& HPOS{hit.globalPosition()};  //Global position of the hit
      const Amg::Vector3D& GLODIRE{hit.globalDirection()};
      const Amg::Vector3D& global_preStepPos{hit.globalPrePosition()};

      ATH_MSG_VERBOSE("Global Direction " << Amg::toString(GLODIRE, 2) );
      ATH_MSG_VERBOSE("Global Position " << Amg::toString(HPOS, 2) );

      int surfHash_wire =  detEL->surfaceHash(idHelper.gasGap(layid), 
                                              sTgcIdHelper::sTgcChannelTypes::Wire);
      ATH_MSG_VERBOSE("Surface Hash for wire plane" << surfHash_wire );
      const Trk::PlaneSurface&  SURF_WIRE = detEL->surface(surfHash_wire);  //Plane of the wire surface in this gasGap
      ATH_MSG_VERBOSE("Wire Surface Defined " <<Amg::toString(SURF_WIRE.center(), 2) );

      const Amg::Transform3D wireTrans = SURF_WIRE.transform().inverse(); 
      Amg::Vector3D LOCDIRE = wireTrans.linear()*GLODIRE;
      Amg::Vector3D LPOS = wireTrans * HPOS;  //Position of the hit on the wire plane in local coordinates

      ATH_MSG_VERBOSE("Local Direction: "<<Amg::toString(LOCDIRE, 2));
      ATH_MSG_VERBOSE("Local Position: " << Amg::toString(LPOS, 2));

      const double scale = Amg::intersect<3>(LPOS, LOCDIRE, Amg::Vector3D::UnitZ(), 0.).value_or(0);
      // Hit on the wire surface in local coordinates
      Amg::Vector3D hitOnSurf_wire = LPOS + scale * LOCDIRE;

      //The hit on the wire in Global coordinates
      Amg::Vector3D glob_hitOnSurf_wire = SURF_WIRE.transform() * hitOnSurf_wire;

      ATH_MSG_VERBOSE("Local Hit on Wire Surface: " << Amg::toString(hitOnSurf_wire, 2));
      ATH_MSG_VERBOSE("Global Hit on Wire Surface: " <<Amg::toString(glob_hitOnSurf_wire, 2));

      ATH_MSG_DEBUG("sTgcDigitizationTool::doDigitization hits mapped");

      const EBC_EVCOLL evColl = EBC_MAINEVCOLL;
      const HepMcParticleLink::PositionFlag idxFlag = (eventId==0) ? HepMcParticleLink::IS_POSITION: HepMcParticleLink::IS_EVENTNUM;
      const int barcode = hit.particleLink().barcode();
      const HepMcParticleLink particleLink(barcode, eventId, evColl, idxFlag);
      const sTGCSimHit temp_hit(hit.sTGCId(), hit.globalTime(),
                                HPOS,
                                hit.particleEncoding(),
                                hit.globalDirection(),
                                hit.depositEnergy(),
                                particleLink,
                                hit_kineticEnergy,
                                global_preStepPos);


      double globalHitTime = temp_hit.globalTime() + eventTime;
      double tof = temp_hit.globalPosition().mag()/CLHEP::c_light;
      double bunchTime = globalHitTime - tof;

      // Create all the digits for this particular Sim Hit
      sTgcDigitVec digiHits = m_digitizer->executeDigi(digitCond, temp_hit);
      if (digiHits.empty()) {
          continue;
      }
      ATH_MSG_VERBOSE("Hit produced " << digiHits.size() << " digits." );
      for( std::unique_ptr<sTgcDigit>& digit : digiHits) {
        /*
          NOTE:
          -----
          Since not every hit might end up resulting in a
          digit, this construction might take place after the hit loop
          in a loop of its own!
        */
        // make new sTgcDigit
        Identifier newDigitId = digit->identify(); //This Identifier should be sufficient to determine which RE the digit is from
        double newTime = digit->time();
        int newChannelType = idHelper.channelType(newDigitId);

        double timeJitterElectronicsStrip = CLHEP::RandGaussZiggurat::shoot(digitCond.rndmEngine, 0, m_timeJitterElectronicsStrip);
        double timeJitterElectronicsPad = CLHEP::RandGaussZiggurat::shoot(digitCond.rndmEngine, 0, m_timeJitterElectronicsPad);
        if(newChannelType== sTgcIdHelper::sTgcChannelTypes::Strip)
          newTime += timeJitterElectronicsStrip;
        else
          newTime += timeJitterElectronicsPad;
        uint16_t newBcTag = bcTagging(newTime+bunchTime);

        if(m_doToFCorrection)
          newTime += bunchTime;
        else
          newTime += globalHitTime;

        double newCharge = digit->charge();

        bool isDead{false}, isPileup{eventId != 0};
        ATH_MSG_VERBOSE("Hit is from the main signal subevent if eventId is zero, eventId = " << eventId << " newTime: " << newTime);
      

        // Create a new digit with updated time and BCTag
        sTgcDigit newDigit(newDigitId, newBcTag, newTime, newCharge, isDead, isPileup);
        ATH_MSG_VERBOSE("Unmerged Digit "<<m_idHelperSvc->toString(newDigitId)
                      <<" BC tag = "    << newDigit.bcTag() 
                      <<" digitTime = " << newDigit.time()
                      <<" charge = "    << newDigit.charge()) ;


        // Create a MuonSimData (SDO) corresponding to the digit
        MuonSimData::Deposit deposit(particleLink, MuonMCData(hit.depositEnergy(), tof));
        std::vector<MuonSimData::Deposit> deposits;
        deposits.push_back(std::move(deposit));
        MuonSimData simData(std::move(deposits), hit.particleEncoding());
        // The sTGC SDO should be placed at the center of the gap, on the wire plane.
        // We use the position from the hit on the wire surface which is by construction in the center of the gap
        // glob_hitOnSurf_wire projects the whole hit to the center of the gap
        simData.setPosition(glob_hitOnSurf_wire);
        simData.setTime(globalHitTime);
        const unsigned int modHash = static_cast<unsigned>(m_idHelperSvc->detElementHash(newDigitId));
        sTgcSimDigitCont& contToPush = newChannelType == sTgcIdHelper::sTgcChannelTypes::Pad ? unmergedPadDigits :
                                       newChannelType == sTgcIdHelper::sTgcChannelTypes::Strip ? unmergedStripDigits : unmergedWireDigits;                     
        /// Resize the container accordingly
        if (contToPush.size() <= modHash) contToPush.resize(modHash + 1);
        contToPush[modHash].emplace_back(std::move(simData), std::move(newDigit));
      } // end of loop digiHits
    } // end of while(i != e)
  } //end of while(m_thpcsTGC->nextDetectorElement(i, e))

  
  /*********************
  * Process Strip Digits *
  *********************/
  /* Comments from Alexandre Laurier, October 2022:
    Big update to VMM handling of digits to sTGC digitization
    For each channel type, the digits are processed on a layer-by-layer level
    This is done to improve the performance of strip neighborOn functionnality
    For wires, pads, and neighborOn=false strips, the digits on each channel
    are ordered by earlier to latest and processed in order.
    The digits are merged according to the VMM merging time window.
    Above threshold digits are saved to output unless a previous digit is found
    within the deadtime window.
    --- For neighborOn=true strips ---
    A strip above threshold forces the VMM readout of neighbor strips, even if
    neighbor strips are below threshold.
    We apply the logic as above, but for strips below threshold we search for one
    direct neighbor strip to be above VMM threshold which triggers the VMM
    to read the strip digit.
  */
  ATH_CHECK(processDigitsWithVMM(ctx, digitCond, unmergedStripDigits, m_deadtimeStrip,
                                 m_doNeighborOn, outputDigits, *sdoContainer));
  /*********************
  * Process Pad Digits *
  *********************/
  ATH_CHECK(processDigitsWithVMM(ctx, digitCond, unmergedPadDigits, m_deadtimePad,
                                 false, outputDigits, *sdoContainer));
  /*********************
  * Process Wire Digits *
  *********************/
  ATH_CHECK(processDigitsWithVMM(ctx, digitCond, unmergedWireDigits, m_deadtimeWire,
                                 false, outputDigits, *sdoContainer));
  /*************************************************
   * Output the digits to the StoreGate collection *
  *************************************************/
  for (sTgcDigitVec& digits : outputDigits) {
     if (digits.empty()) continue;
     const Identifier elemID = m_idHelperSvc->chamberId(digits[0]->identify());
     const IdentifierHash modHash = m_idHelperSvc->moduleHash(elemID);
     std::unique_ptr<sTgcDigitCollection> collection = std::make_unique<sTgcDigitCollection>(elemID, modHash);
     collection->insert(collection->end(), std::make_move_iterator(digits.begin()),
                                           std::make_move_iterator(digits.end()));
     ATH_CHECK(digitContainer->addCollection(collection.release(), modHash));
  }
  return StatusCode::SUCCESS;
}

/*******************************************************************************/
uint16_t sTgcDigitizationTool::bcTagging(const double digitTime) {

  uint16_t bctag = 0;

   int bunchInteger{0};  //Define the absolute distance from t0 in units of BX
   if(digitTime > 0) bunchInteger = (int)(abs(digitTime/25.0));  //absolute bunch for future bunches
   else bunchInteger = (int)(abs(digitTime/25.0)) + 1; //The absolute bunch for negative time needs to be shifted by 1 as there is no negative zero bunch
   bctag = (bctag | bunchInteger);  //Store bitwise the abs(BX).  This should be equivalent to regular variable assignment
   if(digitTime < 0) bctag = ~bctag;  //If from a PREVIOUS BX, apply bitwise negation

  return bctag;
}

double sTgcDigitizationTool::getChannelThreshold(const EventContext& ctx, 
                                                 const Identifier& channelID, 
                                                 const NswCalibDbThresholdData& thresholdData) const {

  float threshold = m_chargeThreshold, elecThrsld{0.f};

  if(!thresholdData.getThreshold(channelID, elecThrsld))
    ATH_MSG_ERROR("Cannot find retrieve VMM threshold from conditions data base!");
  if(!m_calibTool->pdoToCharge(ctx, true, elecThrsld, channelID, threshold))
    ATH_MSG_ERROR("Cannot convert VMM charge threshold via conditions data!");

  return threshold;
}


CLHEP::HepRandomEngine* sTgcDigitizationTool::getRandomEngine(const std::string& streamName, const EventContext& ctx) const
{
  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, streamName);
  std::string rngName = name()+streamName;
  rngWrapper->setSeed( rngName, ctx );
  CLHEP::HepRandomEngine* engine = rngWrapper->getEngine(ctx);
  ATH_MSG_VERBOSE(streamName<<" rngName "<<rngName<<" "<<engine);
  return engine;
}

StatusCode sTgcDigitizationTool::processDigitsWithVMM(const EventContext& ctx,
                                                      const DigiConditions& digiCond,
                                                      sTgcSimDigitCont& unmergedDigits,
                                                      const double vmmDeadTime,
                                                      const bool isNeighbourOn,
                                                      sTgcDigtCont& outDigitContainer,
                                                      MuonSimDataCollection& outSdoContainer) const {
  
  const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
  /// Start the merging by looping over the digit container and grouping the hits from the same layer together.
  for (sTgcSimDigitVec& digitsInCham : unmergedDigits) { 
    
    if (digitsInCham.empty()) continue;
    /// Merge all digits 
    sTgcSimDigitVec mergedDigits = processDigitsWithVMM(ctx, digiCond, vmmDeadTime, 
                                                       digitsInCham, isNeighbourOn);
    /// Update the container iterator to go to the next layer
    if (mergedDigits.empty()) continue;

    const IdentifierHash hash = m_idHelperSvc->moduleHash(mergedDigits.front().identify());
    const unsigned int hashIdx = static_cast<unsigned>(hash);
    /// Assign enough space in the container vector
    if (hash >= outDigitContainer.size()) {
      outDigitContainer.resize(hash + 1);
    }
    for (sTgcSimDigitData& merged : mergedDigits) {
        /// Push back the SDO
        outSdoContainer.insert(std::make_pair(merged.identify(), std::move(merged.getSimData())));
        /// apply the smearing before adding the digit
        bool acceptDigit{true};
        float chargeAfterSmearing = merged.getDigit().charge();
        if (m_doSmearing) {
            ATH_CHECK(m_smearingTool->smearCharge(merged.identify(), chargeAfterSmearing, acceptDigit, 
                                                  digiCond.rndmEngine));
        }
        if (!acceptDigit) {
            continue;
        }
        /// Select strips with charge > 0.001 pC to avoid having zero ADC count when converting
        /// charge [pC] to PDO [ADC count]
        if (idHelper.channelType(merged.identify()) == sTgcIdHelper::sTgcChannelTypes::Strip &&
            chargeAfterSmearing < 0.001) {
            continue;
        }
        std::unique_ptr<sTgcDigit> finalDigit = std::make_unique<sTgcDigit>(std::move(merged.getDigit()));
        if (m_doSmearing) {
            finalDigit->set_charge(chargeAfterSmearing);
        }
        ATH_MSG_VERBOSE("Final Digit "<<m_idHelperSvc->toString(finalDigit->identify())<<
                        " BC tag = "    << finalDigit->bcTag()<<
                        " digitTime = " << finalDigit->time() <<
                        " charge = "    << finalDigit->charge());
        outDigitContainer[hashIdx].push_back(std::move(finalDigit));
    }
  }
  return StatusCode::SUCCESS; 
}
sTgcSimDigitVec sTgcDigitizationTool::processDigitsWithVMM(const EventContext& ctx,
                                                          const DigiConditions& digiCond, 
                                                          const double vmmDeadTime, 
                                                          sTgcSimDigitVec& unmergedDigits,
                                                          const bool isNeighborOn) const {

  const MuonGM::MuonDetectorManager* detMgr{digiCond.detMgr};
  const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
  /// Sort the unmerged digit vector per layer Id -> by channel -> time from early to late arrival
  std::stable_sort(unmergedDigits.begin(), unmergedDigits.end(),  
      [&idHelper](const sTgcSimDigitData& a, const sTgcSimDigitData& b) {
        const int layA = idHelper.gasGap(a.identify()); 
        const int layB = idHelper.gasGap(b.identify());
        if (layA != layB) return layA < layB;
        const int chA = idHelper.channel(a.identify());
        const int chB = idHelper.channel(b.identify());
        if (chA != chB) return chA < chB;
        return a.time() < b.time();
      });
  sTgcSimDigitVec savedDigits{}, premerged{};

  premerged.reserve(unmergedDigits.size());
  savedDigits.reserve(premerged.capacity());

      
  auto passNeigbourLogic = [&](const sTgcSimDigitData& candidate) {
      if (!isNeighborOn || savedDigits.empty()) return false;
      if (savedDigits.back().identify() == candidate.identify() &&
          std::abs(savedDigits.back().time() - candidate.time()) < vmmDeadTime) {
            ATH_MSG_VERBOSE("Digits are too close in time ");
            return false;
      }
      const Identifier digitId = candidate.identify();
      const int channel = idHelper.channel(digitId);
      const int maxChannel = detMgr->getsTgcReadoutElement(digitId)->numberOfStrips(digitId);
      for (int neighbour : {std::max(1, channel -1), std::min(maxChannel, channel+1)}) {
        /// Catch the cases where the channel is  1 or maxChannel
        if (neighbour == channel) continue;
        const Identifier neighbourId = idHelper.channelID(digitId, 
                                                          idHelper.multilayer(digitId),
                                                          idHelper.gasGap(digitId), 
                                                          idHelper.channelType(digitId), neighbour);
        const double threshold = m_useCondThresholds ? getChannelThreshold(ctx, neighbourId, *digiCond.thresholdData)  
                                                     : m_chargeThreshold.value();          
        if (std::find_if(savedDigits.begin(), savedDigits.end(), [&](const sTgcSimDigitData& known){
            return known.identify() == neighbourId && 
                   known.getDigit().charge() > threshold &&
                   std::abs(known.time() - candidate.time()) <  m_hitTimeMergeThreshold;
        }) != savedDigits.end()) return true;
      
      }
      return false;
  };
  // Sort digits on every channel by earliest to latest time
  // Also do hit merging to help with neighborOn logic
  double threshold = m_chargeThreshold;
  for (sTgcSimDigitVec::iterator merge_me = unmergedDigits.begin(); merge_me!= unmergedDigits.end(); ++merge_me) {
    if(m_useCondThresholds) {
      threshold = getChannelThreshold(ctx, (*merge_me).identify(), *digiCond.thresholdData);
    }
    /// merge digits in time. Do weighed average to find time of
    /// digits originally below threshold. Follows what we expect from real VMM.
    sTgcDigit& digit1{(*merge_me).getDigit()};
    double totalCharge = digit1.charge();
    double weightedTime = digit1.time();
      
    sTgcSimDigitVec::iterator merge_with = merge_me + 1;
    for ( ; merge_with!= unmergedDigits.end(); ++merge_with) {
      /// We reached another digit. No need to merge
      if ((*merge_with).identify() != (*merge_me).identify()) {
          break;
      }
      const sTgcDigit& mergeDigit{(*merge_with).getDigit()};
      // If future digits are within window, digit1 absorbs its charge
      if (mergeDigit.time() - digit1.time() > m_hitTimeMergeThreshold) break;
      // If digit1 is not above threshold prior to merging, the new time is
      // a weighted average. Do it for every merging pair.
      if (totalCharge < threshold) {
         weightedTime = (weightedTime * totalCharge + mergeDigit.time() * mergeDigit.charge())
                      / (totalCharge + mergeDigit.charge());
      }
      totalCharge += mergeDigit.charge();
    }
    digit1.set_charge(totalCharge);
    digit1.set_time(weightedTime);
    sTgcSimDigitData& mergedHit{*merge_me};
    if (!savedDigits.empty() && 
        savedDigits.back().identify() == digit1.identify() &&
        std::abs(savedDigits.back().time() - digit1.time()) <= vmmDeadTime) continue;
    if (digit1.charge() > threshold || passNeigbourLogic(mergedHit)){
        savedDigits.emplace_back(std::move(mergedHit));
    } else if (isNeighborOn) {
        premerged.emplace_back(std::move(mergedHit));
    }    
  } // end of time-ordering and hit merging loop
  std::copy_if(std::make_move_iterator(premerged.begin()),
               std::make_move_iterator(premerged.end()),
               std::back_inserter(savedDigits), passNeigbourLogic);
  return savedDigits;
}
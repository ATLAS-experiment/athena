/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////////////////////
//
// RpcDigitizationTool
// ------------
// Authors:
//             Andrea Di Simone  <Andrea.Di.Simone@cern.ch>
//             Gabriele Chiodini <gabriele.chiodini@le.infn.it>
//             Stefania Spagnolo <stefania.spagnolo@le.infn.it>
////////////////////////////////////////////////////////////////////////////////

#include "RPC_Digitization/RpcDigitizationTool.h"

// Inputs
#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "GeoModelHelpers/TransformToStringConverter.h"
#include "MuonSimEvent/RPCSimHit.h"
#include "MuonSimEvent/RPCSimHitCollection.h"

// Geometry
#include "MuonIdHelpers/RpcIdHelper.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "MuonSimEvent/RpcHitIdHelper.h"

// run n. from geometry DB
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "GeometryDBSvc/IGeometryDBSvc.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

// Truth
#include "AtlasHepMC/GenParticle.h"
#include "TruthUtils/HepMCHelpers.h"
#include "GeneratorObjects/HepMcParticleLink.h"
#include "GeoModelKernel/throwExcept.h"
// Random Numbers
#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandExponential.h"
#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGaussZiggurat.h"

// Core includes
#include <TString.h>  // for Form

#include <atomic>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>

#include "EventInfoMgt/ITagInfoMgr.h"
#include "PathResolver/PathResolver.h"

// 12 charge points, 15 BetaGamma points, 180 efficiency points for fcp search
namespace {
    constexpr int N_Charge = 12;
    constexpr int N_Velocity = 15;
    constexpr std::array<double, N_Charge> Charge{0.1, 0.2, 0.3, 0.33, 0.4, 0.5, 0.6, 0.66, 0.7, 0.8, 0.9, 1.0};
    constexpr std::array<double,  N_Velocity> Velocity{0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 2.0, 3.0, 10.0, 100.0, 1000.0};
    constexpr double Eff_garfield[N_Charge][N_Velocity] = {
        {0.8648, 0.3476, 0.1407, 0.0618, 0.0368, 0.0234, 0.0150, 0.0120, 0.0096, 0.0079, 0.0038, 0.0041, 0.0035, 0.0049, 0.0054},
        {0.9999, 0.9238, 0.6716, 0.4579, 0.3115, 0.2238, 0.1727, 0.1365, 0.1098, 0.0968, 0.0493, 0.0451, 0.0528, 0.0694, 0.0708},
        {1.0000, 0.9978, 0.9517, 0.8226, 0.6750, 0.5611, 0.4674, 0.3913, 0.3458, 0.3086, 0.1818, 0.1677, 0.1805, 0.2307, 0.2421},
        {1.0000, 0.9994, 0.9758, 0.8918, 0.7670, 0.6537, 0.5533, 0.4856, 0.4192, 0.3852, 0.2333, 0.2186, 0.2479, 0.2957, 0.2996},
        {1.0000, 1.0000, 0.9972, 0.9699, 0.9022, 0.8200, 0.7417, 0.6660, 0.6094, 0.5622, 0.3846, 0.3617, 0.3847, 0.4578, 0.4583},
        {1.0000, 1.0000, 0.9998, 0.9956, 0.9754, 0.9479, 0.9031, 0.8604, 0.8126, 0.7716, 0.5827, 0.5545, 0.5865, 0.6834, 0.6706},
        {1.0000, 1.0000, 1.0000, 0.9997, 0.9968, 0.9876, 0.9689, 0.9464, 0.9221, 0.8967, 0.7634, 0.7385, 0.7615, 0.8250, 0.8309},
        {1.0000, 1.0000, 1.0000, 1.0000, 0.9995, 0.9952, 0.9866, 0.9765, 0.9552, 0.9427, 0.8373, 0.8127, 0.8412, 0.8899, 0.8891},
        {1.0000, 1.0000, 1.0000, 1.0000, 0.9995, 0.9981, 0.9918, 0.9803, 0.9754, 0.9602, 0.8730, 0.8564, 0.8746, 0.9178, 0.9261},
        {1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 0.9993, 0.9990, 0.9951, 0.9935, 0.9886, 0.9419, 0.9277, 0.9422, 0.9686, 0.9700},
        {1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 0.9998, 0.9996, 0.9980, 0.9966, 0.9786, 0.9718, 0.9748, 0.9875, 0.9882},
        {1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 1.0000, 0.9998, 1.0000, 0.9991, 0.9988, 0.9913, 0.9872, 0.9917, 0.9970, 0.9964}};
    bool
    validIndex(int idx, int arraySize){
      return (idx>=0) and (idx<arraySize);
    }
}  // namespace

using namespace MuonGM;
namespace {
    constexpr double SIG_VEL = 4.8;
}


RpcDigitizationTool::RpcDigitizationTool(const std::string& type, const std::string& name, const IInterface* pIID) :
    PileUpToolBase(type, name, pIID) {}

// member function implementation
//--------------------------------------------
StatusCode RpcDigitizationTool::initialize() {
    ATH_MSG_DEBUG("RpcDigitizationTool:: in initialize()");
    ATH_MSG_DEBUG("Configuration  RpcDigitizationTool ");

    ATH_MSG_DEBUG("InputObjectName        " << m_inputHitCollectionName);
    ATH_MSG_DEBUG("OutputObjectName       " << m_outputDigitCollectionKey.key());
    ATH_MSG_DEBUG("OutputSDOName          " << m_outputSDO_CollectionKey.key());
    ATH_MSG_DEBUG("WindowLowerOffset      " << m_timeWindowLowerOffset);
    ATH_MSG_DEBUG("WindowUpperOffset      " << m_timeWindowUpperOffset);
    ATH_MSG_DEBUG("DeadTime               " << m_deadTime);
    ATH_MSG_DEBUG("RndmSvc                " << m_rndmSvc);
    ATH_MSG_DEBUG("PatchForRpcTime        " << m_patch_for_rpc_time);
    ATH_MSG_DEBUG("RpcTimeShift           " << m_rpc_time_shift);
    ATH_MSG_DEBUG("RPC_TimeSchema         " << m_RPC_TimeSchema);
    ATH_MSG_DEBUG("RPCSDOareRPCDigits     " << m_sdoAreOnlyDigits);

    ATH_MSG_DEBUG("IgnoreRunDependentConfig " << m_ignoreRunDepConfig);
    ATH_MSG_DEBUG("turnON_efficiency      " << m_turnON_efficiency);
    ATH_MSG_DEBUG("Efficiency_fromCOOL    " << m_Efficiency_fromCOOL);
    ATH_MSG_DEBUG("Efficiency_BIS78_fromCOOL" << m_Efficiency_BIS78_fromCOOL);
    ATH_MSG_DEBUG("turnON_clustersize     " << m_turnON_clustersize);
    ATH_MSG_DEBUG("ClusterSize_fromCOOL   " << m_ClusterSize_fromCOOL);
    ATH_MSG_DEBUG("ClusterSize_BIS78_fromCOOL" << m_ClusterSize_BIS78_fromCOOL);
    ATH_MSG_DEBUG("FirstClusterSizeInTail " << m_FirstClusterSizeInTail);
    ATH_MSG_DEBUG("ClusterSize1_2uncorr   " << m_ClusterSize1_2uncorr);
    ATH_MSG_DEBUG("BOG_BOF_DoubletR2_OFF  " << m_BOG_BOF_DoubletR2_OFF);
    ATH_MSG_DEBUG("CutMaxClusterSize      " << m_CutMaxClusterSize);
    ATH_MSG_DEBUG("CutProjectedTracks     " << m_CutProjectedTracks);
    ATH_MSG_DEBUG("ValidationSetup        " << m_validationSetup);
    ATH_MSG_DEBUG("IncludePileUpTruth     " << m_includePileUpTruth);
    ATH_MSG_DEBUG("VetoPileUpTruthLinks   " << m_vetoPileUpTruthLinks);

    ATH_CHECK(m_detMgrKey.initialize());
    if (m_onlyUseContainerName) { ATH_CHECK(m_mergeSvc.retrieve()); }
    ATH_CHECK(detStore()->retrieve(m_idHelper));
    // check the identifiers

    ATH_MSG_INFO("Max Number of RPC Gas Gaps for these Identifiers = " << m_idHelper->gasGapMax());

    // check the input object name
    if (m_hitsContainerKey.key().empty()) {
        ATH_MSG_FATAL("Property InputObjectName not set !");
        return StatusCode::FAILURE;
    }
    if (m_onlyUseContainerName) m_inputHitCollectionName = m_hitsContainerKey.key();
    ATH_MSG_DEBUG("Input objects in container : '" << m_inputHitCollectionName << "'");

    // Initialize ReadHandleKey
    ATH_CHECK(m_hitsContainerKey.initialize());

    // initialize the output WriteHandleKeys
    ATH_CHECK(m_outputDigitCollectionKey.initialize());
    ATH_CHECK(m_outputSDO_CollectionKey.initialize());
    ATH_CHECK(m_simHitValidKey.initialize(m_validationSetup));
    ATH_MSG_DEBUG("Output digits: '" << m_outputDigitCollectionKey.key() << "'");

    // set the configuration based on run1/run2
    ATH_CHECK(initializeRunDependentParameters());
    
    ATH_MSG_DEBUG("Ready to read parameters for cluster simulation from file");

    ATH_CHECK(m_rndmSvc.retrieve());

    // fill the taginfo information
    ATH_CHECK(fillTagInfo());

    ATH_CHECK(m_readKey.initialize(m_RPCInfoFromDb));

    ///////////////////// special test
    //  m_turnON_clustersize=false;
    m_BOF_id = m_idHelper->stationNameIndex("BOF");
    m_BOG_id = m_idHelper->stationNameIndex("BOG");
    m_BOS_id = m_idHelper->stationNameIndex("BOS");
    m_BIL_id = m_idHelper->stationNameIndex("BIL");
    m_BIS_id = m_idHelper->stationNameIndex("BIS");
    m_muonHelper = RpcHitIdHelper::GetHelper(m_idHelper->gasGapMax());

    return StatusCode::SUCCESS;
}

StatusCode RpcDigitizationTool::initializeRunDependentParameters() {
  // TODO This should all be in a conditions Alg
  // Retrieve geometry config information from the database (RUN1, RUN2, etc...)
  SmartIF<IGeoModelSvc> geoModel{Gaudi::svcLocator()->service("GeoModelSvc")};
  if ( !geoModel ) {
    ATH_MSG_ERROR("Could not locate GeoModelSvc");
    return StatusCode::FAILURE;
  }

  // check the DetDescr version
  std::string atlasVersion = geoModel->atlasVersion();

  SmartIF<IRDBAccessSvc> rdbAccess{Gaudi::svcLocator()->service("RDBAccessSvc")};
  if ( !rdbAccess ) {
    ATH_MSG_ERROR("Could not locate RDBAccessSvc");
    return StatusCode::FAILURE;
  }

  enum DataPeriod {Unknown, Run1, Run2, Run3, Run4 };
  DataPeriod run = Unknown;

  std::string configVal = "";

  IRDBRecordset_ptr atlasCommonRec = rdbAccess->getRecordsetPtr("AtlasCommon", atlasVersion, "ATLAS");
  if (atlasCommonRec->size() == 0) {
    run = Run1;
  } else {
    configVal = (*atlasCommonRec)[0]->getString("CONFIG");
    ATH_MSG_INFO("From DD Database, Configuration is " << configVal);
    if (configVal == "RUN1") {
      run = Run1;
    } else if (configVal == "RUN2") {
      run = Run2;
    } else if (configVal == "RUN3") {
      run = Run3;
    } else if (configVal == "RUN4") {
      run = Run4;
    }
    if (run == DataPeriod::Unknown) {
      ATH_MSG_FATAL("Unexpected value for geometry config read from the database: " << configVal);
      return StatusCode::FAILURE;
    }
  }
  if (run == Run3 && m_idHelper->gasGapMax() < 3)
    ATH_MSG_WARNING("Run3,  configVal = " << configVal << " and GasGapMax =" << m_idHelper->gasGapMax());

  if (run == Run1)
    ATH_MSG_INFO("From Geometry DB: MuonSpectrometer configuration is: RUN1 or MuonGeometry = R.06");
  else if (run == Run2)
    ATH_MSG_INFO("From Geometry DB: MuonSpectrometer configuration is: RUN2 or MuonGeometry = R.07");
  else if (run == Run3)
    ATH_MSG_INFO("From Geometry DB: MuonSpectrometer configuration is: RUN3 or MuonGeometry = R.09");
  else if (run == Run4)
    ATH_MSG_INFO("From Geometry DB: MuonSpectrometer configuration is: RUN4 or MuonGeometry = R.10");

  if (m_ignoreRunDepConfig == false) {
        m_BOG_BOF_DoubletR2_OFF = false;
        m_Efficiency_fromCOOL = false;
        m_ClusterSize_fromCOOL = false;
        m_RPCInfoFromDb = false;
        m_kill_deadstrips = false;
        if (run == Run1) {
            // m_BOG_BOF_DoubletR2_OFF = true
            // m_Efficiency_fromCOOL   = true
            // m_ClusterSize_fromCOOL  = true
            m_BOG_BOF_DoubletR2_OFF = true;
            if (configVal == "RUN1") {  // MC12 setup
                m_Efficiency_fromCOOL = true;
                m_ClusterSize_fromCOOL = true;
                m_RPCInfoFromDb = true;
                m_kill_deadstrips = true;
                m_CutProjectedTracks = 50;
            }
        } else {
            // m_BOG_BOF_DoubletR2_OFF = false # do not turn off at digitization the hits in the dbR=2 chambers in the feet
            // m_Efficiency_fromCOOL   = false # use common average values in python conf.
            // m_ClusterSize_fromCOOL  = false # use common average values in python conf.
            m_BOG_BOF_DoubletR2_OFF = false;
            if (run == Run2) {  // MC15c setup
                m_Efficiency_fromCOOL = true;
                m_ClusterSize_fromCOOL = true;
                m_RPCInfoFromDb = true;
                m_kill_deadstrips = false;
                m_CutProjectedTracks = 100;
            } else {
                ATH_MSG_INFO("Run3/4: configuration parameter not from COOL");
                m_Efficiency_fromCOOL = false;
                m_ClusterSize_fromCOOL = false;
                m_RPCInfoFromDb = false;
                m_kill_deadstrips = false;
            }
        }
        ATH_MSG_INFO("RPC Run1/2/3-dependent configuration is enforced");
    } else {
        ATH_MSG_WARNING("Run1/2/3-dependent configuration is bypassed; be careful with option settings");
    }

    ATH_MSG_DEBUG("......RPC Efficiency_fromCOOL    " << m_Efficiency_fromCOOL);
    ATH_MSG_DEBUG("......RPC ClusterSize_fromCOOL   " << m_ClusterSize_fromCOOL);
    ATH_MSG_DEBUG("......RPC BOG_BOF_DoubletR2_OFF  " << m_BOG_BOF_DoubletR2_OFF);
    ATH_MSG_DEBUG("......RPC RPCInfoFromDb          " << m_RPCInfoFromDb);
    ATH_MSG_DEBUG("......RPC KillDeadStrips         " << m_kill_deadstrips);
    ATH_MSG_DEBUG("......RPC CutProjectedTracks     " << m_CutProjectedTracks);


    return StatusCode::SUCCESS;
}

template <class CondType> 
StatusCode RpcDigitizationTool::retrieveCondData(const EventContext& ctx,
                                                 const SG::ReadCondHandleKey<CondType>& key,
                                                 const CondType* & condPtr) const {

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
//--------------------------------------------
StatusCode RpcDigitizationTool::prepareEvent(const EventContext& /*ctx*/, unsigned int) {
    ATH_MSG_DEBUG("RpcDigitizationTool::in prepareEvent()");

    // John's Hacks START
    m_RPCHitCollList.clear();
    m_thpcRPC = std::make_unique<TimedHitCollection<RPCSimHit>>();
    // John's Hacks END

    return StatusCode::SUCCESS;
}

//--------------------------------------------
StatusCode RpcDigitizationTool::processBunchXing(int bunchXing, SubEventIterator bSubEvents, SubEventIterator eSubEvents) {
    ATH_MSG_DEBUG("RpcDigitizationTool::in processBunchXing()");

    typedef PileUpMergeSvc::TimedList<RPCSimHitCollection>::type TimedHitCollList;
    TimedHitCollList hitCollList;

    if (!(m_mergeSvc->retrieveSubSetEvtData(m_inputHitCollectionName, hitCollList, bunchXing, bSubEvents, eSubEvents).isSuccess()) &&
        hitCollList.empty()) {
        ATH_MSG_ERROR("Could not fill TimedHitCollList");
        return StatusCode::FAILURE;
    } else {
        ATH_MSG_VERBOSE(hitCollList.size() << " RPCSimHitCollection with key " << m_inputHitCollectionName << " found");
    }

    TimedHitCollList::iterator iColl(hitCollList.begin());
    TimedHitCollList::iterator endColl(hitCollList.end());

    // Iterating over the list of collections
    for (; iColl != endColl; ++iColl) {
        RPCSimHitCollection* hitCollPtr = new RPCSimHitCollection(*iColl->second);
        PileUpTimeEventIndex timeIndex(iColl->first);

        ATH_MSG_DEBUG("RPCSimHitCollection found with " << hitCollPtr->size() << " hits");
        ATH_MSG_VERBOSE("time index info. time: " << timeIndex.time() << " index: " << timeIndex.index() << " type: " << timeIndex.type());

        m_thpcRPC->insert(timeIndex, hitCollPtr);
        m_RPCHitCollList.emplace_back(hitCollPtr);
    }

    return StatusCode::SUCCESS;
}

//--------------------------------------------
// Get next event and extract collection of hit collections:
StatusCode RpcDigitizationTool::getNextEvent(const EventContext& ctx) {
    ATH_MSG_DEBUG("RpcDigitizationTool::getNextEvent()");

    // initialize pointer
    m_thpcRPC.reset();

    //  get the container(s)
    using TimedHitCollList = PileUpMergeSvc::TimedList<RPCSimHitCollection>::type;

    // In case of single hits container just load the collection using read handles
    if (!m_onlyUseContainerName) {
        SG::ReadHandle<RPCSimHitCollection> hitCollection(m_hitsContainerKey, ctx);
        if (!hitCollection.isValid()) {
            ATH_MSG_ERROR("Could not get RPCSimHitCollection container " << hitCollection.name() << " from store "
                                                                         << hitCollection.store());
            return StatusCode::FAILURE;
        }

        // create a new hits collection
        m_thpcRPC = std::make_unique<TimedHitCollection<RPCSimHit>>(1);
        m_thpcRPC->insert(0, hitCollection.cptr());
        ATH_MSG_DEBUG("RPCSimHitCollection found with " << hitCollection->size() << " hits");

        return StatusCode::SUCCESS;
    }
    // this is a list<pair<time_t, DataLink<RPCSimHitCollection> > >
    TimedHitCollList hitCollList;

    if (!(m_mergeSvc->retrieveSubEvtsData(m_inputHitCollectionName, hitCollList).isSuccess())) {
        ATH_MSG_ERROR("Could not fill TimedHitCollList");
        return StatusCode::FAILURE;
    }
    if (hitCollList.empty()) {
        ATH_MSG_ERROR("TimedHitCollList has size 0");
        return StatusCode::FAILURE;
    } else {
        ATH_MSG_DEBUG(hitCollList.size() << " RPCSimHitCollections with key " << m_inputHitCollectionName << " found");
    }

    // create a new hits collection
    m_thpcRPC = std::make_unique<TimedHitCollection<RPCSimHit>>();
    // now merge all collections into one
    TimedHitCollList::iterator iColl(hitCollList.begin());
    TimedHitCollList::iterator endColl(hitCollList.end());
    while (iColl != endColl) {
        const RPCSimHitCollection* p_collection(iColl->second);
        m_thpcRPC->insert(iColl->first, p_collection);
        // if ( m_debug ) ATH_MSG_DEBUG ( "RPCSimHitCollection found with "
        //   << p_collection->size() << " hits" );    // loop on the hit collections
        ++iColl;
    }
    return StatusCode::SUCCESS;
}

//--------------------------------------------
StatusCode RpcDigitizationTool::mergeEvent(const EventContext& ctx) {
    StatusCode status = StatusCode::SUCCESS;

    ATH_MSG_DEBUG("RpcDigitizationTool::in mergeEvent()");
    // create and record the Digit container in StoreGate
    SG::WriteHandle<RpcDigitContainer> digitContainer(m_outputDigitCollectionKey, ctx);
    ATH_CHECK(digitContainer.record(std::make_unique<RpcDigitContainer>(m_idHelper->module_hash_max())));
    ATH_MSG_DEBUG("RpcDigitContainer recorded in StoreGate.");

    // Create and record the SDO container in StoreGate
    SG::WriteHandle<MuonSimDataCollection> sdoContainer(m_outputSDO_CollectionKey, ctx);
    ATH_CHECK(sdoContainer.record(std::make_unique<MuonSimDataCollection>()));
    ATH_MSG_DEBUG("RpcSDOCollection recorded in StoreGate.");

    //////////////// TEMP////
    m_sdo_tmp_map.clear();
    /////////////////////////

    Collections_t collections;
    status = doDigitization(ctx, collections, sdoContainer.ptr());
    if (status.isFailure()) { ATH_MSG_ERROR("doDigitization Failed"); }
    for (size_t coll_hash = 0; coll_hash < collections.size(); ++coll_hash) {
      if (collections[coll_hash]) {
        ATH_CHECK( digitContainer->addCollection (collections[coll_hash].release(), coll_hash) );
      }
    }

    // Clean-up
    m_RPCHitCollList.clear();

    return status;
}

//--------------------------------------------
StatusCode RpcDigitizationTool::processAllSubEvents(const EventContext& ctx) {
    StatusCode status = StatusCode::SUCCESS;

    // merging of the hit collection in getNextEvent method

    ATH_MSG_DEBUG("RpcDigitizationTool::in digitize()");

    // create and record the Digit container in StoreGate
    SG::WriteHandle<RpcDigitContainer> digitContainer(m_outputDigitCollectionKey, ctx);
    ATH_CHECK(digitContainer.record(std::make_unique<RpcDigitContainer>(m_idHelper->module_hash_max())));
    ATH_MSG_DEBUG("RpcDigitContainer recorded in StoreGate.");

    // Create and record the SDO container in StoreGate
    SG::WriteHandle<MuonSimDataCollection> sdoContainer(m_outputSDO_CollectionKey, ctx);
    ATH_CHECK(sdoContainer.record(std::make_unique<MuonSimDataCollection>()));
    ATH_MSG_DEBUG("RpcSDOCollection recorded in StoreGate.");

    //////////////// TEMP////
    m_sdo_tmp_map.clear();
    /////////////////////////

    if (!m_thpcRPC) {
        status = getNextEvent(ctx);
        if (StatusCode::FAILURE == status) {
            ATH_MSG_INFO("There are no RPC hits in this event");
            return status;  // there are no hits in this event
        }
    }

    Collections_t collections;
    ATH_CHECK(doDigitization(ctx, collections, sdoContainer.ptr()));
    for (size_t coll_hash = 0; coll_hash < collections.size(); ++coll_hash) {
      if (collections[coll_hash]) {
        ATH_CHECK( digitContainer->addCollection (collections[coll_hash].release(), coll_hash) );
      }
    }

    return status;
}

//--------------------------------------------
StatusCode RpcDigitizationTool::doDigitization(const EventContext& ctx,
                                               Collections_t& collections,
                                               MuonSimDataCollection* sdoContainer) {
    ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this);
    rngWrapper->setSeed(name(), ctx);
    CLHEP::HepRandomEngine* rndmEngine = rngWrapper->getEngine(ctx);

    const MuonGM::MuonDetectorManager* detMgr{nullptr};
    ATH_CHECK(retrieveCondData(ctx, m_detMgrKey, detMgr));


    std::unique_ptr<RPCSimHitCollection> inputSimHitColl{std::make_unique<RPCSimHitCollection>("RPC_Hits")};


    // get the iterator pairs for this DetEl
    // iterate over hits
    TimedHitCollection<RPCSimHit>::const_iterator i, e;

    // Perform null check on m_thpcRPC
    if (!m_thpcRPC) {
        ATH_MSG_ERROR("m_thpcRPC is null");
        return StatusCode::FAILURE;
    }

    struct SimDataContent {
        Identifier channelId{};
        std::vector<MuonSimData::Deposit> deposits;
        Amg::Vector3D gpos{Amg::Vector3D::Zero()};
        double simTime{0.};
    };

    while (m_thpcRPC->nextDetectorElement(i, e)) {
        // to store the a single

        std::map<Identifier, SimDataContent> channelSimDataMap;

        // Loop over the hits:
        while (i != e) {
            ATH_MSG_DEBUG("RpcDigitizationTool::loop over the hits");

            TimedHitPtr<RPCSimHit> phit(*i++);

            // the hit
            const RPCSimHit& hit(*phit);
            // the hit id
            const int idHit = hit.RPCid();
            // the global time (G4 time + bunch time)
            const double globalHitTime{hitTime(phit)};
            // the G4 time or TOF from IP
            const double G4Time{hit.globalTime()};
            // the bunch time
            const double bunchTime{globalHitTime - hit.globalTime()};

            ATH_MSG_DEBUG("Global time " << globalHitTime << " G4 time " << G4Time << " Bunch time " << bunchTime);

            if (!m_simHitValidKey.empty()) {
                ATH_MSG_VERBOSE("Validation:  globalHitTime, G4Time, BCtime = " << globalHitTime << " " << G4Time << " " << bunchTime);
                inputSimHitColl->Emplace(idHit, globalHitTime, hit.localPosition(),
                                         HepMcParticleLink::getRedirectedLink(phit->particleLink(), phit.eventId(), ctx), // This link should now correctly resolve to the TruthEvent McEventCollection in the main StoreGateSvc.
                                         hit.postLocalPosition(),
                                         hit.energyDeposit(), hit.stepLength(), hit.particleEncoding(), hit.kineticEnergy());
            }

            // convert sim id helper to offline id
            const std::string stationName = m_muonHelper->GetStationName(idHit);
            const int stationEta = m_muonHelper->GetZSector(idHit);
            const int stationPhi = m_muonHelper->GetPhiSector(idHit);
            const int doubletR = m_muonHelper->GetDoubletR(idHit);
            const int doubletZ = m_muonHelper->GetDoubletZ(idHit);
            const int doubletPhi = m_muonHelper->GetDoubletPhi(idHit);
            int gasGap = m_muonHelper->GetGasGapLayer(idHit);
            
            if (m_muonHelper->GetMeasuresPhi(idHit)) continue;  // Skip phi strip . To be created after efficiency evaluation


            bool isValid{false};
            const Identifier elementID = m_idHelper->elementID(stationName,stationEta,stationPhi,doubletR, isValid);
            if (!isValid) {
                ATH_MSG_WARNING("Failed to construct the element ID from "<<stationName
                            <<", stationEta: "<<stationEta<<", stationPhi: "<<stationPhi<<", doubletR: "<<doubletR);
                continue;
            }
            // construct Atlas identifier from components
            ATH_MSG_DEBUG("creating id for hit in element:"
                          << " stationName " << stationName << " stationEta " << stationEta << " stationPhi " << stationPhi << " doubletR "
                          << doubletR << " doubletZ " << doubletZ << " doubletPhi " << doubletPhi << " gasGap " << gasGap);
            const Identifier detElId{m_idHelper->channelID(elementID, doubletZ, doubletPhi, 1,0, 1, isValid)};
            if (!isValid) {
                continue;
            }
            const RpcReadoutElement* reEle = detMgr->getRpcReadoutElement(detElId);
            /// Let's pray that we will never discover that BIS78 is mounted upside down
            if (false && reEle->rotatedRpcModule()) {
                gasGap = gasGap == 1 ? 2 : 1;
            } 

            
            bool isValidEta{false}, isValidPhi{false};
            const Identifier idpaneleta = m_idHelper->channelID(elementID, doubletZ, doubletPhi, gasGap, 0, 1, isValidEta);
            const Identifier idpanelphi = m_idHelper->channelID(elementID, doubletZ, doubletPhi, gasGap, 1, 1, isValidPhi);
            if (!isValidEta || !isValidPhi) {
                ATH_MSG_WARNING("Found an invalid identifier "
                                << " stationName " << stationName << " stationEta " << stationEta << " stationPhi " << stationPhi
                                << " doubletR " << doubletR << " doubletZ " << doubletZ << " doubletPhi " << doubletPhi << " gasGap "
                                << gasGap);
                continue;
            }
            // loop on eta and phi to apply correlated efficiency between the two views

            /// Use special jitter consant for BIS & BIL chambers.
            const double tmp_CorrJitter = m_idHelper->stationName(idpaneleta) < 2 ? m_CorrJitter_BIS78 : m_CorrJitter;
            /// If a jitter constant has been defined smear it!
            const double corrtimejitter = tmp_CorrJitter > 0.01 ? 
                                                CLHEP::RandGaussZiggurat::shoot(rndmEngine, 0., tmp_CorrJitter) : 0.;  // correlated jitter
            // handle here the special case where eta panel is dead => phi strip status (dead or eff.) cannot be resolved;
            // measured panel eff. will be used in that case and no phi strip killing will happen


            // Extrapolate the hit to the gas gap centre located at x=0 
            const Amg::Vector3D hitDir{(hit.postLocalPosition() - hit.localPosition()).unit()};
            const Amg::Vector3D gapCentre = hit.localPosition() + 
                                            Amg::intersect<3>(hit.localPosition(), hitDir, Amg::Vector3D::UnitX(), 0).value_or(0) * hitDir;

            std::array<int, 3> pcseta = physicalClusterSize(ctx, reEle, idpaneleta, gapCentre, rndmEngine);  // set to one for new algorithms
            ATH_MSG_VERBOSE("Simulated cluster on eta panel: size/first/last= " << pcseta[0] << "/" << pcseta[1] << "/" << pcseta[2]);
            std::array<int, 3> pcsphi = physicalClusterSize(ctx, reEle, idpanelphi, gapCentre, rndmEngine);  // set to one for new algorithms
            ATH_MSG_VERBOSE("Simulated cluster on phi panel: size/first/last= " << pcsphi[0] << "/" << pcsphi[1] << "/" << pcsphi[2]);

            

            // create Identifiers
            const Identifier atlasRpcIdeta = m_idHelper->channelID(elementID, doubletZ, doubletPhi, gasGap, 0, pcseta[1], isValidEta);
            const Identifier atlasRpcIdphi = m_idHelper->channelID(elementID, doubletZ, doubletPhi, gasGap, 1, pcsphi[1], isValidPhi);

            const HepMcParticleLink particleLink = HepMcParticleLink::getRedirectedLink(phit->particleLink(), phit.eventId(), ctx); // This link should now correctly resolve to the TruthEvent McEventCollection in the main StoreGateSvc.
            const auto [etaStripOn, phiStripOn] = detectionEfficiency(ctx, idpaneleta, idpanelphi, rndmEngine, particleLink);
            ATH_MSG_DEBUG("SetPhiOn " << phiStripOn << " SetEtaOn " << etaStripOn);

            for (bool  imeasphi : {false, true}) {
                if (!imeasphi &&  (!etaStripOn || !isValidEta)) continue;
                if (imeasphi && (!phiStripOn || !isValidPhi)) continue;


                // get Identifier and list of clusters for this projection
                const Identifier& atlasId = !imeasphi ? atlasRpcIdeta : atlasRpcIdphi;
                std::array<int, 3> pcs{!imeasphi ? pcseta : pcsphi};

                ATH_MSG_DEBUG("SetOn: stationName " << stationName << " stationEta " << stationEta << " stationPhi " << stationPhi
                                                    << " doubletR " << doubletR << " doubletZ " << doubletZ << " doubletPhi " << doubletPhi
                                                    << " gasGap " << gasGap << " measphi " << imeasphi);

                // pcs contains the cluster size, the first strip number and the last strip number of the cluster
                pcs = TurnOnStrips(reEle, std::move(pcs), atlasId);
                if (pcs[2] < 0){
                    continue;
                } 

                ATH_MSG_DEBUG("Simulated cluster1: size/first/last= " << pcs[0] << "/" << pcs[1] << "/" << pcs[2]);

                
                const Amg::Vector3D pos = fromSimHitToLayer(reEle, atlasId) * hit.localPosition();
                const Amg::Vector3D gpos = reEle->transform(atlasId) * pos;
                
                ATH_MSG_VERBOSE(" evt: "<<ctx.eventID().event_number()
                                <<" hit  "<<m_idHelper->print_to_string(atlasId)
                                <<" local simHit "<<Amg::toString(hit.localPosition())
                                <<" corrected: "<<Amg::toString(pos)
                                <<" transform: "<<GeoTrf::toString(fromSimHitToLayer(reEle, atlasId))
                                <<" local strip: "<<Amg::toString(reEle->localToGlobalTransf(atlasId).inverse()*reEle->stripPos(atlasId))
                                <<" local strip (II): "<<Amg::toString(reEle->transform(atlasId).inverse()*reEle->stripPos(atlasId))
                                <<" global: "<<Amg::toString(gpos)
                                <<" strip Pos: "<<Amg::toString(reEle->stripPos(atlasId)));

                // Calculate propagation time along readout strip in seconds
                double proptime = PropagationTime(reEle, atlasId, gpos);

                double tns = G4Time + proptime + corrtimejitter;  // the time is in nanoseconds
                ATH_MSG_VERBOSE("TOF+propagation time  " << tns << " /s where proptime " << proptime << "/s");

                double time = tns + bunchTime;
                ATH_MSG_VERBOSE("final time in ns: BC+TOF+prop " << time << " /ns");

                // pack propagation time along strip, bunch time and local hit position
                long long int packedMCword = PackMCTruth(proptime, bunchTime, pos.y(), pos.z());
                //cppcheck-suppress invalidPointerCast
                double* b = reinterpret_cast<double*>(&packedMCword);

                //////////////////////////////////////////////////////////////////////////////////
                // create here deposit for MuonSimData
                // MuonMCData first  word is the packing of    : proptime, bunchTime, posy, posz
                // MuonMCData second word is the total hit time: bunchcTime+tof+proptime+correlatedJitter / ns
                MuonSimData::Deposit deposit(particleLink, MuonMCData((*b), time));  // store tof+strip_propagation+corr.jitter
                //                     MuonMCData((*b),G4Time+bunchTime+proptime          )); // store tof+strip_propagation

                // Do not store pile-up truth information
                if (m_includePileUpTruth || !HepMC::ignoreTruthLink(phit->particleLink(), m_vetoPileUpTruthLinks)) {
                  if (std::abs(hit.particleEncoding()) == 13 || hit.particleEncoding() == 0) {
                    if (channelSimDataMap.find(atlasId) == channelSimDataMap.end()) {
                      SimDataContent& content = channelSimDataMap[atlasId];
                      content.channelId = atlasId;
                      content.deposits.push_back(deposit);
                      content.gpos =  reEle->transform(atlasId)* 
                                      fromSimHitToLayer(reEle,atlasId) * gapCentre;
                      content.simTime = hitTime(phit);
                      ATH_MSG_VERBOSE("adding SDO entry: r " << content.gpos.perp() << " z " << content.gpos.z());
                    }
                  }
                }


                //---------------------------------------------------------------------
                // construct new digit and store it in the respective digit collection
                // --------------------------------------------------------------------

                // we create one digit-vector/deposit for each strip in the cluster
                bool isValid{false};
                for (int clus = pcs[1]; clus <= pcs[2]; ++clus) {
                    Identifier newId = m_idHelper->channelID(stationName, stationEta, stationPhi, doubletR, doubletZ,
                                                             doubletPhi, gasGap, imeasphi, clus, isValid);
                    if (!isValid) {
                        ATH_MSG_WARNING(__FILE__<<":"<<__LINE__<< "Channel "<< stationName<<" "<<stationEta<<" "<<stationPhi<<" "<< doubletR<<" "<<doubletZ
                                        <<" "<< doubletPhi<<" "<< gasGap <<" "<< imeasphi<<" "<< clus<<" is invalid");
                        continue;
                    }
                    
                    if (!m_idHelper->valid(newId)) {
                        if (stationName.find("BI") != std::string::npos) {
                            ATH_MSG_WARNING("Temporary skipping creation of RPC digit for stationName="
                                            << stationName << ", eta=" << stationEta << ", phi=" << stationPhi << ", doubletR=" << doubletR
                                            << ", doubletZ=" << doubletZ << ", doubletPhi=" << doubletPhi << ", gasGap=" << gasGap
                                            << ", measuresPhi=" << imeasphi << ", strip=" << clus << ", cf. ATLASRECTS-6124");
                            return StatusCode::SUCCESS;
                        } else {
                            ATH_MSG_ERROR("Created an invalid id, aborting!");
                            m_idHelper->print(newId);
                            return StatusCode::FAILURE;
                        }
                    }

                    ///////////////////////////////////////////////////////////////////
                    /////////////// TEMP, waiting for Reco to learn using clusters...
                    ///////////////////////////////////////////////////////////////////
                    // One identifier but several deposits // name m_sdo_tmp_map is wrong call it m_sdo_map
                    if (m_sdo_tmp_map.find(newId) == m_sdo_tmp_map.end()) {
                        std::vector<MuonSimData::Deposit> newdeps;
                        newdeps.push_back(deposit);
                        m_sdo_tmp_map.insert(std::map<Identifier, std::vector<MuonSimData::Deposit>>::value_type(newId, newdeps));
                    } else {
                        m_sdo_tmp_map[newId].push_back(deposit);
                    }
                }  // end for cluster
            }      // loop on eta and phi
        }          // end loop hits

        if (m_muonOnlySDOs) {
            for (auto it = channelSimDataMap.begin(); it != channelSimDataMap.end(); ++it) {
                MuonSimData simData(it->second.deposits, 0);
                simData.setPosition(it->second.gpos);
                simData.setTime(it->second.simTime);
                auto insertResult = sdoContainer->insert(std::make_pair(it->first, simData));
                if (!insertResult.second)
                    ATH_MSG_WARNING("Attention: this sdo is not recorded, since the identifier already exists in the sdoContainer map");
            }
        }

    }  // end loop detector elements

    ///////// TEMP for Reco not able to use clusterization

    std::map<Identifier, std::vector<MuonSimData::Deposit>>::iterator map_iter = m_sdo_tmp_map.begin();
    ATH_MSG_DEBUG("Start the digit map loop");

    for (; map_iter != m_sdo_tmp_map.end(); ++map_iter) {
        // Identifier
        const Identifier theId = (*map_iter).first;
        ATH_MSG_DEBUG("in the map loop: id " << m_idHelper->show_to_string(theId));
        // Deposit
        const std::vector<MuonSimData::Deposit> theDeps = (*map_iter).second;

        // store the SDO from the muon
        MuonSimData::Deposit theMuon;                       // useful beacuse it sorts the digits in ascending time.
        std::multimap<double, MuonSimData::Deposit> times;  // extract here time info from deposits.

        // loop on the vector deposit
        for (unsigned int k = 0; k < theDeps.size(); k++) {
            double time = theDeps[k].second.secondEntry();
            times.insert(std::multimap<double, MuonSimData::Deposit>::value_type(time, theDeps[k]));
        }

        // now iterate again over the multimap entries and store digits after dead time applied

        IdContext rpcContext = m_idHelper->module_context();  // work on chamber context

        std::multimap<double, MuonSimData::Deposit>::iterator map_dep_iter = times.begin();

        // loop to suppress digits too close in time (emulate Front-End and CMA dead time)
        double last_time = -10000;  // init to high value
        for (; map_dep_iter != times.end(); ++map_dep_iter) {
            double currTime = (*map_dep_iter).first;
            ATH_MSG_VERBOSE("deposit with time " << currTime);

            if (!m_muonOnlySDOs && !m_sdoAreOnlyDigits) {
                // store (before any cut: all G4 hits) in the SDO container
                // Identifier sdo and digit are the same
                if (sdoContainer->find(theId) != sdoContainer->end())  // Identifier exist -> increase deposit
                {
                    std::map<Identifier, MuonSimData>::const_iterator it = sdoContainer->find(theId);
                    std::vector<MuonSimData::Deposit> deps = ((*it).second).getdeposits();
                    deps.push_back((*map_dep_iter).second);
                } else  // Identifier does not exist -> create (Id,deposit)
                {
                    std::vector<MuonSimData::Deposit> deposits;
                    deposits.push_back((*map_dep_iter).second);
                    std::pair<std::map<Identifier, MuonSimData>::iterator, bool> insertResult =
                        sdoContainer->insert(std::make_pair(theId, MuonSimData(deposits, 0)));
                    if (!insertResult.second)
                        ATH_MSG_ERROR(
                            "Attention TEMP: this sdo is not recorded, since the identifier already exists in the sdoContainer map");
                }
            }
            // apply dead time
            if (std::abs(currTime - last_time) > (m_deadTime)) {
                ATH_MSG_DEBUG("deposit with time " << currTime << " is distant enough from previous (if any) hit on teh same strip");
                last_time = (*map_dep_iter).first;

                // first add time jitter to the time:
                double uncorrjitter = 0;
                double tmp_UncorrJitter = m_UncorrJitter;
                if (m_idHelper->stationName(theId) < 2) tmp_UncorrJitter = m_UncorrJitter_BIS78;
                if (tmp_UncorrJitter > 0.01) uncorrjitter = CLHEP::RandGaussZiggurat::shoot(rndmEngine, 0., tmp_UncorrJitter);
                // Historically patch for the cavern background
                // Now we subtract TOF from IP to assume full time calibrated detector (t=0 for particle from IP at light speed)
                // We add a time shift to emulate FE global offset

                const RpcReadoutElement* ele = detMgr->getRpcReadoutElement(theId);
                Amg::Vector3D posi = ele->stripPos(theId);
                double tp = m_patch_for_rpc_time ? posi.mag() / Gaudi::Units::c_light : 0.;
                // Calculate propagation time for a hit at the center of the strip, to be subtructed as well as the nominal TOF
                double propTimeFromStripCenter = PropagationTime(ele, theId, posi);
                double newDigit_time = currTime + uncorrjitter + m_rpc_time_shift - tp - propTimeFromStripCenter;
        
                double digi_ToT = -1.;  // Time over threshold, for Narrow-gap RPCs only
                if (m_idHelper->stationName(theId) < 2) digi_ToT = timeOverThreshold(rndmEngine);  //mn 

                ATH_MSG_VERBOSE("last_time=currTime " << last_time << " jitter " << uncorrjitter << " TOFcorrection " << tp << " shift "
                                                      << m_rpc_time_shift << "  newDigit_time " << newDigit_time);

                // Apply readout window (sensitive detector time window)
                bool outsideDigitizationWindow = outsideWindow(newDigit_time);
                if (outsideDigitizationWindow) {
                    ATH_MSG_VERBOSE("hit outside digitization window - do not produce digits");
                    ATH_MSG_DEBUG("Hit outside time window!!"
                                  << " hit time (ns) = " << newDigit_time << " timeWindow  = " << m_timeWindowLowerOffset << " / "
                                  << m_timeWindowUpperOffset);

                    continue;
                }
                // ok, let's store this digit
                // this is an accepted hit to become digit
                last_time = (*map_dep_iter).first;

                std::unique_ptr<RpcDigit> newDigit = std::make_unique<RpcDigit>(theId, newDigit_time, digi_ToT, false);  
                
                Identifier elemId = m_idHelper->elementID(theId);
                RpcDigitCollection* digitCollection = nullptr;

                IdentifierHash coll_hash;
                if (m_idHelper->get_hash(elemId, coll_hash, &rpcContext)) {
                    ATH_MSG_ERROR("Unable to get RPC hash id from RPC Digit collection "
                                  << "context begin_index = " << rpcContext.begin_index()
                                  << " context end_index  = " << rpcContext.end_index() << " the identifier is ");
                    elemId.show();
                }

                // make new digit
                ATH_MSG_DEBUG("Digit Id = " << m_idHelper->show_to_string(theId) << " digit time " << newDigit_time);

                // remember new collection.
                if (coll_hash >= collections.size()) {
                  collections.resize (coll_hash+1);
                }
                digitCollection = collections[coll_hash].get();
                if (!digitCollection) {
                    collections[coll_hash] = std::make_unique<RpcDigitCollection>(elemId, coll_hash);
                    digitCollection = collections[coll_hash].get();
                }
                digitCollection->push_back(std::move(newDigit));

                if (!m_muonOnlySDOs && m_sdoAreOnlyDigits) {
                    // put SDO collection in StoreGate
                    if (sdoContainer->find(theId) != sdoContainer->end()) {
                        std::map<Identifier, MuonSimData>::const_iterator it = sdoContainer->find(theId);
                        std::vector<MuonSimData::Deposit> deps = ((*it).second).getdeposits();
                        deps.push_back((*map_dep_iter).second);
                    } else {
                        std::vector<MuonSimData::Deposit> deposits;
                        deposits.push_back((*map_dep_iter).second);
                        std::pair<std::map<Identifier, MuonSimData>::iterator, bool> insertResult =
                            sdoContainer->insert(std::make_pair(theId, MuonSimData(deposits, 0)));
                        if (!insertResult.second)
                            ATH_MSG_ERROR(
                                "Attention: this sdo is not recorded, since teh identifier already exists in the sdoContainer map");
                    }
                }

            } else
                ATH_MSG_DEBUG("discarding digit due to dead time: " << (*map_dep_iter).first << " " << last_time);
        }

    }  // loop to suppress digits too close in time ended

    // reset the pointer if it not null
    m_thpcRPC.reset();
    if (!m_simHitValidKey.empty()) {
        SG::WriteHandle<RPCSimHitCollection> validHandle{m_simHitValidKey, ctx};
        ATH_CHECK(validHandle.record(std::move(inputSimHitColl)));
    }

    return StatusCode::SUCCESS;
}
Amg::Transform3D RpcDigitizationTool::fromSimHitToLayer(const MuonGM::RpcReadoutElement* reEle,
                                                        const Identifier& layerId) const {
    
    Amg::Vector3D lGasGapPos = reEle->localGasGapPos(layerId);
    if (reEle->NphiStripPanels() != reEle->nGasGapPerLay()) {
        lGasGapPos.y() =0.;
    }
    
    /// Yep. The second gas gap is upside down. But only for the rotated modules.
    /// If you're asking yourself why that's the case, my fellow reader I've not even the 
    /// glimpse of a clue about this beauty
    const bool flip = reEle->numberOfLayers() == 2 &&  
                      (m_idHelper->gasGap(layerId) == 2) != reEle->rotatedRpcModule();
    const Amg::Transform3D fromHitToGap{reEle->transform(layerId).inverse() *
                                        reEle->absTransform() * Amg::getTranslate3D(lGasGapPos) *
                                        (flip ? Amg::getRotateY3D(180.*Gaudi::Units::deg) : Amg::Transform3D::Identity())};
    ATH_MSG_VERBOSE("Transformation to go from hit to gap restframe "<<m_idHelper->print_to_string(layerId)
                <<" "<<Amg::toString(fromHitToGap));
    return fromHitToGap;
}

//--------------------------------------------
std::array<int, 3> RpcDigitizationTool::physicalClusterSize(const EventContext& ctx,
                                                            const RpcReadoutElement* ele, 
                                                            const Identifier& id, 
                                                            const Amg::Vector3D& gapCentre,
                                                            CLHEP::HepRandomEngine* rndmEngine) const {
    
 
    std::array<int, 3> result{};

    const Amg::Vector3D position = fromSimHitToLayer(ele, id) * gapCentre;
    // protect against numerical errors
    if (std::abs(position.x()) > 1e12 || std::abs(position.y()) > 1e12) {
        ATH_MSG_WARNING("RpcDigitizationTool::physicalClusterSize: position is too large, x=" << position.x() << " y=" << position.y() << " gapCentre=" << Amg::toString(gapCentre));
        return make_array<int, 3>(-1);
    }

    const int doubletPhi = m_idHelper->doubletPhi(id);
    const int gasGap = m_idHelper->gasGap(id);
    const bool measuresPhi = m_idHelper->measuresPhi(id);
    const double pitch= ele->StripPitch(measuresPhi);


    const int nstrip = ele->stripNumber(position.block<2,1>(0,0), id);
    const int numStrips = ele->Nstrips(measuresPhi);

    result[1] = nstrip;
    result[2] = nstrip;

    if (nstrip < 1 || nstrip > numStrips) {
        return make_array<int, 3>(-1);
    }    
    const Amg::Vector3D locStripPos = ele->transform(id).inverse()*ele->stripPos(doubletPhi, gasGap, measuresPhi, nstrip); 
    float xstripnorm = (locStripPos -position).x() / pitch ;
    result[0] = determineClusterSize(ctx, id, xstripnorm, rndmEngine);

    //
    

    if (m_turnON_clustersize == false) result[0] = 1;

    return result;
}

//--------------------------------------------
std::array<int, 3> RpcDigitizationTool::TurnOnStrips(const RpcReadoutElement* ele,
                                                     std::array<int, 3>&& pcs, 
                                                     const Identifier& id) const {


    const int nstrips = ele->Nstrips(m_idHelper->measuresPhi(id));

    if (pcs[0] == -2) {
        pcs[1] = pcs[2] - 1;
    } else if (pcs[0] == 2) {
        pcs[2] = pcs[1] + 1;
    } else if (pcs[0] > 2) {
        pcs[1] = pcs[1] - pcs[0] / 2;
        if (fmod(pcs[0], 2) == 0) pcs[1] = pcs[1] + 1;
        pcs[2] = pcs[1] + pcs[0] - 1;
    } else if (pcs[0] < -2) {
        pcs[1] = pcs[1] + pcs[0] / 2;
        pcs[2] = pcs[1] - pcs[0] - 1;
    }

    // cut the clusters at the beginning and at the end of the chamber

    pcs[1] = std::clamp(pcs[1], 1, nstrips);
    pcs[2] = std::clamp(pcs[2], 1, nstrips);

    pcs[0] = pcs[2] - pcs[1] + 1;

    return pcs;
}

//--------------------------------------------
double RpcDigitizationTool::PropagationTime(const MuonGM::RpcReadoutElement* ele, 
                                            const Identifier& id, 
                                            const Amg::Vector3D& globPos) const {

    double distance{0.};
    if (m_idHelper->measuresPhi(id)) {
        distance = ele->distanceToPhiReadout(globPos);
    } else {
        distance = ele->distanceToEtaReadout(globPos);
    }

    // distance in mm, SIG_VEL in ns/m
    return std::abs(distance * SIG_VEL * 1.e-3);
}

//--------------------------------------------
long long int RpcDigitizationTool::PackMCTruth(float proptime, float bctime, float posy, float posz) const {
    // start with proptime: it is usually ~ns. It comes in ns. We express it in ns/10. use only 8 bits
    if (proptime < 0) {
        ATH_MSG_WARNING("A poblem: packing a propagation time <0 " << proptime << " redefine it as 0");
        proptime = 0.;
    }
    long long int new_proptime = int(proptime * 10) & 0xff;

    // now tof. it is ~100ns. comes in ns. express it in ns/10. 16 bits needed (0-32768)
    // now BC time: it is ~100ns. comes in ns. express it in ns/10. 16 bits needed (0-32768)
    // can be negative (=> add 300 ns)

    long long int new_bctime = int((bctime + 300.) * 10.) & 0xffff;

    // posy: ~1000mm comes in mm, write it in mm*10. need 16 bits (0-32768)
    // can be negative (=>add 1500 mm)

    long long int new_posy = int((posy + 1500.) * 10.) & 0xffff;

    // posz: ~1000mm comes in mm, write it in mm*10. need 16 bits (0-32768)
    // can be negative (=>add 1500 mm)

    long long int new_posz = int((posz + 1500.) * 10.) & 0xffff;

    return (new_proptime + (new_bctime << 8) + (new_posy << 24) + (new_posz << 40));
}

//--------------------------------------------
void RpcDigitizationTool::UnPackMCTruth(double theWord, float& proptime, float& bctime, float& posy, float& posz) {
    // int64_t is just a shorter way of writing long long int
    using Repacker = union

    {
        double dWord;

        int64_t iWord;
    };
    Repacker MCTruth;
    MCTruth.dWord = theWord;
    proptime = ((MCTruth.iWord) & 0x00000000000000ffLL) / 10.;
    bctime = (((MCTruth.iWord) & 0x0000000000ffff00LL) >> 8) / 10.;
    posy = (((MCTruth.iWord) & 0x000000ffff000000LL) >> 24) / 10.;
    posz = (((MCTruth.iWord) & 0x00ffff0000000000LL) >> 40) / 10.;

    //
    bctime = bctime - 300.;
    posy = posy - 1500.;
    posz = posz - 1500.;
}

//--------------------------------------------
StatusCode RpcDigitizationTool::fillTagInfo() {
    // get TagInfoMgr
    SmartIF<ITagInfoMgr> tagInfoMgr{Gaudi::svcLocator()->service("TagInfoMgr")};  // Tag Info Manager
    if (!tagInfoMgr) { return StatusCode::FAILURE; }

    std::string RpctimeSchema = "";
    std::stringstream RpctimeShift;
    RpctimeShift << (int)m_rpc_time_shift;

    if (m_patch_for_rpc_time) {
        RpctimeSchema = "Datalike_TOFoff_TimeShift" + RpctimeShift.str() + "nsec";
    } else {
        RpctimeSchema = "G4like_TOFon_TimeShift" + RpctimeShift.str() + "nsec";
    }

    StatusCode sc = tagInfoMgr->addTag(m_RPC_TimeSchema, RpctimeSchema);

    if (sc.isFailure()) {
        ATH_MSG_WARNING(m_RPC_TimeSchema << " " << RpctimeSchema << " not added to TagInfo ");
        return sc;
    }

    ATH_MSG_DEBUG(m_RPC_TimeSchema << " " << RpctimeSchema << " added to TagInfo ");

    return StatusCode::SUCCESS;
}


//--------------------------------------------
std::pair<bool,bool> RpcDigitizationTool::detectionEfficiency(const EventContext& ctx, 
                                                              const Identifier& IdEta,
                                                              const Identifier& IdPhi, 
                                                              CLHEP::HepRandomEngine* rndmEngine, 
                                                              const HepMcParticleLink& trkParticle) const {
    
    

    ATH_MSG_DEBUG("RpcDigitizationTool::in DetectionEfficiency");

    ATH_MSG_DEBUG("EtaPanelId to look for Eff is " << m_idHelper->show_to_string(IdEta));
    ATH_MSG_DEBUG("PhiPanelId to look for Eff is " << m_idHelper->show_to_string(IdPhi));


    // dead spacers are not simulated in GEANT4  => their effect must be emulated in the digitizer as an effective max. efficiency = 99%
    // (spacers are 1x1cm^2 over a grid of 10x10cm^2 =? geometrical ineff. introduced is 1% for normal incidence)
    float maxGeomEff{0.99}, PhiAndEtaEff{0.99}, OnlyEtaEff{0.f}, OnlyPhiEff{0.f};

    // 2=BML,3=BMS,4=BOL,5=BOS,8=BMF,9=BOF,10=BOG
    int stationName = m_idHelper->stationName(IdEta);
    int stationEta = m_idHelper->stationEta(IdEta);
    int doubletR = m_idHelper->doubletR(IdEta);

    // remove feet extension. driven by joboption
    if (m_BOG_BOF_DoubletR2_OFF && (stationName == m_BOF_id || stationName == m_BOG_id) && doubletR == 2) {
        return std::make_pair(false, false);
    }


    if (!m_turnON_efficiency) {
        return std::make_pair(true, true);
    }
    bool etaStripOn{true}, phiStripOn{true};

    // int stripetadead = 0 ; // not used
    // int stripphidead = 0 ; // not used

    unsigned int index = stationName - 2;
    // BML and BMS, BOL and BOS  come first (stationName= 2 and 3, 4 and 5 -> index 0-3)
    if (stationName > 5 && stationName < 50) index = index - 2;
    // BMF, BOF and BOG are 8,9,10 => must be 4,5 and 6
    else if (stationName > 50)
        index = index - 44;
    // BME and BOE 53 and 54 are at indices 7 and 8

    if (!m_Efficiency_fromCOOL && stationName >= 2) {
        if (index > m_PhiAndEtaEff_A.size() || index > m_OnlyEtaEff_A.size() || index > m_OnlyPhiEff_A.size()) {
            THROW_EXCEPTION("Index out of array in Detection Efficiency SideA " << index << " stationName = " << stationName);
        }

        PhiAndEtaEff = m_PhiAndEtaEff_A[index];
        OnlyEtaEff = m_OnlyEtaEff_A[index];
        OnlyPhiEff = m_OnlyPhiEff_A[index];

        if (stationEta < 0) {
            if (index > m_PhiAndEtaEff_C.size() || index > m_OnlyEtaEff_C.size() || index > m_OnlyPhiEff_C.size()) {
                THROW_EXCEPTION("Index out of array in Detection Efficiency SideC " << index << " stationName = " << stationName);
            }
            PhiAndEtaEff = m_PhiAndEtaEff_C[index];
            OnlyEtaEff = m_OnlyEtaEff_C[index];
            OnlyPhiEff = m_OnlyPhiEff_C[index];
        }
    } else if (stationName < 2 && (!m_Efficiency_fromCOOL || !m_Efficiency_BIS78_fromCOOL)) {  // BIS
        PhiAndEtaEff = m_PhiAndEtaEff_BIS78;
        OnlyEtaEff = m_OnlyEtaEff_BIS78;
        OnlyPhiEff = m_OnlyPhiEff_BIS78;
    } else {  // Efficiency from Cool

        const RpcCondDbData* readCdo{nullptr};                        
        if(!retrieveCondData(ctx, m_readKey, readCdo).isSuccess()){
            THROW_EXCEPTION("Failed to retrieve conditions object");
        }

        ATH_MSG_DEBUG("Efficiencies and cluster size + dead strips will be extracted from COOL");

        double FracDeadStripEta{0.}, FracDeadStripPhi{0.};
        double EtaPanelEfficiency{1.}, PhiPanelEfficiency{1.}, GapEfficiency{1.};
        int RPC_ProjectedTracksEta = 0;
        
        std::optional<double> fracDeadStripEtaFromCOOL = readCdo->getFracDeadStrip(IdEta);
        std::optional<double> fracDeadStripPhiFromCOOL = readCdo->getFracDeadStrip(IdPhi);
        
        bool noEntryInDb = !fracDeadStripEtaFromCOOL || !fracDeadStripPhiFromCOOL;

        FracDeadStripEta = fracDeadStripEtaFromCOOL.value_or(0.);
        FracDeadStripPhi = fracDeadStripPhiFromCOOL.value_or(0.);
        RPC_ProjectedTracksEta = readCdo->getProjectedTrack(IdEta).value_or(0);
        
        EtaPanelEfficiency = readCdo->getEfficiency(IdEta).value_or(1.);
        PhiPanelEfficiency = readCdo->getEfficiency(IdPhi).value_or(1.);
        GapEfficiency = readCdo->getGapEfficiency(IdEta).value_or(1.);

        if (std::abs(FracDeadStripEta - 1.) < 0.001) {
            ATH_MSG_DEBUG("Watch out: SPECIAL CASE: Read from Cool: FracDeadStripEta/Phi "
                          << FracDeadStripEta << "/" << FracDeadStripPhi << " RPC_ProjectedTracksEta " << RPC_ProjectedTracksEta
                          << " Eta/PhiPanelEfficiency " << EtaPanelEfficiency << "/" << PhiPanelEfficiency << " gapEff " << GapEfficiency
                          << " for gas gap " << m_idHelper->show_to_string(IdEta) << " id " << IdEta.get_identifier32().get_compact());
            // dead eta panel => cannot determine the strip status for phi strips
            // FracDeadStripPhi must be reset to 0. and undefinedPhiStripStatus = true
            FracDeadStripPhi = 0.;
            ATH_MSG_VERBOSE("Watch out: SPECIAL CASE: Resetting FracDeadStripPhi " << FracDeadStripPhi << " ignoring phi dead strips ");
        }

        // special test
        // here redefining the efficiencies:
        // EtaPanelEfficiency = 0.92;
        // PhiPanelEfficiency = 0.85;
        // GapEfficiency      = 0.97;
        bool changing = false;
        ATH_MSG_DEBUG("Read from Cool: FracDeadStripEta/Phi " << FracDeadStripEta << "/" << FracDeadStripPhi << " RPC_ProjectedTracksEta "
                                                              << RPC_ProjectedTracksEta << " Eta/PhiPanelEfficiency " << EtaPanelEfficiency
                                                              << "/" << PhiPanelEfficiency << " gapEff " << GapEfficiency);
        // if ((1.-FracDeadStripEta)<EtaPanelEfficiency)
        if ((maxGeomEff - FracDeadStripEta) - EtaPanelEfficiency < -0.011) {
            ATH_MSG_DEBUG("Ineff. from dead strips on Eta Panel larger that measured efficiency: deadFrac="
                          << FracDeadStripEta << " Panel Eff=" << EtaPanelEfficiency << " for Panel " << m_idHelper->show_to_string(IdEta));
            ATH_MSG_DEBUG("... see the corresponding report from RpcDetectorStatusDbTool");
            // EtaPanelEfficiency = 1.-FracDeadStripEta;
            EtaPanelEfficiency = maxGeomEff - FracDeadStripEta;
            changing = true;
        }
        // if ((1.-FracDeadStripPhi)<PhiPanelEfficiency)
        if ((maxGeomEff - FracDeadStripPhi) - PhiPanelEfficiency < -0.011) {
            ATH_MSG_DEBUG("Ineff. from dead strips on Phi Panel larger that measured efficiency: deadFrac="
                          << FracDeadStripPhi << " Panel Eff=" << PhiPanelEfficiency << " for Panel " << m_idHelper->show_to_string(IdPhi));
            ATH_MSG_DEBUG("... see the corresponding report among the warnings of RpcDetectorStatusDbTool");
            // PhiPanelEfficiency = 1.-FracDeadStripPhi;
            PhiPanelEfficiency = maxGeomEff - FracDeadStripPhi;
            changing = true;
        }
        // if ((1.-FracDeadStripEta*FracDeadStripPhi)<GapEfficiency)
        if ((maxGeomEff - FracDeadStripEta * FracDeadStripPhi) - GapEfficiency < -0.011) {
            ATH_MSG_DEBUG("Ineff. from dead strips on Eta/Phi Panels larger that measured EtaORPhi efficiency: deadFrac="
                          << FracDeadStripEta * FracDeadStripPhi << " EtaORPhi Eff=" << GapEfficiency << " for GasGap "
                          << m_idHelper->show_to_string(IdEta));
            ATH_MSG_DEBUG("... see the corresponding report among the warnings of RpcDetectorStatusDbTool");
            // GapEfficiency = 1.-FracDeadStripEta*FracDeadStripPhi;
            GapEfficiency = maxGeomEff - FracDeadStripEta * FracDeadStripPhi;
            changing = true;
        }
        if (changing)
            ATH_MSG_DEBUG("Rinormalized Values from Cool: FracDeadStripEta/Phi "
                          << FracDeadStripEta << "/" << FracDeadStripPhi << " RPC_ProjectedTracksEta " << RPC_ProjectedTracksEta
                          << " Eta/PhiPanelEfficiency " << EtaPanelEfficiency << "/" << PhiPanelEfficiency << " gapEff " << GapEfficiency);

        // gabriele //..stefania - if there are dead strips renormalize the eff. to the active area
        if (m_kill_deadstrips) {
            if ((FracDeadStripEta > 0.0 && FracDeadStripEta < 1.0) || (FracDeadStripPhi > 0.0 && FracDeadStripPhi < 1.0) || (noEntryInDb)) {
                EtaPanelEfficiency = EtaPanelEfficiency / (maxGeomEff - FracDeadStripEta);
                PhiPanelEfficiency = PhiPanelEfficiency / (maxGeomEff - FracDeadStripPhi);
                GapEfficiency = GapEfficiency / (maxGeomEff - FracDeadStripEta * FracDeadStripPhi);

                if (EtaPanelEfficiency > maxGeomEff) EtaPanelEfficiency = maxGeomEff;
                if (PhiPanelEfficiency > maxGeomEff) PhiPanelEfficiency = maxGeomEff;
                if (GapEfficiency > maxGeomEff) GapEfficiency = maxGeomEff;

                if (EtaPanelEfficiency > GapEfficiency) GapEfficiency = EtaPanelEfficiency;
                if (PhiPanelEfficiency > GapEfficiency) GapEfficiency = PhiPanelEfficiency;
                ATH_MSG_DEBUG("Eff Redefined (to correct for deadfrac): FracDeadStripEta/Phi "
                              << " Eta/PhiPanelEfficiency " << EtaPanelEfficiency << "/" << PhiPanelEfficiency << " gapEff "
                              << GapEfficiency);
            }
        }

        // values from COOLDB (eventually overwritten later)
        PhiAndEtaEff = float(EtaPanelEfficiency + PhiPanelEfficiency - GapEfficiency);
        if (PhiAndEtaEff < 0.) PhiAndEtaEff = 0.;
        OnlyEtaEff = float(EtaPanelEfficiency - PhiAndEtaEff);
        if (OnlyEtaEff < 0.) OnlyEtaEff = 0.;
        OnlyPhiEff = float(PhiPanelEfficiency - PhiAndEtaEff);
        if (OnlyPhiEff < 0.) OnlyPhiEff = 0.;

        //  special patch to be true only when m_Efficiency_fromCOOL=true and /RPC/DQMF/ELEMENT_STATUS tag is
        //  RPCDQMFElementStatus_2012_Jaunuary_26
        bool applySpecialPatch = false;
        if (m_EfficiencyPatchForBMShighEta && m_Efficiency_fromCOOL) {
            if (m_idHelper->stationName(IdEta) == 3)  ///// BMS
            {
                if (std::abs(m_idHelper->stationEta(IdEta)) == 6 && m_idHelper->doubletR(IdEta) == 1 &&
                    m_idHelper->doubletZ(IdEta) == 2 && m_idHelper->doubletPhi(IdEta) == 1) {
                    applySpecialPatch = true;
                    ATH_MSG_WARNING(
                        "Applying special patch for BMS at |eta|=6 lowPt plane -dbbZ=2 and dbPhi=1 ... will use default eff. for Id "
                        << m_idHelper->show_to_string(IdEta));
                    ATH_MSG_WARNING(
                        "Applying special patch: THIS HAS TO BE DONE IF /RPC/DQMF/ELEMENT_STATUS tag is "
                        "RPCDQMFElementStatus_2012_Jaunuary_2");
                }
            }
        }

        // if projected tracks number too low or inconsistent values get efficiencies from joboption and overwrite previous values
        if (applySpecialPatch || RPC_ProjectedTracksEta < m_CutProjectedTracks || RPC_ProjectedTracksEta > 10000000 ||
            EtaPanelEfficiency > 1 || EtaPanelEfficiency < 0 || PhiPanelEfficiency > 1 || PhiPanelEfficiency < 0 || GapEfficiency > 1 ||
            GapEfficiency < 0) {
            if (index > m_PhiAndEtaEff_A.size() || index > m_OnlyEtaEff_A.size() || index > m_OnlyPhiEff_A.size()) {
                THROW_EXCEPTION("Index out of array in Detection Efficiency SideA COOLDB" << index << " stationName = " << stationName);
            }
            if (RPC_ProjectedTracksEta < m_CutProjectedTracks)
                ATH_MSG_DEBUG("# of proj tracks = " << RPC_ProjectedTracksEta << " < cut = " << m_CutProjectedTracks
                                                    << " resetting eff. from cool with default(python) values ");

            PhiAndEtaEff = m_PhiAndEtaEff_A[index];
            OnlyEtaEff = m_OnlyEtaEff_A[index];
            OnlyPhiEff = m_OnlyPhiEff_A[index];

            if (stationEta < 0) {
                if (index > m_PhiAndEtaEff_C.size() || index > m_OnlyEtaEff_C.size() || index > m_OnlyPhiEff_C.size()) {
                    THROW_EXCEPTION("Index out of array in Detection Efficiency SideC COOLDB" << index << " stationName = " << stationName);
                }
                PhiAndEtaEff = m_PhiAndEtaEff_C[index];
                OnlyEtaEff = m_OnlyEtaEff_C[index];
                OnlyPhiEff = m_OnlyPhiEff_C[index];
            }

            // if (m_applyEffThreshold) {
            // gabriele Set efficiency from dead strip fraction instead of nominal value
            float effgap = PhiAndEtaEff + OnlyEtaEff + OnlyPhiEff;
            float s_EtaPanelEfficiency = 1. - FracDeadStripEta;
            float s_PhiPanelEfficiency = 1. - FracDeadStripPhi;
            float s_PhiAndEtaEff = s_EtaPanelEfficiency * s_PhiPanelEfficiency / effgap;
            if (s_PhiAndEtaEff < PhiAndEtaEff) PhiAndEtaEff = s_PhiAndEtaEff;
            float s_OnlyEtaEff = s_EtaPanelEfficiency - PhiAndEtaEff;
            float s_OnlyPhiEff = s_PhiPanelEfficiency - PhiAndEtaEff;

            if (s_OnlyEtaEff < OnlyEtaEff) OnlyEtaEff = s_OnlyEtaEff;
            if (s_OnlyPhiEff < OnlyPhiEff) OnlyPhiEff = s_OnlyPhiEff;
            //      }
        }

        float VolEff = PhiAndEtaEff + OnlyEtaEff + OnlyPhiEff;
        if (VolEff > maxGeomEff) {
            PhiAndEtaEff = (PhiAndEtaEff / VolEff) * maxGeomEff;
            OnlyEtaEff = (OnlyEtaEff / VolEff) * maxGeomEff;
            OnlyPhiEff = (OnlyPhiEff / VolEff) * maxGeomEff;
        }

    }  // End eff from COOL

    // Efficiency correction factor for fractional-charged particles(added by Quanyin Li: quli@cern.ch)
    // link to truth particles and calculate the charge and betagamma
    HepMC::ConstGenParticlePtr genparticle = trkParticle.cptr();
    if (genparticle) {
        // only apply efficiency correction to fractional-charged particles based on pdgId betagamma
        if (MC::isGenericMultichargedParticle(genparticle)) {
            const double eff_sf = FCPEfficiency(genparticle);
            // Apply scale factor to the 3 Eff.
            PhiAndEtaEff = PhiAndEtaEff * eff_sf;
            OnlyEtaEff = OnlyEtaEff * eff_sf;
            OnlyPhiEff = OnlyPhiEff * eff_sf;
        }
    }

    float I0 = PhiAndEtaEff;
    float I1 = PhiAndEtaEff + OnlyEtaEff;
    float ITot = PhiAndEtaEff + OnlyEtaEff + OnlyPhiEff;

    float GapEff = ITot ;
    float PhiEff = PhiAndEtaEff + OnlyPhiEff;
    float EtaEff = PhiAndEtaEff + OnlyEtaEff;

    ATH_MSG_DEBUG("DetectionEfficiency: Final Efficiency Values applied for "
                  << m_idHelper->show_to_string(IdEta) << " are " << PhiAndEtaEff << "=PhiAndEtaEff " << OnlyEtaEff
                  << "=OnlyEtaEff " << OnlyPhiEff << "=OnlyPhiEff " << GapEff << "=GapEff " << EtaEff << "=EtaEff " << PhiEff
                  << "=PhiEff ");

    float rndmEff = CLHEP::RandFlat::shoot(rndmEngine, 1);

    if (rndmEff < I0) {
        phiStripOn = true;
        etaStripOn = true;
    } else if ((I0 <= rndmEff) && (rndmEff < I1)) {
        phiStripOn = false;
        etaStripOn = true;
    } else if ((I1 <= rndmEff) && (rndmEff <= ITot)) {
        phiStripOn = true;
        etaStripOn = false;
    } else {
        phiStripOn = false;
        etaStripOn = false;
    }

    return std::make_pair(etaStripOn, phiStripOn);
}

//--------------------------------------------
int RpcDigitizationTool::determineClusterSize(const EventContext& ctx, 
                                              const Identifier& idRpcStrip, 
                                              double xstripnorm,
                                              CLHEP::HepRandomEngine* rndmEngine) const {
    ATH_MSG_DEBUG("RpcDigitizationTool::in determineClusterSize");

    ATH_MSG_DEBUG("Digit Id = " << m_idHelper->show_to_string(idRpcStrip));

    int ClusterSize = 1;

    double FracClusterSize1{1.}, FracClusterSize2{0.}, MeanClusterSize{1.}, 
          FracClusterSizeTail{0.}, MeanClusterSizeTail{1.},
          FracClusterSize2norm{0.};

    // 2=BML,3=BMS,4=BOL,5=BOS,8=BMF,9=BOF,10=BOG
    int stationName = m_idHelper->stationName(idRpcStrip);
    int stationEta = m_idHelper->stationEta(idRpcStrip);
    int measuresPhi = m_idHelper->measuresPhi(idRpcStrip);

    unsigned int index = stationName - 2;
    // BML and BMS, BOL and BOS  come first (stationName= 2 and 3, 4 and 5 -> index 0-3)
    if (stationName > 5 && stationName < 50) index = index - 2;
    // BMF, BOF and BOG are 8,9,10 => must be 4,5 and 6
    else if (stationName > 50)
        index = index - 44;
    // BME and BOE 53 and 54 are at indices 7 and 8

    if (!m_ClusterSize_fromCOOL && stationName >= 2) {
        index += m_FracClusterSize1_A.size() / 2 * measuresPhi;
        if (index >= m_FracClusterSize1_A.size() || 
            index >= m_FracClusterSize2_A.size() || 
            index >= m_FracClusterSizeTail_A.size() ||
            index >= m_MeanClusterSizeTail_A.size()) {
            ATH_MSG_ERROR("Index out of array in determineClusterSize SideA " << index << " statName " << stationName);
            return 1;
        }
        FracClusterSize1 = m_FracClusterSize1_A[index];
        FracClusterSize2 = m_FracClusterSize2_A[index];
        FracClusterSizeTail = m_FracClusterSizeTail_A[index];
        MeanClusterSizeTail = m_MeanClusterSizeTail_A[index];

        if (stationEta < 0) {
            index += m_FracClusterSize1_C.size() / 2 * measuresPhi - m_FracClusterSize1_A.size() / 2 * measuresPhi;
            if (index >= m_FracClusterSize1_C.size() || 
                index >= m_FracClusterSize2_C.size() || 
                index >= m_FracClusterSizeTail_C.size() ||
                index >= m_MeanClusterSizeTail_C.size()) {
                ATH_MSG_ERROR("Index out of array in determineClusterSize SideC " << index << " statName " << stationName);
                return 1;
            }
            FracClusterSize1 = m_FracClusterSize1_C[index];
            FracClusterSize2 = m_FracClusterSize2_C[index];
            FracClusterSizeTail = m_FracClusterSizeTail_C[index];
            MeanClusterSizeTail = m_MeanClusterSizeTail_C[index];
        }
    } else if (stationName < 2 && (!m_ClusterSize_fromCOOL || !m_ClusterSize_BIS78_fromCOOL)) {  // BIS78
        FracClusterSize1 = m_FracClusterSize1_BIS78;
        FracClusterSize2 = m_FracClusterSize2_BIS78;
        FracClusterSizeTail = m_FracClusterSizeTail_BIS78;
        MeanClusterSizeTail = m_MeanClusterSizeTail_BIS78;
    } else {  // Cluster size from COOL
        const RpcCondDbData* readCdo{nullptr};                        
        retrieveCondData(ctx, m_readKey, readCdo).ignore();

        Identifier Id = m_idHelper->panelID(idRpcStrip);

        int RPC_ProjectedTracks = readCdo->getProjectedTrack(Id).value_or(0);
        FracClusterSize1 = readCdo->getFracClusterSize1(Id).value_or(1.);
        FracClusterSize2 = readCdo->getFracClusterSize2(Id).value_or(0.);
        MeanClusterSize = readCdo->getMeanClusterSize(Id).value_or(1.);

      
        ATH_MSG_DEBUG("FracClusterSize1 and 2 " << FracClusterSize1 << " " << FracClusterSize2);

        FracClusterSizeTail = 1. - FracClusterSize1 - FracClusterSize2;

        MeanClusterSizeTail = MeanClusterSize - FracClusterSize1 - 2 * FracClusterSize2;

        ATH_MSG_DEBUG("MeanClusterSizeTail and FracClusterSizeTail " << MeanClusterSizeTail << " " << FracClusterSizeTail);

        // if clustersize have anomalous values set to the average cluster size from joboption
        if (RPC_ProjectedTracks < m_CutProjectedTracks || RPC_ProjectedTracks > 10000000 || MeanClusterSize > m_CutMaxClusterSize ||
            MeanClusterSize <= 1 || FracClusterSizeTail < 0 || FracClusterSize1 < 0 || FracClusterSize2 < 0 || FracClusterSizeTail > 1 ||
            FracClusterSize1 > 1 || FracClusterSize2 > 1) {
            if (stationName >= 2) {
                index += m_FracClusterSize1_A.size() / 2 * measuresPhi;
                if (index >= m_FracClusterSize1_A.size() || 
                    index >= m_FracClusterSize2_A.size() || 
                    index >= m_FracClusterSizeTail_A.size() ||
                    index >= m_MeanClusterSizeTail_A.size()) {
                    ATH_MSG_ERROR("Index out of array in determineClusterSize SideA " << index << " statName " << stationName);
                    return 1;
                }
                FracClusterSize1 = m_FracClusterSize1_A[index];
                FracClusterSize2 = m_FracClusterSize2_A[index];
                FracClusterSizeTail = m_FracClusterSizeTail_A[index];
                MeanClusterSizeTail = m_MeanClusterSizeTail_A[index];

                if (stationEta < 0) {
                    index += m_FracClusterSize1_C.size() / 2 * measuresPhi - m_FracClusterSize1_A.size() / 2 * measuresPhi;
                    if (index > m_FracClusterSize1_C.size() || index > m_FracClusterSize2_C.size() ||
                        index > m_FracClusterSizeTail_C.size() || index > m_MeanClusterSizeTail_C.size()) {
                        ATH_MSG_ERROR("Index out of array in determineClusterSize SideC " << index << " statName " << stationName);
                        return 1;
                    }

                    FracClusterSize1 = m_FracClusterSize1_C[index];
                    FracClusterSize2 = m_FracClusterSize2_C[index];
                    FracClusterSizeTail = m_FracClusterSizeTail_C[index];
                    MeanClusterSizeTail = m_MeanClusterSizeTail_C[index];
                }
            } else {
                FracClusterSize1 = m_FracClusterSize1_BIS78;
                FracClusterSize2 = m_FracClusterSize2_BIS78;
                FracClusterSizeTail = m_FracClusterSizeTail_BIS78;
                MeanClusterSizeTail = m_MeanClusterSizeTail_BIS78;
            }
        }
    }
    FracClusterSize1 = std::min(FracClusterSize1, 1.);
    FracClusterSize2 = std::min(FracClusterSize2, 1.);
    FracClusterSizeTail = std::min(FracClusterSizeTail, 1.);
    float FracTot = FracClusterSize1 + FracClusterSize2 + FracClusterSizeTail;
    if (FracTot != 1. && FracTot > 0) {
        FracClusterSize1 = FracClusterSize1 / FracTot;
        FracClusterSize2 = FracClusterSize2 / FracTot;
        FracClusterSizeTail = FracClusterSizeTail / FracTot;
    }
    if (MeanClusterSizeTail < 0 || MeanClusterSizeTail > 10) MeanClusterSizeTail = 1;

    ATH_MSG_VERBOSE("ClusterSize Final " << FracClusterSize1 << " FracClusterSize1 " << FracClusterSize2 << " FracClusterSize2  "
                                         << FracClusterSizeTail << "   " << FracClusterSizeTail << " MeanClusterSizeTail  "
                                         << MeanClusterSizeTail);

    float FracClusterSize1plus2 = FracClusterSize1 + FracClusterSize2;
    float ITot = FracClusterSize1 + FracClusterSize2 + FracClusterSizeTail;

    if (FracClusterSize1plus2 != 0) {
        // FracClusterSize1norm = FracClusterSize1 / FracClusterSize1plus2 ; // not used
        FracClusterSize2norm = FracClusterSize2 / FracClusterSize1plus2;
    }

    float rndmCS = CLHEP::RandFlat::shoot(rndmEngine, ITot);

    if (stationName >= 2) {  // Legacy RPCs
        // Expanded CS2 of 1.3 to match average CS1 and CS2 (to be investigate)
        if (rndmCS < FracClusterSize1plus2) {
            // deterministic assignment of CS 1 or 2
            if (xstripnorm <= FracClusterSize2norm / 2. * 1.3) {
                ClusterSize = -2;
            } else if ((1.0 - FracClusterSize2norm / 2. * 1.3) <= xstripnorm) {
                ClusterSize = 2;
            } else {
                ClusterSize = 1;
            }
            if (m_ClusterSize1_2uncorr) {
                float rndmCS1_2 = CLHEP::RandFlat::shoot(rndmEngine, 1);
                ClusterSize = 1 + (rndmCS1_2 < FracClusterSize2norm);
            }

        } else if ((FracClusterSize1plus2 <= rndmCS) && (rndmCS <= ITot)) {
            ClusterSize = m_FirstClusterSizeInTail;
            ClusterSize += int(CLHEP::RandExponential::shoot(rndmEngine, MeanClusterSizeTail));
            float rndmLR = CLHEP::RandFlat::shoot(rndmEngine, 1.0);
            if (rndmLR > 0.5) ClusterSize = -ClusterSize;
        } else {
            ClusterSize = 1;
        }

    } else {  // NRPCs
        if (rndmCS < FracClusterSize1) {
            ClusterSize = 1;
        } else if (rndmCS < FracClusterSize1 + FracClusterSize2) {
            ClusterSize = 2;
        } else {
            ClusterSize = int(CLHEP::RandExponential::shoot(rndmEngine, MeanClusterSizeTail));
        }
        ClusterSize = std::max(ClusterSize, 1);
        if (ClusterSize > 1) {
            float rndmLR = CLHEP::RandFlat::shoot(rndmEngine, 1.0);
            if (rndmLR > 0.5) ClusterSize = -ClusterSize;
        }
    }

    // negative CS correspond to left asymmetric cluster with respect to nstrip
    return ClusterSize;
}
double RpcDigitizationTool::FCPEfficiency(const HepMC::ConstGenParticlePtr& genParticle) const {
    double qcharge = 1.;
    const int particlePdgId = genParticle->pdg_id();
    // charge calculation
    qcharge = (static_cast<double>((std::abs(particlePdgId) / 1000) % 100)) / (static_cast<double>((std::abs(particlePdgId) / 10) % 100));
    qcharge = ((static_cast<double>((static_cast<int>(qcharge * 100))))) / 100;
    if (particlePdgId < 0.0) qcharge = -qcharge;
    // BetaGamma calculation
    const double QPx = genParticle->momentum().px();
    const double QPy = genParticle->momentum().py();
    const double QPz = genParticle->momentum().pz();
    const double QE = genParticle->momentum().e();
    const double QM2 = std::pow(QE, 2) - std::pow(QPx, 2) - std::pow(QPy, 2) - std::pow(QPz, 2);
    const double QP = std::hypot(QPx, QPy, QPz);
    const double QM  = QM2 >=0 ? std::sqrt(QM2) : -1.;

    const double qbetagamma = QM > 0. ? QP / QM :  -1.;

    // find the i in the array
    int i_e = -1;
    for (int i = 0; i < 12; i++) {
        if (Charge[i] == std::abs(qcharge)) {
            i_e = i;
            break;
        }
    }
    int i_v = -99, j_v = 99;
    if (qbetagamma != -1) {
        for (int i = 0; i < 15; i++) {
            if (Velocity[i] <= qbetagamma) { i_v = i; }
        }
        for (int i = 14; i >= 0; i--) {
            if (Velocity[i] >= qbetagamma) { j_v = i; }
        }
    }
    // calculate the efficiency according to charge and velocity. Using linear function to calculate efficiency of a specific velocity
    // between velocity1 and velocity2
    double eff_fcp = 1.0, eff_muon = 1.0;
    if (i_e >= 0 && i_e <= 11) {
        if (validIndex(j_v, N_Velocity) && validIndex(i_v, N_Velocity) && (j_v - i_v) == 1) {
            const double delta_v = Velocity[i_v] - Velocity[j_v];
            eff_fcp = (Eff_garfield[i_e][i_v] - Eff_garfield[i_e][j_v]) / delta_v * qbetagamma +
                      (Eff_garfield[i_e][j_v] * Velocity[i_v] - Eff_garfield[i_e][i_v] * Velocity[j_v]) / delta_v;
            eff_muon = (Eff_garfield[11][i_v] - Eff_garfield[11][j_v]) / delta_v * qbetagamma +
                       (Eff_garfield[11][j_v] * Velocity[i_v] - Eff_garfield[11][i_v] * Velocity[j_v]) / delta_v;
        } else if (i_v == 14 && j_v == 99) {
            eff_fcp = Eff_garfield[i_e][14];
            eff_muon = Eff_garfield[11][14];
        } else if (i_v == -99 && j_v == 0) {
            eff_fcp = Eff_garfield[i_e][0];
            eff_muon = Eff_garfield[11][0];
        } else {
            ATH_MSG_WARNING("Wrong particle with unknown velocity! Scale factor is set to be 1.");
        }
    } else {
        ATH_MSG_WARNING("Wrong particle with unknown charge! Scale factor is set to be 1.");
    }
    // A scale factor is calculated by efficiency of fcp / efficiency of muon(charge==1.0
    const double eff_SF = eff_fcp / eff_muon;
    return eff_SF;
}

double RpcDigitizationTool::timeOverThreshold(CLHEP::HepRandomEngine* rndmEngine) {
    //mn Time-over-threshold modeled as a narrow and a wide gaussian
    //mn based on the fit documented in https://its.cern.ch/jira/browse/ATLASRECTS-7820
    constexpr double tot_mean_narrow = 16.;
    constexpr double tot_sigma_narrow = 2.;
    constexpr double tot_mean_wide = 15.;
    constexpr double tot_sigma_wide = 4.5;

    double thetot = 0.;
    
    if (CLHEP::RandFlat::shoot(rndmEngine)<0.75) {
      thetot = CLHEP::RandGaussZiggurat::shoot(rndmEngine, tot_mean_narrow, tot_sigma_narrow);
    } else {
      thetot = CLHEP::RandGaussZiggurat::shoot(rndmEngine, tot_mean_wide, tot_sigma_wide);
    }

    return (thetot > 0.) ? thetot : 0.;
}

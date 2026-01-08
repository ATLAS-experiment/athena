/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "NswCalibDbAlg.h"

#include "TTree.h"
#include "TFile.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CoralBase/Blob.h"
#include "CoralUtilities/blobaccess.h"
#include "MuonCondData/NswCalibDbTimeChargeData.h"
#include "MuonCondData/NswCalibDbThresholdData.h"
#include "MuonNSWCommonDecode/NSWElink.h"
#include "MuonNSWCommonDecode/NSWResourceId.h"
#include "MuonNSWCommonDecode/NSWOfflineHelper.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include "GeoModelKernel/throwExcept.h"

#include<ctime>

namespace Muon{
// Initialize
StatusCode
NswCalibDbAlg::initialize(){

  // retrievals
  ATH_MSG_DEBUG( "initializing " << name() );                
  ATH_CHECK(m_idHelperSvc.retrieve());

  // initialize read keys
  ATH_CHECK(m_readKey_mm_sidea_tdo  .initialize(!m_readKey_mm_sidea_tdo  .empty() && m_idHelperSvc->hasMM()             ));
  ATH_CHECK(m_readKey_mm_sidec_tdo  .initialize(!m_readKey_mm_sidec_tdo  .empty() && m_idHelperSvc->hasMM()             ));
  ATH_CHECK(m_readKey_mm_sidea_pdo  .initialize(!m_readKey_mm_sidea_pdo  .empty() && m_idHelperSvc->hasMM()             ));
  ATH_CHECK(m_readKey_mm_sidec_pdo  .initialize(!m_readKey_mm_sidec_pdo  .empty() && m_idHelperSvc->hasMM()             ));
  ATH_CHECK(m_readKey_mm_sidea_thr  .initialize(!m_readKey_mm_sidea_thr  .empty() && m_idHelperSvc->hasMM() && !m_isData && m_processThresholds));
  ATH_CHECK(m_readKey_mm_sidec_thr  .initialize(!m_readKey_mm_sidec_thr  .empty() && m_idHelperSvc->hasMM() && !m_isData && m_processThresholds));
  ATH_CHECK(m_readKey_stgc_sidea_tdo.initialize(!m_readKey_stgc_sidea_tdo.empty() && m_idHelperSvc->hasSTGC()            ));
  ATH_CHECK(m_readKey_stgc_sidec_tdo.initialize(!m_readKey_stgc_sidec_tdo.empty() && m_idHelperSvc->hasSTGC()            ));
  ATH_CHECK(m_readKey_stgc_sidea_pdo.initialize(!m_readKey_stgc_sidea_pdo.empty() && m_idHelperSvc->hasSTGC()            ));
  ATH_CHECK(m_readKey_stgc_sidec_pdo.initialize(!m_readKey_stgc_sidec_pdo.empty() && m_idHelperSvc->hasSTGC()            ));
  ATH_CHECK(m_readKey_stgc_sidea_thr.initialize(!m_readKey_stgc_sidea_thr.empty() && m_idHelperSvc->hasSTGC() && !m_isData && m_processThresholds));
  ATH_CHECK(m_readKey_stgc_sidec_thr.initialize(!m_readKey_stgc_sidec_thr.empty() && m_idHelperSvc->hasSTGC() && !m_isData && m_processThresholds));
  m_loadMmT0Data = m_loadMmT0Data && m_idHelperSvc->hasMM() && (!m_mmT0FilePath.empty() || !m_readKey_mm_t0.empty());
  m_loadsTgcT0Data = m_loadsTgcT0Data && m_idHelperSvc->hasSTGC() && (!m_stgcT0FilePath.empty() || !m_readKey_stgc_t0.empty()) ;

  ATH_CHECK(m_readKey_mm_t0.initialize(!m_readKey_mm_t0.empty() && m_loadMmT0Data));
  ATH_CHECK(m_readKey_stgc_t0.initialize(!m_readKey_stgc_t0.empty() && m_loadsTgcT0Data));

  // write key for time/charge data
  ATH_CHECK(m_writeKey_tdopdo.initialize());

  // write key for threshold data
  ATH_CHECK(m_writeKey_thr.initialize(!m_isData && m_processThresholds));

  ATH_CHECK(m_writeKey_nswT0.initialize(m_loadMmT0Data || m_loadsTgcT0Data));

  return StatusCode::SUCCESS;
}


// execute
StatusCode 
NswCalibDbAlg::execute(const EventContext& ctx) const {

  ATH_MSG_DEBUG( "execute " << name() );   

  if(processTdoPdoData(ctx).isFailure()) return StatusCode::FAILURE;
  if(processThrData   (ctx).isFailure()) return StatusCode::FAILURE;
  if(m_loadMmT0Data || m_loadsTgcT0Data) ATH_CHECK(processNSWT0Data(ctx));

  return StatusCode::SUCCESS;

}

// processTdoPdoData
StatusCode 
NswCalibDbAlg::processTdoPdoData(const EventContext& ctx) const {

  // set up write handles for time/charge data
  SG::WriteCondHandle wrHdl{m_writeKey_tdopdo, ctx};
  if (wrHdl.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << wrHdl.fullKey() << " is already valid."
        << " In theory this should not be called, but may happen"
        << " if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG("Range of time/charge output is " << wrHdl.getRange());
  auto wrCdo{std::make_unique<NswCalibDbTimeChargeData>(m_idHelperSvc.get())};

  // MM
  if(!m_readKey_mm_sidea_tdo.empty()) {
    ATH_CHECK(loadTimeChargeData(ctx, m_readKey_mm_sidea_tdo  , TimeChargeTech::MM  , TimeChargeType::TDO, wrHdl, wrCdo.get()));
  }
  if(!m_readKey_mm_sidec_tdo.empty()) {
    ATH_CHECK(loadTimeChargeData(ctx, m_readKey_mm_sidec_tdo  , TimeChargeTech::MM  , TimeChargeType::TDO, wrHdl, wrCdo.get()));
  }
  if(!m_readKey_mm_sidea_pdo.empty()) {
    ATH_CHECK(loadTimeChargeData(ctx, m_readKey_mm_sidea_pdo  , TimeChargeTech::MM  , TimeChargeType::PDO, wrHdl, wrCdo.get()));
  }
  if(!m_readKey_mm_sidec_pdo.empty()) {
    ATH_CHECK(loadTimeChargeData(ctx, m_readKey_mm_sidec_pdo  , TimeChargeTech::MM  , TimeChargeType::PDO, wrHdl, wrCdo.get()));
  }

  // sTGC
  if(!m_readKey_stgc_sidea_tdo.empty()) {
    ATH_CHECK(loadTimeChargeData(ctx, m_readKey_stgc_sidea_tdo, TimeChargeTech::STGC, TimeChargeType::TDO, wrHdl, wrCdo.get()));
  }
  if(!m_readKey_stgc_sidec_tdo.empty()) {
    ATH_CHECK(loadTimeChargeData(ctx, m_readKey_stgc_sidec_tdo, TimeChargeTech::STGC, TimeChargeType::TDO, wrHdl, wrCdo.get()));
  }
  if(!m_readKey_stgc_sidea_pdo.empty()) {
    ATH_CHECK(loadTimeChargeData(ctx, m_readKey_stgc_sidea_pdo, TimeChargeTech::STGC, TimeChargeType::PDO, wrHdl, wrCdo.get()));
  }
  if(!m_readKey_stgc_sidec_pdo.empty()) {
    ATH_CHECK(loadTimeChargeData(ctx, m_readKey_stgc_sidec_pdo, TimeChargeTech::STGC, TimeChargeType::PDO, wrHdl, wrCdo.get()));
  }

  // insert/write data for time/charge data
  if (wrHdl.record(std::move(wrCdo)).isFailure()) {
    ATH_MSG_FATAL("Could not record " << wrHdl.key() 
          << " with EventRange " << wrHdl.getRange()
          << " into Conditions Store");
    return StatusCode::FAILURE;
  }      
  ATH_MSG_DEBUG("Recorded new " << wrHdl.key() << " with range " << wrHdl.getRange() << " into Conditions Store");

  return StatusCode::SUCCESS; // nothing to do

}


// processThrData
StatusCode 
NswCalibDbAlg::processThrData(const EventContext& ctx) const {

  if(m_isData || !m_processThresholds) return StatusCode::SUCCESS; // nothing to do

  // set up write handles for threshold data
  SG::WriteCondHandle<NswCalibDbThresholdData> wrHdl{m_writeKey_thr, ctx};
  if (wrHdl.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << wrHdl.fullKey() << " is already valid."
        << " In theory this should not be called, but may happen"
        << " if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG("Range of threshold output is " << wrHdl.getRange());
  std::unique_ptr<NswCalibDbThresholdData> wrCdo{std::make_unique<NswCalibDbThresholdData>(m_idHelperSvc.get())};

  if(!m_readKey_mm_sidea_thr.empty()) {
    ATH_CHECK(loadThresholdData(ctx, m_readKey_mm_sidea_thr  , ThresholdTech::MM  , wrHdl, wrCdo.get()));
  }
  if(!m_readKey_mm_sidec_thr.empty()) {
    ATH_CHECK(loadThresholdData(ctx, m_readKey_mm_sidec_thr  , ThresholdTech::MM  , wrHdl, wrCdo.get()));
  }
  if(!m_readKey_stgc_sidea_thr.empty()) {
    ATH_CHECK(loadThresholdData(ctx, m_readKey_stgc_sidea_thr, ThresholdTech::STGC, wrHdl, wrCdo.get()));
  }
  if(!m_readKey_stgc_sidec_thr.empty()) {
    ATH_CHECK(loadThresholdData(ctx, m_readKey_stgc_sidec_thr, ThresholdTech::STGC, wrHdl, wrCdo.get()));
  }
  
  // insert/write data for threshold data
  if (wrHdl.record(std::move(wrCdo)).isFailure()) {
    ATH_MSG_FATAL("Could not record " << wrHdl.key() 
          << " with EventRange " << wrHdl.getRange()
          << " into Conditions Store");
    return StatusCode::FAILURE;
  }      
  ATH_MSG_DEBUG("Recorded new " << wrHdl.key() << " with range " << wrHdl.getRange() << " into Conditions Store");

  return StatusCode::SUCCESS;
}

// processMmT0Data
StatusCode 
NswCalibDbAlg::processNSWT0Data(const EventContext& ctx) const {
  // set up write handles for MmT0 data
  writeHandleT0_t wrHdl{m_writeKey_nswT0, ctx};
  if (wrHdl.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << wrHdl.fullKey() << " is already valid."
        << " In theory this should not be called, but may happen"
        << " if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG("Range of MmT0 output is " << wrHdl.getRange());
  std::unique_ptr<NswT0Data> wrCdo{std::make_unique<NswT0Data>(m_idHelperSvc.get())};
  if(m_loadMmT0Data){
    if(!m_mmT0FilePath.empty()  ){ // let's read the constants from a  file
      ATH_MSG_INFO("processing MM T0 from file " << m_mmT0FilePath);
      wrHdl.addDependency(EventIDRange(IOVInfiniteRange::infiniteTime()));
      std::unique_ptr<TFile> file (TFile::Open(m_mmT0FilePath.value().c_str()));
      if(!file || file->IsZombie()){
        ATH_MSG_FATAL("Failed to open file containing the MM T0Data. Filepath: "<<m_mmT0FilePath);
        return StatusCode::FAILURE;
      }
      std::unique_ptr<TTree> tree{(TTree*)file->Get("tree_ch")};
      if(!tree){
        ATH_MSG_FATAL("Failed to load tree containing the NswT0Data.");
        return StatusCode::FAILURE;
      }
      ATH_CHECK(loadT0Data(tree, wrCdo.get(), T0Tech::MM));

    } else if(!m_readKey_mm_t0.empty()){
      ATH_MSG_DEBUG("LOAD NSW MM T0 FROM DB");
      std::unique_ptr<TTree> tree; 
      ATH_CHECK(loadT0ToTree(ctx, m_readKey_mm_t0, wrHdl, tree));
      ATH_CHECK(loadT0Data(tree, wrCdo.get(), T0Tech::MM));

    } else {
      ATH_MSG_ERROR("Neither a database folder nor a file have been provided to read the MM T0 constants");
      return StatusCode::FAILURE;
    }
  }
  if(m_loadsTgcT0Data){
    if(!m_stgcT0FilePath.empty()  ){ // let's read the constants from a  file
      ATH_MSG_INFO("processing sTGC T0 from file " << m_stgcT0FilePath);
      wrHdl.addDependency(EventIDRange(IOVInfiniteRange::infiniteTime()));
      std::unique_ptr<TFile> file (TFile::Open(m_stgcT0FilePath.value().c_str()));
      if(!file || file->IsZombie()){
        ATH_MSG_FATAL("Failed to open file containing the sTGC T0Data. Filepath: "<<m_stgcT0FilePath);
        return StatusCode::FAILURE;
      }
      std::unique_ptr<TTree> tree{(TTree*)file->Get("tree_ch")};
      if(!tree){
        ATH_MSG_FATAL("Failed to load tree containing the NswT0Data.");
        return StatusCode::FAILURE;
      }
      ATH_CHECK(loadT0Data(tree, wrCdo.get(), T0Tech::STGC));

    } else if(!m_readKey_stgc_t0.empty()){
      ATH_MSG_DEBUG("LOAD NSW sTGC T0 FROM DB");
      std::unique_ptr<TTree> tree;
      //check failure implies tree is nullptr, and will exit  
      ATH_CHECK(loadT0ToTree(ctx, m_readKey_stgc_t0, wrHdl, tree));
      //coverity[FORWARD_NULL:FALSE]
      ATH_CHECK(loadT0Data(tree, wrCdo.get(), T0Tech::STGC));

    } else {
      ATH_MSG_ERROR("Neither a database folder nor a file have been provided to read the sTGC T0 constants");
      return StatusCode::FAILURE;
    }
  }
  
  // insert/write data for NswT0Data data
  ATH_CHECK(wrHdl.record(std::move(wrCdo)));
  ATH_MSG_DEBUG("Recorded new " << wrHdl.key() << " with range " << wrHdl.getRange() << " into Conditions Store");

  return StatusCode::SUCCESS;
}

StatusCode NswCalibDbAlg::loadT0ToTree(const EventContext& ctx, const readKey_t& readKey, writeHandleT0_t& writeHandle, std::unique_ptr<TTree>& tree) const{
  // set up read handle
  SG::ReadCondHandle readCdo{readKey, ctx};
  if(!readCdo.isValid()){
    ATH_MSG_ERROR("Null pointer to the read conditions object");
    return StatusCode::FAILURE; 
  } 
  writeHandle.addDependency(readCdo);
  ATH_MSG_DEBUG("Size of CondAttrListCollection " << readCdo.fullKey() << " readCdo->size()= " << readCdo->size());
  ATH_MSG_DEBUG("Range of input is " << readCdo.getRange() << ", range of output is " << writeHandle.getRange());

  // iterate through data
  CondAttrListCollection::const_iterator itr;
  for(itr = readCdo->begin(); itr != readCdo->end(); ++itr) {

    // retrieve blob
    const coral::AttributeList& atr = itr->second;
    if(atr["data"].specification().type() != typeid(coral::Blob)) {
      ATH_MSG_FATAL( "Data column is not of type blob!" );
      return StatusCode::FAILURE;
    }
    coral::Blob blob = atr["data"].data<coral::Blob>();
    if(!CoralUtilities::readBlobAsTTree(blob, tree)) {
      ATH_MSG_FATAL( "Cannot retrieve data from coral blob!" );
      return StatusCode::FAILURE;
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode NswCalibDbAlg::loadT0Data(const std::unique_ptr<TTree>& tree, NswT0Data* writeCdo, const T0Tech tech) const{
    int sector{0}, layer{0}, channel{0}, channelType{0}, stationEta{0};
    double time{0};
    tree->SetBranchAddress("sector" , &sector   );
    tree->SetBranchAddress("layer"  , &layer    );
    tree->SetBranchAddress("channel", &channel  );
    tree->SetBranchAddress("mean"   , &time     );
    tree->SetBranchAddress("stationEta"   , &stationEta);
    tree->SetBranchAddress("channel_type", &channelType);
     if (msgLvl(MSG::VERBOSE)) {
          tree->Print();
     }

  ATH_MSG_DEBUG("NSW t0 calibration tree has "<< tree->GetEntries() <<" entries for tech " << (tech==T0Tech::MM ? "MM" : "sTGC"));
  
  for(uint i_channel=0; i_channel<tree->GetEntries(); i_channel++){
    tree->GetEntry(i_channel);
    
    int stationPhi = ((std::abs(sector)-1)/2)+1;
    uint multilayer = (layer<4 ? 1:2); // multilayer 1  corresponds to layers 0-3, ML 2 to layers 4-7
    uint gasGap = layer - (multilayer-1)*4 + 1;

    Identifier id{0};

    if(tech==T0Tech::MM){
      std::string stationName = (sector%2==1 ? "MML" : "MMS");
      bool isValid{true};
      id = m_idHelperSvc->mmIdHelper().channelID(stationName, stationEta, stationPhi,multilayer,gasGap, channel
      // checking the validity of the identifier in production code is too expensiv, therefore only check it in debug build
      #ifndef NDEBUG 
      , isValid
      #endif
      );

      if(!isValid){
        ATH_MSG_ERROR("MM sector "<< sector <<" layer " << layer<< " channel "<< channel << " mean "<< time << " stationEta " << stationEta << " stationPhi " << stationPhi <<" stationName "<< stationName << " multilayer " << multilayer << " gas gap "<< gasGap << " channel " << channel);
        ATH_MSG_ERROR("Failed to build identifier");
        return StatusCode::FAILURE;
      }
    } else {
      std::string stationName = (sector%2==1 ? "STL" : "STS");
      bool isValid{true};
      id = m_idHelperSvc->stgcIdHelper().channelID(stationName, stationEta, stationPhi, multilayer, gasGap, channelType, channel
      // checking the validity of the identifier in production code is too expensiv, therefore only check it in debug build
      #ifndef NDEBUG 
      , isValid
      #endif
      );
      if(!isValid){
        ATH_MSG_ERROR("Failed to build identifier");
        ATH_MSG_DEBUG("STG sector "<< sector <<" layer " << layer<< " channel "<< channel << " mean "<< time << " stationEta " << stationEta << " stationPhi " << stationPhi <<" stationName "<< stationName << " multilayer " << multilayer << " gas gap "<< gasGap << " channel " << channel << " channel type" << channelType);
         return StatusCode::FAILURE;
      }
    }

    writeCdo->setData(id, time);
  }
  return StatusCode::SUCCESS;

}


// loadThresholdData
StatusCode
NswCalibDbAlg::loadThresholdData(const EventContext& ctx, const readKey_t& readKey, const ThresholdTech tech, writeHandleThr_t& writeHandle, NswCalibDbThresholdData* writeCdo) const {

  // set up read handle
  SG::ReadCondHandle readHandle{readKey, ctx};
  const CondAttrListCollection* readCdo{*readHandle}; 
  if(!readCdo){
    ATH_MSG_ERROR("Null pointer to the read conditions object");
    return StatusCode::FAILURE; 
  } 
  writeHandle.addDependency(readHandle);
  ATH_MSG_DEBUG("Size of CondAttrListCollection " << readHandle.fullKey() << " readCdo->size()= " << readCdo->size());
  ATH_MSG_DEBUG("Range of input is " << readHandle.getRange() << ", range of output is " << writeHandle.getRange());

  // iterate through data
  CondAttrListCollection::const_iterator itr;
  unsigned int nObjs = 0;
  for(itr = readCdo->begin(); itr != readCdo->end(); ++itr) {

    // retrieve blob
    const coral::AttributeList& atr = itr->second;
    if(atr["data"].specification().type() != typeid(coral::Blob)) {
      ATH_MSG_FATAL( "Data column is not of type blob!" );
      return StatusCode::FAILURE;
    }
    coral::Blob blob = atr["data"].data<coral::Blob>();
    std::unique_ptr<TTree> tree;
    if(!CoralUtilities::readBlobAsTTree(blob, tree)) {
      ATH_MSG_FATAL( "Cannot retrieve data from coral blob!" );
      return StatusCode::FAILURE;
    }

    // parse tree
    unsigned int elinkId{0}, vmm{0}, channel{0}; 
    float threshold{0.};
    tree->SetBranchAddress("vmm"           , &vmm           );
    tree->SetBranchAddress("channel"       , &channel       );
    tree->SetBranchAddress("elinkId"       , &elinkId       );
    tree->SetBranchAddress("threshold"     , &threshold     );

    // loop over channels
    unsigned int nChns = 0; 
    for(unsigned int iEvt=0; iEvt<tree->GetEntries(); ++iEvt){
      tree->GetEntry(iEvt);
      Identifier channelId;
      if(!buildChannelId(channelId, elinkId, vmm, channel)){
        ATH_MSG_DEBUG("Could not find valid channelId for elink "<<elinkId<<" This is either caused by calibration data of a channel that is known to be not connected to the detector or might point to some issues in the identifier used for the calibration constants");
        continue;
      }
      if(channelId.get_compact()==0){
        writeCdo->setZero(tech, threshold);
        ++nChns;
        continue;
      }
      writeCdo->setData(channelId, threshold);
      ++nChns;
    }
    ATH_MSG_VERBOSE("Retrieved data for "<<nChns<<" channels.");
    ++nObjs;    
  }
  ATH_MSG_VERBOSE("Retrieved data for "<<nObjs<<" objects.");

  return StatusCode::SUCCESS;
}



// loadTimeChargeData
StatusCode
NswCalibDbAlg::loadTimeChargeData(const EventContext& ctx, const readKey_t& readKey, const TimeChargeTech tech, const TimeChargeType type, writeHandleTdoPdo_t& writeHandle, NswCalibDbTimeChargeData* writeCdo) const {

  // set up read handle
  SG::ReadCondHandle readCdo{readKey, ctx};
  if(!readCdo.isValid()){
    ATH_MSG_ERROR("Null pointer to the read conditions object");
    return StatusCode::FAILURE; 
  } 
  writeHandle.addDependency(readCdo);
  ATH_MSG_DEBUG("Size of CondAttrListCollection " << readKey.fullKey() << " readCdo->size()= " << readCdo->size());
  ATH_MSG_DEBUG("Range of input is " << readCdo.getRange() << ", range of output is " << writeHandle.getRange());

  // iterate through data
  CondAttrListCollection::const_iterator itr;
  unsigned int nObjs = 0;
  for(itr = readCdo->begin(); itr != readCdo->end(); ++itr) {

    // retrieve blob
    const coral::AttributeList& atr = itr->second;
    if(atr["data"].specification().type() != typeid(coral::Blob)) {
      ATH_MSG_FATAL( "Data column is not of type blob!" );
      return StatusCode::FAILURE;
    }
    coral::Blob blob = atr["data"].data<coral::Blob>();
    std::unique_ptr<TTree> tree;
    if(!CoralUtilities::readBlobAsTTree(blob, tree)) {
      ATH_MSG_FATAL( "Cannot retrieve data from coral blob!" );
      return StatusCode::FAILURE;
    }
    // parse tree
    unsigned int elinkId{0}, vmm{0}, channel{0};
    float slope{0}, intercept{0};
    //float slope{0}, slopeError{0}, intercept{0},interceptError{0};  

    tree->SetBranchAddress("vmm"           , &vmm           );
    tree->SetBranchAddress("channel"       , &channel       );
    tree->SetBranchAddress("elinkId"       , &elinkId       );
    tree->SetBranchAddress("slope"         , &slope         );
    //tree->SetBranchAddress("slopeError"    , &slopeError    ); // keep for later
    tree->SetBranchAddress("intercept"     , &intercept     );
    //tree->SetBranchAddress("interceptError", &interceptError);
    

    // loop over channels
    unsigned int nChns = 0; 
    for(unsigned int iEvt=0; iEvt<tree->GetEntries(); ++iEvt){
      tree->GetEntry(iEvt);
      Identifier channelId{0};
      if(!buildChannelId(channelId, elinkId, vmm, channel)){
        ATH_MSG_DEBUG("Could not find valid channelId for elink "<<elinkId<<" This is either caused by calibration data of a channel that is known to be not connected to the detector or might point to some issues in the identifier used for the calibration constants");
        continue;
      }

      NswCalibDbTimeChargeData::CalibConstants calib_data{};
      calib_data.slope = slope;
      //calib_data.slopeError = slopeError; // keep for later
      calib_data.intercept = intercept;
      //calib_data.interceptError = interceptError;
      
      if(!channelId.get_compact()){
        writeCdo->setZero(type, tech, calib_data);
        ++nChns;
        continue;
      }

      writeCdo->setData(type, channelId, calib_data);
      ++nChns;
    }
    ATH_MSG_VERBOSE("Retrieved data for "<<nChns<<" channels. "<<tree->GetName()<<" "<<tree->GetEntries());
    ++nObjs;    
  }
  ATH_MSG_VERBOSE("Retrieved data for "<<nObjs<<" objects.");

  return StatusCode::SUCCESS;
}


// buildChannelId
bool
NswCalibDbAlg::buildChannelId(Identifier& channelId, unsigned int elinkId, unsigned int vmm, unsigned int channel) const {

  // return dummy Identifier
  if(elinkId==0){
    channelId = Identifier(0);
    return true;
  }

  // build NSWOfflineHelper
  std::unique_ptr<Muon::nsw::NSWResourceId> resId = std::make_unique<Muon::nsw::NSWResourceId>((uint32_t) elinkId);
  Muon::nsw::helper::NSWOfflineHelper helper(resId.get(), vmm, channel);
  
  std::string stationName;
  if(resId->detId() ==  eformat::MUON_MMEGA_ENDCAP_A_SIDE || resId->detId() ==  eformat::MUON_MMEGA_ENDCAP_C_SIDE) {
      stationName = resId->is_large_station () ? "MML" : "MMS";
  } else if(resId->detId() ==  eformat::MUON_STGC_ENDCAP_A_SIDE || resId->detId() ==  eformat::MUON_STGC_ENDCAP_C_SIDE) {
      stationName = resId->is_large_station () ? "STL" : "STS";
  } else {
      ATH_MSG_ERROR("NSWResource Id "<< elinkId << " does not yield detID that is either sTGC or MMG");
      THROW_EXCEPTION("NSWCalibDbAlg buildChannelId called with detID that is neither sTGC or MMG"); 
  }

  int8_t   stationEta    = resId->station_eta ();
  uint8_t  stationPhi    = resId->station_phi ();
  uint8_t  multiLayer    = resId->multi_layer ();
  uint8_t  gasGap        = resId->gas_gap     ();
  
  uint8_t  channelType   = helper.channel_type  ();
  uint16_t channelNumber = helper.channel_number();

  ATH_MSG_VERBOSE("Station name="    << stationName 
            << " Station eta="    << static_cast <int>          (stationEta)
            << " Station phi="    << static_cast <unsigned int> (stationPhi)
            << " Multilayer="     << static_cast <unsigned int> (multiLayer) 
            << " Gas gap="        << static_cast <unsigned int> (gasGap)
            << " Channel type="   << static_cast <unsigned int> (channelType)
            << " Channel Number=" << channelNumber );


  // MM
  if(resId->detId() ==  eformat::MUON_MMEGA_ENDCAP_A_SIDE || resId->detId() ==  eformat::MUON_MMEGA_ENDCAP_C_SIDE){
    bool isValid {false};
    Identifier chnlId = m_idHelperSvc->mmIdHelper().channelID(stationName, static_cast<int>(stationEta), static_cast<int>(stationPhi), static_cast<int>(multiLayer), static_cast<int>(gasGap), static_cast<int>(channelNumber), isValid);
    if(!isValid){
      ATH_MSG_DEBUG("Could not extract valid channelId for MM elink "<<elinkId);
      return false;
    }
    channelId = chnlId;
  }
  // sTGC
  else if(resId->detId() ==  eformat::MUON_STGC_ENDCAP_A_SIDE || resId->detId() ==  eformat::MUON_STGC_ENDCAP_C_SIDE){
    bool isValid {false};
    Identifier chnlId = m_idHelperSvc->stgcIdHelper().channelID(stationName, static_cast<int>(stationEta), static_cast<int>(stationPhi), static_cast<int>(multiLayer), static_cast<int>(gasGap), static_cast<int>(channelType), static_cast<int>(channelNumber), isValid);
    if(!isValid){
      ATH_MSG_DEBUG("Could not extract valid channelId for STGC elink "<<elinkId);
      return false;
    }
    channelId = chnlId;
  }

  return true;
}
}
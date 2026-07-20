/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixFieldMapsAlg.h"

#include "GaudiKernel/EventIDRange.h"
#include "PathResolver/PathResolver.h"
#include "TFile.h"
#include "TH3F.h"
#include "TH2F.h"
#include <memory>

namespace {
  template <class T>
  std::unique_ptr<T>
  readRootObject(TFile& file, const char* name){
    auto* object = file.Get<T>(name);
    if (!object) {
      return nullptr;
    }
    auto result = std::unique_ptr<T>(object);
    result->SetDirectory(nullptr);
    return result;
  }
  //from EventIDBase
  typedef unsigned int number_type;
  typedef uint64_t     event_number_t;
}

ITkPixFieldMapsAlg::ITkPixFieldMapsAlg(const std::string& name, ISvcLocator* pSvcLocator):
  ::AthCondAlgorithm(name, pSvcLocator)
{
}

StatusCode ITkPixFieldMapsAlg::initialize() {
  ATH_MSG_DEBUG("ITkPixFieldMapsAlg::initialize()");
  ATH_CHECK(m_writeFieldMapKey.initialize());
  EventContext ctx;
  //we need a context eventually, and the following will all be in 'execute'
  //right now, this section is redundant; however by doing it in initialize, we only open
  //the file and read the data once for all events
  number_type runNumber(1);
  event_number_t eventNumber(0);
  number_type timeStamp(0);
  EventIDBase eidRun1 (runNumber, eventNumber, timeStamp);
  ctx.setEventID (eidRun1);
  //
  SG::WriteCondHandle<ITkPixFieldMaps> writeFieldMapHandle(m_writeFieldMapKey, ctx);
  if (writeFieldMapHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeFieldMapHandle.fullKey() << " is already valid.. In theory this should not be called, but may happen if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS; 
  }
  

  // Construct the output Cond Object and fill it in
  auto  writeFieldsCdo(std::make_unique<ITkPixFieldMaps>());

  const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT,                     0,                       
                                              0, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop {EventIDBase::UNDEFNUM,   EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, 
                          EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};

  EventIDRange rangeW{start, stop};

  // Create mapping file for radiation damage simulation
  std::vector<PixelHistoConverter> ramoPotentialMap;
  std::vector<PixelHistoConverter> lorentzMap_e;
  std::vector<PixelHistoConverter> lorentzMap_h;
  std::vector<PixelHistoConverter> distanceMap_e;
  std::vector<PixelHistoConverter> distanceMap_h;
  //returns /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/PixelDigitization/maps_IBL_PL_80V_fl0e14.root
  const std::string fullPath  = PathResolverFindCalibFile(m_efieldFilePath.value());
  ATH_MSG_INFO("Using map located at: "<<fullPath);
  std::unique_ptr<TFile> mapsFile(TFile::Open(fullPath.c_str(), "READ")); //this is the ramo potential
  if (!mapsFile) {
    ATH_MSG_FATAL("Cannot open file: " << m_efieldFilePath.value());
    return StatusCode::FAILURE;
  }

  //Setup ramo weighting field map
  auto ramoPotentialMap_hold = readRootObject<TH3F>(*mapsFile, "hramomap1");
  if (!ramoPotentialMap_hold) {
    ramoPotentialMap_hold = readRootObject<TH3F>(*mapsFile, "ramo3d");
  }
  if (!ramoPotentialMap_hold) {
    ATH_MSG_FATAL("Did not find a Ramo potential map and an approximate form is available yet. Exit...");
    return StatusCode::FAILURE;
  }

  ramoPotentialMap_hold->SetDirectory(nullptr);
  auto lorentzMap_e_hold = readRootObject<TH2F>(*mapsFile, "lorentz_map_e");
  auto lorentzMap_h_hold = readRootObject<TH2F>(*mapsFile, "lorentz_map_h");
  auto distanceMap_e_hold = readRootObject<TH2F>(*mapsFile, "edistance");
  auto distanceMap_h_hold = readRootObject<TH2F>(*mapsFile, "hdistance");

  if (!lorentzMap_e_hold || !lorentzMap_h_hold || !distanceMap_e_hold || !distanceMap_h_hold) {
    ATH_MSG_FATAL("Cannot read one of the histograms needed");
    return StatusCode::FAILURE;
  }


  ramoPotentialMap.emplace_back();
  ATH_CHECK(ramoPotentialMap.back().setHisto3D(ramoPotentialMap_hold.get()));
  lorentzMap_e.emplace_back();
  lorentzMap_h.emplace_back();
  distanceMap_e.emplace_back();
  distanceMap_h.emplace_back();
  ATH_CHECK(lorentzMap_e.back().setHisto2D(lorentzMap_e_hold.get()));
  ATH_CHECK(lorentzMap_h.back().setHisto2D(lorentzMap_h_hold.get()));
  ATH_CHECK(distanceMap_e.back().setHisto2D(distanceMap_e_hold.get()));
  ATH_CHECK(distanceMap_h.back().setHisto2D(distanceMap_h_hold.get()));

  mapsFile->Close();
  
  writeFieldsCdo -> setLorentzMap_e(std::move(lorentzMap_e));
  writeFieldsCdo -> setLorentzMap_h(std::move(lorentzMap_h));
  writeFieldsCdo -> setDistanceMap_e(std::move(distanceMap_e));
  writeFieldsCdo -> setDistanceMap_h(std::move(distanceMap_h));
  writeFieldsCdo -> setRamoPotentialMap(std::move(ramoPotentialMap));

  if (rangeW.stop().isValid() && rangeW.start()>rangeW.stop()) {
    ATH_MSG_FATAL("Invalid intersection rangeW: " << rangeW);
    return StatusCode::FAILURE;
  }
  if (writeFieldMapHandle.record(rangeW, std::move(writeFieldsCdo)).isFailure()) {
    ATH_MSG_FATAL("Could not record ITkPixFieldMaps " << writeFieldMapHandle.key() << " with EventRange " << rangeW << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("recorded new CDO " << writeFieldMapHandle.key() << " with range " << rangeW << " into Conditions Store");

  return StatusCode::SUCCESS;
  
}

StatusCode ITkPixFieldMapsAlg::execute(const EventContext& /*ctx*/) const {
  ATH_MSG_DEBUG("ITkPixFieldMapsAlg::execute()");

  
  return StatusCode::SUCCESS;
}



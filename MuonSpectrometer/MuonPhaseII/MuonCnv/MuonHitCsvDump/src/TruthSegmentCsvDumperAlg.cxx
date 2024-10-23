/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthSegmentCsvDumperAlg.h"

#include "MuonReadoutGeometryR4/MuonReadoutElement.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "GaudiKernel/SystemOfUnits.h"

#include <fstream>
#include <TString.h>

#include <unordered_map>

namespace MuonR4{
TruthSegmentCsvDumperAlg::TruthSegmentCsvDumperAlg(const std::string& name, ISvcLocator* pSvcLocator):
  AthAlgorithm{name, pSvcLocator} {}
StatusCode TruthSegmentCsvDumperAlg::initialize() {
  ATH_CHECK(m_inSegmentKey.initialize());
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(detStore()->retrieve(m_r4DetMgr));
  return StatusCode::SUCCESS;
}

StatusCode TruthSegmentCsvDumperAlg::execute(){
  const EventContext & context = Gaudi::Hive::currentContext();
  std::ofstream file{std::string(Form("event%09zu-",++m_event))+"MuonTruthSegment.csv"};
  const std::string delim = ",";

  file<<"GlobalPositionCentroidX"<<delim<<"GlobalPositionCentroidY"<<delim<<"GlobalPositionCentroidZ"<<delim;
  file<<"GlobalDirectionX"<<delim<<"GlobalDirectionY"<<delim<<"GlobalDirectionZ"<<delim;
  file<<"Time"<<delim<<"TimeError"<<delim;
  file<<"ChiSquared"<<delim<<"NumberDoF"<<delim;
  file<<"PrecisionHits"<<delim<<"PhiLayers"<<delim<<"TrigEtaLayers"<<delim;
  file<<"Sector"<<delim <<"Eta"<<delim<<"Chamber"<<delim<<"Technology"<<std::endl;
  
  SG::ReadHandle<xAOD::MuonSegmentContainer> readTruthSegment(m_inSegmentKey, context);
  if(!readTruthSegment.isValid()){
   ATH_MSG_FATAL("Failed to retrieve truth segment container");
   return StatusCode::FAILURE;
  }

  ATH_CHECK(readTruthSegment.isPresent());
  for (const xAOD::MuonSegment* segment : *readTruthSegment) {
    // Segment information
    float seg_x   = segment->x();
    float seg_y   = segment->y();
    float seg_z   = segment->z();
    float seg_px  = segment->px();
    float seg_py  = segment->py();
    float seg_pz  = segment->pz();

    // time information
    float seg_t0      = segment->t0();
    float seg_t0error = segment->t0error();

    // Fit quality information
    float seg_chiSquared = segment->chiSquared();
    float seg_numberDoF  = segment->numberDoF();

    // nHits
    int seg_PrecisionHits = segment->nPrecisionHits();
    int seg_PhiLayers     = segment->nPhiLayers();
    int seg_TrigEtaLayers = segment->nTrigEtaLayers();

    // Detector information
    int seg_sector                                  = segment->sector();
    int seg_eta                                     = segment->etaIndex();
    Muon::MuonStationIndex::ChIndex seg_chamber     = segment->chamberIndex();
    Muon::MuonStationIndex::TechnologyIndex seg_tec = segment->technology();

    // verbose some information to check
    ATH_MSG_VERBOSE("x: "<<seg_x<<" y: "<<seg_y<<" z: "<<seg_z);
    ATH_MSG_VERBOSE("px: "<<seg_px<<" py: "<<seg_py<<" pz: "<<seg_pz);
    ATH_MSG_VERBOSE("t0: "<<seg_t0<<" t0error: "<<seg_t0error);
    ATH_MSG_VERBOSE("chiSquared: "<<seg_chiSquared<<" numberDoF: "<<seg_numberDoF);
    ATH_MSG_VERBOSE("nPrecisionHits: "<<seg_PrecisionHits<<" nPhiLayers: "<<seg_PhiLayers<<" nTrigEtaLayers: "<<seg_TrigEtaLayers);
    ATH_MSG_VERBOSE("sector: "<<seg_sector<<" eta: "<<seg_eta);
    ATH_MSG_VERBOSE("chamber: "<<seg_chamber<<"technology: "<<seg_tec);

    // save the segment information to the csv file 
    file<<seg_x<<delim<<seg_y<<delim<<seg_z<<delim;
    file<<seg_px<<delim<<seg_py<<delim<<seg_pz<<delim;
    file<<seg_t0<<delim<<seg_t0error<<delim;
    file<<seg_chiSquared<<delim<<seg_numberDoF<<delim;
    file<<seg_PrecisionHits<<delim<<seg_PhiLayers<<delim<<seg_TrigEtaLayers<<delim;
    file<<seg_sector<<delim<<seg_eta<<delim<<seg_chamber<<delim<<seg_tec<<std::endl;
     
  }
     return StatusCode::SUCCESS;
}

}

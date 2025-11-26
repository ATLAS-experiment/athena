/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthSegmentCsvDumperAlg.h"

#include "MuonReadoutGeometryR4/MuonReadoutElement.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "GaudiKernel/SystemOfUnits.h"

#include "MuonPatternHelpers/MatrixUtils.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonSegment/MuonSegment.h"

#include <fstream>
#include <TString.h>

#include <unordered_map>


namespace {
  constexpr int encodeId(const int8_t stName, const int8_t stEta,
               const int8_t sector) {
      return (sector <<16 |stEta << 8| stName);
  }
  constexpr double precCutOff(const double value, const double cutOff = 1.e-15) {
    return std::abs(value) > cutOff ? value : 0.;
  }

}

namespace MuonR4{


StatusCode TruthSegmentCsvDumperAlg::initialize() {
  ATH_CHECK(m_inSegmentKey.initialize());
  ATH_CHECK(m_geoCtxKey.initialize());
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_edmHelperSvc.retrieve());
  ATH_CHECK(detStore()->retrieve(m_detMgr));
  return StatusCode::SUCCESS;
}

const MuonGMR4::SpectrometerSector* TruthSegmentCsvDumperAlg::msSector(const xAOD::MuonSegment& segment) const {
  const auto truthHits = getMatchingSimHits(segment);
  if (truthHits.size()) {
     return m_detMgr->getSectorEnvelope((*truthHits.begin())->identify());
  }
  if (segment.muonSegment().isValid()) {
      return m_detMgr->getSectorEnvelope(m_edmHelperSvc->chamberId(static_cast<const Muon::MuonSegment&>(**segment.muonSegment())));
  }
  ATH_MSG_WARNING("No matching hits were found. Neither the simulated ones or the reconstructed");
  return nullptr;
}

StatusCode TruthSegmentCsvDumperAlg::execute(){
  const EventContext & ctx = Gaudi::Hive::currentContext();
  std::ofstream file{std::string(Form("event%09zu-",++m_event))+"MuonTruthSegment.csv"};
  constexpr std::string_view delim = ",";
  file<<"sectorId"<<delim;
  file<<"globalPositionX"<<delim;
  file<<"globalPositionY"<<delim;
  file<<"globalPositionZ"<<delim;

  file<<"globalDirectionX"<<delim;
  file<<"globalDirectionY"<<delim;
  file<<"globalDirectionZ"<<delim;

  
  file<<"localPositionX"<<delim;
  file<<"localPositionY"<<delim;
  file<<"localPositionZ"<<delim;

  file<<"localDirectionX"<<delim;
  file<<"localDirectionY"<<delim;
  file<<"localDirectionZ"<<delim;

  file<<"time"<<delim;
  file<<"timeError"<<delim;
  file<<"chiSquared"<<delim;
  file<<"nDoF"<<delim;
  file<<"precisionHits"<<delim;
  file<<"phiLayers"<<delim;
  file<<"trigEtaLayers"<<delim;
  file<<std::endl;

  const xAOD::MuonSegmentContainer* readTruthSegment{nullptr};
  ATH_CHECK(SG::get(readTruthSegment, m_inSegmentKey, ctx));

  const ActsTrk::GeometryContext* gctxHandle{nullptr};
  ATH_CHECK(SG::get(gctxHandle, m_geoCtxKey, ctx));

  for (const xAOD::MuonSegment* segment : *readTruthSegment) {
    const MuonGMR4::SpectrometerSector* sector = msSector(*segment);
    if (!sector) {
      continue;
    }
    const Amg::Transform3D globToLoc{sector->globalToLocalTrans(*gctxHandle)};
    /// Segment information
    const Amg::Vector3D globPos = segment->position();
    const Amg::Vector3D globDir = segment->direction();
    /// Local direction
    const Amg::Vector3D locPos = globToLoc * globPos;
    const Amg::Vector3D locDir = globToLoc.linear() * globDir;

    const int secId = encodeId(static_cast<int8_t>(sector->chamberIndex()),
                               sector->side(),
                               sector->sector());
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


    // verbose some information to check
    ATH_MSG_VERBOSE("Segment  global position "<<Amg::toString(globPos)<<", direction: "<<Amg::toString(globDir));
    ATH_MSG_VERBOSE("Segment local position: "<<Amg::toString(locPos)<<", direction: "<<Amg::toString(locDir));
    ATH_MSG_VERBOSE("t0: "<<seg_t0<<" t0error: "<<seg_t0error);
    ATH_MSG_VERBOSE("chiSquared: "<<seg_chiSquared<<" numberDoF: "<<seg_numberDoF);
    ATH_MSG_VERBOSE("nPrecisionHits: "<<seg_PrecisionHits<<" nPhiLayers: "<<seg_PhiLayers<<" nTrigEtaLayers: "<<seg_TrigEtaLayers);

    // save the segment information to the csv file 
    file<<secId<<delim;
    file<<precCutOff(globPos.x())<<delim;
    file<<precCutOff(globPos.y())<<delim;
    file<<precCutOff(globPos.z())<<delim;
    file<<precCutOff(globDir.x())<<delim;
    file<<precCutOff(globDir.y())<<delim;
    file<<precCutOff(globDir.z())<<delim;

    file<<precCutOff(locPos.x())<<delim;
    file<<precCutOff(locPos.y())<<delim;
    file<<precCutOff(locPos.z())<<delim;
    file<<precCutOff(locDir.x())<<delim;
    file<<precCutOff(locDir.y())<<delim;
    file<<precCutOff(locDir.z())<<delim;


    file<<precCutOff(seg_t0)<<delim;
    file<<precCutOff(seg_t0error)<<delim;
    file<<precCutOff(seg_chiSquared)<<delim;
    file<<seg_numberDoF<<delim;
    file<<seg_PrecisionHits<<delim;
    file<<seg_PhiLayers<<delim;
    file<<seg_TrigEtaLayers<<delim;
    file<<std::endl;
  }
  return StatusCode::SUCCESS;
}

}

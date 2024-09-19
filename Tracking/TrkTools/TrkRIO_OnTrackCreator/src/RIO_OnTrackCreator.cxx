/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// RIO_OnTrackCreator.cxx
//   AlgTool for adapting a RIO to a track candidate. It
//   automatically selects the corresponding subdet correction.
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// 2004 - now: Wolfgang Liebig <http://consult.cern.ch/xwho/people/54608>
///////////////////////////////////////////////////////////////////

// --- the base class
#include "TrkRIO_OnTrackCreator/RIO_OnTrackCreator.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
// --- Gaudi stuff
#include "GaudiKernel/TypeNameString.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "Identifier/Identifier.h"

/** il costruttore
 */
Trk::RIO_OnTrackCreator::RIO_OnTrackCreator(const std::string& t,
			      const std::string& n,
			      const IInterface* p)
  :  AthAlgTool(t,n,p){}

// destructor
Trk::RIO_OnTrackCreator::~RIO_OnTrackCreator() = default;

// initialise
StatusCode Trk::RIO_OnTrackCreator::initialize()
{
  if (AlgTool::initialize().isFailure()) return StatusCode::FAILURE;

  if (m_mode == "all") {
    m_enumMode = Mode::all;
  } else if (m_mode == "indet") {
    m_enumMode = Mode::indet;
  } else if (m_mode == "muon") {
    m_enumMode = Mode::muon;
  } else {
    m_enumMode = Mode::invalid;
  }

  if (m_enumMode == Mode::invalid) {
    ATH_MSG_FATAL("Mode is set to unknown value " << m_mode);
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Mode is set to :" <<m_mode);

  // Get the correction tool to create Pixel/SCT/TRT RIO_onTrack
  if (m_enumMode == Mode::all || m_enumMode == Mode::indet) {
    if (!m_pixClusCor.empty()) {
      ATH_CHECK(m_pixClusCor.retrieve());
    } else {
      m_doPixel = false;
    }

    if (!m_sctClusCor.empty()) {
      ATH_CHECK(m_sctClusCor.retrieve());
    } else {
      m_doSCT = false;
    }

    if (!m_trt_Cor.empty()) {
      ATH_CHECK(m_trt_Cor.retrieve());
    } else {
      m_doTRT = false;
    }
  } else {
    m_trt_Cor.disable();
    m_pixClusCor.disable();
    m_sctClusCor.disable();
  }

  if (m_enumMode == Mode::all || m_enumMode == Mode::muon) {
    ATH_CHECK(m_muonDriftCircleCor.retrieve());
    ATH_CHECK(m_muonClusterCor.retrieve());
  } else {
    m_muonClusterCor.disable();
    m_muonDriftCircleCor.disable();
  }

  // Set up ATLAS ID helper to be able to identify the RIO's det-subsystem.
  ATH_CHECK(detStore()->retrieve(m_idHelper, "AtlasID"));

  return StatusCode::SUCCESS;
}

// The sub-detector brancher algorithm
Trk::RIO_OnTrack* 
Trk::RIO_OnTrackCreator::correct(const Trk::PrepRawData& rio,
                                 const TrackParameters& trk,
                                 const EventContext& ctx) const
{

  Identifier id;
  id = rio.identify();

  // --- print RIO
  ATH_MSG_VERBOSE ("RIO ID prints as "<<m_idHelper->print_to_string(id));
  ATH_MSG_VERBOSE ("RIO.locP = ("<<rio.localPosition().x()<<","<<rio.localPosition().y()<<")");

  if (m_doPixel && m_idHelper->is_pixel(id)) {
    if (m_enumMode == Mode::muon) {
      ATH_MSG_WARNING(
          "No tool to correct the current Pixel hit! return nullptr");
      return nullptr;
    }
    return m_pixClusCor->correct(rio, trk, ctx);
  }

  if (m_doSCT && m_idHelper->is_sct(id)) {
    if (m_enumMode == Mode::muon) {
      ATH_MSG_WARNING(
          "No tool to correct the current SCT hit! - Giving back nullptr.");
      return nullptr;
    }
    return m_sctClusCor->correct(rio, trk, ctx);
  }

  if (m_doTRT && m_idHelper->is_trt(id)) {
    if (m_enumMode == Mode::muon) {
      ATH_MSG_WARNING(
          "No tool to correct a TRT DriftCircle! - Giving back nullptr.");
      return nullptr;
    }
    return m_trt_Cor->correct(rio, trk, ctx);
  }

  if (m_idHelper->is_mdt(id)) {
    if (m_enumMode == Mode::indet) {
      ATH_MSG_WARNING(
          "No tool to correct a MDT DriftCircle! - Giving back nullptr.");
      return nullptr;
    }
    return m_muonDriftCircleCor->correct(rio, trk, ctx);
  }

  if ((m_idHelper->is_csc(id)) || (m_idHelper->is_rpc(id)) ||
      (m_idHelper->is_tgc(id)) || (m_idHelper->is_mm(id)) ||
      (m_idHelper->is_stgc(id))) {
    if (m_enumMode == Mode::indet) {
      ATH_MSG_WARNING("No tool to correct a CSC/RPC/TGC/MM/sTGC hit! - Giving back nullptr.");
      return nullptr;
    }
    return m_muonClusterCor->correct(rio, trk, ctx);
  }

  ATH_MSG_WARNING("idHelper could not identify sub-detector for: "
                  << m_idHelper->print_to_string(id)
                  << ". Return nil RIO_OnTrack");
  return nullptr;
}

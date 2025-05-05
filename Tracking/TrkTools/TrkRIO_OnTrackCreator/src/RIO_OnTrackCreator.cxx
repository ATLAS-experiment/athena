/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "TrkPrepRawData/PrepRawData.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
// --- Gaudi stuff
#include "GaudiKernel/TypeNameString.h"
#include "AtlasDetDescr/AtlasDetectorID.h"
#include "Identifier/Identifier.h"

// destructor
Trk::RIO_OnTrackCreator::~RIO_OnTrackCreator() = default;

// initialise
StatusCode Trk::RIO_OnTrackCreator::initialize() {

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
  const bool doId = m_enumMode == Mode::all || m_enumMode == Mode::indet;
  const bool doMuon = m_enumMode == Mode::all || m_enumMode == Mode::muon;
  ATH_CHECK(m_pixClusCor.retrieve(EnableTool{!m_pixClusCor.empty() && doId}));
  ATH_CHECK(m_sctClusCor.retrieve(EnableTool{!m_sctClusCor.empty() && doId}));
  ATH_CHECK(m_trt_Cor.retrieve(EnableTool{!m_trt_Cor.empty() && doId}));
  
  ATH_CHECK(m_muonDriftCircleCor.retrieve(EnableTool{!m_muonDriftCircleCor.empty() && doMuon}));
  ATH_CHECK(m_muonClusterCor.retrieve(EnableTool{!m_muonClusterCor.empty() && doMuon}));
  return StatusCode::SUCCESS;
}

// The sub-detector brancher algorithm
Trk::RIO_OnTrack* 
Trk::RIO_OnTrackCreator::correct(const Trk::PrepRawData& rio,
                                 const TrackParameters& trk,
                                 const EventContext& ctx) const{

 
  // --- print RIO
  ATH_MSG_VERBOSE ("RIO ID prints as "<<rio);
  ATH_MSG_VERBOSE ("RIO.locP = "<<Amg::toString(rio.localPosition()));

  if (rio.type(Trk::PrepRawDataType::PixelCluster)) {
    if (m_pixClusCor.isEnabled()) {
        return m_pixClusCor->correct(rio, trk, ctx);
    }
  } else if (rio.type(Trk::PrepRawDataType::SCT_Cluster)) {
    if (m_sctClusCor.isEnabled()) {
      return m_sctClusCor->correct(rio, trk, ctx);
    }
  } else if (rio.type(Trk::PrepRawDataType::TRT_DriftCircle)) {
    if (m_trt_Cor.isEnabled()){
      return m_trt_Cor->correct(rio, trk, ctx);
    }
  } else if (rio.type(Trk::PrepRawDataType::MdtPrepData)) {
    if (m_muonDriftCircleCor.isEnabled()) {
        return m_muonDriftCircleCor->correct(rio, trk, ctx);
    }
  } else if (rio.type(Trk::PrepRawDataType::RpcPrepData) ||
             rio.type(Trk::PrepRawDataType::TgcPrepData) ||
             rio.type(Trk::PrepRawDataType::sTgcPrepData) ||
             rio.type(Trk::PrepRawDataType::MMPrepData) ||
             rio.type(Trk::PrepRawDataType::CscPrepData)) {
      if (m_muonClusterCor.isEnabled()) {
          return m_muonClusterCor->correct(rio, trk, ctx);
      }
  }
  ATH_MSG_WARNING("Cannot calibrate "<<rio<< ". Return a nullptr.");
  return nullptr;
}

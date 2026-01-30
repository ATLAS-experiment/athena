/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkEventCnvTools/EventCnvSuperTool.h"

#include <typeinfo>

#include "AtlasDetDescr/AtlasDetectorID.h"
#include "TrkEventCnvTools/DetElementSurface.h"
#include "TrkEventCnvTools/ITrkEventCnvTool.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkSurfaces/Surface.h"

Trk::EventCnvSuperTool::EventCnvSuperTool(const std::string& t,
                                          const std::string& n,
                                          const IInterface* p)
    : base_class(t, n, p),
      m_detID(nullptr),
      m_haveIdCnvTool(false),    // Will be set to true on retrieval
      m_haveMuonCnvTool(false),  // Will be set to true on retrieval
      m_doMuons(true),
      m_doID(true),
      m_doTrackOverlay(false),
      m_errCount(0),
      m_maxErrCount(10) {
  declareProperty("DoMuons", m_doMuons,
                  "If true (default), attempt to retrieve Muon helper tool and "
                  "convert Muon objects.");
  declareProperty("DoID", m_doID,
                  "If true (default), attempt to retrieve Inner Detector "
                  "helper tool and convert ID objects.");
  declareProperty("DoTrackOverlay", m_doTrackOverlay,
                  "If true, ID on-track conversion tools will look for "
                  "background PRD collections");
  declareProperty("MaxErrorCount", m_maxErrCount,
                  "Maximum number of errors that will be reported");
}

Trk::EventCnvSuperTool::~EventCnvSuperTool() {
  if (m_errCount > m_maxErrCount)
    ATH_MSG_WARNING("Suppressed " << (m_errCount - m_maxErrCount)
                                  << " warning or error messages");
}

StatusCode Trk::EventCnvSuperTool::initialize() {
  // Try to get AtlasID
  StatusCode sc = detStore()->retrieve(m_detID, "AtlasID");
  if (sc.isFailure()) {
    ATH_MSG_WARNING("Could not get AtlasDetectorID ");
  }

  if (!m_doID && !m_doMuons) {
    ATH_MSG_WARNING(
        "This tool has been configured without either Muons or ID, and so "
        "can't do anything. Problems likely.");
  }

  // Now try to get the tools
  if (m_doID && !m_idCnvTool.empty()) {
    if (m_idCnvTool.retrieve().isFailure()) {
      ATH_MSG_DEBUG(
          "Failed to retrieve InDet helper tool "
          << m_idCnvTool
          << ". Will not be able to recreate ID Surfaces / Det Elements.");
      m_doID = false;
    } else {
      ATH_MSG_VERBOSE("Retrieved tool " << m_idCnvTool);
      m_haveIdCnvTool = true;
    }
  } else {
    m_idCnvTool.setTypeAndName("");
  }

  if (m_doMuons && !m_muonCnvTool.empty()) {
    if (m_muonCnvTool.retrieve().isFailure()) {
      ATH_MSG_DEBUG(
          "Failed to retrieve Muon helper tool "
          << m_muonCnvTool
          << ". Will not be able to recreate ID Surfaces / Det Elements.");
      m_doMuons = false;
    } else {
      ATH_MSG_VERBOSE("Retrieved tool " << m_muonCnvTool);
      m_haveMuonCnvTool = true;
    }
  } else {
    m_muonCnvTool.setTypeAndName("");
  }

  // Print an extra warning if neither tool found.
  if (!m_haveIdCnvTool && !m_haveMuonCnvTool) {
    ATH_MSG_WARNING(
        "Failed to retrieve either and InDet or a Muon tool. Will not be able "
        "to recreate surfaces / detector elements.");
    m_maxErrCount = 0;  // No point in further WARNINGs
  }

  return StatusCode::SUCCESS;
}

StatusCode Trk::EventCnvSuperTool::finalize() {
  msg() << "Finalize().";
  if (m_errCount > 0)
    msg() << " Tried to print " << m_errCount
          << " error/warning messages (with maximum permissable = "
          << m_maxErrCount << ")";
  msg() << endmsg;
  return StatusCode::SUCCESS;
}

const Trk::ITrkEventCnvTool* Trk::EventCnvSuperTool::getCnvTool(
    const Identifier& id) const {
  if (m_detID == nullptr)
    return nullptr;

  if (m_detID->is_indet(id)) {
    if (m_haveIdCnvTool) {
      return &(*m_idCnvTool);
    } else {
      if ((m_errCount++) < m_maxErrCount)
        ATH_MSG_WARNING("ID RIO_OnTrack, but have no ID cnv tool!");
      return nullptr;
    }
  } else {
    if (m_detID->is_muon(id)) {
      if (m_haveMuonCnvTool) {
        return &(*m_muonCnvTool);
      } else {
        if ((m_errCount++) < m_maxErrCount)
          ATH_MSG_WARNING(
              "Muon RIO_OnTrack, but have no muon cnv tool. Cannot set check "
              "RoT.");
        return nullptr;
      }
    }
  }

  if ((m_errCount++) < m_maxErrCount) {
    std::string ident = m_detID->show_to_string(id);
    ATH_MSG_WARNING("Unknown Identifier: (" << ident << "), that is (" << id
                                            << ")");
  }
  return nullptr;
}

const Trk::Surface* Trk::EventCnvSuperTool::getSurface(
    const Identifier& id) const {
  const Surface* surface = nullptr;
  const Trk::ITrkEventCnvTool* cnvTool = getCnvTool(id);
  if (cnvTool != nullptr) {
    const TrkDetElementBase* detEl = cnvTool->getDetectorElement(id);
    if (detEl != nullptr)
      surface = &(detEl->surface(id));
    else if ((m_errCount++) < m_maxErrCount)
      ATH_MSG_WARNING("getSurface: could not get detector element from id:"
                      << id << " Returning 0.");
  } else {
    if ((m_errCount++) < m_maxErrCount)
      ATH_MSG_WARNING(
          "getSurface: could not get cnv tool for Identifier:" << id);
  }
  return surface;
}

void Trk::EventCnvSuperTool::recreateRIO_OnTrack(Trk::RIO_OnTrack* RoT) const {
  using namespace std;
  const Trk::ITrkEventCnvTool* cnvTool = getCnvTool(RoT->identify());
  if (cnvTool != nullptr) {
    cnvTool->recreateRIO_OnTrack(RoT);
  } else {
    const type_info& info = typeid(*RoT);
    if ((m_errCount++) < m_maxErrCount)
      ATH_MSG_WARNING(
          "recreateRIO_OnTrack: could not get cnv tool. Returning without "
          "correctly filling ROT of type: "
          << info.name());
  }
}

void Trk::EventCnvSuperTool::prepareRIO_OnTrack(Trk::RIO_OnTrack* RoT) const {
  const Trk::ITrkEventCnvTool* cnvTool = getCnvTool(RoT->identify());
  if (cnvTool != nullptr) {
    cnvTool->prepareRIO_OnTrack(RoT);
  } else {
    if ((m_errCount++) < m_maxErrCount)
      msg() << "prepareRIO_OnTrack could not find appropriate tool to prepare: "
            << *RoT << std::endl;
  }
}

void Trk::EventCnvSuperTool::prepareRIO_OnTrackLink(const Trk::RIO_OnTrack* RoT,
                                                    ELKey_t& key,
                                                    ELIndex_t& index) const {
  const Trk::ITrkEventCnvTool* cnvTool = getCnvTool(RoT->identify());
  if (cnvTool != nullptr) {
    cnvTool->prepareRIO_OnTrackLink(RoT, key, index);
  } else {
    if ((m_errCount++) < m_maxErrCount)
      msg() << "prepareRIO_OnTrack could not find appropriate tool to prepare: "
            << *RoT << std::endl;
  }
}

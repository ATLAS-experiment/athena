/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_ACTSTOXAOD_TRACKCONVERTERALG_H
#define ACTSTRACKRECONSTRUCTION_ACTSTOXAOD_TRACKCONVERTERALG_H

// Base Class
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/PersistentTrackContainer.h"
#include "ActsEvent/TrackContainerHandlesHelper.h"
#include "ActsEvent/ContextUtility.h"

namespace ActsTrk {
  /** @brief Conversion algorithm to transform the Acts track container into an
   *         xAOD track container which can be persitified in an ESD */
  class ActsToXAODTrackConverterAlg : public AthReentrantAlgorithm {
    public:
    
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    virtual ~ActsToXAODTrackConverterAlg() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    /** @brief Auxiliary class to access the magnetic field, geometry and calibration context */
    ContextUtility m_ctxProvider{this};
    /** @brief Key to access the track container considered for persitification */
    SG::ReadHandleKey< ActsTrk::TrackContainer > m_inputTrackContainerKey {this, "InputActsTracksLocation", ""};
    /** @brief Key under which the xAOD type track container will be written to storegate */
    SG::WriteHandleKey< ActsTrk::PersistentTrackContainer > m_outputTrackContainerKey {this, "OutputActsTracksLocation", ""};
    /** @brief Auxiliary class taking over the conversion of the Acts -> xAOD conversion */
    ActsTrk::MutableTrackContainerHandlesHelper m_tracksBackendHandlesHelper{this};

  };

} // namespace

#endif

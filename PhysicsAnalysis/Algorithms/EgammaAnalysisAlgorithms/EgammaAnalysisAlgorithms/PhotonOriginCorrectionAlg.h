/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

#ifndef EGAMMA_ANALYSIS_ALGORITHMS__PHOTON_ORIGIN_CORRECTION_ALG_H
#define EGAMMA_ANALYSIS_ALGORITHMS__PHOTON_ORIGIN_CORRECTION_ALG_H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgTools/PropertyWrapper.h>
#include <EgammaAnalysisAlgorithms/CopyHelpers.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>

#include "xAODEgamma/PhotonContainer.h"
#include "xAODTracking/VertexContainer.h"

namespace CP {
/// \brief an algorithm for correctiong the origin of a photon
/// wrt the Primary Vertex

class PhotonOriginCorrectionAlg final : public EL::AnaReentrantAlgorithm {
  /// \brief the standard constructor
 public:
  PhotonOriginCorrectionAlg(const std::string& name, ISvcLocator* pSvcLocator);

 public:
  StatusCode initialize() override;

 public:
  StatusCode execute(const EventContext &ctx) const override;

  /// \brief the egamma collection we run on
 private:
  SysCopyHandle<xAOD::PhotonContainer> m_PhotonHandle{
      this, "photons", "Photons", "the egamma collection to run on"};

  /// \brief the systematics list we run
 private:
  SysListHandle m_systematicsList{this};

  /// \brief the preselection we apply to our input
 private:
  SysReadSelectionHandle m_preselection{this, "preselection", "",
                                        "the preselection to apply"};

  /// \brief the vertices to loop over to select the Primary
  SG::ReadHandleKey<xAOD::VertexContainer> m_primVertices{
      this, "Vertices", "PrimaryVertices", "Collection of Primary Vertices"};
};
}  // namespace CP

#endif

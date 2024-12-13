/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#ifndef OVERLAYUTILITIES_OVERLAYVERTEXSKIMMINGALG_H
#define OVERLAYUTILITIES_OVERLAYVERTEXSKIMMINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "EventBookkeeperTools/FilterReporterParams.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODTracking/VertexContainer.h"

#include <string>

class OverlayVertexSkimmingAlg : public AthReentrantAlgorithm {
 public:
  OverlayVertexSkimmingAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~OverlayVertexSkimmingAlg() = default;

  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;
  virtual StatusCode finalize() override final;

 private:
  SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainerKey{
      this, "PrimaryVertexContainerName", "Bkg_PrimaryVertices"};

  FilterReporterParams m_filterParams{
      this, "OverlayVertexSkimmingAlg",
      "Select events with at least one primary vertex"};
};

#endif

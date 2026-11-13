/* Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration */

#ifndef EFLOWREC_PFTRACKCALOEXTENSIONTOOL_H
#define EFLOWREC_PFTRACKCALOEXTENSIONTOOL_H

#include "eflowTrackExtrapolatorBaseAlgTool.h"

#include "ActsEvent/ContextUtility.h"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "AthenaBaseComps/AthAlgTool.h"

#include "xAODTracking/TrackParticleContainer.h"

/**
 Inherits from eflowTrackExtrapolatorBaseAlgTool and AthAlgTool. Uses ACTS to extrapolate tracks to the calorimeter, and creates an eflowTrackCaloPoints object.
*/
class PFTrackCaloExtensionTool: virtual public eflowTrackExtrapolatorBaseAlgTool, public AthAlgTool {

public:

  using AthAlgTool::AthAlgTool;
  ~PFTrackCaloExtensionTool() {};

  virtual StatusCode initialize() override;
  virtual std::unique_ptr<eflowTrackCaloPoints> execute(const EventContext& ctx, const xAOD::TrackParticle* track) const override;
  virtual StatusCode finalize() override;

private:

     /** The ContextUtility will give us access to the Acts geometry context */
     ActsTrk::ContextUtility m_ctxProvider{this};

    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_extensionReadDecorKey{this, "ExtensionDecoration", "InDetTrackParticles.caloExtensionLink"};

};
#endif

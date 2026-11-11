/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUELECTRONVETOVARIABLES_H
#define TAURECTOOLS_TAUELECTRONVETOVARIABLES_H

#include "tauRecTools/TauRecToolBase.h"

#include "ActsEvent/ContextUtility.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"

#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"

namespace Trk {
  class IParticleCaloExtensionTool;
}

/**
 * @brief Calculate variables sensitive on electrons.
 * 
 *  The variables are mainly used by the electron veto in the TauDiscriminant package.
 * 
 * @author Zofia Czyczula
 */

class TauElectronVetoVariables : public TauRecToolBase {

public:
  
  ASG_TOOL_CLASS2(TauElectronVetoVariables, TauRecToolBase, ITauToolBase)

  TauElectronVetoVariables(const std::string& name);

  virtual ~TauElectronVetoVariables() = default;

  virtual StatusCode initialize() override;
  
  using TauRecToolBase::executeTool;
  virtual StatusCode executeTool(xAOD::TauJet& pTau,
				 const EventContext& ctx) const override;

private:

  Gaudi::Property<bool> m_doVertexCorrection {this, "VertexCorrection", true, "switch of vertex correction"}; 
  
  // ACTS Extrapolator
  Gaudi::Property<bool> m_useACTSExtrapolator{this, "useACTSExtrapolator", false};
  ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool {this, "ExtrapolationTool", "ExtrapolationTool", "Tool to run propagation in an ACTS tracking geometry" };
  ServiceHandle<ActsTrk::ITrackingGeometrySvc> m_trackingGeometrySvc{this, "TrackingGeometrySvc", "ActsTrackingGeometrySvc"};
  SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_extensionReadDecorKey{this, "ExtensionDecoration", "InDetTrackParticles.caloExtensionLink"};

  /** The ContextUtility will give us access to the Acts geometry context */
  ActsTrk::ContextUtility m_ctxProvider{this};

  //Map each identifier volume and layer pair onto a calorimeter sampling layer.
  std::map<std::pair<std::size_t, std::size_t>, std::string> m_caloVolumeLayerMap; 


};

#endif // TAURECTOOLS_TAUELECTRONVETOVARIABLES_H

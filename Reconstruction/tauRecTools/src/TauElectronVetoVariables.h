/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUELECTRONVETOVARIABLES_H
#define TAURECTOOLS_TAUELECTRONVETOVARIABLES_H

#include "tauRecTools/TauRecToolBase.h"
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

};

#endif // TAURECTOOLS_TAUELECTRONVETOVARIABLES_H

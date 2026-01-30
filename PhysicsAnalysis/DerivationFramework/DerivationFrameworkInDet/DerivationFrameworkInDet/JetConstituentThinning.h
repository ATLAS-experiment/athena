/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// JetConstituentThinning.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_JETCONSTITUENTTHINNING_H
#define DERIVATIONFRAMEWORK_JETCONSTITUENTTHINNING_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IThinningTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ThinningHandleKey.h"
#include "xAODJet/JetContainer.h"
#include "xAODPFlow/FlowElementContainer.h"
#include "xAODBase/IParticleContainer.h"

#include "ExpressionEvaluation/ExpressionParserUser.h"

namespace DerivationFramework {

class JetConstituentThinning
    : public extends<ExpressionParserUser<AthAlgTool>, IThinningTool> {
public:
  JetConstituentThinning(const std::string &t, const std::string &n,
                         const IInterface *p);
  virtual ~JetConstituentThinning();
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual StatusCode doThinning() const override;

private:
  StringProperty m_streamName{this, "StreamName", "",
                              "Name of the stream being thinned"};

  SG::ReadHandleKey<xAOD::JetContainer> m_jetSGKey{
      this, "JetKey", "", "SG key for jet container"};

  StringProperty m_selectionString{
      this, "SelectionString", "", "Selection string for jets"};

  StringProperty m_jetConstituentName{
      this, "JetConstituentName", "CHSG", "Prefix for jet constituent containers (e.g., CHSG for CHSGChargedParticleFlowObjects)"};

  StringProperty m_globalConstituentName{
      this, "GlobalConstituentName", "Global", "Prefix for global constituent containers (e.g., Global for GlobalChargedParticleFlowObjects)"};

  StringProperty m_otherObjectsName{
      this, "OtherObjectsName", "", "Optional container name for otherObjects (e.g., CaloCalTopoClusters). If empty, otherObjects thinning is disabled."};

  SG::ThinningHandleKey<xAOD::FlowElementContainer> m_jetChargedKey;
  SG::ThinningHandleKey<xAOD::FlowElementContainer> m_jetNeutralKey;
  SG::ThinningHandleKey<xAOD::FlowElementContainer> m_globalChargedKey;
  SG::ThinningHandleKey<xAOD::FlowElementContainer> m_globalNeutralKey;
  SG::ThinningHandleKey<xAOD::IParticleContainer> m_otherObjectsKey;
};

} // namespace DerivationFramework

#endif // DERIVATIONFRAMEWORK_JETCONSTITUENTTHINNING_H

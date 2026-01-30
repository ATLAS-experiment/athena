/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// JetGhostThinning.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_JETGHOSTTHINNING_H
#define DERIVATIONFRAMEWORK_JETGHOSTTHINNING_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IThinningTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ThinningHandleKey.h"
#include "xAODJet/JetContainer.h"
#include "xAODBase/IParticleContainer.h"

#include "ExpressionEvaluation/ExpressionParserUser.h"

namespace DerivationFramework {

class JetGhostThinning
    : public extends<ExpressionParserUser<AthAlgTool>, IThinningTool> {
public:
  JetGhostThinning(const std::string &t, const std::string &n,
                   const IInterface *p);
  virtual ~JetGhostThinning();
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

  StringProperty m_ghostName{
      this, "GhostName", "", "Name of the ghost association (e.g., GhostTower)"};

  StringProperty m_ghostContainerName{
      this, "GhostContainerName", "", "Name of the ghost object container (e.g., CaloCalFwdTopoTowers)"};

  SG::ThinningHandleKey<xAOD::IParticleContainer> m_ghostContainerKey;
};

} // namespace DerivationFramework

#endif // DERIVATIONFRAMEWORK_JETGHOSTTHINNING_H

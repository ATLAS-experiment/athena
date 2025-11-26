/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EGTransverseMassTool.h
// author: giovanni.marchiori@cern.ch
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_EGTRANSVERSEMASSTOOL_H
#define DERIVATIONFRAMEWORK_EGTRANSVERSEMASSTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
//
#include "ExpressionEvaluation/ExpressionParserUser.h"
#include "GaudiKernel/EventContext.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODMissingET/MissingETContainer.h"
//
#include <string>
#include <vector>

namespace DerivationFramework {

class EGTransverseMassTool : public extends<ExpressionParserUser<AthAlgTool>, IAugmentationTool>
{
public:
  using base_class::base_class;

  virtual StatusCode initialize() override final;
  virtual StatusCode addBranches(const EventContext& ctx) const override final;

private:
  StatusCode getTransverseMasses(const EventContext& ctx, std::vector<float>&)
    const;

  Gaudi::Property<std::string> m_expression1{this, "ObjectRequirements", "true"};
  Gaudi::Property<float> m_METmin{this, "METmin", -999.f};
  Gaudi::Property<float> m_mass1Hypothesis{this, "ObjectMassHypothesis", 0.f};

  SG::WriteHandleKey<std::vector<float>> m_sgName{ this,
                                                   "StoreGateEntryName",
                                                   "",
                                                   "SG key of output object" };

  SG::ReadHandleKey<xAOD::IParticleContainer> m_container1Name{
    this,
    "ObjectContainerName",
    "",
    "SG key of first container"
  };
  SG::ReadHandleKey<xAOD::MissingETContainer> m_container2Name{
    this,
    "METContainerName",
    "MET_LocHadTopo",
    "SG key of second container"
  };

  SG::ReadHandleKey<std::vector<float>> m_pt1BranchName{
    this,
    "ObjectPtBranchName",
    "",
    "Pt1 if different than default"
  };

  SG::ReadHandleKey<std::vector<float>> m_phi1BranchName{
    this,
    "ObjectPhiBranchName",
    "",
    "Phi1 if different than default"
  };

  SG::ReadHandleKey<std::vector<float>> m_pt2BranchName{
    this,
    "METPtBranchName",
    "",
    "Pt2 if different than default"
  };

  SG::ReadHandleKey<std::vector<float>> m_phi2BranchName{
    this,
    "METPhiBranchName",
    "",
    "Phi2 if different than default"
  };
};
}

#endif // DERIVATIONFRAMEWORK_EGTRANSVERSEMASSTOOL_H

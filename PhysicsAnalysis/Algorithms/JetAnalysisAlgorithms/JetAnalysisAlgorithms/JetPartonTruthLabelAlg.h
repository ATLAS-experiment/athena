/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/



#ifndef JET_ANALYSIS_ALGORITHMS__JET_PARTON_TRUTH_LABEL_ALG_H
#define JET_ANALYSIS_ALGORITHMS__JET_PARTON_TRUTH_LABEL_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODJet/JetContainer.h>
#include "ParticleJetTools/JetPartonTruthLabel.h"

namespace CP
{
  /// \brief an algorithm for adding ghost muons to jets

  class JetPartonTruthLabelAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    JetPartonTruthLabelAlg (const std::string& name,
                                ISvcLocator* pSvcLocator);

  public:
    StatusCode initialize () override;

  public:
    StatusCode execute () override;


    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the jet collection we run on
  private:
    SysCopyHandle<xAOD::JetContainer> m_jetHandle {
      this, "jets", "", "the jet collection to run on"};

  private:
    ToolHandle<Analysis::JetPartonTruthLabel> m_labelTool {
      this, "LabelTool", "", "jet truth labeling tool"
    };

  };
}

#endif // JET_ANALYSIS_ALGORITHMS__JET_PARTON_TRUTH_LABEL_ALG_H

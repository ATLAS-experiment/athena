/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Jackson Burzynski


#ifndef JET_ANALYSIS_ALGORITHMS__RC_JET_CALIBRATION_ALG_H
#define JET_ANALYSIS_ALGORITHMS__RC_JET_CALIBRATION_ALG_H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODJet/JetContainer.h>

namespace CP
{
  /// \brief an algorithm for calibration reclustered large-R jets

  class ReclusteredJetCalibrationAlg final : public EL::AnaReentrantAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
    virtual StatusCode initialize () override;
    virtual StatusCode execute (const EventContext &ctx) const override;

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the jet collection we run on
  private:
    SysCopyHandle<xAOD::JetContainer> m_reclusteredJetHandle{
      this, "reclusteredJets", "", "the reclustered jet collection to run on"};
    SysReadHandle<xAOD::JetContainer> m_smallRJetHandle{
      this, "smallRJets", "", "the calibrated small-R jet collection to use"};
  };
}

#endif

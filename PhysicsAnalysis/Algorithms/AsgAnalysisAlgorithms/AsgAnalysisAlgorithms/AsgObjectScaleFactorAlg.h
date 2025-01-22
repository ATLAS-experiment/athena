/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef ASG_ANALYSIS_ALGORITHMS__ASG_OBJECT_SCALE_FACTOR_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__ASG_OBJECT_SCALE_FACTOR_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODBase/IParticleContainer.h>

namespace CP
{
  /// \brief an algorithm for combining object scale factors

  class AsgObjectScaleFactorAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;

  private:
    /// \brief the systematics list we run
    SysListHandle m_systematicsList {this};

    /// \brief the particle container we run on
    SysReadHandle<xAOD::IParticleContainer> m_particlesHandle {
      this, "particles", "", "the asg collection to run on"};

    Gaudi::Property<std::vector<std::string>> m_inScaleFactors {
      this, "inScaleFactors", {}, "list of input scale factors to combine"};

    std::vector<SysReadDecorHandle<float>> m_inSFHandles;

    SysWriteDecorHandle<float> m_outSFHandle {
      this, "outScaleFactor", "effSF_%SYS%", "decoration name for combined scale factor"};
  };

}

#endif

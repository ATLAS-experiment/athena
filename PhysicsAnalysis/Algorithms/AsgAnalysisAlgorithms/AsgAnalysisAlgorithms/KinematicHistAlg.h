/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef ASG_ANALYSIS_ALGORITHMS__KINEMATIC_HIST_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__KINEMATIC_HIST_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODBase/IParticleContainer.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP
{
  /// \brief an algorithm for dumping the kinematics of an IParticle
  /// container into histograms
  ///
  /// This is mostly meant as a temporary helper algorithm to debug
  /// the common CP algorithms as they get developed.

  class KinematicHistAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;


    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the jet collection we run on
  private:
    SysReadHandle<xAOD::IParticleContainer> m_inputHandle {
      this, "input", "", "the input collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the pattern for histogram names
  private:
    Gaudi::Property<std::string> m_histPattern {this, "histPattern", "%VAR%_%SYS%", "the pattern for histogram names"};


    /// \brief the histograms we fill per systematic and object
  private:
    struct HistSubgroup
    {
      TH1 *pt = nullptr;
      TH1 *eta = nullptr;
      TH1 *phi = nullptr;
    };


    /// \brief the histograms we fill per systematic
  private:
    struct HistGroup
    {
      TH1 *multiplicity = nullptr;
      std::vector<HistSubgroup> perObject;
    };

    /// \brief the created histograms
  private:
    std::unordered_map<CP::SystematicSet,HistGroup> m_hist;
  };
}

#endif

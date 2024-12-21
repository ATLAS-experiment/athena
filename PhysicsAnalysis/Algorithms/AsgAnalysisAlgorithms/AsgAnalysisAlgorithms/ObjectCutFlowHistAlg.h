/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack
/// @author Baptiste Ravina


#ifndef ASG_ANALYSIS_ALGORITHMS__OBJECT_CUT_FLOW_HIST_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__OBJECT_CUT_FLOW_HIST_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SelectionHelpers/ISelectionNameSvc.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODBase/IParticleContainer.h>

namespace CP
{
  /// \brief an algorithm for dumping the object-level cutflow

  class ObjectCutFlowHistAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;


    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the particle collection we run on
  private:
    SysReadHandle<xAOD::IParticleContainer> m_inputHandle {
      this, "input", "", "the input particle container to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the pattern for histogram names
  private:
    Gaudi::Property<std::string> m_histPattern {this, "histPattern", "cutflow_%SYS%", "the pattern for histogram names"};

    /// \brief the selection name service
  private:
    ServiceHandle<ISelectionNameSvc> m_selectionNameSvc {"SelectionNameSvc", "ObjectCutFlowHistAlg"};

    /// \brief the histogram title to use
  private:
    Gaudi::Property<std::string> m_histTitle {this, "histTitle", "object cut flow", "title for the created histograms"};

    /// \brief the input object selections for which to create a cutflow
  private:
    SysReadSelectionHandleArray m_selections {
      this, "selections", {}, "the inputs to the object cutflow"};

    /// \brief force cut sequence
  private:
    Gaudi::Property<bool> m_forceCutSequence {this, "forceCutSequence", false, "force cut sequence and do not accept if previous cuts failed"};

    /// \brief the total number of cuts configured (needed to
    /// configure histograms)
  private:
    unsigned m_allCutsNum = 0;

    /// \brief the created histograms
  private:
    std::unordered_map<CP::SystematicSet,TH1*> m_hist;

    /// \brief histogram bin labels
  private:
    std::vector<std::string> m_labels;
  };
}

#endif

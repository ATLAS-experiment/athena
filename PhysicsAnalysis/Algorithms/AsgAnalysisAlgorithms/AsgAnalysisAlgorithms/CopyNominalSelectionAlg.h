/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef ASG_ANALYSIS_ALGORITHMS__COPY_NOMINAL_SELECTION_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__COPY_NOMINAL_SELECTION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SelectionHelpers/ISelectionNameSvc.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODBase/IParticleContainer.h>

namespace CP
{
  /// \brief an algorithm that copies the nominal selections over to all other
  /// object systematics
  ///
  /// This is mostly meant for Overlap Removal in the nominal-only scenario.  In
  /// that case the selection will have been only decorated on the nominal
  /// container.  However, if we want to use it for any preselections we also
  /// need it available for all other systematics, and as such we need to copy
  /// it over from the nominal container to all other containers.  If all we
  /// want is to write it out to the n-tuple we probably don't need this, as we
  /// are likely only writing out the nominal and wouldn't try to read the
  /// decoration from any other systematic.
  ///
  /// This may or may not work in other situations in which we suppress
  /// systematics.  Likely it would need some generalizations, but I'd rather
  /// address these as they arise.

  class CopyNominalSelectionAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    CopyNominalSelectionAlg (const std::string& name, ISvcLocator* pSvcLocator);


  public:
    StatusCode initialize () override;

  public:
    StatusCode execute (const EventContext& ctx) override;



    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the particle continer we run on
  private:
    SysReadHandle<xAOD::IParticleContainer> m_particlesHandle {
      this, "particles", "", "the asg collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the decoration for the asg selection
  private:
    SysWriteSelectionHandle m_selectionHandle {
      this, "selectionDecoration", "", "the decoration for the asg selection"};

    /// \brief the accessor we use
  private:
    std::unique_ptr<ISelectionReadAccessor> m_readAccessor;
  };
}

#endif

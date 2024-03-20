/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

#ifndef ASG_ANALYSIS_ALGORITHMS__UNION_PRE_SELECTION_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__UNION_PRE_SELECTION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AthContainers/AuxElement.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/ISelectionWriteAccessor.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODBase/IParticleContainer.h>
#include <AsgTools/PropertyWrapper.h>
#include <optional>


namespace CP
{
  /// \brief an algorithm for turning a systematics depending selection
  /// decoration into a selection decoration without systematics
  ///
  /// The idea here is that if e.g. Overlap Removal creates systematics
  /// dependent selection decorations (normally "passesOR_%SYS%") into a
  /// decoration that can be used for preselecting objects for subsequent
  /// algorithms (e.g. "preselectOR").  That avoids the systematics from the
  /// decorations propagating into whatever outputs the algorithm produces.
  ///
  /// This is separate from the \ref AsgUnionSelectionAlg (aimed at preselection
  /// for output) in that it does the merger based on each systematic copy of
  /// the container.  The reasoning behind that is that regular algorithms will
  /// have to retrieve all the systematic copies anyways, so the preselection
  /// can still be split here.  Also note that this is only usable in
  /// preselection for efficiency.  If you need an actual selection (e.g. to
  /// match to a candidate decay signature) you will still have to use all the
  /// systematic variations on the selection.

  class AsgUnionPreselectionAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    virtual StatusCode initialize () override;
    virtual StatusCode execute () override;



    /// \brief the systematics list we run and have containers
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

    /// \brief the decoration of the selection
  private:
    Gaudi::Property<std::string> m_selectionDecoration {this, "selectionDecoration", "", "the decoration for the union selection"};

    /// \brief the accessor for \ref m_selectionDecoration
  private:
    std::optional<SG::AuxElement::Decorator<char>> m_decorator;
  };

} // namespace CP

#endif

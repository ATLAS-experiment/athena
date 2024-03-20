/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef ASG_ANALYSIS_ALGORITHMS__ASG_VIEW_FROM_SELECTION_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__ASG_VIEW_FROM_SELECTION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <xAODBase/IParticleContainer.h>
#include <SelectionHelpers/ISelectionReadAccessor.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <AsgTools/PropertyWrapper.h>
#include <limits>

namespace CP
{
  /// \brief create a view container based on selection decorations
  ///
  /// This is a generic algorithm that works for all object types to
  /// read our standard selection decorations and create a view
  /// container.  This avoids the need for the algorithm/tool to read
  /// the selection decoration on the input, instead it can just
  /// assume that all its input objects are meant to be processed.

  class AsgViewFromSelectionAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;


    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the input collection we run on
  private:
    SysReadHandle<xAOD::IParticleContainer> m_inputHandle {
      this, "input", "", "the input collection to run on"};

    /// \brief the output view container we produce
  private:
    SysWriteHandle<xAOD::IParticleContainer> m_outputHandle {
      this, "output", "", "the output view container to produce"};

    /// \todo this would probably better be an std::map, but this
    /// isn't supported as a property type for AnaAlgorithm right now
  private:
    Gaudi::Property<std::vector<std::string>> m_selection {this, "selection", {}, "the list of selection decorations"};

  private:
    Gaudi::Property<std::vector<SelectionType>> m_ignore {this, "ignore", {}, "the list of cuts to *ignore* for each selection"};

    /// \brief Sort the output (view) container by pT
  private:
    Gaudi::Property<bool> m_sortPt {this, "sortPt", false, "whether to sort objects in pt"};

    /// \brief Allow the input container to be missing
  private:
    Gaudi::Property<bool> m_allowMissing {this, "allowMissing", false, "Allow the input container to be missing"};

    /// \brief Perform a deep copy for creating the output container
  private:
    Gaudi::Property<bool> m_deepCopy {this, "deepCopy", false, "perform a deep copy"};

  private:
    Gaudi::Property<std::size_t> m_sizeLimit {this, "sizeLimit", std::numeric_limits<std::size_t>::max(), "the limit on the size of the output container"};

    /// the list of accessors and cut ignore list
  private:
    std::vector<std::pair<std::unique_ptr<ISelectionReadAccessor>,SelectionType> > m_accessors;

    /// \brief the templated version of execute for a single systematic
  private:
    template<typename Type> StatusCode
    executeTemplate (const CP::SystematicSet& sys);

    /// \brief the version of execute to find the type
  private:
    StatusCode executeFindType (const CP::SystematicSet& sys);

    /// \brief The version of execute for missing input containers
  private:
    StatusCode executeMissing (const CP::SystematicSet& sys);

  private:
    StatusCode (AsgViewFromSelectionAlg::* m_function) (const CP::SystematicSet& sys) {&AsgViewFromSelectionAlg::executeFindType};
  };
}

#endif

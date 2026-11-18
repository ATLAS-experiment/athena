/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_EXAMPLE_TOOLS_PRESELECTION_EXAMPLE_TOOL_H
#define COLUMNAR_EXAMPLE_TOOLS_PRESELECTION_EXAMPLE_TOOL_H

#include <AsgTools/AsgTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/ParticleDef.h>
#include <ColumnarCore/SelectionColumn.h>

namespace ADD_CMODE(columnar)
{
  /// @brief example of a columnar tool with a preselection column
  ///
  /// This is a variation of the `SimpleSelectorExampleTool` that
  /// demonstrates the standard way for a tool to support an (optional)
  /// preselection, similar to the `preselection` property of the CP
  /// algorithms.

  class PreselectionExampleTool final
    : public asg::AsgTool,
      public columnar::ColumnarTool<>
  {
  public:

    // Create a proper constructor for Athena
    ASG_TOOL_CLASS( PreselectionExampleTool, asg::IAsgTool )

    PreselectionExampleTool (const std::string& name);

    virtual StatusCode initialize () override;

    virtual void callEvents (columnar::EventContextRange events) const override;


    /// @brief the pt cut to apply
    Gaudi::Property<float> m_ptCut {this, "ptCut", 10e3, "pt cut (in MeV)"};


    /// @brief the object accessor for the particles
    columnar::ParticleAccessor<columnar::ObjectColumn> particlesHandle {*this, "Particles"};


    /// @brief the pt accessor for the particle container
    columnar::ParticleAccessor<float> ptAcc {*this, "pt"};


    /// @brief the accessor for the preselection
    ///
    /// The preselection is an optional input column using the standard
    /// name and the `SelectionColumn` type.  If it is not present all
    /// objects are processed.  Otherwise objects failing the
    /// preselection get a default output value, which each tool needs
    /// to choose for itself (here: failing the selection).
    columnar::ParticleAccessor<columnar::SelectionColumn> preselectionAcc {*this, columnar::preselectionName, {.isOptional = true}};


    /// @brief the selection decorator for the particles
    columnar::ParticleDecorator<columnar::SelectionColumn> selectionDec {*this, "selection"};
  };
}

#endif

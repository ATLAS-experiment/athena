/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_EXAMPLE_TOOLS_CONFIGURABLE_COLUMN_EXAMPLE_TOOL_H
#define COLUMNAR_EXAMPLE_TOOLS_CONFIGURABLE_COLUMN_EXAMPLE_TOOL_H

#include <AsgTools/AsgTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/ParticleDef.h>

namespace columnar
{
  /// @brief example of a columnar tool with optional columns
  ///
  /// This is a variation of the `SimpleSelectorExampleTool` that
  /// demonstrates the use of columns that have a user configurable
  /// name.

  class ConfigurableColumnExampleTool final
    : public asg::AsgTool,
      public ColumnarTool<>
  {
  public:

    ConfigurableColumnExampleTool (const std::string& name);

    virtual StatusCode initialize () override;

    virtual void callEvents (EventContextRange events) const override;


    /// @brief the pt variable to use
    Gaudi::Property<std::string> m_ptVar {this, "ptVar", "pt", "pt variable"};

    /// @brief the pt cut to apply
    Gaudi::Property<float> m_ptCut {this, "ptCut", 10e3, "pt cut (in MeV)"};


    /// @brief the object accessor for the particles
    ///
    /// This is equivalent to a `ReadHandleKey` in the xAOD world.  It
    /// is used to access the particle range/container for a given
    /// event.
    ParticleAccessor<ObjectColumn> particlesHandle {*this, "Particles"};


    /// @brief the pt accessor for the particle container
    ///
    /// This is the equivalent to an `AuxElement::Accessor` in the xAOD
    /// world.  Note that this accessor is not initialized here, but
    /// will be initialized in the `initialize` method.
    ParticleAccessor<float> ptAcc;


    /// @brief the selection decorator for the particles
    ///
    /// This is the equivalent to an `AuxElement::Decorator` in the xAOD
    /// world.  One thing to note is that the tools generally run on
    /// many objects, not a a single object.  So the tool doesn't have
    /// the option to return individual output values.  Instead it needs
    /// to provide an output value per object, which in the columnar
    /// world is done by filling a column.
    ParticleDecorator<char> selectionDec {*this, "selection"};
  };
}

#endif

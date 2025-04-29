/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_EXAMPLE_TOOLS_SIMPLE_SELECTOR_EXAMPLE_TOOL_H
#define COLUMNAR_EXAMPLE_TOOLS_SIMPLE_SELECTOR_EXAMPLE_TOOL_H

#include <AsgTools/AsgTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/ParticleDef.h>

namespace columnar
{
  /// @brief this is the simplest example of a columnar tool
  ///
  /// This tool is mostly meant to demonstrate how to write a very
  /// simple columnar tool, applying just a simple pt cut.  It is not
  /// really meant to do anything useful, but outline how a tool can
  /// look like.  Note that this tool is written from scratch, whereas
  /// most actual columnar tools are expected to be conversions of xAOD
  /// tools (for now).
  ///
  /// Some things to note:
  /// * Besides inheriting from `AsgTool`, the tool also inherits from
  ///   `ColumnarTool<>`.  The later contains all the accounting needed
  ///   for columnar accessors.  Please note that `ColumnarTool` is a
  ///   template (with a default argument), so you need to be sure not
  ///   to forget the `<>` when inheriting from it.
  /// * Inside `initialize`, the tool calls `initializeColumns`.  This
  ///   is a method provided by `ColumnarTool` that does whatever is
  ///   needed to initialize the columnar accessors.
  /// * The `callEvents` method provides an algorithm-like interface
  ///   for the tool.  Is is called with an event-range and is expected
  ///   to loop over the events and do whatever is needed for each event.

  class SimpleSelectorExampleTool final
    : public asg::AsgTool,
      public ColumnarTool<>
  {
  public:

    SimpleSelectorExampleTool (const std::string& name);

    virtual StatusCode initialize () override;

    void callSingleEvent (ParticleRange particles) const;

    virtual void callEvents (EventContextRange events) const override;


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
    /// world.  The main difference is that it registers with the tool,
    /// as that is needed for column accessors.  Also, it is specific to
    /// the container, and can't be used with other containers.
    ParticleAccessor<float> ptAcc {*this, "pt"};


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

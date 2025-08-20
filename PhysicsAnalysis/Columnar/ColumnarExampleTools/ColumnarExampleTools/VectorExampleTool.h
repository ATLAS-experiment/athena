/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_EXAMPLE_TOOLS_VECTOR_EXAMPLE_TOOL_H
#define COLUMNAR_EXAMPLE_TOOLS_VECTOR_EXAMPLE_TOOL_H

#include <AsgTools/AsgTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/ParticleDef.h>
#include <ColumnarCore/VectorColumn.h>

namespace columnar
{
  /// @brief an example of a columnar tool that reads a vector column
  ///
  /// This illustrates how to read vector columns in a tool.  The actual
  /// example is a bit arcane, but it hopefully illustrates how to read
  /// vector columns.

  class VectorExampleTool final
    : public asg::AsgTool,
      public ColumnarTool<>
  {
  public:

    // Create a proper constructor for Athena
    ASG_TOOL_CLASS( VectorExampleTool, asg::IAsgTool )

    VectorExampleTool (const std::string& name);

    virtual StatusCode initialize () override;

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


    /// @brief a vector column accessor
    ///
    /// There aren't a lot of cases in which there is a simple
    /// POD-vector column in PHYSLITE, so I picked up this rather
    /// obscure one.
    ParticleAccessor<std::vector<int>> trknumAcc {*this, "NumTrkPt500"};


    /// @brief a vector accessor involving retyping
    ///
    /// This mostly illustrates and tests that view can also read
    /// columns that need to do a conversion of the underlying type.  In
    /// PHYSLITE this mostly happens for ElementLink columns, and maybe
    /// enum columns, but for a simple example I'm changing from `float`
    /// to `double`.
    ParticleAccessor<std::vector<RetypeColumn<double,float>>> trksumptAcc {*this, "SumPtTrkPt500"};


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

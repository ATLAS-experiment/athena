/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_EXAMPLE_TOOLS_MODULAR_EXAMPLE_TOOL_H
#define COLUMNAR_EXAMPLE_TOOLS_MODULAR_EXAMPLE_TOOL_H

#include <AsgTools/AsgTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/ParticleDef.h>

namespace columnar
{
  /// @brief an example of a columnar tool with a modular structure
  ///
  /// This is a variation of the `SimpleSelectorExampleTool` that
  /// delegates funtionality to subobjects.  This is somewhat more
  /// complicated in the columnar world, as the subobjects need to be
  /// connected to the base class so that the accessors can be properly
  /// connected in columnar mode.
  ///
  /// This tool has two subobjects, to illustrate the two ways a
  /// subobject can be connected.  The first subobject is connected
  /// after it is set up, while the second subobject is connected in the
  /// constructor.  The second way is generally simpler, but the first
  /// way may align better with existing code.

  class ModularExampleTool final
    : public asg::AsgTool,
      public ColumnarTool<>
  {
  public:

    // Create a proper constructor for Athena
    ASG_TOOL_CLASS( ModularExampleTool, asg::IAsgTool )

    ModularExampleTool (const std::string& name);

    virtual StatusCode initialize () override;

    virtual void callEvents (EventContextRange events) const override;


    /// @brief the pt cut to apply
    Gaudi::Property<float> m_ptCut {this, "ptCut", 10e3, "pt cut (in MeV)"};

    /// @brief the eta cut to apply
    Gaudi::Property<float> m_etaCut {this, "etaCut", 2.47, "max eta cut"};


    /// @brief the object accessor for the particles
    ///
    /// This is equivalent to a `ReadHandleKey` in the xAOD world.  It
    /// is used to access the particle range/container for a given
    /// event.
    ParticleAccessor<ObjectColumn> particlesHandle {*this, "Particles"};


    /// @brief the selection decorator for the particles
    ///
    /// This is the equivalent to an `AuxElement::Decorator` in the xAOD
    /// world.  One thing to note is that the tools generally run on
    /// many objects, not a a single object.  So the tool doesn't have
    /// the option to return individual output values.  Instead it needs
    /// to provide an output value per object, which in the columnar
    /// world is done by filling a column.
    ParticleDecorator<char> selectionDec {*this, "selection"};


    /// @brief a simple subobject that does a selection on the pt
    ///
    /// This is a bit of a silly example, but many tools will have some
    /// subobjects that need accessors.
    struct SubtoolPt : public ColumnarTool<>
    {
      SubtoolPt (float val_cutValue);
      bool select (ParticleId particle) const;

      // this accessor isn't actually used, but it needs to be declared
      // so that in columnar mode the accessors know the name of the
      // object for each column.  it the subtool is connected before the
      // accessors get connected (i.e. via the base class constructor),
      // this isn't needed.
      ParticleAccessor<ObjectColumn> particlesHandle {*this, "Particles"};

      ParticleAccessor<float> ptAcc {*this, "pt"};
      float m_cutValue = 0;
    };
    std::unique_ptr<SubtoolPt> m_subtoolPt;




    /// @brief a simple subobject that does a selection on eta
    ///
    /// This is a bit of a silly example, but many tools will have some
    /// subobjects that need accessors.
    ///
    /// This class demonstrates taking the parent tool in the
    /// constructor, to avoid having to declare containers the parent
    /// tool already declared.
    struct SubtoolEta : public ColumnarTool<>
    {
      SubtoolEta (ColumnarTool<>* parent, float val_cutValue);
      bool select (ParticleId particle) const;

      ParticleAccessor<float> etaAcc {*this, "eta"};
      float m_cutValue = 0;
    };
    std::unique_ptr<SubtoolEta> m_subtoolEta;
  };
}

#endif

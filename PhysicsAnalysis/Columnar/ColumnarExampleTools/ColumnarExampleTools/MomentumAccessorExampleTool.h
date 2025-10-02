/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_EXAMPLE_TOOLS_MOMENTUM_ACCESSOR_EXAMPLE_TOOL_H
#define COLUMNAR_EXAMPLE_TOOLS_MOMENTUM_ACCESSOR_EXAMPLE_TOOL_H

#include <AsgTools/AsgTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarCore/MomentumAccessors.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/ObjectTypeAccessor.h>
#include <ColumnarCore/ParticleDef.h>

namespace columnar
{
  /// @brief an example of a columnar tool that uses momentum accessors
  ///
  /// This is a variation of the `SimpleSelectorExampleTool` but it uses
  /// the momentum accessors to access the energy of the particles,
  /// instead of using column accessors to read variables directly from
  /// the file.

  class MomentumAccessorExampleTool final
    : public asg::AsgTool,
      public ColumnarTool<>
  {
  public:

    // Create a proper constructor for Athena
    ASG_TOOL_CLASS( MomentumAccessorExampleTool, asg::IAsgTool )

    MomentumAccessorExampleTool (const std::string& name);

    virtual StatusCode initialize () override;

    void callSingleEvent (ParticleRange particles) const;

    virtual void callEvents (EventContextRange events) const override;


    /// @brief the energy cut to apply
    Gaudi::Property<float> m_energyCut {this, "energyCut", 10e3, "energy cut (in MeV)"};


    /// @brief the object accessor for the particles
    ///
    /// This is equivalent to a `ReadHandleKey` in the xAOD world.  It
    /// is used to access the particle range/container for a given
    /// event.
    ParticleAccessor<ObjectColumn> particlesHandle {*this, "Particles"};


    /// @brief the momentum accessors for the particle container
    ///
    /// This is an accessor that provides access to all the available
    /// momentum variables and can be reconfigured at configuration time
    /// to different momentum accessors.
    MomentumAccessors<ContainerId::particle> momAcc;

    /// @brief the object type accessor for the particle container
    ///
    /// This is a bit of a hybrid between a property accessor and a
    /// column accessor, as it needs to be able to access the property
    /// type at configuration time (to set the correct momentum
    /// accessor).
    ObjectTypeAccessor<ContainerId::particle> objectTypeAcc {*this, "ObjectType", "the object type of the particles"};

    // If you want to use a statically configured momentum accessor,
    // this would be the basic way to do it. For now (24 Jul 25) I don't
    // think this is worth the effort, as the dynamic momentum accessors
    // seem to be working pretty well.
    // Detail::FullMomentumAccessorsPtEtaPhiM<Detail::CoreMomentumAccessorsPtEtaPhiReadM<ContainerId::particle,ColumnarModeDefault>> momAcc {*this};


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

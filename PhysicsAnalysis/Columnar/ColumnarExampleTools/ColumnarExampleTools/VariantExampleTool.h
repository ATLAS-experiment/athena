/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_EXAMPLE_TOOLS_VARIANT_EXAMPLE_TOOL_H
#define COLUMNAR_EXAMPLE_TOOLS_VARIANT_EXAMPLE_TOOL_H

#include <AsgTools/AsgTool.h>
#include <AsgTools/PropertyWrapper.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/ColumnarTool.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/ParticleDef.h>
#include <ColumnarVariant/VariantDef.h>
#include <ColumnarVariant/VariantObjectId.h>
#include <ColumnarVariant/VariantAccessor.h>
#include <ColumnarEgamma/EgammaDef.h>
#include <ColumnarMuon/MuonDef.h>

namespace columnar
{
  /// @brief an example of a columnar tool using "variant" object ids
  /// and column accessors
  ///
  /// A "variant" object id is one that is not tied to a single
  /// container in the store, but can refer to an object in one of
  /// several containers. This pairs with "variant" column accessors
  /// that can then look up the correct column in any of these N
  /// containers.
  ///
  /// If you come from the xAOD world, this is essentially the default
  /// behavior there, because all data is accessed indirectly through
  /// the aux-store, meaning any data access automatically goes to the
  /// correct container.
  ///
  /// For columnar code a lot more has to happen to make this possible,
  /// and it comes with a number of limitations. As such it should only
  /// be used when it is actually needed, i.e. when not using it would
  /// mean you implement similar functionality in your own code.

  class VariantExampleTool final
    : public asg::AsgTool,
      public ColumnarTool<>
  {
  public:

    // Create a proper constructor for Athena
    ASG_TOOL_CLASS( VariantExampleTool, asg::IAsgTool )

    VariantExampleTool (const std::string& name);

    virtual StatusCode initialize () override;

    void callSingleEvent (ElectronRange electrons, MuonRange muons) const;

    virtual void callEvents (EventContextRange events) const override;


    /// @brief the object accessor for the underlying containers
    ///
    /// This is equivalent to a `ReadHandleKey` in the xAOD world.  It
    /// is used to access the range/container for a given event.
    ElectronAccessor<ObjectColumn> electronsHandle {*this, "AnalysisElectrons"};
    MuonAccessor<ObjectColumn> muonsHandle {*this, "AnalysisMuons"};


    /// @brief the variant definition we are using
    ///
    /// For use of variants in columnar code, we need to create a
    /// specific variant definition that lists all the containers we
    /// want to use this with. Please note that the first container
    /// listed is not a "real" container, but it is used for defining a
    /// base type/container that is used in xAOD mode for the internal
    /// pointer. If you also want to use it as a possible "variant"
    /// container id, you need to list it twice.
    using MyVariantDef = VariantContainerId<ContainerId::particle, ContainerId::electron, ContainerId::muon>;


    /// @brief the pt and eta accessors for the variant container
    ///
    /// This works like a "regular" ColumnAccessor, but under the hood
    /// (in columnar mode) it creates a separate column accessor for
    /// each "variant" container and picks the correct one at runtime.
    /// This is really the main drawback of "variant" accessors: you get
    /// a large number of input columns very quickly, particularly if
    /// you have many "variant" containers in your variant definitions.
    ///
    /// Note that not every accessor needs to be a "variant" accessor.
    /// You can also convert the "variant" object id to a "regular"
    /// object id and use it directly.
    ColumnAccessor<MyVariantDef,float> ptAcc {*this, "pt"};
    ColumnAccessor<MyVariantDef,float> etaAcc {*this, "eta"};


    /// @brief the pt-rank decorator for the variant container
    ///
    /// Just like accessors, we can have decorators for our variant
    /// container as well.
    ColumnDecorator<MyVariantDef,std::uint16_t> ptRankDec {*this, "ptRank"};

    /// @brief a eta-rank decorator just for electrons
    ///
    /// this is to show how you can have accessors/decorators for just
    /// one of the contained "variants".
    ColumnDecorator<ContainerId::electron,std::uint16_t> etaRankSpecialDec {*this, "etaRank"};
  };
}

#endif

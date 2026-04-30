/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_MET_MET_DEF_H
#define COLUMNAR_MET_MET_DEF_H

#include <ColumnarCore/ContainerId.h>
#include <xAODBase/IParticleContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include <xAODMissingET/MissingETAssociationMap.h>

namespace columnar
{
  namespace MetHelperDefs
  {
    using iplink_t = ElementLink<xAOD::IParticleContainer>;

    static const SG::AuxElement::ConstAccessor< iplink_t  > acc_originalObject("originalObjectLink");
    static const SG::AuxElement::ConstAccessor< iplink_t  > acc_nominalObject("nominalObjectLink");

    static const SG::AuxElement::Accessor< std::vector<iplink_t> > dec_constitObjLinks("ConstitObjectLinks");
    static const SG::AuxElement::Accessor< std::vector<float> > dec_constitObjWeights("ConstitObjectWeights");
  }

  struct MetDef : RegularContainerId<xAOD::MissingET,xAOD::MissingETContainer>
  {
    static constexpr std::string_view idName = "met";
  };
  using Met0Def = MetDef;
  using MutableMetDef = MutableContainerId<MetDef>;

  struct Met1Def : MetDef
  {
    static constexpr std::string_view idName = "met1";
  };

  struct MetAssociationDef : RegularContainerId<xAOD::MissingETAssociation,xAOD::MissingETAssociationMap>
  {
    static constexpr std::string_view idName = "metAssociation";
  };

  using MetRange = ObjectRange<MetDef>;
  using MetId = ObjectId<MetDef>;
  using OptMetId = OptObjectId<MetDef>;
  template<typename CT,typename CM=ColumnarModeDefault> using MetAccessor  = AccessorTemplate<MetDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using MetDecorator = AccessorTemplate<MetDef,CT,ColumnAccessMode::output,CM>;

  using Met0Range = ObjectRange<Met0Def>;
  using Met0Id = ObjectId<Met0Def>;
  using OptMet0Id = OptObjectId<Met0Def>;
  template<typename CT,typename CM=ColumnarModeDefault> using Met0Accessor  = AccessorTemplate<Met0Def,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Met0Decorator = AccessorTemplate<Met0Def,CT,ColumnAccessMode::output,CM>;

  using Met1Range = ObjectRange<Met1Def>;
  using Met1Id = ObjectId<Met1Def>;
  using OptMet1Id = OptObjectId<Met1Def>;
  template<typename CT,typename CM=ColumnarModeDefault> using Met1Accessor  = AccessorTemplate<Met1Def,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Met1Decorator = AccessorTemplate<Met1Def,CT,ColumnAccessMode::output,CM>;

  using MutableMetRange = ObjectRange<MutableMetDef>;
  using MutableMetId = ObjectId<MutableMetDef>;
  using OptMutableMetId = OptObjectId<MutableMetDef>;
  template<typename CT,typename CM=ColumnarModeDefault> using MutableMetAccessor  = AccessorTemplate<MutableMetDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using MutableMetDecorator = AccessorTemplate<MutableMetDef,CT,ColumnAccessMode::output,CM>;

  using MetAssociationRange = ObjectRange<MetAssociationDef>;
  using MetAssociationId = ObjectId<MetAssociationDef>;
  using OptMetAssociationId = OptObjectId<MetAssociationDef>;
}

#endif

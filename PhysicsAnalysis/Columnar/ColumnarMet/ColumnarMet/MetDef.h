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

  template<ColumnarMode CM> using MetRange = ObjectRange<MetDef,CM>;
  template<ColumnarMode CM> using MetId = ObjectId<MetDef,CM>;
  template<ColumnarMode CM> using OptMetId = OptObjectId<MetDef,CM>;
  template<typename CT,ColumnarMode CM> using MetAccessor  = AccessorTemplate<MetDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,ColumnarMode CM> using MetDecorator = AccessorTemplate<MetDef,CT,ColumnAccessMode::output,CM>;

  template<ColumnarMode CM> using Met0Range = ObjectRange<Met0Def,CM>;
  template<ColumnarMode CM> using Met0Id = ObjectId<Met0Def,CM>;
  template<ColumnarMode CM> using OptMet0Id = OptObjectId<Met0Def,CM>;
  template<typename CT,ColumnarMode CM> using Met0Accessor  = AccessorTemplate<Met0Def,CT,ColumnAccessMode::input,CM>;
  template<typename CT,ColumnarMode CM> using Met0Decorator = AccessorTemplate<Met0Def,CT,ColumnAccessMode::output,CM>;

  template<ColumnarMode CM> using Met1Range = ObjectRange<Met1Def,CM>;
  template<ColumnarMode CM> using Met1Id = ObjectId<Met1Def,CM>;
  template<ColumnarMode CM> using OptMet1Id = OptObjectId<Met1Def,CM>;
  template<typename CT,ColumnarMode CM> using Met1Accessor  = AccessorTemplate<Met1Def,CT,ColumnAccessMode::input,CM>;
  template<typename CT,ColumnarMode CM> using Met1Decorator = AccessorTemplate<Met1Def,CT,ColumnAccessMode::output,CM>;

  template<ColumnarMode CM> using MutableMetRange = ObjectRange<MutableMetDef,CM>;
  template<ColumnarMode CM> using MutableMetId = ObjectId<MutableMetDef,CM>;
  template<ColumnarMode CM> using OptMutableMetId = OptObjectId<MutableMetDef,CM>;
  template<typename CT,ColumnarMode CM> using MutableMetAccessor  = AccessorTemplate<MutableMetDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,ColumnarMode CM> using MutableMetDecorator = AccessorTemplate<MutableMetDef,CT,ColumnAccessMode::output,CM>;

  template<ColumnarMode CM> using MetAssociationRange = ObjectRange<MetAssociationDef,CM>;
  template<ColumnarMode CM> using MetAssociationId = ObjectId<MetAssociationDef,CM>;
  template<ColumnarMode CM> using OptMetAssociationId = OptObjectId<MetAssociationDef,CM>;
}

#endif

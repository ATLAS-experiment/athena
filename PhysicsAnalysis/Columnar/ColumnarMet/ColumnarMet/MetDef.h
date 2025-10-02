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
  namespace MetDef
  {
    using iplink_t = ElementLink<xAOD::IParticleContainer>;

    static const SG::AuxElement::ConstAccessor< iplink_t  > acc_originalObject("originalObjectLink");
    static const SG::AuxElement::ConstAccessor< iplink_t  > acc_nominalObject("nominalObjectLink");

    static const SG::AuxElement::Accessor< std::vector<iplink_t> > dec_constitObjLinks("ConstitObjectLinks");
    static const SG::AuxElement::Accessor< std::vector<float> > dec_constitObjWeights("ConstitObjectWeights");
  }

  namespace ContainerId
  {
    struct met : regularCIBase<xAOD::MissingET,xAOD::MissingETContainer>
    {
      static constexpr std::string_view idName = "met";
    };
    using met0 = met;
    using mutableMet = mutableCI<met>;

    struct met1 : met
    {
      static constexpr std::string_view idName = "met1";
    };

    struct metAssociation : regularCIBase<xAOD::MissingETAssociation,xAOD::MissingETAssociationMap>
    {
      static constexpr std::string_view idName = "metAssociation";
    };
  }

  using MetRange = ObjectRange<ContainerId::met>;
  using MetId = ObjectId<ContainerId::met>;
  using OptMetId = OptObjectId<ContainerId::met>;
  template<typename CT,typename CM=ColumnarModeDefault> using MetAccessor  = AccessorTemplate<ContainerId::met,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using MetDecorator = AccessorTemplate<ContainerId::met,CT,ColumnAccessMode::output,CM>;

  using Met0Range = ObjectRange<ContainerId::met0>;
  using Met0Id = ObjectId<ContainerId::met0>;
  using OptMet0Id = OptObjectId<ContainerId::met0>;
  template<typename CT,typename CM=ColumnarModeDefault> using Met0Accessor  = AccessorTemplate<ContainerId::met0,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Met0Decorator = AccessorTemplate<ContainerId::met0,CT,ColumnAccessMode::output,CM>;

  using Met1Range = ObjectRange<ContainerId::met1>;
  using Met1Id = ObjectId<ContainerId::met1>;
  using OptMet1Id = OptObjectId<ContainerId::met1>;
  template<typename CT,typename CM=ColumnarModeDefault> using Met1Accessor  = AccessorTemplate<ContainerId::met1,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Met1Decorator = AccessorTemplate<ContainerId::met1,CT,ColumnAccessMode::output,CM>;

  using MutableMetRange = ObjectRange<ContainerId::mutableMet>;
  using MutableMetId = ObjectId<ContainerId::mutableMet>;
  using OptMutableMetId = OptObjectId<ContainerId::mutableMet>;
  template<typename CT,typename CM=ColumnarModeDefault> using MutableMetAccessor  = AccessorTemplate<ContainerId::mutableMet,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using MutableMetDecorator = AccessorTemplate<ContainerId::mutableMet,CT,ColumnAccessMode::output,CM>;

  using MetAssociationRange = ObjectRange<ContainerId::metAssociation>;
  using MetAssociationId = ObjectId<ContainerId::metAssociation>;
  using OptMetAssociationId = OptObjectId<ContainerId::metAssociation>;
}

#endif

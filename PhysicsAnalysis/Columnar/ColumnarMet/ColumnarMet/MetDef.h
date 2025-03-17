/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_MET_MET_DEF_H
#define COLUMNAR_MET_MET_DEF_H

#include <ColumnarCore/ContainerId.h>
#include <xAODBase/IParticleContainer.h>

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

  template<ContainerId CI> requires (CI == ContainerId::met0 || CI == ContainerId::met1)
  struct ContainerIdTraits<CI> final
  {
    static constexpr bool isDefined = true;
    static constexpr bool isMutable = false;
    static constexpr bool perEventRange = true;
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = const xAOD::MissingET;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = const xAOD::MissingETContainer;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = xAOD::MissingETContainer;
  };

  template<>
  struct ContainerIdTraits<ContainerId::mutableMet> final
  {
    static constexpr bool isDefined = true;
    static constexpr bool isMutable = true;
    static constexpr ContainerId constId = ContainerId::met;
    static constexpr bool perEventRange = true;
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = xAOD::MissingET;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = xAOD::MissingETContainer;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = xAOD::MissingETContainer;
  };

  template<>
  struct ContainerIdTraits<ContainerId::metAssociation> final
  {
    static constexpr bool isDefined = true;
    static constexpr bool isMutable = false;
    static constexpr bool perEventRange = true;
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = const xAOD::MissingETAssociation;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = const xAOD::MissingETAssociationMap;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = xAOD::MissingETAssociationMap;
  };
}

#endif

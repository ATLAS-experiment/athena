/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_EGAMMA_EGAMMA_DEF_H
#define COLUMNAR_EGAMMA_EGAMMA_DEF_H

#include <ColumnarCore/ContainerId.h>
#include <xAODEgamma/EgammaContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/PhotonContainer.h>

namespace columnar
{
  template<> struct ContainerIdTraits<ContainerId::electron> final
  {
    static constexpr bool isDefined = true;
    static constexpr bool isMutable = false;
    static constexpr bool perEventRange = true;
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = const xAOD::Electron;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = const xAOD::ElectronContainer;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = xAOD::ElectronContainer;
  };

  template<> struct ContainerIdTraits<ContainerId::photon> final
  {
    static constexpr bool isDefined = true;
    static constexpr bool isMutable = false;
    static constexpr bool perEventRange = true;
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = const xAOD::Photon;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = const xAOD::PhotonContainer;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = xAOD::PhotonContainer;
  };

  template<> struct ContainerIdTraits<ContainerId::egamma> final
  {
    static constexpr bool isDefined = true;
    static constexpr bool isMutable = false;
    static constexpr bool perEventRange = true;
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = const xAOD::Egamma;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = const xAOD::EgammaContainer;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = xAOD::EgammaContainer;
  };

  template<> struct ContainerIdTraits<ContainerId::mutableEgamma> final
  {
    static constexpr bool isDefined = true;
    static constexpr bool isMutable = true;
    static constexpr ContainerId constId = ContainerId::egamma;
    static constexpr bool perEventRange = true;
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = xAOD::Egamma;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = xAOD::EgammaContainer;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = xAOD::EgammaContainer;
  };
}

#endif

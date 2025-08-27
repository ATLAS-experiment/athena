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
  namespace ContainerId
  {
    struct electron : regularCIBase<xAOD::Electron,xAOD::ElectronContainer>
    {
      static constexpr std::string_view idName = "electron";
    };

    struct photon : regularCIBase<xAOD::Photon,xAOD::PhotonContainer>
    {
      static constexpr std::string_view idName = "photon";
    };

    struct egamma : regularCIBase<xAOD::Egamma,xAOD::EgammaContainer>
    {
      static constexpr std::string_view idName = "egamma";
    };
    using mutableEgamma = mutableCI<egamma>;
  }

  using ElectronRange = ObjectRange<ContainerId::electron>;
  using ElectronId = ObjectId<ContainerId::electron>;
  using OptElectronId = OptObjectId<ContainerId::electron>;
  template<typename CT,typename CM=ColumnarModeDefault> using ElectronAccessor  = AccessorTemplate<ContainerId::electron,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using ElectronDecorator = AccessorTemplate<ContainerId::electron,CT,ColumnAccessMode::output,CM>;

  using PhotonRange = ObjectRange<ContainerId::photon>;
  using PhotonId = ObjectId<ContainerId::photon>;
  using OptPhotonId = OptObjectId<ContainerId::photon>;
  template<typename CT,typename CM=ColumnarModeDefault> using PhotonAccessor  = AccessorTemplate<ContainerId::photon,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using PhotonDecorator = AccessorTemplate<ContainerId::photon,CT,ColumnAccessMode::output,CM>;

  using EgammaRange = ObjectRange<ContainerId::egamma>;
  using EgammaId = ObjectId<ContainerId::egamma>;
  using OptEgammaId = OptObjectId<ContainerId::egamma>;
  template<typename CT,typename CM=ColumnarModeDefault> using EgammaAccessor  = AccessorTemplate<ContainerId::egamma,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using EgammaDecorator = AccessorTemplate<ContainerId::egamma,CT,ColumnAccessMode::output,CM>;

  using MutableEgammaRange = ObjectRange<ContainerId::mutableEgamma>;
  using MutableEgammaId = ObjectId<ContainerId::mutableEgamma>;
  using OptMutableEgammaId = OptObjectId<ContainerId::mutableEgamma>;
  template<typename CT,typename CM=ColumnarModeDefault> using MutableEgammaAccessor  = AccessorTemplate<ContainerId::mutableEgamma,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using MutableEgammaDecorator = AccessorTemplate<ContainerId::mutableEgamma,CT,ColumnAccessMode::output,CM>;
}

#endif

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
  struct ElectronDef : RegularContainerId<xAOD::Electron,xAOD::ElectronContainer>
  {
    static constexpr std::string_view idName = "electron";
  };

  struct PhotonDef : RegularContainerId<xAOD::Photon,xAOD::PhotonContainer>
  {
    static constexpr std::string_view idName = "photon";
  };

  struct EgammaDef : RegularContainerId<xAOD::Egamma,xAOD::EgammaContainer>
  {
    static constexpr std::string_view idName = "egamma";
  };
  using MutableEgammaDef = MutableContainerId<EgammaDef>;

  template<ColumnarMode CM> using ElectronRange = ObjectRange<ElectronDef,CM>;
  template<ColumnarMode CM> using ElectronId = ObjectId<ElectronDef,CM>;
  template<ColumnarMode CM> using OptElectronId = OptObjectId<ElectronDef,CM>;
  template<typename CT, ColumnarMode CM> using ElectronAccessor  = AccessorTemplate<ElectronDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT, ColumnarMode CM> using ElectronDecorator = AccessorTemplate<ElectronDef,CT,ColumnAccessMode::output,CM>;

  template<ColumnarMode CM> using PhotonRange = ObjectRange<PhotonDef,CM>;
  template<ColumnarMode CM> using PhotonId = ObjectId<PhotonDef,CM>;
  template<ColumnarMode CM> using OptPhotonId = OptObjectId<PhotonDef,CM>;
  template<typename CT, ColumnarMode CM> using PhotonAccessor  = AccessorTemplate<PhotonDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT, ColumnarMode CM> using PhotonDecorator = AccessorTemplate<PhotonDef,CT,ColumnAccessMode::output,CM>;

  template<ColumnarMode CM> using EgammaRange = ObjectRange<EgammaDef,CM>;
  template<ColumnarMode CM> using EgammaId = ObjectId<EgammaDef,CM>;
  template<ColumnarMode CM> using OptEgammaId = OptObjectId<EgammaDef,CM>;
  template<typename CT, ColumnarMode CM> using EgammaAccessor  = AccessorTemplate<EgammaDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT, ColumnarMode CM> using EgammaDecorator = AccessorTemplate<EgammaDef,CT,ColumnAccessMode::output,CM>;

  template<ColumnarMode CM> using MutableEgammaRange = ObjectRange<MutableEgammaDef,CM>;
  template<ColumnarMode CM> using MutableEgammaId = ObjectId<MutableEgammaDef,CM>;
  template<ColumnarMode CM> using OptMutableEgammaId = OptObjectId<MutableEgammaDef,CM>;
  template<typename CT, ColumnarMode CM> using MutableEgammaAccessor  = AccessorTemplate<MutableEgammaDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT, ColumnarMode CM> using MutableEgammaDecorator = AccessorTemplate<MutableEgammaDef,CT,ColumnAccessMode::output,CM>;
}

#endif

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

  using ElectronRange = ObjectRange<ElectronDef, ColumnarModeDefault>;
  using ElectronId = ObjectId<ElectronDef>;
  using OptElectronId = OptObjectId<ElectronDef>;
  template<typename CT,typename CM=ColumnarModeDefault> using ElectronAccessor  = AccessorTemplate<ElectronDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using ElectronDecorator = AccessorTemplate<ElectronDef,CT,ColumnAccessMode::output,CM>;

  using PhotonRange = ObjectRange<PhotonDef, ColumnarModeDefault>;
  using PhotonId = ObjectId<PhotonDef>;
  using OptPhotonId = OptObjectId<PhotonDef>;
  template<typename CT,typename CM=ColumnarModeDefault> using PhotonAccessor  = AccessorTemplate<PhotonDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using PhotonDecorator = AccessorTemplate<PhotonDef,CT,ColumnAccessMode::output,CM>;

  using EgammaRange = ObjectRange<EgammaDef, ColumnarModeDefault>;
  using EgammaId = ObjectId<EgammaDef>;
  using OptEgammaId = OptObjectId<EgammaDef>;
  template<typename CT,typename CM=ColumnarModeDefault> using EgammaAccessor  = AccessorTemplate<EgammaDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using EgammaDecorator = AccessorTemplate<EgammaDef,CT,ColumnAccessMode::output,CM>;

  using MutableEgammaRange = ObjectRange<MutableEgammaDef, ColumnarModeDefault>;
  using MutableEgammaId = ObjectId<MutableEgammaDef>;
  using OptMutableEgammaId = OptObjectId<MutableEgammaDef>;
  template<typename CT,typename CM=ColumnarModeDefault> using MutableEgammaAccessor  = AccessorTemplate<MutableEgammaDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using MutableEgammaDecorator = AccessorTemplate<MutableEgammaDef,CT,ColumnAccessMode::output,CM>;
}

#endif

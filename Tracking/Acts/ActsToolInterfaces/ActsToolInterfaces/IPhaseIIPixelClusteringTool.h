/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IPIXELPIXELCLUSTERINGTOOLPHASEII_H
#define ACTSTOOLINTERFACES_IPIXELPIXELCLUSTERINGTOOLPHASEII_H

#include <InDetIdentifier/PixelID.h>
#include "InDetRawData/PhaseIIPixelRawDataContainer.h"
#include <xAODInDetMeasurement/PixelClusterContainer.h>
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "ICellClusteringToolBase.h"
#include <any>


namespace ActsTrk {

class IPhaseIIPixelClusteringTool;
template <>
struct RDOContainerTraits<PhaseIIPixelRawDataContainer> {
   using PerModuleRDOs = PhaseII::PixelRawDataTypeTraits<>::RawDataContainerProxy;
   using IClusteringToolType = IPhaseIIPixelClusteringTool;
};

// forward declaration (defintion  Tracking/Acts/ActsDataPreparation/src/details/CellContainer.h)
template <typename coordinates_t, std::size_t NDIM, std::unsigned_integral index_t>
struct CellContainer;

class IPhaseIIPixelClusteringTool : public ICellClusteringToolBase<PhaseIIPixelRawDataContainer,xAOD::PixelClusterContainer, 2> {
public:
    DeclareInterfaceID(IPhaseIIPixelClusteringTool, 1, 0);

    using IDHelper = PixelID;
    using ClusterAuxContainer = xAOD::PixelClusterAuxContainer;

 };

}

#endif

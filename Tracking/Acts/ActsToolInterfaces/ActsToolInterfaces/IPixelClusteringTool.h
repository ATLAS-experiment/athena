/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IPIXELPIXELCLUSTERINGTOOL_H
#define ACTSTOOLINTERFACES_IPIXELPIXELCLUSTERINGTOOL_H

#include <InDetIdentifier/PixelID.h>
#include <InDetRawData/InDetRawDataCollection.h>
#include <InDetRawData/PixelRDO_Container.h>
#include <InDetRawData/PixelRDORawData.h>
#include <xAODInDetMeasurement/PixelClusterContainer.h>
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "ICellClusteringToolBase.h"
#include <any>


namespace ActsTrk {

// forward declaration (defintion  Tracking/Acts/ActsDataPreparation/src/details/CellContainer.h)
template <typename coordinates_t, std::size_t NDIM, std::unsigned_integral index_t>
struct CellContainer;

class IPixelClusteringTool : public ICellClusteringToolBase<PixelRDO_Container,xAOD::PixelClusterContainer, 2> {
public:
    DeclareInterfaceID(IPixelClusteringTool, 1, 0);

    using IDHelper = PixelID;
    using ClusterAuxContainer = xAOD::PixelClusterAuxContainer;

 };

}

#endif

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef PixelRDOContainerTraits_H
#define PixelRDOContainerTraits_H

#include "ActsToolInterfaces/IPixelClusteringTool.h"
#include "ActsToolInterfaces/IPhaseIIPixelClusteringTool.h"
#include "InDetRawData/PhaseIIPixelRawDataContainer.h"

template <typename T_RDOContainer>
struct PixelRDOContainerTraits;

template <>
struct PixelRDOContainerTraits<PixelRDO_Container> {
   using PerModuleRDOs = InDetRawDataCollection<PixelRDORawData>;
   using IPixelClusteringToolType = ActsTrk::IPixelClusteringTool;
};

template <>
struct PixelRDOContainerTraits<PhaseIIPixelRawDataContainer> {
   using PerModuleRDOs = PhaseII::PixelRawDataTypeTraits<>::RawDataContainerProxy;
   using IPixelClusteringToolType = ActsTrk::IPhaseIIPixelClusteringTool;
};

#endif

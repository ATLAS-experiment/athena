/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_ISTRIPSTRIPCLUSTERINGTOOL_H
#define ACTSTOOLINTERFACES_ISTRIPSTRIPCLUSTERINGTOOL_H

#include <InDetIdentifier/SCT_ID.h>
#include <InDetRawData/InDetRawDataCollection.h>
#include <InDetRawData/SCT_RDO_Container.h>
#include <InDetRawData/SCT_RDORawData.h>
#include <xAODInDetMeasurement/StripClusterContainer.h>
#include <xAODInDetMeasurement/StripClusterAuxContainer.h>
#include "ICellClusteringToolBase.h"

namespace ActsTrk {

class IStripClusteringTool : public ICellClusteringToolBase<SCT_RDO_Container,xAOD::StripClusterContainer, 1> {
public:
  DeclareInterfaceID(IStripClusteringTool, 1, 0);

  using IDHelper = SCT_ID;
  using ClusterAuxContainer = xAOD::StripClusterAuxContainer;
};
  
} // namespace

#endif

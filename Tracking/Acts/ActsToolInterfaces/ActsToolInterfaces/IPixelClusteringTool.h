/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IPIXELPIXELCLUSTERINGTOOL_H
#define ACTSTOOLINTERFACES_IPIXELPIXELCLUSTERINGTOOL_H

#include <GaudiKernel/IAlgTool.h>
#include <InDetIdentifier/PixelID.h>
#include <InDetRawData/InDetRawDataCollection.h>
#include <InDetRawData/PixelRDO_Container.h>
#include <InDetRawData/PixelRDORawData.h>
#include <xAODInDetMeasurement/PixelClusterContainer.h>
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "InDetReadoutGeometry/SiDetectorElementStatus.h"

namespace ActsTrk {

class IPixelClusteringTool : virtual public IAlgTool {
public:
    DeclareInterfaceID(IPixelClusteringTool, 1, 0);

    using RDOContainer = PixelRDO_Container;
    using RawDataCollection = RDOContainer::base_value_type;
    using IDHelper = PixelID;
    using ClusterContainer = xAOD::PixelClusterContainer;
    using ClusterAuxContainer = xAOD::PixelClusterAuxContainer;

    struct Cell {
      Cell(int row, int col, int tot, int lvl1, Identifier::value_type id):
        ROW(row), COL(col), TOT(tot), LVL1(lvl1), ID(id) {};
      
      int           ROW;
      int           COL;
      int           TOT;
      int           LVL1;
      Identifier::value_type    ID ;
    };

    using CellCollection = std::vector<Cell>;

    struct Cluster {
        std::vector<Identifier::value_type> ids;
        std::vector<int> tots;
        int lvl1min = std::numeric_limits<int>::max();
    };
    using ClusterCollection = std::vector<Cluster>;

    virtual StatusCode
    clusterize(const EventContext& ctx,
	       const RawDataCollection& RDOs,
	       const InDet::SiDetectorElementStatus& pixelDetElStatus,
	       const InDetDD::SiDetectorElement& element,
	       std::vector<ClusterCollection>& collection) const = 0;
  
    virtual StatusCode
    makeClusters(const EventContext& ctx,
		 ClusterCollection& cluster,
		 const InDetDD::SiDetectorElement& element,
		 typename ClusterContainer::iterator itrContainer) const = 0;
};

}

#endif

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_ISTRIPSTRIPCLUSTERINGTOOL_H
#define ACTSTOOLINTERFACES_ISTRIPSTRIPCLUSTERINGTOOL_H

#include <GaudiKernel/IAlgTool.h>
#include <InDetIdentifier/SCT_ID.h>
#include <InDetRawData/InDetRawDataCollection.h>
#include <InDetRawData/SCT_RDO_Container.h>
#include <InDetRawData/SCT_RDORawData.h>
#include <InDetReadoutGeometry/SiDetectorElement.h>
#include <InDetReadoutGeometry/SiDetectorElementStatus.h>
#include <xAODInDetMeasurement/StripClusterContainer.h>
#include <xAODInDetMeasurement/StripClusterAuxContainer.h>
#include <Acts/Clusterization/Clusterization.hpp>

namespace ActsTrk {

class IStripClusteringTool : virtual public IAlgTool {
public:
  DeclareInterfaceID(IStripClusteringTool, 1, 0);

  using RDOContainer = SCT_RDO_Container;
  using RawDataCollection = RDOContainer::base_value_type;
  using IDHelper = SCT_ID;
  using ClusterContainer = xAOD::StripClusterContainer;
  using ClusterAuxContainer = xAOD::StripClusterAuxContainer;
  
  struct Cell {
    size_t index;
    Identifier id;
    std::bitset<3> timeBits;
    
    Cell(size_t i, Identifier id, const std::bitset<3>& timeBits)
      : index(i), id(id), timeBits(timeBits) {}
  };  
  using CellCollection = std::vector<Cell>;
  
  struct Cluster {
    std::vector<Identifier::value_type> ids;
    uint16_t hitsInThirdTimeBin{0};
  };  
  using ClusterCollection = std::vector<Cluster>;
  
  virtual StatusCode
  clusterize(const EventContext& ctx,
	     const RawDataCollection& RDOs,
	     const InDet::SiDetectorElementStatus& stripDetElStatus,
	     const InDetDD::SiDetectorElement& element,
       Acts::Ccl::ClusteringData& data,
	     std::vector<ClusterCollection>& collection) const = 0;
  
  virtual StatusCode
  makeClusters(const EventContext& ctx,
	       ClusterCollection& cluster,
	       const InDetDD::SiDetectorElement& element,
	       typename ClusterContainer::iterator itrContainer) const = 0;
};
  
} // namespace

#endif

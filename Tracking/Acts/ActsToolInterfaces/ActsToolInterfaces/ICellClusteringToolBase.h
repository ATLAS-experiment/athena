/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_ICELLCLUSTERINGTOOLBASE_H
#define ACTSTOOLINTERFACES_ICELLCLUSTERINGTOOLBASE_H

#include <GaudiKernel/IAlgTool.h>
#include <InDetIdentifier/SCT_ID.h>
#include <InDetReadoutGeometry/SiDetectorElement.h>
#include <InDetReadoutGeometry/SiDetectorElementStatus.h>
#include <any>
#include <vector>
#include <utility>

namespace ActsTrk {

// forward declaration (defintion  Tracking/Acts/ActsDataPreparation/src/details/CellContainer.h)
template <typename coordinates_t, std::size_t NDIM, std::unsigned_integral index_t>
struct CellContainer;

template <typename T_RDO_Container, typename T_OutputContainer, std::size_t NDIM, std::integral coordinate_t=std::int16_t>
class ICellClusteringToolBase : virtual public IAlgTool {
public:
  using RDOContainer = T_RDO_Container;
  using RawDataCollection = typename RDOContainer::base_value_type;
  using ClusterContainer = T_OutputContainer;

  using CellContainer = ActsTrk::CellContainer<coordinate_t,NDIM,std::uint16_t>;

  /// @brief count the number of cells and expected number of clusters for the given RDO collection.
  /// @return a pair of the number of expected clusters, and the number of cells.
  /// These numbers will be used to reserve storage. Thus, too small numbers will likely introduce
  /// in-efficiencies caused by memory reallocation and copying, and too large numbers will
  /// increase temporary memory consumption.
  virtual std::pair<unsigned int, unsigned int>
  countCells(const RDOContainer& rdo_collection,
             const std::vector<IdentifierHash> &listOfIds,
             const InDetDD::SiDetectorElementCollection &detector_elements) const =0;

  /// @brief clusterize the given RDOs.
  /// Will cluster the given RDOs. The result will be in the provided cell container, in which
  /// cells are sorted such that clusters are consecutive groups of cells.
  virtual StatusCode
  clusterize(const EventContext& ctx,
             const RawDataCollection& RDOs,
             const InDet::SiDetectorElementStatus& stripDetElStatus,
             const InDetDD::SiDetectorElement& element,
             CellContainer &cell_container) const = 0;

  /// @brief Create a per event cache which can be used to speed up the construction of the final cluster collection
  /// @param nClusterRDOs the total number of all RDOs/cells of all clusters.
  /// @return the cache expected by @ref makeClusters packed in an any.
  virtual std::any createEventDataCache(ClusterContainer& cont,
                                        std::size_t nClusterRDOs) const = 0;

  /// @brief create the final cluster collection based on the temporary cluster collection.
  /// @param rdo_container the RDO container
  /// @param cell_container the result of @ref clusterize
  /// @param module_i the index of the module for which the clusters are to be created.
  /// @param element the detector element of this module.
  /// @param icluster the index of the cluster in the output collection.
  /// @param cont the cluster container.
  /// @param vars the event data cache created by @ref createEventDataCache
  /// Will create the final cluster collection from the clustered cell container. The
  /// Final cluster container is expected to contain the exact amount of uninitialized
  /// clusters.
  virtual StatusCode
  makeClusters(const EventContext& ctx,
               const RDOContainer &rdo_container,
               const CellContainer& cell_container,
               unsigned int module_i,
               const InDetDD::SiDetectorElement& element,
               unsigned int icluster,
               ClusterContainer& cont,
               std::any& vars) const = 0;
};
}
#endif

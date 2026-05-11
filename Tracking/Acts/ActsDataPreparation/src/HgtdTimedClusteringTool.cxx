/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "src/HgtdTimedClusteringTool.h"
#include "HGTD_ReadoutGeometry/HGTD_ModuleDesign.h"
#include "AthContainers/JaggedVecUtils.h"
#include "xAODCore/VariableStruct.h"
#include "details/InPlaceClusterization.h"

#include "details/HgtdCollectionAdapter.h"

namespace {
// specialized connection test for Cells with time.
// Require that the time difference is smaller than 3
// in addition to the regular spatial connection test
template <::Acts::InPlaceClusterization::EConnectionType CONNECTION_TYPE, std::integral T_Coord>
struct ConnectionHelperForCellsWithTime
   : Acts::InPlaceClusterization::ConnectionHelper<ActsTrk::IHGTDClusteringTool::CellContainer::Cell,
                                                   CONNECTION_TYPE> 
{
  int time_cut = 2;
  explicit ConnectionHelperForCellsWithTime(int a_time_cut)
      : time_cut(a_time_cut) {}
  bool isConnected(const std::array<T_Coord, 3>& coordinates_diff) {
    std::span<const T_Coord, 2> spatial_coordinates(
        coordinates_diff.data(), 2);
    if constexpr (CONNECTION_TYPE == Acts::InPlaceClusterization::
                  EConnectionType::CommonEdgeOrCorner) {
      return coordinates_diff[2] < time_cut &&
             Acts::InPlaceClusterization::isConnectedCommonEdgeOrCorner(
                 spatial_coordinates);
    } else {
      return coordinates_diff[2] < time_cut &&
             Acts::InPlaceClusterization::isConnectedCommonEdge(
                 spatial_coordinates);
    }
  }
};
}  // namespace

namespace ActsTrk {

  StatusCode HgtdTimedClusteringTool::initialize()
  {
    ATH_CHECK(HgtdClusteringToolBase::initialize());
    ATH_MSG_DEBUG(m_timeTollerance);
    return StatusCode::SUCCESS;
  }

   namespace {
      template <std::integral T, std::integral T2>
      T check_cast(T2 &&input) {
         assert( input == static_cast<T>(input));
         return static_cast<T>(input);
      }
   }
   
   StatusCode HgtdTimedClusteringTool::clusterize(const EventContext& /*ctx*/,
                                                  const IHGTDClusteringTool::RawDataCollectionVariant& RDOs,
                                                  IHGTDClusteringTool::CellContainer &cellContainer) const
   {
     auto [idHash,RDOs_is_empty] = std::visit([](const auto *RDOs) {
        return std::make_pair(RDOs->identifierHash(),RDOs->empty());
     }, RDOs);

    IHGTDClusteringTool::CellContainer::ModuleRangeGuard rangeGuard(cellContainer.startNewModule(idHash));
    if (!RDOs_is_empty) {
       std::visit([this,
                   &cellContainer](const auto *RDOs) {
          Identifier waferId = RDOs->identify();
          HgtdCollectionAdapter< std::remove_cvref_t<decltype(*RDOs)> > rdoAdapter(*m_hgtd_det_mgr,
                                                                                  *m_hgtd_tdc_calib_tool,
                                                                                  waferId);

          unsigned int rdo_i=0;
          --rdo_i; // to allow incrementing rdo_i at top of loop before any flow control
          for (const auto* rdo : *RDOs) {
             ++rdo_i;
             assert(rdo);
             Identifier rdo_id(rdo->identify());
             uint8_t raw_time_of_flight = rdoAdapter.rawTime(*rdo);
             assert ( raw_time_of_flight <= std::numeric_limits<std::int8_t>::max());
             std::array<std::int8_t,3> coordinates{check_cast<std::int8_t>(m_hgtd_id->phi_index(rdo_id)),
                                                   check_cast<std::int8_t>(m_hgtd_id->eta_index(rdo_id)),
                                                   static_cast<std::int8_t>(raw_time_of_flight)};
             cellContainer.emplace_back_cell(coordinates, rdo_i);
          }
       },RDOs);

       std::span<IHGTDClusteringTool::CellContainer::Cell>
          cellRange = rangeGuard.moduleCellSpan();

       ATH_MSG_DEBUG("Clustering on " << cellRange.size() << " RDOs using time information");
       static constexpr unsigned int SORT_BY_LOCAL_X=0u;
       static constexpr std::int8_t time_cut = 2;
       namespace CL=Acts::InPlaceClusterization;
       CL::clusterize<SORT_BY_LOCAL_X, std::uint16_t>(cellRange,
                                                      ConnectionHelperForCellsWithTime<CL::EConnectionType::CommonEdgeOrCorner,
                                                      std::int8_t>(time_cut));
       // set the cell range per cluster
       Acts::InPlaceClusterization::for_each_cluster(cellRange,[&cellContainer]([[maybe_unused]] std::span<IHGTDClusteringTool::CellContainer::Cell> &the_range,
                                                                                unsigned int idx_begin, unsigned int idx_end) {
          cellContainer.registerNewCluster(idx_begin,idx_end);
       });
    }
    cellContainer.registerClustersForNewModule(rangeGuard.range());
    ATH_MSG_DEBUG("   \\_ " << cellContainer.m_moduleClusterRange.back().nClusters() << " clusters reconstructed");
    return StatusCode::SUCCESS;
  }
} // namespace


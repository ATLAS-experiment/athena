/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HgtdClusteringTool.h"
#include "HGTD_ReadoutGeometry/HGTD_ModuleDesign.h"
#include "details/HgtdCollectionAdapter.h"

namespace ActsTrk {

   namespace {
      template <std::integral T, std::integral T2>
      T check_cast(T2 &&input) {
         assert( input == static_cast<T>(input));
         return static_cast<T>(input);
      }
   }
   
   StatusCode HgtdClusteringTool::clusterize(const EventContext& /*ctx*/,
                                             const IHGTDClusteringTool::RawDataCollectionVariant& RDOs,
                                             IHGTDClusteringTool::CellContainer &cellContainer) const
  {
     auto [idHash,RDOs_is_empty] = std::visit([](const auto *RDOs) {
        return std::make_pair(RDOs->identifierHash(),RDOs->empty());
     }, RDOs);

    IHGTDClusteringTool::CellContainer::ModuleRangeGuard rangeGuard(cellContainer.startNewModule(idHash));
    if (!RDOs_is_empty) {
    std::visit([this,&cellContainer](const auto *RDOs) {
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
    if (m_sortByLocalx) {
       static constexpr unsigned int SORT_BY_LOCAL_X=0u;
       std::sort(cellRange.begin(),cellRange.end(),[](IHGTDClusteringTool::CellContainer::Cell &a,
                                                      IHGTDClusteringTool::CellContainer::Cell &b) {
          return a.coordinates[SORT_BY_LOCAL_X] < b.coordinates[SORT_BY_LOCAL_X];
       });
    }
    // register each cell as a cluster
    for (unsigned int cell_i=0; cell_i<cellRange.size(); ++cell_i) {
       cellContainer.registerNewCluster(cell_i,cell_i+1);
    }
    
    }
    cellContainer.registerClustersForNewModule(rangeGuard.range());
    return StatusCode::SUCCESS;
  }

} // namespace


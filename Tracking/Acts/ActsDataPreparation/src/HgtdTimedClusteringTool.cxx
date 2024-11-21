/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/HgtdTimedClusteringTool.h"
#include "HGTD_ReadoutGeometry/HGTD_ModuleDesign.h"
#include "Acts/Clusterization/TimedClusterization.hpp"


namespace Hgtd {
  static inline int getCellRow(const Hgtd::UnpackedHgtdRDO& cell)
  { return cell.ROW; }
  
  static inline int getCellColumn(const Hgtd::UnpackedHgtdRDO& cell)
  { return cell.COL; }

  static inline int& getCellLabel(Hgtd::UnpackedHgtdRDO& cell)
  { return cell.NCL; }

  static inline double getCellTime(const Hgtd::UnpackedHgtdRDO& cell)
  { return cell.TOA; }
  
  static inline void clusterAddCell(ActsTrk::HgtdTimedClusteringTool::Cluster& cl,
				    const ActsTrk::HgtdTimedClusteringTool::Cell& cell)
  {
    cl.ids.push_back(cell.ID);
    cl.tots.push_back(cell.TOT);
    cl.times.push_back(cell.TOA);
  }
}


namespace ActsTrk {  
  HgtdTimedClusteringTool::HgtdTimedClusteringTool(const std::string& type,
						   const std::string& name,
						   const IInterface* parent)
    : base_class(type, name, parent)
  {}
  
  StatusCode HgtdTimedClusteringTool::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_CHECK(detStore()->retrieve(m_hgtd_det_mgr, "HGTD"));
    ATH_CHECK(detStore()->retrieve(m_hgtd_id, "HGTD_ID"));

    ATH_MSG_DEBUG(m_timeTollerance);
    
    return StatusCode::SUCCESS;
  }

  StatusCode HgtdTimedClusteringTool::clusterize(const EventContext& ctx,
						 const RawDataCollection& RDOs,
						 ClusterContainer& container) const
  {
    // Unpack RDOs (would need a proper function here)
    CellCollection cells;
    cells.reserve(RDOs.size());    
    for (const HGTD_RDO* rdo : RDOs) {
      Identifier id = rdo->identify();
      cells.emplace_back(-1,
			 m_hgtd_id->phi_index(id),
			 m_hgtd_id->eta_index(id),
			 rdo->getTOA(),
			 rdo->getTOT(),
			 id);
    }

    ATH_MSG_DEBUG("Clustering on " << RDOs.size() << " RDOs using time information");
    ClusterCollection clusters =
      Acts::Ccl::createClusters<CellCollection, ClusterCollection, 2>
      (cells, Acts::Ccl::TimedConnect<Cell, 2ul>(m_timeTollerance.value(), m_addCorners.value()));
    ATH_MSG_DEBUG("   \\_ " << clusters.size() << " clusters reconstructed");
    
    // Fast insertion trick
    std::size_t previousSizeContainer = container.size();
    std::vector<xAOD::HGTDCluster*> toAdd;
    toAdd.reserve(clusters.size());
    for (std::size_t i(0), n(clusters.size()); i < n; ++i)
      toAdd.push_back( new xAOD::HGTDCluster() );
    container.insert(container.end(), toAdd.begin(), toAdd.end());

    for (std::size_t i(0); i<clusters.size(); ++i) {
      const typename HgtdTimedClusteringTool::Cluster& cluster = clusters[i];
      ATH_CHECK(makeCluster(ctx, cluster, *container[previousSizeContainer+i]));
    }

    return StatusCode::SUCCESS;
  }

  StatusCode HgtdTimedClusteringTool::makeCluster(const EventContext& /*ctx*/,
						  const typename HgtdTimedClusteringTool::Cluster &cluster,
						  xAOD::HGTDCluster& xaodcluster) const
  {
    if (cluster.ids.empty()) return StatusCode::SUCCESS;
    
    InDetDD::SiLocalPosition pos_acc(0,0);
    double tot_time = 0;
      
    for (size_t i = 0; i < cluster.ids.size(); i++) {
      Identifier rdo_id = cluster.ids[i];
      
      InDetDD::HGTD_DetectorElement* element = m_hgtd_det_mgr->getDetectorElement(rdo_id);
      InDetDD::SiCellId si_cell_id = element->cellIdFromIdentifier(rdo_id);
      InDetDD::SiLocalPosition si_pos = element->design().localPositionOfCell(si_cell_id);

      pos_acc += si_pos;
      tot_time += cluster.times[i];
    }

    pos_acc /= cluster.ids.size();
    tot_time /= cluster.ids.size();

    // Create the cluster
    Eigen::Matrix<float, 3, 1> loc_pos(pos_acc.xPhi(), pos_acc.xEta(), tot_time);
    Eigen::Matrix<float, 3, 3> cov_matrix= Eigen::Matrix<float, 3, 3>::Zero();
    
    constexpr float xWidth = 1.3;
    constexpr float yWidth = 1.3;
    cov_matrix(0,0) = xWidth * xWidth / 12 / cluster.ids.size(); // i.e. Cov XX
    cov_matrix(1,1) = yWidth * yWidth / 12 / cluster.ids.size(); // i.e. Cov YY
    constexpr float time_of_arrival_err = 0.035;
    cov_matrix(2,2) = time_of_arrival_err * time_of_arrival_err / cluster.ids.size(); // i.e. Cov TT
    
    IdentifierHash id_hash = m_hgtd_det_mgr->getDetectorElement(cluster.ids.front())->identifyHash();
    
    // Fill
    xaodcluster.setMeasurement<3>(id_hash,loc_pos,cov_matrix);
    xaodcluster.setIdentifier(cluster.ids.front().get_compact());	
    xaodcluster.setRDOlist(std::move(cluster.ids));
    xaodcluster.setToTlist(std::move(cluster.tots));
        
    return StatusCode::SUCCESS;
  }
  
} // namespace


/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "src/HgtdClusteringTool.h"
#include "HGTD_ReadoutGeometry/HGTD_ModuleDesign.h"
#include "xAODCore/VariableStruct.h"
#include "details/HgtdAuxDataCache.h"

namespace ActsTrk {

  HgtdClusteringTool::HgtdClusteringTool(const std::string& type,
                                         const std::string& name,
                                         const IInterface* parent)
    : base_class(type, name, parent)
  {}

  StatusCode HgtdClusteringTool::initialize()
  {
    ATH_MSG_INFO("Initializing HgtdClusteringTool...");

    ATH_CHECK(detStore()->retrieve(m_hgtd_det_mgr, "HGTD"));
    ATH_CHECK(m_hgtd_tdc_calib_tool.retrieve(EnableTool{m_use_altiroc_rdo.value()}));

    return StatusCode::SUCCESS;
  }

  std::any HgtdClusteringTool::createEventDataCache(xAOD::HGTDClusterContainer& cont, std::size_t nClusterRDOs) const {
      return HgtdAuxDataCache(cont,nClusterRDOs);
  }

  StatusCode HgtdClusteringTool::clusterize(const EventContext&,
                                            const RawDataCollection& RDOs,
                                            std::vector<ClusterCollection>& collection) const
  {
    ATH_MSG_DEBUG("Clustering hits...");
    collection.emplace_back();
    ClusterCollection &clusters=collection.back();
    clusters.reserve(RDOs.size());
    for	(std::size_t i(0), n(RDOs.size()); i < n; ++i) {
      const auto* rdo = RDOs[i];
      Identifier rdo_id = rdo->identify();
      clusters.emplace_back();
      clusters.back().ids.push_back(rdo_id.get_compact());
      clusters.back().times.push_back(rdo->getTOA());
      clusters.back().tots.push_back(rdo->getTOT());
    }
    return StatusCode::SUCCESS;
  }

  StatusCode HgtdClusteringTool::makeClusters(const EventContext& ctx,
                                              const ClusterCollection& clusters,
                                              xAOD::HGTDClusterContainer& container,
                                              size_t& icluster,
                                              std::any& cache) const
  {
    HgtdAuxDataCache* auxDataCache = std::any_cast<HgtdAuxDataCache> (&cache);
    if (!auxDataCache) throw std::bad_any_cast();
    assert( icluster+clusters.size() <= container.size() );
    for (std::size_t i(0); i<clusters.size(); ++i) {
      const HgtdClusteringTool::Cluster& cluster = clusters[i];
      assert(icluster+i == container[icluster+i]->index());
      ATH_CHECK(makeCluster(ctx, cluster, *container[icluster+i],auxDataCache));
    }

    return StatusCode::SUCCESS;
  }

  StatusCode HgtdClusteringTool::makeCluster(const EventContext& /*ctx*/,
                                             const typename HgtdClusteringTool::Cluster &cluster,
                                             xAOD::HGTDCluster& xaodcluster,
                                             HgtdAuxDataCache* auxDataCache) const
  {
    unsigned int icluster=xaodcluster.index();
    unsigned int n_rdos = auxDataCache->rdoList.getBeginIndex(icluster);
    assert(icluster==0 || n_rdos == auxDataCache->totList.getBeginIndex(icluster));
    assert( n_rdos+cluster.ids.size() <= auxDataCache->rdoList.nObjects() );
    assert( cluster.tots.size() == cluster.ids.size());
    assert( cluster.times.size() == cluster.ids.size());
    assert( cluster.ids.size() == 1);
    for (size_t rdo_i = 0; rdo_i < cluster.ids.size(); ++rdo_i) {
      Identifier rdo_id( cluster.ids[rdo_i]);
      // @TODO get detector element only once by caller aka. makeClusters
      const InDetDD::HGTD_DetectorElement* element = m_hgtd_det_mgr->getDetectorElement(rdo_id);

      InDetDD::SiCellId si_cell_id = element->cellIdFromIdentifier(rdo_id);

      InDetDD::SiLocalPosition si_pos = element->design().localPositionOfCell(si_cell_id);


      Eigen::Matrix<float, 3, 1> loc_pos(si_pos.xPhi(), si_pos.xEta(),cluster.times[rdo_i]);
      Eigen::Matrix<float, 3, 3> cov_matrix= Eigen::Matrix<float, 3, 3>::Zero();

      float xWidth = 1.3;
      float yWidth = 1.3;
      cov_matrix(0,0) = xWidth * xWidth / 12; // i.e. Cov XX
      cov_matrix(1,1) = yWidth * yWidth / 12; // i.e. Cov YY
      float time_of_arrival_err = 0.035;
      cov_matrix(2,2) = time_of_arrival_err * time_of_arrival_err; // i.e. Cov TT

      auxDataCache->rdoList.setValue(n_rdos,rdo_id.get_compact());
      auxDataCache->totList.setValue(n_rdos,cluster.tots[rdo_i]);
      ++n_rdos;
      // this will set the position and covariance to the last and only RDO
      IdentifierHash id_hash = element->identifyHash();
      xaodcluster.setMeasurement<3>(id_hash,loc_pos,cov_matrix);
    }


    // Fill
    xaodcluster.setIdentifier(cluster.ids.front());
    auxDataCache->rdoList.updateEndIndex(icluster,n_rdos);
    auxDataCache->totList.updateEndIndex(icluster,n_rdos);

    return StatusCode::SUCCESS;
  }

  StatusCode HgtdClusteringTool::clusterize(const EventContext&,
                                            const HGTD_ALTIROC_RDO_Collection& RDOs,
                                            std::vector<ClusterCollection>& collection) const
  {
    ATH_MSG_DEBUG("Clustering hits...");
    collection.emplace_back();
    ClusterCollection &clusters=collection.back();
    clusters.reserve(RDOs.size());

    for	(std::size_t i(0), n(RDOs.size()); i < n; ++i) {
      const auto* rdo = RDOs[i];
      Identifier rdo_id = rdo->identify();
      clusters.emplace_back();
      clusters.back().ids.push_back(rdo_id.get_compact());
      const InDetDD::HGTD_DetectorElement* element = m_hgtd_det_mgr->getDetectorElement(rdo_id);
      clusters.back().times.push_back(m_hgtd_tdc_calib_tool->TOA2Time(element, rdo->getToA()));
      ATH_MSG_DEBUG("Recovered Time of Arrival: " << m_hgtd_tdc_calib_tool->TOA2Time(element, rdo->getToA()));
      clusters.back().tots.push_back(rdo->getToT());
    }
    return StatusCode::SUCCESS;
  }

} // namespace


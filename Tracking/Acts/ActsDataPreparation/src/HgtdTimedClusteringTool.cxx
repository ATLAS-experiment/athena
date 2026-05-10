/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "src/HgtdTimedClusteringTool.h"
#include "HGTD_ReadoutGeometry/HGTD_ModuleDesign.h"
#include "Acts/Clusterization/TimedClusterization.hpp"
#include "AthContainers/JaggedVecUtils.h"
#include "xAODCore/VariableStruct.h"
#include "AthenaKernel/Units.h"

namespace Hgtd {
  static inline int getCellRow(const Hgtd::UnpackedHgtdRDO& cell)
  { return cell.ROW; }

  static inline int getCellColumn(const Hgtd::UnpackedHgtdRDO& cell)
  { return cell.COL; }

  static inline double getCellTime(const Hgtd::UnpackedHgtdRDO& cell)
  { return cell.TOA; }

  static inline void clusterAddCell(ActsTrk::HgtdTimedClusteringTool::Cluster& cl,
				    const ActsTrk::HgtdTimedClusteringTool::Cell& cell)
  {
    cl.ids.push_back(cell.ID.get_compact());
    cl.tots.push_back(cell.TOT);
    cl.times.push_back(cell.TOA);
  }
}

namespace {
   static constexpr float acts_ns_over_athena_ns = Acts::UnitConstants::ns / Athena::Units::nanosecond;
   float toActsTime(float athena_time) {
      return athena_time * acts_ns_over_athena_ns;
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
    ATH_CHECK(m_hgtd_tdc_calib_tool.retrieve(EnableTool{m_use_altiroc_rdo.value()}));

    ATH_MSG_DEBUG(m_timeTollerance);

    return StatusCode::SUCCESS;
  }

  // @TODO move to header
  struct HgtdAuxDataCache : xAOD::VariableStruct {
      HgtdAuxDataCache(SG::AuxVectorData& cont, unsigned int n_cluster_rdos)
         : xAOD::VariableStruct(cont)
      {
         if (n_cluster_rdos>0) {
            auto *store = cont.getStore();
            assert(store);
            SG::setJaggedVectorData(cont,*store,xAOD::HGTDCluster::rdoListAcc(), n_cluster_rdos, rdoList, rdoListPayload);
            SG::setJaggedVectorData(cont,*store,xAOD::HGTDCluster::totListAcc(), n_cluster_rdos, totList, totListPayload);
         }
      }
      SG::Accessor<SG::JaggedVecElt<Identifier::value_type> >::Elt_span     rdoList;
      SG::Accessor<SG::JaggedVecElt<Identifier::value_type> >::Payload_span rdoListPayload;
      SG::Accessor<SG::JaggedVecElt<int> >::Elt_span     totList;
      SG::Accessor<SG::JaggedVecElt<int> >::Payload_span totListPayload;
  };

  std::any HgtdTimedClusteringTool::createEventDataCache(xAOD::HGTDClusterContainer& cont, std::size_t nClusterRDOs) const {
      return HgtdAuxDataCache(cont,nClusterRDOs);
  }

   StatusCode HgtdTimedClusteringTool::clusterize(const EventContext& /*ctx*/,
						 const RawDataCollection& RDOs,
                                                 std::vector<ClusterCollection>& collection) const
  {
    collection.emplace_back();
    ClusterCollection &clusters=collection.back();
    // best guess about number of expected clusters is one cluster per RDO
    clusters.reserve(RDOs.size());
    // Unpack RDOs (would need a proper function here)
    CellCollection cells;
    cells.reserve(RDOs.size());
    for (const HGTD_RDO* rdo : RDOs) {
      Identifier id = rdo->identify();
      cells.emplace_back(-1,
			 m_hgtd_id->phi_index(id),
			 m_hgtd_id->eta_index(id),
			 toActsTime(rdo->getTOA()),
			 rdo->getTOT(),
			 id);
  }

    ATH_MSG_DEBUG("Clustering on " << RDOs.size() << " RDOs using time information");
    Acts::Ccl::ClusteringData data;
    Acts::Ccl::createClusters<CellCollection, ClusterCollection, 2>
      (data, cells, clusters, Acts::Ccl::TimedConnect<Cell, 2ul>(m_timeTollerance.value(), m_addCorners.value()));
    ATH_MSG_DEBUG("   \\_ " << clusters.size() << " clusters reconstructed");
    return StatusCode::SUCCESS;
  }

  StatusCode HgtdTimedClusteringTool::makeClusters(const EventContext& ctx,
                                                   const ClusterCollection& clusters,
                                                   xAOD::HGTDClusterContainer& container,
                                                   size_t& icluster,
                                                   std::any& cache) const
  {
    HgtdAuxDataCache* auxDataCache = std::any_cast<HgtdAuxDataCache> (&cache);
    if (!auxDataCache) throw std::bad_any_cast();
    assert( icluster+clusters.size() <= container.size() );
    for (std::size_t i(0); i<clusters.size(); ++i) {
      const typename HgtdTimedClusteringTool::Cluster& cluster = clusters[i];
      assert(icluster+i == container[icluster+i]->index());
      ATH_CHECK(makeCluster(ctx, cluster, *container[icluster+i],auxDataCache));
    }

    return StatusCode::SUCCESS;
  }

  StatusCode HgtdTimedClusteringTool::makeCluster(const EventContext& /*ctx*/,
						  const typename HgtdTimedClusteringTool::Cluster &cluster,
						  xAOD::HGTDCluster& xaodcluster,
                                                  HgtdAuxDataCache* auxDataCache) const
  {
    if (cluster.ids.empty()) return StatusCode::SUCCESS;

    InDetDD::SiLocalPosition pos_acc(0,0);
    double tot_time = 0;

    unsigned int icluster=xaodcluster.index();
    unsigned int n_rdos = (icluster> 0 ?  auxDataCache->rdoList[icluster-1].end() : 0u);
    assert(icluster==0 || n_rdos == auxDataCache->totList[icluster-1].end());
    assert( n_rdos+cluster.ids.size() <= auxDataCache->rdoList.size() );
    assert( cluster.tots.size() == cluster.ids.size());
    for (size_t i = 0; i < cluster.ids.size(); i++) {
      Identifier rdo_id(cluster.ids[i]);
      // @TODO get detector only once, all clusters should belong to the same detector element
      const InDetDD::HGTD_DetectorElement* element = m_hgtd_det_mgr->getDetectorElement(rdo_id);
      InDetDD::SiCellId si_cell_id = element->cellIdFromIdentifier(rdo_id);
      InDetDD::SiLocalPosition si_pos = element->design().localPositionOfCell(si_cell_id);

      auxDataCache->rdoListPayload[n_rdos]=rdo_id.get_compact();
      auxDataCache->totListPayload[n_rdos]=cluster.tots[i];
      ++n_rdos;

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
    // @TODO get detector element only once by caller aka. makeClusters
    IdentifierHash id_hash = m_hgtd_det_mgr->getDetectorElement(Identifier(cluster.ids.front()))->identifyHash();

    // Fill
    xaodcluster.setMeasurement<3>(id_hash,loc_pos,cov_matrix);
    xaodcluster.setIdentifier(cluster.ids.front());
    auxDataCache->rdoList[icluster] = n_rdos;
    auxDataCache->totList[icluster] = n_rdos;

    return StatusCode::SUCCESS;
  }

StatusCode HgtdTimedClusteringTool::clusterize(const EventContext& /*ctx*/,
    const HGTD_ALTIROC_RDO_Collection& RDOs,
    std::vector<ClusterCollection>& collection) const
{
  collection.emplace_back();
  ClusterCollection &clusters=collection.back();
  // best guess about number of expected clusters is one cluster per RDO
  clusters.reserve(RDOs.size());
  // Unpack RDOs (would need a proper function here)
  CellCollection cells;
  cells.reserve(RDOs.size());
  for (const HGTD_ALTIROC_RDO* rdo : RDOs) {
    Identifier id = rdo->identify();

    const InDetDD::HGTD_DetectorElement* element = m_hgtd_det_mgr->getDetectorElement(id);
    uint8_t time_of_flight = m_hgtd_tdc_calib_tool->TOA2Time(element, rdo->getToA());

    ATH_MSG_DEBUG("Recovered Time of Arrival: " << time_of_flight);

    cells.emplace_back(-1,
      m_hgtd_id->phi_index(id),
      m_hgtd_id->eta_index(id),
      toActsTime(time_of_flight),
      rdo->getToT(),
      id);
  }

  ATH_MSG_DEBUG("Clustering on " << RDOs.size() << " RDOs using time information");
  Acts::Ccl::ClusteringData data;
  Acts::Ccl::createClusters<CellCollection, ClusterCollection, 2>
    (data, cells, clusters, Acts::Ccl::TimedConnect<Cell, 2ul>(m_timeTollerance.value(), m_addCorners.value()));
  ATH_MSG_DEBUG("   \\_ " << clusters.size() << " clusters reconstructed");
  return StatusCode::SUCCESS;
}

} // namespace


/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "HgtdClusterizationAlg.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "ActsInterop/TableUtils.h"
#include "AthAllocators/DataPool.h"
#include "details/CellContainer.h"

namespace ActsTrk {

  HgtdClusterizationAlg::HgtdClusterizationAlg(const std::string& name,
					       ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode HgtdClusterizationAlg::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name() << " ...");
    ATH_CHECK(m_clusteringTool.retrieve());
    ATH_CHECK(m_monTool.retrieve(EnableTool{not m_monTool.empty()}));

    ATH_CHECK(m_rdoContainerKey.initialize(not m_use_altiroc_rdo.value()));
    ATH_CHECK(m_altiroc_rdo_rh_key.initialize(m_use_altiroc_rdo.value()));
    ATH_CHECK(m_clusterContainerKey.initialize());

    return StatusCode::SUCCESS;

  }

  StatusCode HgtdClusterizationAlg::finalize() {
    ATH_MSG_INFO("Clusterization statistics" << std::endl << makeTable(m_stat,
								       std::array<std::string, kNStat>{
									 "RDOs",
									 "Clusters"
								       }).columnWidth(10));
    
    return StatusCode::SUCCESS;    
  }
  
  StatusCode HgtdClusterizationAlg::execute(const EventContext& ctx) const
  {
    ATH_MSG_DEBUG("Executing " << name() << " ...");

    auto timer = Monitored::Timer<std::chrono::milliseconds>( "TIME_execute" );
    auto mon = Monitored::Group( m_monTool, timer );
      
    SG::WriteHandle<xAOD::HGTDClusterContainer> clusterContainer = SG::makeHandle(m_clusterContainerKey, ctx);
    ATH_CHECK(clusterContainer.record(std::make_unique<xAOD::HGTDClusterContainer>(SG::VIEW_ELEMENTS, SG::ALWAYS_TRACK_INDICES),
				      std::make_unique<xAOD::HGTDClusterAuxContainer>()));


    auto getRDOContainer= [this, &ctx]() -> IHGTDClusteringTool::RDOContainerVariant {
       if (m_use_altiroc_rdo){
          SG::ReadHandle<HGTD_ALTIROC_RDO_Container> rdoContainer = SG::makeHandle(m_altiroc_rdo_rh_key, ctx);
          if (!rdoContainer.isValid()) {
             ATH_MSG_ERROR("Failed to retrieve HGTD ALTIROC RDO container" << m_altiroc_rdo_rh_key.key());
          }
          return rdoContainer.cptr();
       }
       else {
          SG::ReadHandle<HGTD_RDO_Container> rdoContainer = SG::makeHandle(m_rdoContainerKey, ctx);
          if (!rdoContainer.isValid()) {
             ATH_MSG_ERROR("Failed to retrieve HGTD RDO container " << m_rdoContainerKey.key());
          }
          return rdoContainer.cptr();
       }
    };
    IHGTDClusteringTool::RDOContainerVariant rdoContainer = getRDOContainer();
    if (!std::visit([](const auto *rdoContainer) -> bool { return rdoContainer != nullptr; }, rdoContainer)) {
       return StatusCode::FAILURE;
    }

    unsigned int n_modules=std::visit([](const auto *rdoContainer) { return rdoContainer->size();}, rdoContainer);

    auto [n_cluster_total, n_rdos_total] = m_clusteringTool->countCells(rdoContainer,
                                                                        std::vector<IdentifierHash>{}  /* all */ );
    typename IHGTDClusteringTool::CellContainer cellContainer(n_modules, n_cluster_total, n_rdos_total);

    
    unsigned int nRDOs=0;
    unsigned int nClusters=0;
    ATH_CHECK( std::visit([this, &cellContainer, &ctx, &nRDOs, &nClusters](const auto *rdoContainer) {
       for (const auto rdoCollection : *rdoContainer) {
          if (rdoCollection->empty()) {
             continue;
          }
          nRDOs+=rdoCollection->size();
          ATH_CHECK(m_clusteringTool->clusterize(ctx, rdoCollection, cellContainer));
          nClusters += cellContainer.m_moduleClusterRange.back().nClusters();
       }
       return StatusCode(StatusCode::SUCCESS);
    }, rdoContainer));
       
    DataPool<xAOD::HGTDCluster> pool (nClusters);
    clusterContainer->push_new(nClusters, [&pool]() {return pool.nextElementPtr();});
    std::any cache = m_clusteringTool->createEventDataCache(*clusterContainer,nRDOs);

    std::size_t icluster=0;
    for (std::size_t icollection=0u; icollection<cellContainer.size(); ++icollection) {
       ATH_CHECK( m_clusteringTool->makeClusters(ctx,
                                                 rdoContainer,
                                                 cellContainer,
                                                 icollection,
                                                 icluster,
                                                 *clusterContainer,
                                                 cache));
       unsigned int n_new_clusters=cellContainer.moduleClusterRange(icollection).nClusters();
       icluster += n_new_clusters;
    }
    assert( clusterContainer->size() == icluster);
    assert( clusterContainer->size() == cellContainer.nClustersTotal());
    
    m_stat[kNRdo] += nRDOs;
    m_stat[kNClusters] += clusterContainer->size();
    ATH_MSG_DEBUG("Clusters produced size: "<<clusterContainer->size());  
    return StatusCode::SUCCESS;
  }
  

}

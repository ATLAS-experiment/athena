/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "HgtdClusterizationAlg.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "ActsInterop/TableUtils.h"

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

    ATH_CHECK(m_rdoContainerKey.initialize());
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
    
    SG::ReadHandle<HGTD_RDO_Container> rdoContainer = SG::makeHandle(m_rdoContainerKey, ctx);
    if (!rdoContainer.isValid()) {
        ATH_MSG_ERROR("Failed to retrieve HGTD RDO container");
        return StatusCode::FAILURE;
    }
      
    SG::WriteHandle<xAOD::HGTDClusterContainer> clusterContainer = SG::makeHandle(m_clusterContainerKey, ctx);
    ATH_CHECK(clusterContainer.record(std::make_unique<xAOD::HGTDClusterContainer>(),
				      std::make_unique<xAOD::HGTDClusterAuxContainer>()));

    for (const auto rdoCollection : *rdoContainer) {
        if (rdoCollection->empty()) {
            continue;
        }
	m_stat[kNRdo] += rdoCollection->size();
        ATH_CHECK(m_clusteringTool->clusterize(ctx, *rdoCollection, *clusterContainer));
    }

    m_stat[kNClusters] += clusterContainer->size();
    ATH_MSG_DEBUG("Clusters produced size: "<<clusterContainer->size());  
    return StatusCode::SUCCESS;
  }
  

}

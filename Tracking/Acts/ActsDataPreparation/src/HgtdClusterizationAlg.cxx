/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "HgtdClusterizationAlg.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElement.h"
#include "AthenaMonitoringKernel/Monitored.h"

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

        ATH_CHECK(m_clusteringTool->clusterize(*rdoCollection, ctx, *clusterContainer));
    }

    return StatusCode::SUCCESS;
  }
  

}

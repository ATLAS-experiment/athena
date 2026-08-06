/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "CaloClusterTimingFilter.h"

#include "xAODCaloEvent/CaloClusterAuxContainer.h"


StatusCode CaloClusterTimingFilter::initialize() {
  ATH_CHECK(m_inputKey.initialize());
  ATH_CHECK(m_outputKey.initialize());

  ATH_CHECK(m_cellLinkKey.initialize(! m_cellLinkKey.key().empty()));

  if (m_minTime > m_maxTime) {
    ATH_MSG_ERROR("MinTime is greater than MaxTime");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode CaloClusterTimingFilter::execute(const EventContext& ctx) const {
  SG::ReadHandle<xAOD::CaloClusterContainer> inputHandle(m_inputKey, ctx);
  if (!inputHandle.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve " << m_inputKey.key());
    return StatusCode::FAILURE;
  }

  auto output    = std::make_unique<xAOD::CaloClusterContainer>();
  auto outputAux = std::make_unique<xAOD::CaloClusterAuxContainer>();
  output->setStore(outputAux.get());
  output->reserve(inputHandle->size());

  const bool writeCellLinks = !m_cellLinkKey.empty();
  std::unique_ptr<CaloClusterCellLinkContainer> cellLinks;
  if (writeCellLinks) {
    cellLinks = std::make_unique<CaloClusterCellLinkContainer>();
  }

  for (const xAOD::CaloCluster* cluster : *inputHandle) {
    const double time = cluster->time();

    auto blocks = [&](xAOD::CaloCluster::MomentType m, double thr) {
      double v = -1.;
      return thr >= 0 && cluster->retrieveMoment(m, v) && v < thr;
    };
    if (!blocks(xAOD::CaloCluster::CELL_SIGNIFICANCE, m_cellSignificanceThreshold) &&
        !blocks(xAOD::CaloCluster::BADLARQ_FRAC,      m_badLArQFracThreshold) &&
        (time < m_minTime || time > m_maxTime)) {
      continue;
    }

    auto* outCluster = new xAOD::CaloCluster();
    output->push_back(outCluster);
    *outCluster = *cluster;
    if (writeCellLinks) {
      outCluster->setLink(cellLinks.get(), ctx);
    }
  }

  SG::WriteHandle<xAOD::CaloClusterContainer> outputHandle(m_outputKey, ctx);
  ATH_CHECK(outputHandle.record(std::move(output), std::move(outputAux)));

  if (writeCellLinks) {
    SG::WriteHandle<CaloClusterCellLinkContainer> cellLinkHandle(m_cellLinkKey, ctx);
    ATH_CHECK(cellLinkHandle.record(std::move(cellLinks)));
  }

  return StatusCode::SUCCESS;
}

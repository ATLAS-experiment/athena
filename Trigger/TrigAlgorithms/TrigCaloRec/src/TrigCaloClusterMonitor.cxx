//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

/********************************************************************
 *
 * NAME:      TrigCaloClusterMonitor
 * PACKAGE:   Trigger/TrigAlgorithms/TrigCaloRec
 *
 * AUTHOR:    Nuno Fernandes
 * CREATED:   September 2024
 *
 *          Stores the relevant information for monitoring topological clustering
 *          through monitored variables, separately from the actual algorithm.
 *********************************************************************/

#include "TrigCaloClusterMonitor.h"

#include "AthenaMonitoringKernel/MonitoredCollection.h"
#include "AthenaMonitoringKernel/MonitoredGroup.h"
#include "AthenaMonitoringKernel/MonitoredScalar.h"
#include "AthenaMonitoringKernel/MonitoredTimer.h"
#include "StoreGate/ReadDecorHandle.h"
#include "GaudiKernel/StatusCode.h"


/////////////////////////////////////////////////////////////////////
// CONSTRUCTOR:
/////////////////////////////////////////////////////////////////////
//
TrigCaloClusterMonitor::TrigCaloClusterMonitor(const std::string & name, ISvcLocator * pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode TrigCaloClusterMonitor::initialize()
{
  CHECK(m_moniTool.retrieve());
  ATH_CHECK(m_clustersKey.initialize());

  ATH_CHECK(m_avgMuKey.initialize());

  ATH_CHECK(m_noiseCDOKey.initialize(m_monitorCells));
  ATH_CHECK(m_cellsKey.initialize(m_monitorCells));

  return StatusCode::SUCCESS;
}


StatusCode TrigCaloClusterMonitor::execute(const EventContext & ctx) const
{
  const auto monitoring_number = ctx.eventID().event_number();

  if (monitoring_number %  m_monitorInterval != 0)
    {
      return StatusCode::SUCCESS;
    }

  const bool really_monitor_cells = m_monitorCells && (monitoring_number % (m_monitorInterval * m_monitorCellsInterval) == 0);


  SG::ReadHandle<xAOD::CaloClusterContainer> cluster_collection(m_clustersKey, ctx);

  const xAOD::CaloClusterContainer * cluster_collection_ptr = cluster_collection.ptr();

  if (!cluster_collection.isValid())
    {
      ATH_MSG_ERROR("Cannot retrieve CaloClusterContainer: " << m_clustersKey.key());
      return StatusCode::FAILURE;
    }

  std::vector<double>       clus_phi;
  std::vector<double>       clus_eta;
  std::vector<double>       clus_n_bad_cells;
  std::vector<double>       clus_eng_frac_max;
  std::vector<unsigned int> size_vec;
  clus_phi.reserve(1024);
  clus_eta.reserve(1024);
  clus_n_bad_cells.reserve(1024);
  clus_eng_frac_max.reserve(1024);
  size_vec.reserve(1024);

  auto mon_clusEt = Monitored::Collection("Et", *cluster_collection_ptr, &xAOD::CaloCluster::et);
  auto mon_clusSignalState = Monitored::Collection("signalState", *cluster_collection_ptr, &xAOD::CaloCluster::signalState);
  auto mon_clusSize = Monitored::Collection("clusterSize", *cluster_collection_ptr, &xAOD::CaloCluster::clusterSize);
  auto mon_clusPhi = Monitored::Collection("Phi", clus_phi);
  auto mon_clusEta = Monitored::Collection("Eta", clus_eta);
  auto mon_badCells = Monitored::Collection("N_BAD_CELLS", clus_n_bad_cells);
  auto mon_engFrac = Monitored::Collection("ENG_FRAC_MAX", clus_eng_frac_max);
  auto mon_size = Monitored::Collection("size", size_vec);
  auto monmu = Monitored::Scalar("mu", -999.0);
  auto mon_container_size = Monitored::Scalar("container_size", 0);
  auto mon_container_size_by_mu  = Monitored::Scalar("container_size_by_mu", 0.);

  mon_container_size = cluster_collection_ptr->size();

  for (const xAOD::CaloCluster * cl : *cluster_collection_ptr)
    {
      const CaloClusterCellLink * num_cell_links = cl->getCellLinks();
      if (!num_cell_links)
        {
          size_vec.push_back(0);
        }
      else
        {
          size_vec.push_back(num_cell_links->size());
        }
      clus_phi.push_back(cl->phi());
      clus_eta.push_back(cl->eta());
      clus_n_bad_cells.push_back(cl->getMomentValue(xAOD::CaloCluster::N_BAD_CELLS));
      clus_eng_frac_max.push_back(cl->getMomentValue(xAOD::CaloCluster::ENG_FRAC_MAX));
    }

  float read_mu = 0;

  SG::ReadDecorHandle<xAOD::EventInfo, float> eventInfoDecor(m_avgMuKey, ctx);
  if (eventInfoDecor.isPresent())
    {
      read_mu = eventInfoDecor(0);
      monmu = read_mu;
    }
  else
    {
      ATH_MSG_WARNING("EventInfo decoration not present: " << m_avgMuKey.key());
    }

  int count_1thrsigma = 0, count_2thrsigma = 0;

  if (really_monitor_cells)
    {
      SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);
      if ( !cell_collection.isValid() )
        {
          ATH_MSG_ERROR( " Cannot retrieve CaloCellContainer: " << cell_collection.name()  );
          return StatusCode::FAILURE;
        }

      SG::ReadCondHandle<CaloNoise> noiseHdl{m_noiseCDOKey, ctx};
      const CaloNoise * noisep = *noiseHdl;
      for (const auto cell : *cell_collection)
        {
          const CaloDetDescrElement * cdde = cell->caloDDE();

          const bool is_tile = cdde->is_tile();

          if (m_excludeTile && is_tile)
            {
              continue;
            }

          const float cell_energy = cell->energy();

          if (cell_energy < 0)
            {
              continue;
            }

          const float thr = ( is_tile && m_useTwoGaussianNoise ?
                              noisep->getEffectiveSigma(cdde->identifyHash(), cell->gain(), cell_energy) :
                              noisep->getNoise(cdde->identifyHash(), cell->gain())
                            );

          if (cell_energy > m_monitoring1thr * thr)
            {
              count_1thrsigma += 1;
              if (cell_energy > m_monitoring2thr * thr)
                {
                  count_2thrsigma += 1;
                }
            }
        }
    }


  if (really_monitor_cells)
    {

      auto moncount_1thrsigma = Monitored::Scalar("count_1thrsigma", -999.0);
      auto moncount_2thrsigma = Monitored::Scalar("count_2thrsigma", -999.0);
      auto moncount_1thrsigma_by_mu2 = Monitored::Scalar("count_1thrsigma_by_mu2", -999.0);
      auto moncount_2thrsigma_by_mu2 = Monitored::Scalar("count_2thrsigma_by_mu2", -999.0);


      if (read_mu > 5)
        {
          const float rev_mu = 1.f / read_mu;
          mon_container_size_by_mu = rev_mu * cluster_collection_ptr->size();
          const float sqr_rev_mu = rev_mu * rev_mu;
          moncount_1thrsigma_by_mu2 = sqr_rev_mu * count_1thrsigma;
          moncount_2thrsigma_by_mu2 = sqr_rev_mu * count_2thrsigma;
        }

      moncount_1thrsigma = count_1thrsigma;
      moncount_2thrsigma = count_2thrsigma;

      auto monitorIt = Monitored::Group( m_moniTool, mon_container_size, mon_clusEt, mon_clusPhi, mon_clusEta,
                                         mon_clusSignalState, mon_clusSize, mon_badCells, mon_engFrac, mon_size,
                                         monmu, mon_container_size_by_mu, moncount_1thrsigma, moncount_2thrsigma,
                                         moncount_1thrsigma_by_mu2, moncount_2thrsigma_by_mu2 );
    }
  else
    {
      if (read_mu > 5)
        {
          mon_container_size_by_mu = cluster_collection_ptr->size() / read_mu;
        }
      auto monitorIt = Monitored::Group( m_moniTool, mon_container_size, mon_clusEt, mon_clusPhi, mon_clusEta, 
                                         mon_clusSignalState, mon_clusSize, mon_badCells, mon_engFrac, mon_size,
                                         monmu, mon_container_size_by_mu);
    }

  return StatusCode::SUCCESS;
}

//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloMomentsDumper.h"
#include "CaloRecGPU/StandaloneDataIO.h"

#include <fstream>
#include <filesystem>

using namespace CaloRecGPU;


StatusCode CaloMomentsDumper::execute (const EventContext & ctx, xAOD::CaloClusterContainer * cluster_collection) const
{

  const auto err1 = StandaloneDataIO::prepare_folder_for_output(std::string(m_savePath));
  if (err1 != StandaloneDataIO::ErrorState::OK)
    {
      return StatusCode::FAILURE;
    }

  const std::filesystem::path save_file = m_savePath + "/" + StandaloneDataIO::build_filename((m_filePrefix.size() > 0 ? m_filePrefix + "_moments" : "moments"),
                                                                                                ctx.evt(), m_fileSuffix, "txt", m_numWidth);

  std::ofstream out_file(save_file);

  for (auto cluster_iter = cluster_collection->begin(); cluster_iter != cluster_collection->end(); ++cluster_iter)
    {
      const xAOD::CaloCluster * cluster = (*cluster_iter);
      const CaloClusterCellLink * cell_links = cluster->getCellLinks();
      if (!cell_links)
        {
          ATH_MSG_ERROR("Can't get valid links to CaloCells (CaloClusterCellLink)!");
          return StatusCode::FAILURE;
        }

      out_file << cluster->pt() << " " << cluster->eta() << " " << cluster->phi() << " " << cluster->m() << " " << cluster->e() << " " << cluster->rapidity() << " " << cluster->et() << "\n";
      out_file << "------------------------------------------------\n";

      for (int i = 0; i < 28; ++i)
        {
          out_file << cluster->eSample((CaloSampling::CaloSample) i) << " " << cluster->etaSample((CaloSampling::CaloSample) i)
                   << " " << cluster->phiSample((CaloSampling::CaloSample) i) << " " << cluster->energy_max((CaloSampling::CaloSample) i)
                   << " " << cluster->etamax((CaloSampling::CaloSample) i) << " " << cluster->phimax((CaloSampling::CaloSample) i)
                   << " " << cluster->numberCellsInSampling((CaloSampling::CaloSample) i) << "\n";
        }
        
      out_file << "------------------------------------------------\n";
      out_file << cluster->numberCells() << " " << cluster->time() << " " << cluster->secondTime() << " " << cluster->samplingPattern() << " " << cluster->size() << "\n";
      out_file << "------------------------------------------------\n";
      
      out_file << cluster->getMomentValue(xAOD::CaloCluster::FIRST_PHI)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::FIRST_ETA)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::SECOND_R)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::SECOND_LAMBDA)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::DELTA_PHI)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::DELTA_THETA)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::DELTA_ALPHA)

               << "\n2:\n" << cluster->getMomentValue(xAOD::CaloCluster::CENTER_X)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::CENTER_Y)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::CENTER_Z)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::CENTER_MAG)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::CENTER_LAMBDA)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::LATERAL)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::LONGITUDINAL)

               << "\n3:\n" << cluster->getMomentValue(xAOD::CaloCluster::ENG_FRAC_EM)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_FRAC_MAX)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_FRAC_CORE)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::FIRST_ENG_DENS)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::SECOND_ENG_DENS)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ISOLATION)

               << "\n4:\n" << cluster->getMomentValue(xAOD::CaloCluster::ENG_BAD_CELLS)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::N_BAD_CELLS)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::N_BAD_CELLS_CORR)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::BAD_CELLS_CORR_E)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::BADLARQ_FRAC)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_POS)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::SIGNIFICANCE)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::CELL_SIGNIFICANCE)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::CELL_SIG_SAMPLING)

               << "\n5:\n" << cluster->getMomentValue(xAOD::CaloCluster::AVG_LAR_Q)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::AVG_TILE_Q)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_BAD_HV_CELLS)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::N_BAD_HV_CELLS)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::PTD)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::MASS)

               << "\n6:\n" << cluster->getMomentValue(xAOD::CaloCluster::EM_PROBABILITY)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::HAD_WEIGHT)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::OOC_WEIGHT)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::DM_WEIGHT)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::TILE_CONFIDENCE_LEVEL)

               << "\n" << cluster->getMomentValue(xAOD::CaloCluster::SECOND_TIME)

               << "\n7:\n" << cluster->getMomentValue(xAOD::CaloCluster::VERTEX_FRACTION)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::NVERTEX_FRACTION)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ETACALOFRAME)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::PHICALOFRAME)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ETA1CALOFRAME)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::PHI1CALOFRAME)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ETA2CALOFRAME)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::PHI2CALOFRAME)

               << "\n8:\n" << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_TOT)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_OUT_L)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_OUT_M)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_OUT_T)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_L)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_M)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_T)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_EMB0)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_EME0)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_TILEG3)

               << "\n9:\n" << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_TOT)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_EMB0)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_TILE0)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_TILEG3)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_EME0)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_HEC0)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_FCAL)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_LEAKAGE)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_UNCLASS)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_FRAC_EM)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_FRAC_HAD)
               << " " << cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_FRAC_REST);

      out_file << "\n------------------------------------------------\n";
      out_file << "------------------------------------------------\n";

      out_file << "\n\n";

    }

  out_file << std::endl;

  return StatusCode::SUCCESS;

}

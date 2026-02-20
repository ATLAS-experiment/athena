//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "GPUToAthenaImporterWithMoments.h"

#include "CaloRecGPU/CUDAFriendlyClasses.h"

#include "AthenaKernel/errorcheck.h"

#include <vector>
#include <algorithm>
#include <memory>

#include "xAODCaloEvent/CaloClusterKineHelper.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

using namespace CaloRecGPU;

GPUToAthenaImporterWithMoments::GPUToAthenaImporterWithMoments(const std::string & type, const std::string & name, const IInterface * parent):
  base_class(type, name, parent),
  CaloGPUTimed(this),
  m_doHVMoments(false)
{
}

#include "MacroHelpers.h"

StatusCode GPUToAthenaImporterWithMoments::initialize()
{
  ATH_CHECK( m_cellsKey.initialize() );

  ATH_CHECK( detStore()->retrieve(m_calo_id, "CaloCell_ID") );

  ATH_CHECK(m_caloMgrKey.initialize());

  ATH_CHECK(m_HVCablingKey.initialize(m_fillHVMoments));
  ATH_CHECK(m_HVScaleKey.initialize(m_fillHVMoments));

  auto get_cluster_size_from_string = [](const std::string & str, bool & failed)
  {
    failed = false;
    CRGPU_RECURSIVE_MACRO(
            CRGPU_CHEAP_STRING_TO_ENUM( str, xAOD::CaloCluster,
                                        SW_55ele,
                                        SW_35ele,
                                        SW_37ele,
                                        SW_55gam,
                                        SW_35gam,
                                        SW_37gam,
                                        SW_55Econv,
                                        SW_35Econv,
                                        SW_37Econv,
                                        SW_softe,
                                        Topo_420,
                                        Topo_633,
                                        SW_7_11,
                                        SuperCluster,
                                        Tower_01_01,
                                        Tower_005_005,
                                        Tower_fixed_area
                                      )
    )
    //I know Topological Clustering only supports a subset of those,
    //but this is supposed to be a general data exporting tool...
    else
      {
        //failed = true;
        return xAOD::CaloCluster::CSize_Unknown;
      }
  };

  bool size_failed = false;
  m_clusterSize = get_cluster_size_from_string(m_clusterSizeString, size_failed);

  if (m_clusterSize == xAOD::CaloCluster::CSize_Unknown)
    {
      ATH_MSG_ERROR("Invalid Cluster Size: " << m_clusterSizeString);
    }

  if (size_failed)
    {
      return StatusCode::FAILURE;
    }


  auto get_moment_from_string = [](const std::string & str, bool & failed)
  {
    failed = false;
    CRGPU_RECURSIVE_MACRO(
            CRGPU_CHEAP_STRING_TO_ENUM( str, xAOD::CaloCluster,
                                        FIRST_PHI,
                                        FIRST_ETA,
                                        SECOND_R,
                                        SECOND_LAMBDA,
                                        DELTA_PHI,
                                        DELTA_THETA,
                                        DELTA_ALPHA,
                                        CENTER_X,
                                        CENTER_Y,
                                        CENTER_Z,
                                        CENTER_MAG,
                                        CENTER_LAMBDA,
                                        LATERAL,
                                        LONGITUDINAL,
                                        ENG_FRAC_EM,
                                        ENG_FRAC_MAX,
                                        ENG_FRAC_CORE,
                                        FIRST_ENG_DENS,
                                        SECOND_ENG_DENS,
                                        ISOLATION,
                                        ENG_BAD_CELLS,
                                        N_BAD_CELLS,
                                        N_BAD_CELLS_CORR,
                                        BAD_CELLS_CORR_E,
                                        BADLARQ_FRAC,
                                        ENG_POS,
                                        SIGNIFICANCE,
                                        CELL_SIGNIFICANCE,
                                        CELL_SIG_SAMPLING,
                                        AVG_LAR_Q,
                                        AVG_TILE_Q,
                                        ENG_BAD_HV_CELLS,
                                        N_BAD_HV_CELLS,
                                        PTD,
                                        MASS,
                                        EM_PROBABILITY,
                                        HAD_WEIGHT,
                                        OOC_WEIGHT,
                                        DM_WEIGHT,
                                        TILE_CONFIDENCE_LEVEL,
                                        SECOND_TIME,
                                        NCELL_SAMPLING,
                                        VERTEX_FRACTION,
                                        NVERTEX_FRACTION,
                                        ETACALOFRAME,
                                        PHICALOFRAME,
                                        ETA1CALOFRAME,
                                        PHI1CALOFRAME,
                                        ETA2CALOFRAME,
                                        PHI2CALOFRAME,
                                        ENG_CALIB_TOT,
                                        ENG_CALIB_OUT_L,
                                        ENG_CALIB_OUT_M,
                                        ENG_CALIB_OUT_T,
                                        ENG_CALIB_DEAD_L,
                                        ENG_CALIB_DEAD_M,
                                        ENG_CALIB_DEAD_T,
                                        ENG_CALIB_EMB0,
                                        ENG_CALIB_EME0,
                                        ENG_CALIB_TILEG3,
                                        ENG_CALIB_DEAD_TOT,
                                        ENG_CALIB_DEAD_EMB0,
                                        ENG_CALIB_DEAD_TILE0,
                                        ENG_CALIB_DEAD_TILEG3,
                                        ENG_CALIB_DEAD_EME0,
                                        ENG_CALIB_DEAD_HEC0,
                                        ENG_CALIB_DEAD_FCAL,
                                        ENG_CALIB_DEAD_LEAKAGE,
                                        ENG_CALIB_DEAD_UNCLASS,
                                        ENG_CALIB_FRAC_EM,
                                        ENG_CALIB_FRAC_HAD,
                                        ENG_CALIB_FRAC_REST)
    )
    else
      {
        failed = true;
        return xAOD::CaloCluster::ENERGY_DigiHSTruth;
      }
  };


  auto process_moments = [&](const std::vector<std::string> & moment_names, std::string & invalid_names)
  {
    for (const std::string & mom_name : moment_names)
      {
        bool failed = false;
        const int linear_num = MomentsOptionsArray::moment_to_linear(get_moment_from_string(mom_name, failed));

        failed = failed || linear_num >= MomentsOptionsArray::num_moments;

        if (failed)
          {
            if (invalid_names.size() == 0)
              {
                invalid_names = "'" + mom_name + "'";
              }
            else
              {
                invalid_names += ", '" + mom_name + "'";
              }
          }
        else
          {
            m_momentsToDo.array[linear_num] = true;
          }
      }
  };

  std::string invalid_names;

  process_moments(m_momentsNames, invalid_names);

  if (invalid_names.size() > 0)
    {
      ATH_MSG_ERROR( "Moments " << invalid_names
                     << " are not valid moments and will be ignored!" );
    }

  m_doHVMoments = (m_momentsToDo[xAOD::CaloCluster::ENG_BAD_HV_CELLS] || m_momentsToDo[xAOD::CaloCluster::N_BAD_HV_CELLS]) && m_fillHVMoments;
  return StatusCode::SUCCESS;
}


StatusCode GPUToAthenaImporterWithMoments::convert (const EventContext & ctx,
                                                    const ConstantDataHolder &,
                                                    EventDataHolder & ed,
                                                    xAOD::CaloClusterContainer * cluster_container) const
{

  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  cluster_container->clear();

  const auto start = clock_type::now();

  SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);
  
  if ( !cell_collection.isValid() )
    {
      ATH_MSG_ERROR( " Cannot retrieve CaloCellContainer: " << cell_collection.name()  );
      return StatusCode::FAILURE;
    }
  const DataLink<CaloCellContainer> cell_collection_link (cell_collection.name(), ctx);

  size_t extra_times[6];

  const auto before_export = clock_type::now();

  ed.returnAndExportClusters(cluster_container,
                             &cell_collection_link,
                             m_momentsToDo,
                             false,
                             m_saveUncalibrated,
                             false,
                             m_missingCellsToFill,
                             m_measureTimes ? extra_times : nullptr);

  const auto after_export = clock_type::now();

  for (size_t i = 0; i < cluster_container->size(); ++i)
    {
      (*cluster_container)[i]->setClusterSize(m_clusterSize);
    }

  const auto after_size = clock_type::now();

  if (m_doHVMoments)
    {
      SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl(m_HVCablingKey, ctx);
      SG::ReadCondHandle<ILArHVScaleCorr> hvScaleHdl(m_HVScaleKey, ctx);
      const LArOnOffIdMapping * cabling = *cablingHdl;
      const ILArHVScaleCorr * hvcorr = *hvScaleHdl;

      std::vector<double> HV_energies(ed.m_clusters->number, 0.);
      std::vector<int>    HV_numbers(ed.m_clusters->number, 0.);

      for (int i = 0; i < ed.m_clusters->number_cells; ++i)
        {
          const int this_cluster = ed.m_clusters->clusterIndices[i];
            
          const int this_cell_index = ed.m_clusters->cells.indices[i];
          const int this_hash_ID = ed.m_cell_info->get_hash_ID(this_cell_index, ed.m_cell_info->complete);

          if (GeometryArr::is_tile(this_hash_ID))
            {
              continue;
            }
          
          HWIdentifier hwid = cabling->createSignalChannelIDFromHash(this_hash_ID);
          const float corr = hvcorr->HVScaleCorr(hwid);

          if (corr > 0.f && corr < 100.f && fabsf(corr - 1.f) > m_HVthreshold)
            {
              HV_energies[this_cluster] += fabsf(ed.m_cell_info->energy[this_cell_index]);
              ++HV_numbers[this_cluster];
            }
        }

      for (int i = 0; i < ed.m_clusters->number; ++i)
        {
          xAOD::CaloCluster * cluster = (*cluster_container)[i];
          if (m_momentsToDo[xAOD::CaloCluster::ENG_BAD_HV_CELLS])
            {
              cluster->insertMoment(xAOD::CaloCluster::ENG_BAD_HV_CELLS, HV_energies[i]);
            }
          if (m_momentsToDo[xAOD::CaloCluster::N_BAD_HV_CELLS])
            {
              cluster->insertMoment(xAOD::CaloCluster::N_BAD_HV_CELLS, HV_numbers[i]);
            }
        }
    }
  
  if (!m_keepGPUData)
    {
      ed.clear_GPU();
    }
  
  const auto after_HV = clock_type::now();

  if (m_measureTimes)
    {
      record_times(ctx.evt(),
                   time_cast(start, before_export),
                   extra_times[0],
                   extra_times[1],
                   extra_times[2],
                   extra_times[3],
                   extra_times[4],
                   extra_times[5],
                   time_cast(after_export, after_size),
                   time_cast(after_size, after_HV)
                  );
    }

  return StatusCode::SUCCESS;

}


StatusCode GPUToAthenaImporterWithMoments::finalize()
{
  if (m_measureTimes)
    {
      print_times("Preprocessing Number_and_State Link_Creation Cell_Processing Sorting Basic_Info Moments Cluster_Size HV_Moments", 9);
    }
  return StatusCode::SUCCESS;
}

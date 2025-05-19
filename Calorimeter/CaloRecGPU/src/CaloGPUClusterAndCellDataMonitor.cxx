//
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloGPUClusterAndCellDataMonitor.h"
#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "StoreGate/DataHandle.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "CaloRecGPU/CaloClusterGPUProcessor.h"

#include "AthenaMonitoringKernel/Monitored.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include <map>
#include <numeric>
#include <algorithm>

using namespace CaloRecGPU;

CaloGPUClusterAndCellDataMonitor::CaloGPUClusterAndCellDataMonitor(const std::string & type, const std::string & name, const IInterface * parent):
  base_class(type, name, parent),
  m_plottedVariablesInitialized(false)
{
}

StatusCode CaloGPUClusterAndCellDataMonitor::initialize()
{
  ATH_CHECK( m_cellsKey.initialize() );

  ATH_CHECK( detStore()->retrieve(m_calo_id, "CaloCell_ID") );

  const std::string this_name = this->name();

  const std::string algorithm_name_prefix = this_name.substr(0, this_name.rfind('.'));
  //This is so we take into account the fact that tools
  //are prefixed with the parent algorithm's name.

  auto final_string = [& algorithm_name_prefix](const std::string & unpref_str) -> std::string
  {
    return algorithm_name_prefix + "." + unpref_str;
  };

  const MatchingOptions opts = m_matchingOptions;

  m_min_similarity = opts.min_similarity;
  m_seed_weight = opts.seed_w;
  m_grow_weight = opts.grow_w;
  m_terminal_weight = opts.term_w;

  for (const auto & tool : m_toolsToPlot)
    {
      const std::string tool_name = final_string(tool.tool);
      m_toolToIdMap[tool_name] = tool.plot_id;
      m_toolsToCheckFor[tool_name] = -1;
    }

  auto add_tool_from_pair = [this](const std::string & name) -> int
  {
    if (!m_toolsToCheckFor.count(name))
      {
        m_toolsToCheckFor[name] = m_numToolsToKeep;
        m_toolToIdMap[name] = "";
        return m_numToolsToKeep++;
      }
    else
      {
        const int current = m_toolsToCheckFor[name];
        if (current >= 0)
          {
            return current;
          }
        else
          {
            m_toolsToCheckFor[name] = m_numToolsToKeep;
            return m_numToolsToKeep++;
          }
      }
  };

  for (const auto & pair : m_pairsToPlot)
    {
      const int first_index = add_tool_from_pair(final_string(pair.tool_ref));
      const int second_index = add_tool_from_pair(final_string(pair.tool_test));
      m_toolCombinations.emplace_back(pair_to_plot{first_index, second_index, pair.plot_id,
                                                   pair.match_in_energy,
                                                   pair.match_without_shared,
                                                   pair.match_perfectly});
    }

  ATH_CHECK( m_moniTool.retrieve() );

  return StatusCode::SUCCESS;
}

StatusCode CaloGPUClusterAndCellDataMonitor::finalize_plots() const
{
  //Well, not do plots, just monitor the number of events and the total number of clusters...

  auto mon_num_events = Monitored::Scalar("num_events", m_numEvents);

  for (const auto & k_v : m_toolToIdMap)
    {
      auto mon_num_clust = Monitored::Scalar(k_v.second + "_num_total_clusters", m_numClustersPerTool.at(k_v.first).load());
    }

  return StatusCode::SUCCESS;
}

StatusCode CaloGPUClusterAndCellDataMonitor::update_plots_start(const EventContext & /*ctx*/,
                                                                const ConstantDataHolder & /*constant_data*/,
                                                                const xAOD::CaloClusterContainer * /*cluster_collection_ptr*/) const
{
  if (!m_plottedVariablesInitialized.load())
    {
      std::lock_guard<std::mutex> lock_guard(m_mutex);
      if (!m_plottedVariablesInitialized.load())
        {
          CaloGPUClusterAndCellDataMonitor * dhis ATLAS_THREAD_SAFE = const_cast<CaloGPUClusterAndCellDataMonitor *>(this);
          //We have the mutex.
          //It's safe.
          ATH_CHECK( dhis->initialize_plotted_variables() );
          m_plottedVariablesInitialized.store(true);
        }
    }
  if (m_numToolsToKeep > 0)
    {
      m_storageHolder.get_one().resize(m_numToolsToKeep);
    }
  //Allocate a vector of data holders for this thread and resize it to the necessary size.

  return StatusCode::SUCCESS;
}

StatusCode CaloGPUClusterAndCellDataMonitor::update_plots_end(const EventContext & ctx,
                                                              const ConstantDataHolder & constant_data,
                                                              const xAOD::CaloClusterContainer * /*cluster_collection_ptr*/) const
{
  ATH_MSG_INFO("");

  for (const auto & combination : m_toolCombinations)
    {
      if (combination.index_ref < 0 || combination.index_test < 0)
        {
          ATH_MSG_WARNING("Invalid tool combination, please check your configuration! " << combination.prefix);
          continue;
        }
      ATH_CHECK( add_combination(ctx, constant_data, combination.index_ref, combination.index_test, combination.prefix,
                                 combination.match_in_energy, combination.match_without_shared, combination.match_perfectly) );
    }

  ATH_MSG_INFO("");

  if (m_numToolsToKeep > 0)
    {
      m_storageHolder.release_one();
      //Release the tool storage.
    }

  return StatusCode::SUCCESS;
}

StatusCode CaloGPUClusterAndCellDataMonitor::update_plots(const EventContext & ctx,
                                                          const ConstantDataHolder & constant_data,
                                                          const xAOD::CaloClusterContainer * cluster_collection_ptr,
                                                          const CaloClusterCollectionProcessor * tool) const
{
  if (filter_tool_by_name(tool->name()))
    {
      Helpers::CPU_object<CellInfoArr> cell_info;
      Helpers::CPU_object<CellStateArr> cell_state;
      Helpers::CPU_object<ClusterInfoArr> clusters;
      Helpers::CPU_object<ClusterMomentsArr> moments;

      ATH_CHECK( convert_to_GPU_data_structures(ctx, constant_data, cluster_collection_ptr,
                                                cell_info, cell_state, clusters, moments        )  );

      return add_data(ctx, constant_data, cell_info, cell_state, clusters, moments, tool->name());
    }
  else
    {
      return StatusCode::SUCCESS;
    }
}

StatusCode CaloGPUClusterAndCellDataMonitor::update_plots(const EventContext & ctx,
                                                          const ConstantDataHolder & constant_data,
                                                          const xAOD::CaloClusterContainer * /*cluster_collection_ptr*/,
                                                          const EventDataHolder & event_data,
                                                          const ICaloClusterGPUInputTransformer * tool) const
{
  if (filter_tool_by_name(tool->name()))
    {
      return add_data(ctx, constant_data, event_data.m_cell_info_dev, event_data.m_cell_state_dev,
                      event_data.m_clusters_dev, event_data.m_moments_dev, tool->name());
    }
  else
    {
      return StatusCode::SUCCESS;
    }
}

StatusCode CaloGPUClusterAndCellDataMonitor::update_plots(const EventContext & ctx,
                                                          const ConstantDataHolder & constant_data,
                                                          const xAOD::CaloClusterContainer * /*cluster_collection_ptr*/,
                                                          const EventDataHolder & event_data,
                                                          const CaloClusterGPUProcessor * tool) const
{
  if (filter_tool_by_name(tool->name()))
    {
      Helpers::CPU_object<CellInfoArr> cell_info = event_data.m_cell_info_dev;
      Helpers::CPU_object<CellStateArr> cell_state =  event_data.m_cell_state_dev;
      Helpers::CPU_object<ClusterInfoArr> clusters = event_data.m_clusters_dev;
      Helpers::CPU_object<CaloRecGPU::ClusterMomentsArr> moments = event_data.m_moments_dev;

      ATH_CHECK( compactify_clusters(ctx, constant_data, cell_info, cell_state, clusters, moments) );

      return add_data(ctx, constant_data, cell_info, cell_state, clusters, moments, tool->name());
    }
  else
    {
      return StatusCode::SUCCESS;
    }
}

StatusCode CaloGPUClusterAndCellDataMonitor::update_plots(const EventContext & ctx,
                                                          const ConstantDataHolder & constant_data,
                                                          const xAOD::CaloClusterContainer * cluster_collection_ptr,
                                                          const EventDataHolder & /*event_data*/,
                                                          const ICaloClusterGPUOutputTransformer * tool) const
{
  if (filter_tool_by_name(tool->name()))
    {
      Helpers::CPU_object<CellInfoArr> cell_info;
      Helpers::CPU_object<CellStateArr> cell_state;
      Helpers::CPU_object<ClusterInfoArr> clusters;
      Helpers::CPU_object<CaloRecGPU::ClusterMomentsArr> moments;

      ATH_CHECK( convert_to_GPU_data_structures(ctx, constant_data, cluster_collection_ptr,
                                                cell_info, cell_state, clusters, moments          )  );

      return add_data(ctx, constant_data, cell_info, cell_state, clusters, moments, tool->name());
    }
  else
    {
      return StatusCode::SUCCESS;
    }
}

bool CaloGPUClusterAndCellDataMonitor::filter_tool_by_name(const std::string & tool_name)
const
{
  ATH_MSG_DEBUG("Checking : '" << tool_name << "': " << m_toolsToCheckFor.count(tool_name));
  return m_toolsToCheckFor.count(tool_name) > 0;
}

//Note: The following is essentially copied over from two tools.
//      Maybe we could prevent the repetition?

StatusCode CaloGPUClusterAndCellDataMonitor::convert_to_GPU_data_structures(const EventContext & ctx,
                                                                            const ConstantDataHolder & /*constant_data*/,
                                                                            const xAOD::CaloClusterContainer * cluster_collection,
                                                                            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> & ret_info,
                                                                            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> & ret_state,
                                                                            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> & ret_clusts,
                                                                            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterMomentsArr> & ret_moments) const
{
  SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);
  if ( !cell_collection.isValid() )
    {
      ATH_MSG_ERROR( " Cannot retrieve CaloCellContainer: " << cell_collection.name()  );
      return StatusCode::FAILURE;
    }

  ret_info.allocate();

  if (cluster_collection != nullptr)
    {
      ret_state.allocate();
      ret_clusts.allocate();
      ret_moments.allocate();
    }

  for (int i = 0; i < NCaloCells; ++i)
    {
      ret_info->energy[i] = 0;
      ret_info->gain[i] = GainConversion::invalid_gain();
      ret_info->time[i] = 0;
      ret_info->qualityProvenance[i] = 0;

      if (cluster_collection != nullptr)
        {
          ret_state->clusterTag[i] = ClusterTag::make_invalid_tag();
        }
    }

  for (CaloCellContainer::const_iterator iCells = cell_collection->begin(); iCells != cell_collection->end(); ++iCells)
    {
      const CaloCell * cell = (*iCells);

      const int index = m_calo_id->calo_cell_hash(cell->ID());

      const float energy = cell->energy();

      const unsigned int gain = GainConversion::from_standard_gain(cell->gain());

      ret_info->energy[index] = energy;
      ret_info->gain[index] = gain;
      ret_info->time[index] = cell->time();
      ret_info->qualityProvenance[index] = QualityProvenance{cell->quality(), cell->provenance()};

    }

  if (cluster_collection != nullptr)
    {
      const auto cluster_end = cluster_collection->end();
      auto cluster_iter = cluster_collection->begin();

      for (int cluster_number = 0; cluster_iter != cluster_end; ++cluster_iter, ++cluster_number )
        {
          const xAOD::CaloCluster * cluster = (*cluster_iter);
          const CaloClusterCellLink * cell_links = cluster->getCellLinks();
          if (!cell_links)
            {
              ATH_MSG_ERROR("Can't get valid links to CaloCells (CaloClusterCellLink)!");
              return StatusCode::FAILURE;
            }

          ret_clusts->clusterEnergy[cluster_number] = cluster->e();
          ret_clusts->clusterEt[cluster_number] = cluster->et();
          ret_clusts->clusterEta[cluster_number] = cluster->eta();
          ret_clusts->clusterPhi[cluster_number] = cluster->phi();
          ret_clusts->seedCellID[cluster_number] = m_calo_id->calo_cell_hash(cluster->cell_begin()->ID());
          for (int i = 0; i < NumSamplings; ++i)
            {
              ret_moments->energyPerSample[i][cluster_number] = cluster->eSample((CaloSampling::CaloSample) i);
              ret_moments->maxEPerSample[i][cluster_number] = cluster->energy_max((CaloSampling::CaloSample) i);
              ret_moments->maxPhiPerSample[i][cluster_number] = cluster->phimax((CaloSampling::CaloSample) i);
              ret_moments->maxEtaPerSample[i][cluster_number] = cluster->etamax((CaloSampling::CaloSample) i);
              ret_moments->etaPerSample[i][cluster_number] = cluster->etaSample((CaloSampling::CaloSample) i);
              ret_moments->phiPerSample[i][cluster_number] = cluster->phiSample((CaloSampling::CaloSample) i);
              ret_moments->nCellSampling[i][cluster_number] = cluster->numberCellsInSampling((CaloSampling::CaloSample) i);
            }
          ret_moments->time[cluster_number] = cluster->time();
          ret_moments->firstPhi[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::FIRST_PHI);
          ret_moments->firstEta[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::FIRST_ETA);
          ret_moments->secondR[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::SECOND_R);
          ret_moments->secondLambda[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::SECOND_LAMBDA);
          ret_moments->deltaPhi[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::DELTA_PHI);
          ret_moments->deltaTheta[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::DELTA_THETA);
          ret_moments->deltaAlpha[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::DELTA_ALPHA);
          ret_moments->centerX[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::CENTER_X);
          ret_moments->centerY[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::CENTER_Y);
          ret_moments->centerZ[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::CENTER_Z);
          ret_moments->centerMag[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::CENTER_MAG);
          ret_moments->centerLambda[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::CENTER_LAMBDA);
          ret_moments->lateral[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::LATERAL);
          ret_moments->longitudinal[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::LONGITUDINAL);
          ret_moments->engFracEM[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_FRAC_EM);
          ret_moments->engFracMax[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_FRAC_MAX);
          ret_moments->engFracCore[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_FRAC_CORE);
          ret_moments->firstEngDens[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::FIRST_ENG_DENS);
          ret_moments->secondEngDens[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::SECOND_ENG_DENS);
          ret_moments->isolation[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ISOLATION);
          ret_moments->engBadCells[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_BAD_CELLS);
          ret_moments->nBadCells[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::N_BAD_CELLS);
          ret_moments->nBadCellsCorr[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::N_BAD_CELLS_CORR);
          ret_moments->badCellsCorrE[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::BAD_CELLS_CORR_E);
          ret_moments->badLArQFrac[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::BADLARQ_FRAC);
          ret_moments->engPos[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_POS);
          ret_moments->significance[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::SIGNIFICANCE);
          ret_moments->cellSignificance[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::CELL_SIGNIFICANCE);
          ret_moments->cellSigSampling[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::CELL_SIG_SAMPLING);
          ret_moments->avgLArQ[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::AVG_LAR_Q);
          ret_moments->avgTileQ[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::AVG_TILE_Q);
          ret_moments->engBadHVCells[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_BAD_HV_CELLS);
          ret_moments->nBadHVCells[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::N_BAD_HV_CELLS);
          ret_moments->PTD[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::PTD);
          ret_moments->mass[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::MASS);
          ret_moments->EMProbability[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::EM_PROBABILITY);
          ret_moments->hadWeight[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::HAD_WEIGHT);
          ret_moments->OOCweight[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::OOC_WEIGHT);
          ret_moments->DMweight[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::DM_WEIGHT);
          ret_moments->tileConfidenceLevel[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::TILE_CONFIDENCE_LEVEL);
          ret_moments->secondTime[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::SECOND_TIME);
          ret_moments->nExtraCellSampling[cluster_number] = cluster->numberCellsInSampling(CaloSampling::EME2, true);
          ret_moments->numCells[cluster_number] = cluster->numberCells();
          ret_moments->vertexFraction[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::VERTEX_FRACTION);
          ret_moments->nVertexFraction[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::NVERTEX_FRACTION);
          ret_moments->etaCaloFrame[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ETACALOFRAME);
          ret_moments->phiCaloFrame[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::PHICALOFRAME);
          ret_moments->eta1CaloFrame[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ETA1CALOFRAME);
          ret_moments->phi1CaloFrame[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::PHI1CALOFRAME);
          ret_moments->eta2CaloFrame[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ETA2CALOFRAME);
          ret_moments->phi2CaloFrame[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::PHI2CALOFRAME);
          ret_moments->engCalibTot[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_TOT);
          ret_moments->engCalibOutL[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_OUT_L);
          ret_moments->engCalibOutM[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_OUT_M);
          ret_moments->engCalibOutT[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_OUT_T);
          ret_moments->engCalibDeadL[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_L);
          ret_moments->engCalibDeadM[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_M);
          ret_moments->engCalibDeadT[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_T);
          ret_moments->engCalibEMB0[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_EMB0);
          ret_moments->engCalibEME0[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_EME0);
          ret_moments->engCalibTileG3[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_TILEG3);
          ret_moments->engCalibDeadTot[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_TOT);
          ret_moments->engCalibDeadEMB0[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_EMB0);
          ret_moments->engCalibDeadTile0[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_TILE0);
          ret_moments->engCalibDeadTileG3[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_TILEG3);
          ret_moments->engCalibDeadEME0[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_EME0);
          ret_moments->engCalibDeadHEC0[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_HEC0);
          ret_moments->engCalibDeadFCAL[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_FCAL);
          ret_moments->engCalibDeadLeakage[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_LEAKAGE);
          ret_moments->engCalibDeadUnclass[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_DEAD_UNCLASS);
          ret_moments->engCalibFracEM[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_FRAC_EM);
          ret_moments->engCalibFracHad[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_FRAC_HAD);
          ret_moments->engCalibFracRest[cluster_number] = cluster->getMomentValue(xAOD::CaloCluster::ENG_CALIB_FRAC_REST);
          for (auto it = cell_links->begin(); it != cell_links->end(); ++it)
            {
              const int cell_ID = m_calo_id->calo_cell_hash(it->ID());
              const float weight = it.weight();

              uint32_t weight_as_int = 0;
              std::memcpy(&weight_as_int, &weight, sizeof(float));
              //On the platforms we expect to be running this, it should be fine.
              //Still UB.
              //With C++20, we could do that bit-cast thing.

              if (weight_as_int == 0)
                {
                  weight_as_int = 1;
                  //Subnormal,
                  //but just to distinguish from
                  //a non-shared cluster.
                }

              const ClusterTag other_tag = ret_state->clusterTag[cell_ID];

              const int other_index = other_tag.is_part_of_cluster() ? other_tag.cluster_index() : -1;

              if (other_index < 0)
                {
                  if (weight < 0.5f)
                    {
                      ret_state->clusterTag[cell_ID] = ClusterTag::make_tag(cluster_number, weight_as_int, 0);
                    }
                  else
                    {
                      ret_state->clusterTag[cell_ID] = ClusterTag::make_tag(cluster_number);
                    }
                }
              else if (weight > 0.5f)
                {
                  ret_state->clusterTag[cell_ID] = ClusterTag::make_tag(cluster_number, other_tag.secondary_cluster_weight(), other_index);
                }
              else if (weight == 0.5f)
                //Unlikely, but...
                {
                  const int max_cluster = cluster_number > other_index ? cluster_number : other_index;
                  const int min_cluster = cluster_number > other_index ? other_index : cluster_number;
                  ret_state->clusterTag[cell_ID] = ClusterTag::make_tag(max_cluster, weight_as_int, min_cluster);
                }
              else /*if (weight < 0.5f)*/
                {
                  ret_state->clusterTag[cell_ID] = ClusterTag::make_tag(other_index, weight_as_int, cluster_number);
                }
            }
        }

      ret_clusts->number = cluster_collection->size();
    }

  return StatusCode::SUCCESS;
}

StatusCode CaloGPUClusterAndCellDataMonitor::compactify_clusters(const EventContext &,
                                                                 const CaloRecGPU::ConstantDataHolder &,
                                                                 const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> & cell_info,
                                                                 CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> & cell_state,
                                                                 CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> & clusters,
                                                                 CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterMomentsArr> & moments) const
{
  std::map<int, int> tag_map;

  std::vector<int> cluster_order(clusters->number);

  std::iota(cluster_order.begin(), cluster_order.end(), 0);

  std::sort(cluster_order.begin(), cluster_order.end(), [&](const int a, const int b)
  {
    if (clusters->seedCellID[a] < 0)
      {
        return false;
        //This means that clusters with no cells
        //(marked as invalid) always compare lower,
        //so they appear in the end.
      }
    else if (clusters->seedCellID[b] < 0)
      {
        return true;
      }
    return clusters->clusterEt[a] > clusters->clusterEt[b];
  } );

  int real_cluster_numbers = clusters->number;

  for (size_t i = 0; i < cluster_order.size(); ++i)
    {
      const int this_id = cluster_order[i];
      if (clusters->seedCellID[this_id] < 0)
        {
          tag_map[this_id] = -1;
          --real_cluster_numbers;
        }
      else
        {
          tag_map[this_id] = i;
        }
    }

  const Helpers::CPU_object<ClusterInfoArr> temp_clusters(clusters);
  const Helpers::CPU_object<ClusterMomentsArr> temp_moments(moments);

  clusters->number = real_cluster_numbers;

  for (int i = 0; i < temp_clusters->number; ++i)
    {
      clusters->clusterEnergy[i]      =   temp_clusters->clusterEnergy[cluster_order[i]];
      clusters->clusterEt[i]          =   temp_clusters->clusterEt[cluster_order[i]];
      clusters->clusterEta[i]         =   temp_clusters->clusterEta[cluster_order[i]];
      clusters->clusterPhi[i]         =   temp_clusters->clusterPhi[cluster_order[i]];
      clusters->seedCellID[i]         =   temp_clusters->seedCellID[cluster_order[i]];
      for (int j = 0; j < NumSamplings; ++j)
        {
          moments->energyPerSample[j][i]     =   temp_moments->energyPerSample[j][cluster_order[i]];
          moments->maxEPerSample[j][i]       =   temp_moments->maxEPerSample[j][cluster_order[i]];
          moments->maxPhiPerSample[j][i]     =   temp_moments->maxPhiPerSample[j][cluster_order[i]];
          moments->maxEtaPerSample[j][i]     =   temp_moments->maxEtaPerSample[j][cluster_order[i]];
          moments->etaPerSample[j][i]        =   temp_moments->etaPerSample[j][cluster_order[i]];
          moments->phiPerSample[j][i]        =   temp_moments->phiPerSample[j][cluster_order[i]];
        }
      moments->time[i]                =   temp_moments->time[cluster_order[i]];
      moments->firstPhi[i]            =   temp_moments->firstPhi[cluster_order[i]];
      moments->firstEta[i]            =   temp_moments->firstEta[cluster_order[i]];
      moments->secondR[i]             =   temp_moments->secondR[cluster_order[i]];
      moments->secondLambda[i]        =   temp_moments->secondLambda[cluster_order[i]];
      moments->deltaPhi[i]            =   temp_moments->deltaPhi[cluster_order[i]];
      moments->deltaTheta[i]          =   temp_moments->deltaTheta[cluster_order[i]];
      moments->deltaAlpha[i]          =   temp_moments->deltaAlpha[cluster_order[i]];
      moments->centerX[i]             =   temp_moments->centerX[cluster_order[i]];
      moments->centerY[i]             =   temp_moments->centerY[cluster_order[i]];
      moments->centerZ[i]             =   temp_moments->centerZ[cluster_order[i]];
      moments->centerMag[i]           =   temp_moments->centerMag[cluster_order[i]];
      moments->centerLambda[i]        =   temp_moments->centerLambda[cluster_order[i]];
      moments->lateral[i]             =   temp_moments->lateral[cluster_order[i]];
      moments->longitudinal[i]        =   temp_moments->longitudinal[cluster_order[i]];
      moments->engFracEM[i]           =   temp_moments->engFracEM[cluster_order[i]];
      moments->engFracMax[i]          =   temp_moments->engFracMax[cluster_order[i]];
      moments->engFracCore[i]         =   temp_moments->engFracCore[cluster_order[i]];
      moments->firstEngDens[i]        =   temp_moments->firstEngDens[cluster_order[i]];
      moments->secondEngDens[i]       =   temp_moments->secondEngDens[cluster_order[i]];
      moments->isolation[i]           =   temp_moments->isolation[cluster_order[i]];
      moments->engBadCells[i]         =   temp_moments->engBadCells[cluster_order[i]];
      moments->nBadCells[i]           =   temp_moments->nBadCells[cluster_order[i]];
      moments->nBadCellsCorr[i]       =   temp_moments->nBadCellsCorr[cluster_order[i]];
      moments->badCellsCorrE[i]       =   temp_moments->badCellsCorrE[cluster_order[i]];
      moments->badLArQFrac[i]         =   temp_moments->badLArQFrac[cluster_order[i]];
      moments->engPos[i]              =   temp_moments->engPos[cluster_order[i]];
      moments->significance[i]        =   temp_moments->significance[cluster_order[i]];
      moments->cellSignificance[i]    =   temp_moments->cellSignificance[cluster_order[i]];
      moments->cellSigSampling[i]     =   temp_moments->cellSigSampling[cluster_order[i]];
      moments->avgLArQ[i]             =   temp_moments->avgLArQ[cluster_order[i]];
      moments->avgTileQ[i]            =   temp_moments->avgTileQ[cluster_order[i]];
      moments->engBadHVCells[i]       =   temp_moments->engBadHVCells[cluster_order[i]];
      moments->nBadHVCells[i]         =   temp_moments->nBadHVCells[cluster_order[i]];
      moments->PTD[i]                 =   temp_moments->PTD[cluster_order[i]];
      moments->mass[i]                =   temp_moments->mass[cluster_order[i]];
      moments->EMProbability[i]       =   temp_moments->EMProbability[cluster_order[i]];
      moments->hadWeight[i]           =   temp_moments->hadWeight[cluster_order[i]];
      moments->OOCweight[i]           =   temp_moments->OOCweight[cluster_order[i]];
      moments->DMweight[i]            =   temp_moments->DMweight[cluster_order[i]];
      moments->tileConfidenceLevel[i] =   temp_moments->tileConfidenceLevel[cluster_order[i]];
      moments->secondTime[i]          =   temp_moments->secondTime[cluster_order[i]];
      for (int j = 0; j < NumSamplings; ++j)
        {
          moments->nCellSampling[j][i]       =   temp_moments->nCellSampling[j][cluster_order[i]];
        }
      moments->nExtraCellSampling[i]  =   temp_moments->nExtraCellSampling[cluster_order[i]];
      moments->numCells[i]            =   temp_moments->numCells[cluster_order[i]];
      moments->vertexFraction[i]      =   temp_moments->vertexFraction[cluster_order[i]];
      moments->nVertexFraction[i]     =   temp_moments->nVertexFraction[cluster_order[i]];
      moments->etaCaloFrame[i]        =   temp_moments->etaCaloFrame[cluster_order[i]];
      moments->phiCaloFrame[i]        =   temp_moments->phiCaloFrame[cluster_order[i]];
      moments->eta1CaloFrame[i]       =   temp_moments->eta1CaloFrame[cluster_order[i]];
      moments->phi1CaloFrame[i]       =   temp_moments->phi1CaloFrame[cluster_order[i]];
      moments->eta2CaloFrame[i]       =   temp_moments->eta2CaloFrame[cluster_order[i]];
      moments->phi2CaloFrame[i]       =   temp_moments->phi2CaloFrame[cluster_order[i]];
      moments->engCalibTot[i]         =   temp_moments->engCalibTot[cluster_order[i]];
      moments->engCalibOutL[i]        =   temp_moments->engCalibOutL[cluster_order[i]];
      moments->engCalibOutM[i]        =   temp_moments->engCalibOutM[cluster_order[i]];
      moments->engCalibOutT[i]        =   temp_moments->engCalibOutT[cluster_order[i]];
      moments->engCalibDeadL[i]       =   temp_moments->engCalibDeadL[cluster_order[i]];
      moments->engCalibDeadM[i]       =   temp_moments->engCalibDeadM[cluster_order[i]];
      moments->engCalibDeadT[i]       =   temp_moments->engCalibDeadT[cluster_order[i]];
      moments->engCalibEMB0[i]        =   temp_moments->engCalibEMB0[cluster_order[i]];
      moments->engCalibEME0[i]        =   temp_moments->engCalibEME0[cluster_order[i]];
      moments->engCalibTileG3[i]      =   temp_moments->engCalibTileG3[cluster_order[i]];
      moments->engCalibDeadTot[i]     =   temp_moments->engCalibDeadTot[cluster_order[i]];
      moments->engCalibDeadEMB0[i]    =   temp_moments->engCalibDeadEMB0[cluster_order[i]];
      moments->engCalibDeadTile0[i]   =   temp_moments->engCalibDeadTile0[cluster_order[i]];
      moments->engCalibDeadTileG3[i]  =   temp_moments->engCalibDeadTileG3[cluster_order[i]];
      moments->engCalibDeadEME0[i]    =   temp_moments->engCalibDeadEME0[cluster_order[i]];
      moments->engCalibDeadHEC0[i]    =   temp_moments->engCalibDeadHEC0[cluster_order[i]];
      moments->engCalibDeadFCAL[i]    =   temp_moments->engCalibDeadFCAL[cluster_order[i]];
      moments->engCalibDeadLeakage[i] =   temp_moments->engCalibDeadLeakage[cluster_order[i]];
      moments->engCalibDeadUnclass[i] =   temp_moments->engCalibDeadUnclass[cluster_order[i]];
      moments->engCalibFracEM[i]      =   temp_moments->engCalibFracEM[cluster_order[i]];
      moments->engCalibFracHad[i]     =   temp_moments->engCalibFracHad[cluster_order[i]];
      moments->engCalibFracRest[i]    =   temp_moments->engCalibFracRest[cluster_order[i]];
    }

  for (int i = 0; i < NCaloCells; ++i)
    {
      if (!cell_info->is_valid(i))
        {
          continue;
        }
      const ClusterTag this_tag = cell_state->clusterTag[i];
      if (!this_tag.is_part_of_cluster())
        {
          cell_state->clusterTag[i] = ClusterTag::make_invalid_tag();
        }
      else if (this_tag.is_part_of_cluster())
        {
          const int old_idx = this_tag.cluster_index();
          const int new_idx = tag_map[old_idx];
          const int old_idx2 = this_tag.is_shared_between_clusters() ? this_tag.secondary_cluster_index() : -1;
          const int new_idx2 = old_idx2 >= 0 ? tag_map[old_idx2] : -1;
          if (new_idx < 0 && new_idx2 < 0)
            {
              cell_state->clusterTag[i] = ClusterTag::make_invalid_tag();
            }
          else if (new_idx < 0)
            {
              cell_state->clusterTag[i] = ClusterTag::make_tag(new_idx2);
            }
          else if (new_idx2 < 0)
            {
              cell_state->clusterTag[i] = ClusterTag::make_tag(new_idx);
            }
          else
            {
              cell_state->clusterTag[i] = ClusterTag::make_tag(new_idx, this_tag.secondary_cluster_weight(), new_idx2);
            }
        }
    }

  return StatusCode::SUCCESS;
}

namespace
{
  template <class ... Args>
  struct multi_class_holder
  {
    static constexpr size_t size()
    {
      return sizeof...(Args);
    }
  };

  //To deal with potential lack of maybe_unused in pack expansions.
  template <class Arg>
  static constexpr decltype(auto) suppress_warning(Arg && a)
  {
    return std::forward<Arg>(a);
  }

  template <class F, class ... Types, class ... Args>
  void apply_to_multi_class(F && f, const multi_class_holder<Types...> &, Args && ... args)
  {
    size_t i = 0;
    if constexpr (std::is_same_v < decltype(f((Types{}, ...), i, std::forward<Args>(args)...)), void > )
      {
        (f(Types{}, i++, std::forward<Args>(args)...), ...);
      }
    else
      {
        (suppress_warning(f(Types{}, i++, std::forward<Args>(args)...)), ...);
      }
  }

  static float float_unhack(const unsigned int bits)
  {
    float res;
    std::memcpy(&res, &bits, sizeof(float));
    //In C++20, we should bit-cast. For now, for our platform, works.
    return res;
  }

  static double protect_from_zero(const double x)
  {
    return x == 0 ? 1e-15 : x;
  }

  static float protect_from_zero(const float x)
  {
    return x == 0 ? 1e-7 : x;
  }
}

StatusCode CaloGPUClusterAndCellDataMonitor::match_clusters(sample_comparisons_holder & sch,
                                                            const CaloRecGPU::ConstantDataHolder & constant_data,
                                                            const CaloRecGPU::CellInfoArr & cell_info,
                                                            const CaloRecGPU::CellStateArr & cell_state_1,
                                                            const CaloRecGPU::CellStateArr & cell_state_2,
                                                            const CaloRecGPU::ClusterInfoArr & cluster_info_1,
                                                            const CaloRecGPU::ClusterInfoArr & cluster_info_2,
                                                            const CaloRecGPU::ClusterMomentsArr & /*moments_1*/,
                                                            const CaloRecGPU::ClusterMomentsArr & /*moments_2*/,
                                                            const bool match_in_energy,
                                                            const bool match_without_shared) const
{
  sch.r2t_table.clear();
  sch.r2t_table.resize(cluster_info_1.number, -1);

  sch.t2r_table.clear();
  sch.t2r_table.resize(cluster_info_2.number, -1);

  std::vector<double> similarity_map(cluster_info_1.number * cluster_info_2.number, 0.);

  std::vector<double> ref_normalization(cluster_info_1.number, 0.);
  std::vector<double> test_normalization(cluster_info_2.number, 0.);

  for (int i = 0; i < NCaloCells; ++i)
    {
      const ClusterTag ref_tag = cell_state_1.clusterTag[i];
      const ClusterTag test_tag = cell_state_2.clusterTag[i];

      if (!cell_info.is_valid(i))
        {
          continue;
        }

      double SNR = 0.00001;

      if (!cell_info.is_bad(*(constant_data.m_geometry), i))
        {
          const int gain = cell_info.gain[i];

          const double cellNoise = constant_data.m_cell_noise->get_noise(i, gain);
          if (std::isfinite(cellNoise) && cellNoise > 0.0f)
            {
              SNR = std::abs(cell_info.energy[i] / cellNoise);
            }
        }

      const double quantity = ( match_in_energy ? std::abs(cell_info.energy[i]) : SNR );
      const double weight = (quantity + 1e-8) *
                            ( SNR > m_seedThreshold ? (match_in_energy ? 1000 : m_seed_weight) :
                              (
                                      SNR > m_growThreshold ? (match_in_energy ? 950 : m_grow_weight) :
                                      (
                                              SNR > m_termThreshold ? (match_in_energy ? 900 : m_terminal_weight) : (match_in_energy ? 100 : 0)
                                      )
                              )
                            );
      int ref_c1 = -1, ref_c2 = -1, test_c1 = -1, test_c2 = -1;

      if (ref_tag.is_part_of_cluster())
        {
          if (match_without_shared && ref_tag.is_shared_between_clusters())
            {
              continue;
            }
          ref_c1 = ref_tag.cluster_index();
          ref_c2 = ref_tag.is_shared_between_clusters() ? ref_tag.secondary_cluster_index() : ref_c1;
        }

      if (test_tag.is_part_of_cluster())
        {
          if (match_without_shared && test_tag.is_shared_between_clusters())
            {
              continue;
            }
          test_c1 = test_tag.cluster_index();
          test_c2 = test_tag.is_shared_between_clusters() ? test_tag.secondary_cluster_index() : test_c1;
        }

      float ref_rev_cw = float_unhack(ref_tag.secondary_cluster_weight());
      float test_rev_cw = float_unhack(test_tag.secondary_cluster_weight());

      float ref_cw = 1.0f - ref_rev_cw;
      float test_cw = 1.0f - test_rev_cw;

      if (ref_c1 >= int(cluster_info_1.number) || ref_c2 >= int(cluster_info_1.number) ||
          test_c1 >= int(cluster_info_2.number) || test_c2 >= int(cluster_info_2.number) )
        {
          ATH_MSG_DEBUG(  "Error in matches: " << i << " " << ref_c1 << " " << ref_c2 << " "
                          << test_c1 << " " << test_c2 << " ("
                          << cluster_info_1.number << " | " << cluster_info_2.number << ")"   );
          continue;
        }

      if (ref_c1 >= 0 && test_c1 >= 0)
        {
          similarity_map[test_c1 * cluster_info_1.number + ref_c1] += weight * ref_cw * test_cw;
          similarity_map[test_c1 * cluster_info_1.number + ref_c2] += weight * ref_rev_cw * test_cw;
          similarity_map[test_c2 * cluster_info_1.number + ref_c1] += weight * ref_cw * test_rev_cw;
          similarity_map[test_c2 * cluster_info_1.number + ref_c2] += weight * ref_rev_cw * test_rev_cw;
        }
      if (ref_c1 >= 0)
        {
          ref_normalization[ref_c1] += weight * ref_cw * ref_cw;
          ref_normalization[ref_c2] += weight * ref_rev_cw * ref_rev_cw;
        }
      if (test_c1 >= 0)
        {
          test_normalization[test_c1] += weight * test_cw * test_cw;
          test_normalization[test_c2] += weight * test_rev_cw * test_rev_cw;
        }
    }

  for (int testc = 0; testc < cluster_info_2.number; ++testc)
    {
      const double test_norm = test_normalization[testc] + double(test_normalization[testc] == 0.);
      for (int refc = 0; refc < cluster_info_1.number; ++refc)
        {
          const double ref_norm = ref_normalization[refc] + double(ref_normalization[refc] == 0.);
          similarity_map[testc * cluster_info_1.number + refc] /= std::sqrt(ref_norm * test_norm);
        }
    }

  //In essence, the Gale-Shapley Algorithm

  std::vector<std::vector<int>> sorted_GPU_matches;

  sorted_GPU_matches.reserve(cluster_info_2.number);

  for (int testc = 0; testc < cluster_info_2.number; ++testc)
    {
      std::vector<int> sorter(cluster_info_1.number);
      std::iota(sorter.begin(), sorter.end(), 0);

      std::sort(sorter.begin(), sorter.end(),
                [&](const int a, const int b)
      {
        const double a_weight = similarity_map[testc * cluster_info_1.number + a];
        const double b_weight = similarity_map[testc * cluster_info_1.number + b];
        return a_weight > b_weight;
      }
               );

      size_t wanted_size = 0;

      for (; wanted_size < sorter.size(); ++wanted_size)
        {
          const double match_weight = similarity_map[testc * cluster_info_1.number + sorter[wanted_size]];
          if (match_weight < m_min_similarity)
            {
              break;
            }
        }

      //Yeah, we could do a binary search for best worst-case complexity,
      //but we are expecting 1~2 similar clusters and the rest garbage,
      //so we're expecting only 1~2 iterations.
      //This actually means all that sorting is way way overkill,
      //but we must make sure in the most general case that this works...

      sorter.resize(wanted_size);

      sorted_GPU_matches.push_back(sorter);
    }

  int num_iter = 0;

  constexpr int max_iter = 32;

  std::vector<double> matched_weights(cluster_info_1.number, -1.);

  std::vector<size_t> skipped_matching(cluster_info_2.number, 0);

  for (int stop_counter = 0; stop_counter < cluster_info_2.number && num_iter < max_iter; ++num_iter)
    {
      stop_counter = 0;
      for (int testc = 0; testc < int(sorted_GPU_matches.size()); ++testc)
        {
          if (skipped_matching[testc] < sorted_GPU_matches[testc].size())
            {
              const int match_c = sorted_GPU_matches[testc][skipped_matching[testc]];
              const double match_weight = similarity_map[testc * cluster_info_1.number + match_c];
              if (match_weight >= m_min_similarity && match_weight > matched_weights[match_c])
                {
                  const int prev_match = sch.r2t_table[match_c];
                  if (prev_match >= 0)
                    {
                      ++skipped_matching[prev_match];
                      --stop_counter;
                    }
                  sch.r2t_table[match_c] = testc;
                  matched_weights[match_c] = match_weight;
                  ++stop_counter;
                }
              else
                {
                  ++skipped_matching[testc];
                }
            }
          else
            {
              ++stop_counter;
            }
        }
    }

  sch.unmatched_ref_list.clear();
  sch.unmatched_test_list.clear();

  for (size_t i = 0; i < sch.r2t_table.size(); ++i)
    {
      const int match = sch.r2t_table[i];
      if (match < 0)
        {
          sch.unmatched_ref_list.push_back(i);
        }
      else
        {
          sch.t2r_table[match] = i;
        }
    }

  for (size_t i = 0; i < sch.t2r_table.size(); ++i)
    {
      if (sch.t2r_table[i] < 0)
        {
          sch.unmatched_test_list.push_back(i);
        }
    }

  {
    char message_buffer[256];
    snprintf(message_buffer, 256,
             "%2d: %5d / %5d || %5d / %5d || %3d || %5d | %5d || %5d",
             num_iter,
             int(sch.r2t_table.size()) - int(sch.unmatched_ref_list.size()), int(sch.r2t_table.size()),
             int(sch.t2r_table.size()) - int(sch.unmatched_test_list.size()), int(sch.t2r_table.size()),
             int(sch.r2t_table.size()) - int(sch.t2r_table.size()),
             int(sch.unmatched_ref_list.size()),
             int(sch.unmatched_test_list.size()),
             int(sch.unmatched_ref_list.size()) - int(sch.unmatched_test_list.size())
            );
    ATH_MSG_INFO(message_buffer);
  }

  return StatusCode::SUCCESS;

}

StatusCode CaloGPUClusterAndCellDataMonitor::match_clusters_perfectly(sample_comparisons_holder & sch,
                                                                      const CaloRecGPU::ConstantDataHolder & /*constant_data*/,
                                                                      const CaloRecGPU::CellInfoArr & cell_info,
                                                                      const CaloRecGPU::CellStateArr & cell_state_1,
                                                                      const CaloRecGPU::CellStateArr & cell_state_2,
                                                                      const CaloRecGPU::ClusterInfoArr & cluster_info_1,
                                                                      const CaloRecGPU::ClusterInfoArr & cluster_info_2,
                                                                      const CaloRecGPU::ClusterMomentsArr & /*moments_1*/,
                                                                      const CaloRecGPU::ClusterMomentsArr & /*moments_2*/,
                                                                      const bool match_without_shared) const
{
  sch.r2t_table.clear();
  sch.r2t_table.resize(cluster_info_1.number, -1);

  sch.t2r_table.clear();
  sch.t2r_table.resize(cluster_info_2.number, -1);

  std::vector<char> match_possibilities(cluster_info_1.number * cluster_info_2.number, 1);

  for (int i = 0; i < NCaloCells; ++i)
    {
      const ClusterTag ref_tag = cell_state_1.clusterTag[i];
      const ClusterTag test_tag = cell_state_2.clusterTag[i];

      if (!cell_info.is_valid(i))
        {
          continue;
        }

      int ref_c1 = -1, ref_c2 = -1, test_c1 = -1, test_c2 = -1;

      if (ref_tag.is_part_of_cluster())
        {
          if (match_without_shared && ref_tag.is_shared_between_clusters())
            {
              continue;
            }
          ref_c1 = ref_tag.cluster_index();
          ref_c2 = ref_tag.is_shared_between_clusters() ? ref_tag.secondary_cluster_index() : -1;
        }

      if (test_tag.is_part_of_cluster())
        {
          if (match_without_shared && test_tag.is_shared_between_clusters())
            {
              continue;
            }
          test_c1 = test_tag.cluster_index();
          test_c2 = test_tag.is_shared_between_clusters() ? test_tag.secondary_cluster_index() : -1;
        }

      for (int refc = 0; refc < cluster_info_1.number; ++refc)
        {
          if (refc == ref_c1 || refc == ref_c2)
            {
              continue;
            }

          if (test_c1 >= 0)
            {
              match_possibilities[test_c1 * cluster_info_1.number + refc] = 0;
            }
          if (test_c2 >= 0)
            {
              match_possibilities[test_c2 * cluster_info_1.number + refc] = 0;
            }
        }

      for (int testc = 0; testc < cluster_info_2.number; ++testc)
        {
          if (testc == test_c1 || testc == test_c2)
            {
              continue;
            }

          if (ref_c1 >= 0)
            {
              match_possibilities[testc * cluster_info_1.number + ref_c1] = 0;
            }
          if (ref_c2 >= 0)
            {
              match_possibilities[testc * cluster_info_1.number + ref_c2] = 0;
            }
        }

    }

  for (int testc = 0; testc < cluster_info_2.number; ++testc)
    {
      for (int refc = 0; refc < cluster_info_1.number; ++refc)
        {
          if (match_possibilities[testc * cluster_info_1.number + refc] > 0)
            {
              sch.r2t_table[refc] = testc;
              sch.t2r_table[testc] = refc;
            }
        }
    }

  for (int refc = 0; refc < cluster_info_1.number; ++refc)
    {
      if (sch.r2t_table[refc] < 0)
        {
          sch.unmatched_ref_list.push_back(refc);
        }
    }

  for (int testc = 0; testc < cluster_info_2.number; ++testc)
    {
      if (sch.t2r_table[testc] < 0)
        {
          sch.unmatched_test_list.push_back(testc);
        }
    }

  {
    char message_buffer[256];
    snprintf(message_buffer, 256,
             "%2d: %5d / %5d || %5d / %5d || %3d || %5d | %5d || %5d",
             0,
             int(sch.r2t_table.size()) - int(sch.unmatched_ref_list.size()), int(sch.r2t_table.size()),
             int(sch.t2r_table.size()) - int(sch.unmatched_test_list.size()), int(sch.t2r_table.size()),
             int(sch.r2t_table.size()) - int(sch.t2r_table.size()),
             int(sch.unmatched_ref_list.size()),
             int(sch.unmatched_test_list.size()),
             int(sch.unmatched_ref_list.size()) - int(sch.unmatched_test_list.size())
            );
    ATH_MSG_INFO(message_buffer);
  }

  return StatusCode::SUCCESS;

}

namespace
{

  namespace ClusterProperties
  {
#define CALORECGPU_BASIC_CLUSTER_PROPERTY(NAME, ...)                                                                              \
  struct clusters_ ## NAME                                                                                                        \
  {                                                                                                                               \
    static std::string name()                                                                                                     \
    {                                                                                                                             \
      return # NAME;                                                                                                              \
    }                                                                                                                             \
    static double get_property([[maybe_unused]] const ConstantDataHolder & constant_data,                                         \
                               [[maybe_unused]] const CaloRecGPU::CellInfoArr & cell_info,                                        \
                               [[maybe_unused]] const CaloRecGPU::CellStateArr & cell_state,                                      \
                               [[maybe_unused]] const CaloRecGPU::ClusterInfoArr & cluster_info,                                  \
                               [[maybe_unused]] const CaloRecGPU::ClusterMomentsArr & cluster_moments,                            \
                               [[maybe_unused]] const int cluster_index                                   )                       \
    {                                                                                                                             \
      __VA_ARGS__                                                                                                                 \
    }                                                                                                                             \
  };

    CALORECGPU_BASIC_CLUSTER_PROPERTY(E, return cluster_info.clusterEnergy[cluster_index] / CLHEP::MeV;)

    CALORECGPU_BASIC_CLUSTER_PROPERTY(abs_E, return std::abs(cluster_info.clusterEnergy[cluster_index]) / CLHEP::MeV;)

    CALORECGPU_BASIC_CLUSTER_PROPERTY(Et, return cluster_info.clusterEt[cluster_index] / CLHEP::MeV;)

    CALORECGPU_BASIC_CLUSTER_PROPERTY(eta, return cluster_info.clusterEta[cluster_index];)

    CALORECGPU_BASIC_CLUSTER_PROPERTY(phi, return cluster_info.clusterPhi[cluster_index];)

    //CALORECGPU_BASIC_CLUSTER_PROPERTY(time, return cluster_moments.time[cluster_index] / CLHEP::us;)

#define CALORECGPU_CLUSTER_MOMENT(...) CALORECGPU_CLUSTER_MOMENT_INNER(__VA_ARGS__, 1, 1)
#define CALORECGPU_CLUSTER_MOMENT_INNER(NAME, PROPERTY, UNIT, ...) CALORECGPU_BASIC_CLUSTER_PROPERTY(moments_ ## NAME, return cluster_moments . PROPERTY [cluster_index] / UNIT;)

    CALORECGPU_CLUSTER_MOMENT(time, time, CLHEP::us)
    CALORECGPU_CLUSTER_MOMENT(FIRST_PHI, firstPhi)
    CALORECGPU_CLUSTER_MOMENT(FIRST_ETA, firstEta)
    CALORECGPU_CLUSTER_MOMENT(SECOND_R, secondR)
    CALORECGPU_CLUSTER_MOMENT(SECOND_LAMBDA, secondLambda)
    CALORECGPU_CLUSTER_MOMENT(DELTA_PHI, deltaPhi)
    CALORECGPU_CLUSTER_MOMENT(DELTA_THETA, deltaTheta)
    CALORECGPU_CLUSTER_MOMENT(DELTA_ALPHA, deltaAlpha)
    CALORECGPU_CLUSTER_MOMENT(CENTER_X, centerX)
    CALORECGPU_CLUSTER_MOMENT(CENTER_Y, centerY)
    CALORECGPU_CLUSTER_MOMENT(CENTER_Z, centerZ)
    CALORECGPU_CLUSTER_MOMENT(CENTER_MAG, centerMag)
    CALORECGPU_CLUSTER_MOMENT(CENTER_LAMBDA, centerLambda)
    CALORECGPU_CLUSTER_MOMENT(LATERAL, lateral)
    CALORECGPU_CLUSTER_MOMENT(LONGITUDINAL, longitudinal)
    CALORECGPU_CLUSTER_MOMENT(ENG_FRAC_EM, engFracEM)
    CALORECGPU_CLUSTER_MOMENT(ENG_FRAC_MAX, engFracMax)
    CALORECGPU_CLUSTER_MOMENT(ENG_FRAC_CORE, engFracCore)
    CALORECGPU_CLUSTER_MOMENT(FIRST_ENG_DENS, firstEngDens)
    CALORECGPU_CLUSTER_MOMENT(SECOND_ENG_DENS, secondEngDens)
    CALORECGPU_CLUSTER_MOMENT(ISOLATION, isolation)
    CALORECGPU_CLUSTER_MOMENT(ENG_BAD_CELLS, engBadCells)
    CALORECGPU_CLUSTER_MOMENT(N_BAD_CELLS, nBadCells)
    CALORECGPU_CLUSTER_MOMENT(N_BAD_CELLS_CORR, nBadCellsCorr)
    CALORECGPU_CLUSTER_MOMENT(BAD_CELLS_CORR_E, badCellsCorrE)
    CALORECGPU_CLUSTER_MOMENT(BADLARQ_FRAC, badLArQFrac)
    CALORECGPU_CLUSTER_MOMENT(ENG_POS, engPos)
    CALORECGPU_CLUSTER_MOMENT(SIGNIFICANCE, significance)
    CALORECGPU_CLUSTER_MOMENT(CELL_SIGNIFICANCE, cellSignificance)
    CALORECGPU_CLUSTER_MOMENT(CELL_SIG_SAMPLING, cellSigSampling)
    CALORECGPU_CLUSTER_MOMENT(AVG_LAR_Q, avgLArQ)
    CALORECGPU_CLUSTER_MOMENT(AVG_TILE_Q, avgTileQ)
    CALORECGPU_CLUSTER_MOMENT(ENG_BAD_HV_CELLS, engBadHVCells)
    CALORECGPU_CLUSTER_MOMENT(N_BAD_HV_CELLS, nBadHVCells)
    CALORECGPU_CLUSTER_MOMENT(PTD, PTD)
    CALORECGPU_CLUSTER_MOMENT(MASS, mass)
    CALORECGPU_CLUSTER_MOMENT(EM_PROBABILITY, EMProbability)
    CALORECGPU_CLUSTER_MOMENT(HAD_WEIGHT, hadWeight)
    CALORECGPU_CLUSTER_MOMENT(OOC_WEIGHT, OOCweight)
    CALORECGPU_CLUSTER_MOMENT(DM_WEIGHT, DMweight)
    CALORECGPU_CLUSTER_MOMENT(TILE_CONFIDENCE_LEVEL, tileConfidenceLevel)
    CALORECGPU_CLUSTER_MOMENT(SECOND_TIME, secondTime)
    CALORECGPU_CLUSTER_MOMENT(number_of_cells, numCells)
    CALORECGPU_CLUSTER_MOMENT(VERTEX_FRACTION, vertexFraction)
    CALORECGPU_CLUSTER_MOMENT(NVERTEX_FRACTION, nVertexFraction)
    CALORECGPU_CLUSTER_MOMENT(ETACALOFRAME, etaCaloFrame)
    CALORECGPU_CLUSTER_MOMENT(PHICALOFRAME, phiCaloFrame)
    CALORECGPU_CLUSTER_MOMENT(ETA1CALOFRAME, eta1CaloFrame)
    CALORECGPU_CLUSTER_MOMENT(PHI1CALOFRAME, phi1CaloFrame)
    CALORECGPU_CLUSTER_MOMENT(ETA2CALOFRAME, eta2CaloFrame)
    CALORECGPU_CLUSTER_MOMENT(PHI2CALOFRAME, phi2CaloFrame)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_TOT, engCalibTot)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_OUT_L, engCalibOutL)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_OUT_M, engCalibOutM)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_OUT_T, engCalibOutT)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_L, engCalibDeadL)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_M, engCalibDeadM)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_T, engCalibDeadT)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_EMB0, engCalibEMB0)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_EME0, engCalibEME0)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_TILEG3, engCalibTileG3)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_TOT, engCalibDeadTot)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_EMB0, engCalibDeadEMB0)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_TILE0, engCalibDeadTile0)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_TILEG3, engCalibDeadTileG3)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_EME0, engCalibDeadEME0)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_HEC0, engCalibDeadHEC0)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_FCAL, engCalibDeadFCAL)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_LEAKAGE, engCalibDeadLeakage)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_DEAD_UNCLASS, engCalibDeadUnclass)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_FRAC_EM, engCalibFracEM)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_FRAC_HAD, engCalibFracHad)
    CALORECGPU_CLUSTER_MOMENT(ENG_CALIB_FRAC_REST, engCalibFracRest)

    using BasicClusterProperties = multi_class_holder <

                                   clusters_E,
                                   clusters_abs_E,
                                   clusters_Et,
                                   clusters_eta,
                                   clusters_phi,
                                   clusters_moments_time,
                                   clusters_moments_FIRST_PHI,
                                   clusters_moments_FIRST_ETA,
                                   clusters_moments_SECOND_R,
                                   clusters_moments_SECOND_LAMBDA,
                                   clusters_moments_DELTA_PHI,
                                   clusters_moments_DELTA_THETA,
                                   clusters_moments_DELTA_ALPHA,
                                   clusters_moments_CENTER_X,
                                   clusters_moments_CENTER_Y,
                                   clusters_moments_CENTER_Z,
                                   clusters_moments_CENTER_MAG,
                                   clusters_moments_CENTER_LAMBDA,
                                   clusters_moments_LATERAL,
                                   clusters_moments_LONGITUDINAL,
                                   clusters_moments_ENG_FRAC_EM,
                                   clusters_moments_ENG_FRAC_MAX,
                                   clusters_moments_ENG_FRAC_CORE,
                                   clusters_moments_FIRST_ENG_DENS,
                                   clusters_moments_SECOND_ENG_DENS,
                                   clusters_moments_ISOLATION,
                                   clusters_moments_ENG_BAD_CELLS,
                                   clusters_moments_N_BAD_CELLS,
                                   clusters_moments_N_BAD_CELLS_CORR,
                                   clusters_moments_BAD_CELLS_CORR_E,
                                   clusters_moments_BADLARQ_FRAC,
                                   clusters_moments_ENG_POS,
                                   clusters_moments_SIGNIFICANCE,
                                   clusters_moments_CELL_SIGNIFICANCE,
                                   clusters_moments_CELL_SIG_SAMPLING,
                                   clusters_moments_AVG_LAR_Q,
                                   clusters_moments_AVG_TILE_Q,
                                   clusters_moments_ENG_BAD_HV_CELLS,
                                   clusters_moments_N_BAD_HV_CELLS,
                                   clusters_moments_PTD,
                                   clusters_moments_MASS,
                                   clusters_moments_EM_PROBABILITY,
                                   clusters_moments_HAD_WEIGHT,
                                   clusters_moments_OOC_WEIGHT,
                                   clusters_moments_DM_WEIGHT,
                                   clusters_moments_TILE_CONFIDENCE_LEVEL,
                                   clusters_moments_SECOND_TIME,
                                   clusters_moments_number_of_cells,
                                   clusters_moments_VERTEX_FRACTION,
                                   clusters_moments_NVERTEX_FRACTION,
                                   clusters_moments_ETACALOFRAME,
                                   clusters_moments_PHICALOFRAME,
                                   clusters_moments_ETA1CALOFRAME,
                                   clusters_moments_PHI1CALOFRAME,
                                   clusters_moments_ETA2CALOFRAME,
                                   clusters_moments_PHI2CALOFRAME,
                                   clusters_moments_ENG_CALIB_TOT,
                                   clusters_moments_ENG_CALIB_OUT_L,
                                   clusters_moments_ENG_CALIB_OUT_M,
                                   clusters_moments_ENG_CALIB_OUT_T,
                                   clusters_moments_ENG_CALIB_DEAD_L,
                                   clusters_moments_ENG_CALIB_DEAD_M,
                                   clusters_moments_ENG_CALIB_DEAD_T,
                                   clusters_moments_ENG_CALIB_EMB0,
                                   clusters_moments_ENG_CALIB_EME0,
                                   clusters_moments_ENG_CALIB_TILEG3,
                                   clusters_moments_ENG_CALIB_DEAD_TOT,
                                   clusters_moments_ENG_CALIB_DEAD_EMB0,
                                   clusters_moments_ENG_CALIB_DEAD_TILE0,
                                   clusters_moments_ENG_CALIB_DEAD_TILEG3,
                                   clusters_moments_ENG_CALIB_DEAD_EME0,
                                   clusters_moments_ENG_CALIB_DEAD_HEC0,
                                   clusters_moments_ENG_CALIB_DEAD_FCAL,
                                   clusters_moments_ENG_CALIB_DEAD_LEAKAGE,
                                   clusters_moments_ENG_CALIB_DEAD_UNCLASS,
                                   clusters_moments_ENG_CALIB_FRAC_EM,
                                   clusters_moments_ENG_CALIB_FRAC_HAD,
                                   clusters_moments_ENG_CALIB_FRAC_REST

                                   >;

  }

  using ClusterProperties::BasicClusterProperties;

  namespace ExtraClusterComparisons
  {
#define CALORECGPU_COMPARED_CLUSTER_PROPERTY(NAME, ...)                                                                           \
  struct clusters_ ## NAME                                                                                                        \
  {                                                                                                                               \
    static std::string name()                                                                                                     \
    {                                                                                                                             \
      return # NAME;                                                                                                              \
    }                                                                                                                             \
    static double get_property([[maybe_unused]] const ConstantDataHolder & constant_data,                                         \
                               [[maybe_unused]] const CaloRecGPU::CellInfoArr & cell_info_1,                                      \
                               [[maybe_unused]] const CaloRecGPU::CellStateArr & cell_state_1,                                    \
                               [[maybe_unused]] const CaloRecGPU::ClusterInfoArr & cluster_info_1,                                \
                               [[maybe_unused]] const CaloRecGPU::ClusterMomentsArr & cluster_moments_1,                          \
                               [[maybe_unused]] const int cluster_index_1,                                                        \
                               [[maybe_unused]] const CaloRecGPU::CellInfoArr & cell_info_2,                                      \
                               [[maybe_unused]] const CaloRecGPU::CellStateArr & cell_state_2,                                    \
                               [[maybe_unused]] const CaloRecGPU::ClusterInfoArr & cluster_info_2,                                \
                               [[maybe_unused]] const CaloRecGPU::ClusterMomentsArr & cluster_moments_2,                          \
                               [[maybe_unused]] const int cluster_index_2)                                                        \
    {                                                                                                                             \
      __VA_ARGS__                                                                                                                 \
    }                                                                                                                             \
  };

    CALORECGPU_COMPARED_CLUSTER_PROPERTY(delta_phi_in_range,
                                         return Helpers::regularize_angle( Helpers::regularize_angle(cluster_info_2.clusterPhi[cluster_index_2]) -
                                                                           Helpers::regularize_angle(cluster_info_1.clusterPhi[cluster_index_1])    );
                                        )

    CALORECGPU_COMPARED_CLUSTER_PROPERTY(delta_R,
                                         const double delta_eta = cluster_info_2.clusterEta[cluster_index_2] - cluster_info_1.clusterEta[cluster_index_1];
                                         const double delta_phi = Helpers::regularize_angle( Helpers::regularize_angle(cluster_info_2.clusterPhi[cluster_index_2]) -
                                                                                             Helpers::regularize_angle(cluster_info_1.clusterPhi[cluster_index_1])    );
                                         return std::sqrt(delta_eta * delta_eta + delta_phi * delta_phi);

                                        )


    using ComparedClusterProperties = multi_class_holder <

                                      clusters_delta_phi_in_range,
                                      clusters_delta_R

                                      >;

  }

  using ExtraClusterComparisons::ComparedClusterProperties;

  namespace CellProperties
  {
#define CALORECGPU_BASIC_CELL_PROPERTY(NAME, ...)                                                                                 \
  struct cells_ ## NAME                                                                                                           \
  {                                                                                                                               \
    static std::string name()                                                                                                     \
    {                                                                                                                             \
      return # NAME;                                                                                                              \
    }                                                                                                                             \
    static double get_property([[maybe_unused]] const ConstantDataHolder & constant_data,                                         \
                               [[maybe_unused]] const CaloRecGPU::CellInfoArr & cell_info,                                        \
                               [[maybe_unused]] const CaloRecGPU::CellStateArr & cell_state,                                      \
                               [[maybe_unused]] const CaloRecGPU::ClusterInfoArr & cluster_info,                                  \
                               [[maybe_unused]] const CaloRecGPU::ClusterMomentsArr & cluster_moments,                            \
                               [[maybe_unused]] const int cell                                          )                         \
    {                                                                                                                             \
      __VA_ARGS__                                                                                                                 \
    }                                                                                                                             \
  };

    CALORECGPU_BASIC_CELL_PROPERTY(E, return cell_info.energy[cell] / CLHEP::MeV;)
    CALORECGPU_BASIC_CELL_PROPERTY(abs_E, return std::abs(cell_info.energy[cell]) / CLHEP::MeV; )

    CALORECGPU_BASIC_CELL_PROPERTY(gain, return cell_info.gain[cell];)

    CALORECGPU_BASIC_CELL_PROPERTY(noise, return constant_data.m_cell_noise->get_noise(cell, cell_info.gain[cell]);)

    CALORECGPU_BASIC_CELL_PROPERTY(SNR, return cell_info.energy[cell] /
                                               protect_from_zero(constant_data.m_cell_noise->get_noise(cell, cell_info.gain[cell]));
                                  )

    CALORECGPU_BASIC_CELL_PROPERTY(abs_SNR,  return std::abs(cell_info.energy[cell] /
                                                             protect_from_zero(constant_data.m_cell_noise->get_noise(cell, cell_info.gain[cell])));)

    CALORECGPU_BASIC_CELL_PROPERTY(time, return cell_info.time[cell] / CLHEP::us;)

    CALORECGPU_BASIC_CELL_PROPERTY(index, return cell;)

    CALORECGPU_BASIC_CELL_PROPERTY(sampling, return constant_data.m_geometry->sampling(cell);)


    CALORECGPU_BASIC_CELL_PROPERTY(x, return constant_data.m_geometry->x[cell] / CLHEP::cm; )
    CALORECGPU_BASIC_CELL_PROPERTY(y, return constant_data.m_geometry->y[cell] / CLHEP::cm; )
    CALORECGPU_BASIC_CELL_PROPERTY(z, return constant_data.m_geometry->z[cell] / CLHEP::cm; )

    CALORECGPU_BASIC_CELL_PROPERTY(phi, return constant_data.m_geometry->phi[cell];)

    CALORECGPU_BASIC_CELL_PROPERTY(eta, return constant_data.m_geometry->eta[cell];)


    CALORECGPU_BASIC_CELL_PROPERTY(primary_cluster_index, return ClusterTag{cell_state.clusterTag[cell]}.cluster_index(); )

    CALORECGPU_BASIC_CELL_PROPERTY(secondary_cluster_index, return ClusterTag{cell_state.clusterTag[cell]}.secondary_cluster_index(); )

    CALORECGPU_BASIC_CELL_PROPERTY(primary_weight,
    {
      const ClusterTag tag = cell_state.clusterTag[cell];
      if (tag.is_part_of_cluster())
        {
          float rev_weight = float_unhack(tag.secondary_cluster_weight());
          return 1.0f - rev_weight;
        }
      else
        {
          return 0.f;
        }
    }
                                  )

    CALORECGPU_BASIC_CELL_PROPERTY(secondary_weight,
    {
      const ClusterTag tag = cell_state.clusterTag[cell];
      if (tag.is_part_of_cluster())
        {
          float rev_weight = float_unhack(tag.secondary_cluster_weight());
          return rev_weight;
        }
      else
        {
          return 0.f;
        }
    }
                                  )

    using BasicCellProperties = multi_class_holder <

                                cells_E,
                                cells_abs_E,
                                cells_gain,
                                cells_noise,
                                cells_SNR,
                                cells_abs_SNR,
                                cells_time,
                                cells_index,
                                cells_sampling,
                                cells_x,
                                cells_y,
                                cells_z,
                                cells_phi,
                                cells_eta,
                                cells_primary_cluster_index,
                                cells_secondary_cluster_index,
                                cells_primary_weight,
                                cells_secondary_weight

                                >;

  }

  using CellProperties::BasicCellProperties;

  namespace CellTypes
  {
#define CALORECGPU_BASIC_CELL_TYPE(NAME, ...)                                                                                     \
  struct cell_type_ ## NAME                                                                                                       \
  {                                                                                                                               \
    static std::string name()                                                                                                     \
    {                                                                                                                             \
      return # NAME;                                                                                                              \
    }                                                                                                                             \
    static bool is_type([[maybe_unused]] const ConstantDataHolder & constant_data,                                                \
                        [[maybe_unused]] const CaloRecGPU::CellInfoArr & cell_info,                                               \
                        [[maybe_unused]] const CaloRecGPU::CellStateArr & cell_state,                                             \
                        [[maybe_unused]] const CaloRecGPU::ClusterInfoArr & cluster_info,                                         \
                        [[maybe_unused]] const CaloRecGPU::ClusterMomentsArr & cluster_moments,                                   \
                        [[maybe_unused]] const int cell                                          )                                \
    {                                                                                                                             \
      __VA_ARGS__                                                                                                                 \
    }                                                                                                                             \
  };

    CALORECGPU_BASIC_CELL_TYPE(intracluster, return ClusterTag{cell_state.clusterTag[cell]}.is_part_of_cluster();)

    CALORECGPU_BASIC_CELL_TYPE(extracluster, return !ClusterTag{cell_state.clusterTag[cell]}.is_part_of_cluster();)

    CALORECGPU_BASIC_CELL_TYPE(shared,
                               ClusterTag tag = cell_state.clusterTag[cell];
                               return tag.is_part_of_cluster() && tag.is_shared_between_clusters(); )

    using BasicCellTypes = multi_class_holder <

                           cell_type_intracluster,
                           cell_type_extracluster,
                           cell_type_shared

                           >;

  }

  using CellTypes::BasicCellTypes;


  enum ExtraThingsToCalculate
  {
    ClusterSize = 0,
    ClusterComparedSize,
    DiffCells,
    SameECells,
    SameECellsCombined,
    SameSNRCells,
    SameSNRCellsCombined,
    ExtraThingsSize
  };
}

StatusCode CaloGPUClusterAndCellDataMonitor::initialize_plotted_variables()
{
  const std::vector<std::string> histo_strings = m_moniTool->histogramService()->getHists();
  //Small problem: other histograms with matching names.
  //Mitigated by the fact that we use cell_<property> and cluster_<property>...

  m_clusterPropertiesToDo.resize(BasicClusterProperties::size(), false);
  m_comparedClusterPropertiesToDo.resize(BasicClusterProperties::size(), false);
  m_extraComparedClusterPropertiesToDo.resize(ComparedClusterProperties::size(), false);
  m_cellPropertiesToDo.resize(BasicCellProperties::size(), false);
  m_comparedCellPropertiesToDo.resize(BasicCellProperties::size(), false);
  m_cellTypesToDo.resize(BasicCellTypes::size(), false);
  m_comparedCellTypesToDo.resize(BasicCellTypes::size(), false);
  m_extraThingsToDo.resize(ExtraThingsSize, false);

  auto string_contains = [](const std::string & container, const std::string & contained) -> bool
  {
    return container.find(contained) != std::string::npos;
  };

  auto search_lambda = [&](const auto & prop, const size_t count, bool & check,
                           const std::string & str, std::vector<bool> & to_do,
                           const std::string & prefix = "", const std::string & suffix = "")
  {
    if (string_contains(str, prefix + prop.name() + suffix))
      {
        to_do[count] = true;
        check = true;
      }
  };

  for (const auto & str : histo_strings)
    {
      bool found = false;

      apply_to_multi_class(search_lambda, BasicCellProperties{}, found, str, m_cellPropertiesToDo, "_cell_");
      apply_to_multi_class(search_lambda, BasicCellTypes{}, found, str, m_cellTypesToDo, "_", "_cells");

      if (found)
        {
          m_doCells = true;
        }

      found = false;

      apply_to_multi_class(search_lambda, BasicClusterProperties{}, found, str, m_clusterPropertiesToDo, "_cluster_");

      if (found)
        {
          m_doClusters = true;
        }

      found = false;

      apply_to_multi_class(search_lambda, BasicCellProperties{}, found, str, m_comparedCellPropertiesToDo, "_cell_delta_");
      apply_to_multi_class(search_lambda, BasicCellProperties{}, found, str, m_comparedCellPropertiesToDo, "_cell_", "_ref");
      apply_to_multi_class(search_lambda, BasicCellProperties{}, found, str, m_comparedCellPropertiesToDo, "_cell_", "_test");
      apply_to_multi_class(search_lambda, BasicCellTypes{}, found, str, m_comparedCellTypesToDo, "num_ref_", "_cells");
      apply_to_multi_class(search_lambda, BasicCellTypes{}, found, str, m_comparedCellTypesToDo, "num_test_", "_cells");
      apply_to_multi_class(search_lambda, BasicCellTypes{}, found, str, m_comparedCellTypesToDo, "delta_", "_cells");

      if (found)
        {
          m_doCombinedCells = true;
        }

      found = false;

      apply_to_multi_class(search_lambda, BasicClusterProperties{}, found, str, m_comparedClusterPropertiesToDo, "_cluster_delta_", "");
      apply_to_multi_class(search_lambda, BasicClusterProperties{}, found, str, m_comparedClusterPropertiesToDo, "_cluster_", "_ref");
      apply_to_multi_class(search_lambda, BasicClusterProperties{}, found, str, m_comparedClusterPropertiesToDo, "_cluster_", "_test");
      apply_to_multi_class(search_lambda, ComparedClusterProperties{}, found, str, m_extraComparedClusterPropertiesToDo);

      if (found)
        {
          m_doCombinedClusters = true;
        }

      if ( string_contains(str, "cluster_size_ref")            ||
           string_contains(str, "cluster_size_test")           ||
           string_contains(str, "cluster_delta_size")          ||
           string_contains(str, "cluster_weighted_size_ref")   ||
           string_contains(str, "cluster_weighted_size_test")  ||
           string_contains(str, "cluster_delta_weighted_size")    )
        {
          m_extraThingsToDo[ClusterComparedSize] = true;
          m_doCombinedCells = true;
          m_doCombinedClusters = true;
        }
      else if ( string_contains(str, "cluster_size")          ||
                string_contains(str, "cluster_weighted_size")    )
        {
          m_extraThingsToDo[ClusterSize] = true;
          m_doCells = true;
          m_doClusters = true;
        }

      if (string_contains(str, "cluster_diff_cells"))
        {
          m_extraThingsToDo[DiffCells] = true;
          m_doCombinedCells = true;
          m_doCombinedClusters = true;
        }

      if ( string_contains(str, "_num_same_E_cells_ref")      ||
           string_contains(str, "_num_same_E_cells_test")     ||
           string_contains(str, "delta_num_same_E_cells")     ||
           string_contains(str, "_num_same_abs_E_cells_ref")  ||
           string_contains(str, "_num_same_abs_E_cells_test") ||
           string_contains(str, "delta_num_same_abs_E_cells")    )
        {
          m_extraThingsToDo[SameECellsCombined] = true;
          m_doCombinedCells = true;
        }
      else if ( string_contains(str, "_num_same_E_cells")     ||
                string_contains(str, "_num_same_abs_E_cells")    )
        {
          m_extraThingsToDo[SameECells] = true;
          m_doCells = true;
        }

      if ( string_contains(str, "_num_same_SNR_cells_ref")      ||
           string_contains(str, "_num_same_SNR_cells_test")     ||
           string_contains(str, "delta_num_same_SNR_cells")     ||
           string_contains(str, "_num_same_abs_SNR_cells_ref")  ||
           string_contains(str, "_num_same_abs_SNR_cells_test") ||
           string_contains(str, "delta_num_same_abs_SNR_cells")    )
        {
          m_extraThingsToDo[SameSNRCellsCombined] = true;
          m_doCombinedCells = true;
        }
      else if ( string_contains(str, "_num_same_SNR_cells")     ||
                string_contains(str, "_num_same_abs_SNR_cells")    )
        {
          m_extraThingsToDo[SameSNRCells] = true;
          m_doCells = true;
        }
    }

  return StatusCode::SUCCESS;

}


StatusCode CaloGPUClusterAndCellDataMonitor::add_data(const EventContext & /*ctx*/,
                                                      const ConstantDataHolder & constant_data,
                                                      const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> & cell_info,
                                                      const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> & cell_state,
                                                      const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> & clusters,
                                                      const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterMomentsArr> & moments,
                                                      const std::string & tool_name) const
{
  m_numClustersPerTool[tool_name].fetch_add(clusters->number);

  const std::string prefix = m_toolToIdMap.at(tool_name);

  const int index = m_toolsToCheckFor.at(tool_name);

  if (index >= 0 && m_numToolsToKeep > 0)
    {
      std::vector<per_tool_storage> & store_vec = m_storageHolder.get_for_thread();
      store_vec[index].cell_info = *cell_info;
      store_vec[index].cell_state = *cell_state;
      store_vec[index].clusters = *clusters;
      store_vec[index].moments = *moments;
    }

  if (prefix != "")
    //Tools that are not meant to be plotted individually
    //have the empty string as a prefix.
    {
      std::unordered_map<std::string, std::vector<double>> cluster_properties, cell_properties;

      std::unordered_map<std::string, long long int> cell_counts;

      cluster_properties["size"].resize(clusters->number, 0.);
      cluster_properties["weighted_size"].resize(clusters->number, 0.);

      long long int same_energy = 0, same_abs_energy = 0, same_snr = 0, same_abs_snr = 0;

      std::set<double> energies, snrs;

      if (m_doCells)
        {

          for (int cell = 0; cell < NCaloCells; ++cell)
            {
              if (!cell_info->is_valid(cell))
                {
                  continue;
                }

              apply_to_multi_class([&](const auto & prop, const size_t i)
              {
                if (m_cellPropertiesToDo[i])
                  {
                    cell_properties[prop.name()].push_back(prop.get_property(constant_data, *cell_info, *cell_state, *clusters, *moments, cell));
                  }
              }, BasicCellProperties{});

              apply_to_multi_class([&](const auto & prop, const size_t i)
              {
                if (m_cellTypesToDo[i])
                  {
                    cell_counts[prop.name()] += prop.is_type(constant_data, *cell_info, *cell_state, *clusters, *moments, cell);
                  }
              }, BasicCellTypes{});

              const float this_energy = cell_info->energy[cell];

              if (m_extraThingsToDo[SameECells])
                {

                  if (energies.count(this_energy))
                    {
                      ++same_energy;
                      ++same_abs_energy;
                    }
                  else if (energies.count(-this_energy))
                    {
                      ++same_abs_energy;
                    }
                  energies.insert(this_energy);
                }

              if (m_extraThingsToDo[SameSNRCells])
                {

                  const float this_snr = this_energy / protect_from_zero(constant_data.m_cell_noise->get_noise(cell, cell_info->gain[cell]));

                  if (snrs.count(this_snr))
                    {
                      ++same_snr;
                      ++same_abs_snr;
                    }
                  else if (snrs.count(-this_snr))
                    {
                      ++same_abs_snr;
                    }
                  snrs.insert(this_snr);
                }

              if (m_extraThingsToDo[ClusterSize])
                {
                  const ClusterTag tag = cell_state->clusterTag[cell];
                  const float weight = float_unhack(tag.secondary_cluster_weight());
                  if (tag.is_part_of_cluster())
                    {
                      cluster_properties["size"][tag.cluster_index()] += 1;
                      cluster_properties["weighted_size"][tag.cluster_index()] += 1.0f - weight;
                      if (tag.is_shared_between_clusters())
                        {
                          cluster_properties["size"][tag.secondary_cluster_index()] += 1;
                          cluster_properties["weighted_size"][tag.secondary_cluster_index()] += weight;
                        }
                    }
                }
            }
        }

      if (m_doClusters)
        {
          for (int cluster = 0; cluster < clusters->number; ++cluster)
            {
              apply_to_multi_class([&](const auto & prop, const size_t i)
              {
                if (m_clusterPropertiesToDo[i])
                  {
                    cluster_properties[prop.name()].push_back(prop.get_property(constant_data, *cell_info, *cell_state, *clusters, *moments, cluster));
                  }
              }, BasicClusterProperties{});
            }
        }

      using coll_type = decltype(Monitored::Collection("", std::declval<std::vector<double> &>()));
      using scalar_type = decltype(Monitored::Scalar("", std::declval<long long int>()));

      std::vector<coll_type> collections;
      std::vector<scalar_type> count_scalars;
      std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> cluster_group, cell_group, counts_group;

      collections.reserve(cluster_properties.size() + cell_properties.size());
      count_scalars.reserve(cell_counts.size());
      cluster_group.reserve(cluster_properties.size());
      cell_group.reserve(cell_properties.size());
      counts_group.reserve(cell_counts.size() + 5);

      auto mon_clus_num = Monitored::Scalar(prefix + "_num_clusters", clusters->number);
      auto mon_same_energy = Monitored::Scalar(prefix + "_num_same_E_cells", same_energy);
      auto mon_same_abs_energy = Monitored::Scalar(prefix + "_num_same_abs_E_cells", same_abs_energy);
      auto mon_same_snr = Monitored::Scalar(prefix + "_num_same_SNR_cells", same_snr);
      auto mon_same_abs_snr = Monitored::Scalar(prefix + "_num_same_abs_SNR_cells", same_abs_snr);

      counts_group.push_back(std::ref(mon_clus_num));
      counts_group.push_back(std::ref(mon_same_energy));
      counts_group.push_back(std::ref(mon_same_abs_energy));
      counts_group.push_back(std::ref(mon_same_snr));
      counts_group.push_back(std::ref(mon_same_abs_snr));

      //If we're not doing these plots,
      //we're still saving,
      //which is slightly inefficient, but.. let's not complicate.

      for (const auto & k_v : cluster_properties)
        {
          collections.emplace_back(Monitored::Collection(prefix + "_cluster_" + k_v.first, k_v.second));
          cluster_group.push_back(std::ref(collections.back()));
        }

      for (const auto & k_v : cell_properties)
        {
          collections.emplace_back(Monitored::Collection(prefix + "_cell_" + k_v.first, k_v.second));
          cell_group.push_back(std::ref(collections.back()));
        }

      for (const auto & k_v : cell_counts)
        {
          count_scalars.emplace_back(Monitored::Scalar(prefix + "_num_" + k_v.first + "_cells", k_v.second));
          counts_group.push_back(std::ref(count_scalars.back()));
        }

      // Taking a std::ref of the back() iterator above is safe because the vector
      // has been reserved with the correct number of elements.
      // cppcheck-suppress invalidContainer
      auto monitor_clusters = Monitored::Group(m_moniTool, cluster_group);
      auto monitor_cells = Monitored::Group(m_moniTool, cell_group);
      auto monitor_counts = Monitored::Group(m_moniTool, counts_group);

    }
  return StatusCode::SUCCESS;
}


StatusCode CaloGPUClusterAndCellDataMonitor::add_combination(const EventContext & /*ctx*/,
                                                             const CaloRecGPU::ConstantDataHolder & constant_data,
                                                             const int index_1,
                                                             const int index_2,
                                                             const std::string & prefix,
                                                             const bool match_in_energy,
                                                             const bool match_without_shared,
                                                             const bool match_perfectly) const
{

  //Note: Part of the work here is superfluous in the case
  //      where we are monitoring the tools individually too,
  //      but in the most generic case that is not guaranteed.
  //      Partially wasted work, but it's cleaner than the alternative...

  std::vector<per_tool_storage> & store_vec = m_storageHolder.get_for_thread();

  const CaloRecGPU::CellInfoArr & cell_info_1 = store_vec[index_1].cell_info;
  const CaloRecGPU::CellStateArr & cell_state_1 = store_vec[index_1].cell_state;
  const CaloRecGPU::ClusterInfoArr & clusters_1 = store_vec[index_1].clusters;
  const CaloRecGPU::ClusterMomentsArr & moments_1 = store_vec[index_1].moments;

  const CaloRecGPU::CellInfoArr & cell_info_2 = store_vec[index_2].cell_info;
  const CaloRecGPU::CellStateArr & cell_state_2 = store_vec[index_2].cell_state;
  const CaloRecGPU::ClusterInfoArr & clusters_2 = store_vec[index_2].clusters;
  const CaloRecGPU::ClusterMomentsArr & moments_2 = store_vec[index_2].moments;

  sample_comparisons_holder sch;

  if (match_perfectly)
    {
      ATH_CHECK( match_clusters_perfectly(sch, constant_data, cell_info_1, cell_state_1, cell_state_2,
                                          clusters_1, clusters_2, moments_1, moments_2, match_without_shared) );
    }
  else
    {
      ATH_CHECK( match_clusters(sch, constant_data, cell_info_1, cell_state_1, cell_state_2,
                                clusters_1, clusters_2, moments_1, moments_2, match_in_energy, match_without_shared) );
    }

  std::unordered_map<std::string, std::vector<double>> cluster_properties, cell_properties;

  std::unordered_map<std::string, long long int> cell_counts;

  std::vector<double> ref_size_vec(clusters_1.number, 0.), test_size_vec(clusters_2.number, 0.),
      ref_weighted_size_vec(clusters_1.number, 0.), test_weighted_size_vec(clusters_2.number, 0.),
      ref_diff_cells(clusters_1.number, 0.), test_diff_cells(clusters_2.number, 0.),
      ref_diff_cells_weight(clusters_1.number, 0.), test_diff_cells_weight(clusters_2.number, 0.);
  //We can store integers up to 2^53 on a double...

  long long int same_energy_1 = 0, same_energy_2 = 0,
                same_abs_energy_1 = 0, same_abs_energy_2 = 0,
                same_snr_1 = 0, same_snr_2 = 0,
                same_abs_snr_1 = 0, same_abs_snr_2 = 0,
                same_cluster_cells_count = 0, diff_cluster_cells_count = 0;

  std::set<double> energies_1, energies_2, snrs_1, snrs_2;

  if (m_doCombinedCells)
    {
      for (int cell = 0; cell < NCaloCells; ++cell)
        {
          if (!cell_info_1.is_valid(cell) || !cell_info_2.is_valid(cell))
            {
              continue;
            }

          apply_to_multi_class([&](const auto & prop, const size_t i)
          {
            if (m_comparedCellPropertiesToDo[i])
              {
                const auto prop_1 = prop.get_property(constant_data, cell_info_1, cell_state_1, clusters_1, moments_1, cell);
                const auto prop_2 = prop.get_property(constant_data, cell_info_2, cell_state_2, clusters_2, moments_2, cell);

                cell_properties[prop.name() + "_ref"].push_back(prop_1);
                cell_properties[prop.name() + "_test"].push_back(prop_2);

                cell_properties["delta_" + prop.name()].push_back(prop_2 - prop_1);
                cell_properties["delta_" + prop.name() + "_rel_ref"].push_back((prop_2 - prop_1) / protect_from_zero(std::abs(prop_1)));
                cell_properties["delta_" + prop.name() + "_rel_test"].push_back((prop_2 - prop_1) / protect_from_zero(std::abs(prop_2)));
              }
          }, BasicCellProperties{});

          apply_to_multi_class([&](const auto & prop, const size_t i)
          {
            if (m_comparedCellTypesToDo[i])
              {
                const auto is_1 = prop.is_type(constant_data, cell_info_1, cell_state_1, clusters_1, moments_1, cell);
                const auto is_2 = prop.is_type(constant_data, cell_info_2, cell_state_2, clusters_2, moments_2, cell);

                cell_counts["num_" + prop.name() + "_cells_ref"] += is_1;
                cell_counts["num_" + prop.name() + "_cells_test"] += is_2;
                cell_counts["delta_num_" + prop.name() + "_cells"] += is_2 - is_1;
              }
          }, BasicCellTypes{});

          const float this_energy_1 = cell_info_1.energy[cell];
          const float this_energy_2 = cell_info_2.energy[cell];

          if (m_extraThingsToDo[SameECellsCombined])
            {

              if (energies_1.count(this_energy_1))
                {
                  ++same_energy_1;
                  ++same_abs_energy_1;
                }
              else if (energies_1.count(-this_energy_1))
                {
                  ++same_abs_energy_1;
                }
              energies_1.insert(this_energy_1);

              if (energies_2.count(this_energy_2))
                {
                  ++same_energy_2;
                  ++same_abs_energy_2;
                }
              else if (energies_2.count(-this_energy_2))
                {
                  ++same_abs_energy_2;
                }
              energies_2.insert(this_energy_2);
            }

          if (m_extraThingsToDo[SameSNRCellsCombined])
            {
              const float this_snr_1 = this_energy_1 / protect_from_zero(constant_data.m_cell_noise->get_noise(cell, cell_info_1.gain[cell]));

              if (snrs_1.count(this_snr_1))
                {
                  ++same_snr_1;
                  ++same_abs_snr_1;
                }
              else if (snrs_1.count(-this_snr_1))
                {
                  ++same_abs_snr_1;
                }
              snrs_1.insert(this_snr_1);


              const float this_snr_2 = this_energy_2 / protect_from_zero(constant_data.m_cell_noise->get_noise(cell, cell_info_2.gain[cell]));

              if (snrs_2.count(this_snr_2))
                {
                  ++same_snr_2;
                  ++same_abs_snr_2;
                }
              else if (snrs_2.count(-this_snr_2))
                {
                  ++same_abs_snr_2;
                }
              snrs_2.insert(this_snr_2);
            }

          if (m_extraThingsToDo[DiffCells] || m_extraThingsToDo[ClusterComparedSize])
            {

              const ClusterTag ref_tag = cell_state_1.clusterTag[cell];
              const ClusterTag test_tag = cell_state_2.clusterTag[cell];
              int ref_c1 = -3, ref_c2 = -3, test_c1 = -3, test_c2 = -3;
              if (ref_tag.is_part_of_cluster())
                {
                  ref_c1 = ref_tag.cluster_index();
                  ref_c2 = ref_tag.is_shared_between_clusters() ? ref_tag.secondary_cluster_index() : -2;
                }

              if (test_tag.is_part_of_cluster())
                {
                  test_c1 = test_tag.cluster_index();
                  test_c2 = test_tag.is_shared_between_clusters() ? test_tag.secondary_cluster_index() : -2;
                }

              const int match_1 = test_c1 < 0 ? -1 : sch.t2r(test_c1);
              const int match_2 = test_c2 < 0 ? -1 : sch.t2r(test_c2);

              const float ref_rev_weight = float_unhack(ref_tag.secondary_cluster_weight());
              const float test_rev_weight = float_unhack(test_tag.secondary_cluster_weight());

              const float ref_weight = 1.0f - ref_rev_weight;
              const float test_weight = 1.0f - test_rev_weight;

              bool cell_is_diff = false;

              if (ref_c1 >= 0)
                {
                  ref_size_vec[ref_c1] += 1;
                  ref_weighted_size_vec[ref_c1] += ref_weight;
#if CALORECGPU_DISABLE_STRICT_MATCHING
                  if (!(ref_c1 == match_1 || ref_c1 == match_2 || (match_1 < 0 || match_2 < 0)))
#else
                  if (!(ref_c1 == match_1 || ref_c1 == match_2))
#endif
                    {
                      cell_is_diff = true;
                      ref_diff_cells[ref_c1] += 1;
                      ref_diff_cells_weight[ref_c1] += ref_weight;
                    }
                }

              if (ref_c2 >= 0)
                {
                  ref_size_vec[ref_c2] += 1;
                  ref_weighted_size_vec[ref_c2] += ref_rev_weight;
#if CALORECGPU_DISABLE_STRICT_MATCHING
                  if (!(ref_c2 == match_1 || ref_c2 == match_2 || (match_1 < 0 || match_2 < 0)))
#else
                  if (!(ref_c2 == match_1 || ref_c2 == match_2))
#endif
                    {
                      cell_is_diff = true;
                      ref_diff_cells[ref_c2] += 1;
                      ref_diff_cells_weight[ref_c2] += ref_rev_weight;
                    }
                }

              if (test_c1 >= 0)
                {
                  test_size_vec[test_c1] += 1;
                  test_weighted_size_vec[test_c1] += test_weight;
#if CALORECGPU_DISABLE_STRICT_MATCHING
                  if (match_1 >= 0 && !(match_1 == ref_c1 || match_1 == ref_c2 || (ref_c1 < 0 || ref_c2 < 0)))
#else
                  if (match_1 < 0 || !(match_1 == ref_c1 || match_1 == ref_c2))
#endif
                    {
                      cell_is_diff = true;
                      test_diff_cells[test_c1] += 1;
                      test_diff_cells_weight[test_c1] += test_weight;
                    }
                }

              if (test_c2 >= 0)
                {
                  test_size_vec[test_c2] += 1;
                  test_weighted_size_vec[test_c2] += test_rev_weight;
#if CALORECGPU_DISABLE_STRICT_MATCHING
                  if (match_2 >= 0 && !(match_2 == ref_c1 || match_2 == ref_c2 || (ref_c1 < 0 || ref_c2 < 0)))
#else
                  if (match_2 < 0 || !(match_2 == ref_c1 || match_2 == ref_c2))
#endif
                    {
                      cell_is_diff = true;
                      test_diff_cells[test_c2] += 1;
                      test_diff_cells_weight[test_c2] += test_rev_weight;
                    }
                }

              if (!cell_is_diff && (ref_c1 >= 0 || ref_c2 >= 0 || test_c1 >= 0 || test_c2 >= 0))
                {
                  ++same_cluster_cells_count;
                }
              else if (cell_is_diff)
                {
                  char message_buffer[256];
                  snprintf(message_buffer, 256,
                           "%7d | %18f || %6d | %6d | %6d | %6d || %6d | %6d || %016llX | %016llX",
                           cell, this_energy_1 / protect_from_zero(constant_data.m_cell_noise->get_noise(cell, cell_info_1.gain[cell])),
                           ref_c1, ref_c2, test_c1, test_c2,
                           match_1, match_2,
                           static_cast<tag_type>(ref_tag), static_cast<tag_type>(test_tag));
                  ATH_MSG_DEBUG(message_buffer);
                  ++diff_cluster_cells_count;
                }

            }
        }
    }

  if (m_doCombinedClusters)
    {

      for (int cluster = 0; cluster < clusters_1.number; ++cluster)
        {
          const int match = sch.r2t(cluster);
          if (match < 0)
            //The cluster isn't matched.
            {
              continue;
            }

          apply_to_multi_class([&](const auto & prop, const size_t i)
          {
            if (m_comparedClusterPropertiesToDo[i])
              {
                const auto prop_1 = prop.get_property(constant_data, cell_info_1, cell_state_1, clusters_1, moments_1, cluster);
                const auto prop_2 = prop.get_property(constant_data, cell_info_2, cell_state_2, clusters_2, moments_2, match);

                cluster_properties[prop.name() + "_ref"].push_back(prop_1);
                cluster_properties[prop.name() + "_test"].push_back(prop_2);

                cluster_properties["delta_" + prop.name()].push_back(prop_2 - prop_1);
                cluster_properties["delta_" + prop.name() + "_rel_ref"].push_back((prop_2 - prop_1) / protect_from_zero(std::abs(prop_1)));
                cluster_properties["delta_" + prop.name() + "_rel_test"].push_back((prop_2 - prop_1) / protect_from_zero(std::abs(prop_2)));
              }
          }, BasicClusterProperties{});

          apply_to_multi_class([&](const auto & prop, const size_t i)
          {
            if (m_extraComparedClusterPropertiesToDo[i])
              {
                cluster_properties[prop.name()].push_back(prop.get_property(constant_data, cell_info_1, cell_state_1, clusters_1, moments_1, cluster,
                                                                            cell_info_2, cell_state_2, clusters_2, moments_2, match));
              }
          }, ComparedClusterProperties{});

          if (m_extraThingsToDo[ClusterComparedSize])
            {
              cluster_properties["size_ref"].push_back(ref_size_vec[cluster]);
              cluster_properties["size_test"].push_back(test_size_vec[match]);
              cluster_properties["delta_size"].push_back(ref_size_vec[cluster] - test_size_vec[match]);
              cluster_properties["delta_size_rel_ref"].push_back((ref_size_vec[cluster] - test_size_vec[match]) / protect_from_zero(ref_size_vec[cluster]));
              cluster_properties["delta_size_rel_test"].push_back((ref_size_vec[cluster] - test_size_vec[match]) / protect_from_zero(test_size_vec[match]));

              cluster_properties["weighted_size_ref"].push_back(ref_weighted_size_vec[cluster]);
              cluster_properties["weighted_size_test"].push_back(test_weighted_size_vec[match]);
              cluster_properties["delta_weighted_size"].push_back(ref_weighted_size_vec[cluster] - test_weighted_size_vec[match]);
              cluster_properties["delta_weighted_size_rel_ref"].push_back((ref_weighted_size_vec[cluster] - test_weighted_size_vec[match]) / protect_from_zero(ref_weighted_size_vec[cluster]));
              cluster_properties["delta_weighted_size_rel_test"].push_back((ref_weighted_size_vec[cluster] - test_weighted_size_vec[match]) / protect_from_zero(test_weighted_size_vec[match]));
            }

          if (m_extraThingsToDo[DiffCells])
            {
              cluster_properties["diff_cells_ref"].push_back(ref_diff_cells[cluster]);
              cluster_properties["diff_cells_ref_rel_size"].push_back(ref_diff_cells[cluster] / protect_from_zero(ref_size_vec[cluster]));
              cluster_properties["diff_cells_test"].push_back(test_diff_cells[match]);
              cluster_properties["diff_cells_test_rel_size"].push_back(test_diff_cells[match] / protect_from_zero(test_size_vec[match]));
              cluster_properties["diff_cells"].push_back(ref_diff_cells[cluster] + test_diff_cells[match]);

              cluster_properties["weighted_diff_cells_ref"].push_back(ref_diff_cells_weight[cluster]);
              cluster_properties["weighted_diff_cells_ref_rel_size"].push_back(ref_diff_cells_weight[cluster] / protect_from_zero(ref_weighted_size_vec[cluster]));
              cluster_properties["weighted_diff_cells_test"].push_back(test_diff_cells_weight[match]);
              cluster_properties["weighted_diff_cells_test_rel_size"].push_back(test_diff_cells_weight[match] / protect_from_zero(test_weighted_size_vec[match]));
              cluster_properties["weighted_diff_cells"].push_back(ref_diff_cells_weight[cluster] + test_diff_cells_weight[match]);
            }
        }
    }

  using coll_type = decltype(Monitored::Collection("", std::declval<std::vector<double> &>()));
  using scalar_type = decltype(Monitored::Scalar("", std::declval<long long int>()));

  std::vector<coll_type> collections;
  std::vector<scalar_type> count_scalars;
  std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> cluster_group, cell_group, counts_group;

  collections.reserve(cluster_properties.size() + cell_properties.size());
  count_scalars.reserve(cell_counts.size() + 6 * 3);
  cluster_group.reserve(cluster_properties.size());
  cell_group.reserve(cell_properties.size());
  counts_group.reserve(cell_counts.size() + 3 + 6 * 3);

  auto add_count_vars = [&](const std::string & name, const long long int ref_num, const long long int test_num)
  {
    count_scalars.emplace_back(Monitored::Scalar(prefix + "_" + name + "_ref", ref_num));
    counts_group.push_back(std::ref(count_scalars.back()));

    count_scalars.emplace_back(Monitored::Scalar(prefix + "_" + name + "_test", test_num));
    counts_group.push_back(std::ref(count_scalars.back()));  // cppcheck-suppress invalidContainer; reserve used

    count_scalars.emplace_back(Monitored::Scalar(prefix + "_delta_" + name, test_num - ref_num));
    counts_group.push_back(std::ref(count_scalars.back()));  // cppcheck-suppress invalidContainer; reserve used
  };

  add_count_vars("num_clusters", clusters_1.number, clusters_2.number);
  add_count_vars("num_unmatched_clusters", sch.ref_unmatched(), sch.test_unmatched());

  add_count_vars("num_same_E_cells", same_energy_1, same_energy_2);
  add_count_vars("num_same_abs_E_cells", same_abs_energy_1, same_abs_energy_2);
  add_count_vars("num_same_SNR_cells", same_snr_1, same_snr_2);
  add_count_vars("num_same_abs_SNR_cells", same_abs_snr_1, same_abs_snr_2);

  auto mon_total_unmatched = Monitored::Scalar(prefix + "_num_unmatched_clusters", sch.ref_unmatched() + sch.test_unmatched());
  auto mon_same_cluster_cell = Monitored::Scalar(prefix + "_same_cluster_cells", same_cluster_cells_count);
  auto mon_diff_cluster_cell = Monitored::Scalar(prefix + "_diff_cluster_cells", diff_cluster_cells_count);

  counts_group.push_back(std::ref(mon_total_unmatched));
  counts_group.push_back(std::ref(mon_same_cluster_cell));
  counts_group.push_back(std::ref(mon_diff_cluster_cell));

  for (const auto & k_v : cluster_properties)
    {
      collections.emplace_back(Monitored::Collection(prefix + "_cluster_" + k_v.first, k_v.second));
      cluster_group.push_back(std::ref(collections.back()));
    }

  for (const auto & k_v : cell_properties)
    {
      collections.emplace_back(Monitored::Collection(prefix + "_cell_" + k_v.first, k_v.second));
      cell_group.push_back(std::ref(collections.back()));
    }

  for (const auto & k_v : cell_counts)
    {
      count_scalars.emplace_back(Monitored::Scalar(prefix + "_" + k_v.first, k_v.second));
      counts_group.push_back(std::ref(count_scalars.back()));
    }

  // Taking a std::ref of the back() iterator above is safe because the vector
  // has been reserved with the correct number of elements.
  // cppcheck-suppress invalidContainer
  auto monitor_clusters = Monitored::Group(m_moniTool, cluster_group);
  auto monitor_cells = Monitored::Group(m_moniTool, cell_group);
  auto monitor_counts = Monitored::Group(m_moniTool, counts_group);

  return StatusCode::SUCCESS;
}

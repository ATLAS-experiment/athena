//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloCPUOutput.h"
#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/StandaloneDataIO.h"
#include "StoreGate/DataHandle.h"

using namespace CaloRecGPU;

CaloCPUOutput::CaloCPUOutput(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent)
{
  declareInterface<CaloClusterCollectionProcessor> (this);
}


StatusCode CaloCPUOutput::initialize()
{
  ATH_CHECK( m_cellsKey.initialize() );

  ATH_CHECK( detStore()->retrieve(m_calo_id, "CaloCell_ID") );

  return StatusCode::SUCCESS;
}

StatusCode CaloCPUOutput::execute (const EventContext & ctx, xAOD::CaloClusterContainer * cluster_collection) const
{

  Helpers::CPU_object<CellInfoArr> ret_info;

  Helpers::CPU_object<CellStateArr> ret_state;
  Helpers::CPU_object<ClusterInfoArr> ret_clusts;

  SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);
  if ( !cell_collection.isValid() )
    {
      ATH_MSG_ERROR( " Cannot retrieve CaloCellContainer: " << cell_collection.name()  );
      return StatusCode::FAILURE;
    }


  ret_info.allocate();
  ret_state.allocate();
  ret_clusts.allocate();

  for (int i = 0; i < NCaloCells; ++i)
    {
      ret_info->energy[i] = 0;
      ret_info->gain[i] = GainConversion::invalid_gain();
      ret_info->time[i] = 0;
      ret_info->qualityProvenance[i] = 0;

      ret_state->clusterTag[i] = ClusterTag::make_invalid_tag();

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

  size_t shared_count = 0;

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

      //--- fill data ---
      ret_clusts->clusterEnergy[cluster_number] = cluster->e();
      ret_clusts->clusterEt[cluster_number] = cluster->et();
      ret_clusts->clusterEta[cluster_number] = cluster->eta();
      ret_clusts->clusterPhi[cluster_number] = cluster->phi();

      const int seed_cell_index = m_calo_id->calo_cell_hash(cluster->cell_begin()->ID());

      ret_clusts->seedCellID[cluster_number] = seed_cell_index;

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
              ++shared_count;
            }
          else if (weight == 0.5f)
            //Unlikely, but...
            {
              const int max_cluster = cluster_number > other_index ? cluster_number : other_index;
              const int min_cluster = cluster_number > other_index ? other_index : cluster_number;
              ret_state->clusterTag[cell_ID] = ClusterTag::make_tag(max_cluster, weight_as_int, min_cluster);
              ++shared_count;
            }
          else /*if (weight < 0.5f)*/
            {
              ret_state->clusterTag[cell_ID] = ClusterTag::make_tag(other_index, weight_as_int, cluster_number);
              ++shared_count;
            }
        }
    }

  ret_clusts->number = cluster_collection->size();

  ATH_MSG_INFO("Clusters:     " << ret_clusts->number);
  ATH_MSG_INFO("Shared Cells: " << shared_count);

  if (m_saveCellInfo)
    {
      const auto err =  StandaloneDataIO::save_event_to_folder(ctx.evt(), std::string(m_savePath),
                                                               ret_info, ret_state, ret_clusts,
                                                               m_filePrefix, m_fileSuffix, m_numWidth);

      if (err != StandaloneDataIO::ErrorState::OK)
        {
          return StatusCode::FAILURE;
        }
    }
  else
    {
      const auto err1 = StandaloneDataIO::save_cell_state_to_folder(ctx.evt(), std::string(m_savePath),
                                                                    ret_state, m_filePrefix, m_fileSuffix, m_numWidth);

      const auto err2 = StandaloneDataIO::save_clusters_to_folder(ctx.evt(), std::string(m_savePath),
                                                                  ret_clusts, m_filePrefix, m_fileSuffix, m_numWidth);

      if (err1 != StandaloneDataIO::ErrorState::OK || err2 != StandaloneDataIO::ErrorState::OK)
        {
          return StatusCode::FAILURE;
        }
    }

  return StatusCode::SUCCESS;

}

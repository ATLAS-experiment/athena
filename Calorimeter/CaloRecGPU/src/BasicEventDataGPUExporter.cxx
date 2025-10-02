//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "BasicEventDataGPUExporter.h"

#include "CaloRecGPU/CUDAFriendlyClasses.h"

#include "AthenaKernel/errorcheck.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "StoreGate/DataHandle.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "TileEvent/TileCell.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

using namespace CaloRecGPU;

BasicEventDataGPUExporter::BasicEventDataGPUExporter(const std::string & type, const std::string & name, const IInterface * parent):
  base_class(type, name, parent),
  CaloGPUTimed(this)
{
}

StatusCode BasicEventDataGPUExporter::initialize()
{

  //ATH_CHECK(m_noiseCDOKey.initialize());
  //For Two Gaussian Noise comparisons.
  
  ATH_CHECK( m_cellsKey.initialize() );

  ATH_CHECK( detStore()->retrieve(m_calo_id, "CaloCell_ID") );

  return StatusCode::SUCCESS;
}

StatusCode BasicEventDataGPUExporter::convert(const EventContext & ctx,
                                              const ConstantDataHolder & /*cd*/,
                                              const xAOD::CaloClusterContainer * cluster_collection,
                                              EventDataHolder & ed) const
{
  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  const auto start = clock_type::now();

  ed.m_cell_info.allocate();

  SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);
  if ( !cell_collection.isValid() )
    {
      ATH_MSG_ERROR( " Cannot retrieve CaloCellContainer: " << cell_collection.name()  );
      return StatusCode::FAILURE;
    }
  
  /*
  SG::ReadCondHandle<CaloNoise> noise_handle(m_noiseCDOKey, ctx);
  const CaloNoise * noise_tool = *noise_handle;
  //For Two Gaussian Noise comparisons.
  */
  
  auto export_cell = [&](const CaloCell * cell, const int cell_index)
  {
    const float energy = cell->energy();
    const unsigned int gain = GainConversion::from_standard_gain(cell->gain());
    ed.m_cell_info->energy[cell_index] = energy;
    ed.m_cell_info->gain[cell_index] = gain;
    ed.m_cell_info->time[cell_index] = cell->time();
    
    
    if (CaloRecGPU::GeometryArr::is_tile(cell_index))
      {
        const TileCell * tile_cell = static_cast<const TileCell *> (cell);

        ed.m_cell_info->qualityProvenance[cell_index] = QualityProvenance{tile_cell->qual1(),
                                                                          tile_cell->qual2(),
                                                                          tile_cell->qbit1(),
                                                                          tile_cell->qbit2()};
        
        /*
        //For Two Gaussian Noise comparisons.
        
        const float original_snr = noise_tool->getEffectiveSigma(cell->ID(),cell->gain(),cell->energy());
        const float our_snr = cd.m_cell_noise->get_double_gaussian_noise(cell_index, gain, energy);
        if (our_snr != original_snr)
        {
          std::cout << "---------------------------------------- ERR: " << cell_index << " " << our_snr << " " << original_snr << " " << our_snr - original_snr << std::endl;
        }
        */
      }
    else
      {
        ed.m_cell_info->qualityProvenance[cell_index] = QualityProvenance{cell->quality(), cell->provenance()};
      }
    
  };
  
  //For Two Gaussian Noise comparisons.
  //std::cout << "-------------------------------------------------------------------------- START" << std::endl;

  if (cell_collection->isOrderedAndComplete())
    //Fast path: cell indices within the collection and identifierHashes match!
    {
      ATH_MSG_DEBUG("Taking quick path on event " << ctx.evt());
      int cell_index = 0;
      for (CaloCellContainer::const_iterator iCells = cell_collection->begin(); iCells != cell_collection->end(); ++iCells, ++cell_index)
        {
          const CaloCell * cell = (*iCells);
          export_cell(cell, cell_index);
        }
    }
  else if (cell_collection->isOrdered() && m_missingCellsToFill.size() > 0)
    //Remediated: we know the missing cells, force them to be invalid.
    //(Tests so far, on samples both oldish and newish, had 186986 and 187352 missing...)
    {
      ATH_MSG_DEBUG("Taking remediated fast path on event " << ctx.evt());
      int cell_index = 0;
      size_t missing_cell_count = 0;
      for (CaloCellContainer::const_iterator iCells = cell_collection->begin(); iCells != cell_collection->end(); ++iCells, ++cell_index)
        {
          const CaloCell * cell = (*iCells);

          if (missing_cell_count < m_missingCellsToFill.size() && cell_index == m_missingCellsToFill[missing_cell_count])
            {
              --iCells;
              ed.m_cell_info->gain[cell_index] = GainConversion::invalid_gain();
              ++missing_cell_count;
              continue;
            }
          else
            {
              export_cell(cell, cell_index);
            }
        }
    }
  else
    //Slow path: be careful.
    {
      /*
      std::vector<bool> has_cell(NCaloCells, false);
      // */
      ATH_MSG_DEBUG("Taking slow path on event " << ctx.evt());
      for (int cell_index = 0; cell_index < NCaloCells; ++cell_index)
        {
          ed.m_cell_info->gain[cell_index] = GainConversion::invalid_gain();
        }

      for (CaloCellContainer::const_iterator iCells = cell_collection->begin(); iCells != cell_collection->end(); ++iCells)
        {
          const CaloCell * cell = (*iCells);

          //const int cell_index = m_calo_id->calo_cell_hash(cell->ID());
          const int cell_index = cell->caloDDE()->calo_hash();
          //See calodde
          
          export_cell(cell, cell_index);

          /*
          has_cell[cell_index] = true;
          // */

        }

      /*
      //Useful output for detecting missing cells in the calorimeter collection...
      
      for (size_t i = 0; i < has_cell.size(); ++i)
        {
          if (!has_cell[i])
            {
              const auto identifier = m_calo_id->cell_id(i);
              const auto sampling = m_calo_id->calo_sample(m_calo_id->cell_id((IdentifierHash) i));
              std::cout << i << " " << sampling << std::endl;
            }
        }
      // */
    }

  //For Two Gaussian Noise comparisons.
  //std::cout << "-------------------------------------------------------------------------- END" << std::endl;

  const auto post_cells = clock_type::now();
  
  ed.m_clusters.allocate();
  ed.m_cell_state.allocate();

  if (cluster_collection->size() > 0)
    {

      for (int i = 0; i < NCaloCells; ++i)
        {
          ed.m_cell_state->clusterTag[i] = ClusterTag::make_invalid_tag();
        }

      const auto cluster_end = cluster_collection->end();
      auto cluster_iter = cluster_collection->begin();

      for (int cluster_number = 0; cluster_iter != cluster_end; ++cluster_iter, ++cluster_number )
        {
          const xAOD::CaloCluster * cluster = (*cluster_iter);
          const CaloClusterCellLink * cell_links = cluster->getCellLinks();

          ed.m_clusters->clusterEnergy[cluster_number] = cluster->e();
          ed.m_clusters->clusterEt[cluster_number] = cluster->et();
          ed.m_clusters->clusterEta[cluster_number] = cluster->eta();
          ed.m_clusters->clusterPhi[cluster_number] = cluster->phi();

          const int seed_cell_index = m_calo_id->calo_cell_hash(cluster->cell_begin()->ID());

          ed.m_clusters->seedCellID[cluster_number] = seed_cell_index;

          for (auto it = cell_links->begin(); it != cell_links->end(); ++it)
            {
              if (m_considerSharedCells)
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

                  const ClusterTag other_tag = ed.m_cell_state->clusterTag[cell_ID];

                  const int other_index = other_tag.is_part_of_cluster() ? other_tag.cluster_index() : -1;

                  if (other_index < 0)
                    {
                      if (weight < 0.5f)
                        {
                          ed.m_cell_state->clusterTag[cell_ID] = ClusterTag::make_tag(cluster_number, weight_as_int, 0);
                        }
                      else
                        {
                          ed.m_cell_state->clusterTag[cell_ID] = ClusterTag::make_tag(cluster_number);
                        }
                    }
                  else if (weight > 0.5f)
                    {
                      ed.m_cell_state->clusterTag[cell_ID] = ClusterTag::make_tag(cluster_number, other_tag.secondary_cluster_weight(), other_index);
                    }
                  else if (weight == 0.5f)
                    //Unlikely, but...
                    {
                      const int max_cluster = cluster_number > other_index ? cluster_number : other_index;
                      const int min_cluster = cluster_number > other_index ? other_index : cluster_number;
                      ed.m_cell_state->clusterTag[cell_ID] = ClusterTag::make_tag(max_cluster, weight_as_int, min_cluster);
                    }
                  else /*if (weight < 0.5f)*/
                    {
                      ed.m_cell_state->clusterTag[cell_ID] = ClusterTag::make_tag(other_index, weight_as_int, cluster_number);
                    }
                }
              else
                {
                  ed.m_cell_state->clusterTag[m_calo_id->calo_cell_hash(it->ID())] = ClusterTag::make_tag(cluster_number);
                }
            }
        }

      ed.m_clusters->number = cluster_collection->size();
    }

  const auto post_clusters = clock_type::now();

  const bool has_cluster_info = cluster_collection->size() > 0;

  ed.sendToGPU(!m_keepCPUData, has_cluster_info, has_cluster_info, false);

  const auto post_send = clock_type::now();

  if (m_measureTimes)
    {
      record_times(ctx.evt(),
                   time_cast(start, post_cells),
                   time_cast(post_cells, post_clusters),
                   time_cast(post_clusters, post_send)
                  );
    }

  return StatusCode::SUCCESS;

}


StatusCode BasicEventDataGPUExporter::finalize()
{

  if (m_measureTimes)
    {
      print_times("Cells Clusters Transfer_to_GPU", 3);
    }
  return StatusCode::SUCCESS;
}

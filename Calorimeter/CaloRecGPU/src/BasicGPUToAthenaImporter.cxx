//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "BasicGPUToAthenaImporter.h"

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

BasicGPUToAthenaImporter::BasicGPUToAthenaImporter(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent),
  CaloGPUTimed(this)
{
  declareInterface<ICaloClusterGPUOutputTransformer> (this);

}

#include "MacroHelpers.h"

StatusCode BasicGPUToAthenaImporter::initialize()
{
  ATH_CHECK( m_cellsKey.initialize() );

  ATH_CHECK( detStore()->retrieve(m_calo_id, "CaloCell_ID") );

  auto get_option_from_string = [](const std::string & str, bool & failed)
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
  m_clusterSize = get_option_from_string(m_clusterSizeString, size_failed);

  if (m_clusterSize == xAOD::CaloCluster::CSize_Unknown)
    {
      ATH_MSG_ERROR("Invalid Cluster Size: " << m_clusterSizeString);
    }

  if (size_failed)
    {
      return StatusCode::FAILURE;
    }

  return StatusCode::SUCCESS;
}



StatusCode BasicGPUToAthenaImporter::convert (const EventContext & ctx,
                                              const ConstantDataHolder &,
                                              EventDataHolder & ed,
                                              xAOD::CaloClusterContainer * cluster_container) const
{


  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  const auto start = clock_type::now();

  SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);
  if ( !cell_collection.isValid() )
    {
      ATH_MSG_ERROR( " Cannot retrieve CaloCellContainer: " << cell_collection.name()  );
      return StatusCode::RECOVERABLE;
    }
  const DataLink<CaloCellContainer> cell_collection_link (cell_collection.name(), ctx);

  ed.returnToCPU(!m_keepGPUData, true, true, false);

  const auto after_send = clock_type::now();

  std::vector<std::unique_ptr<CaloClusterCellLink>> cell_links;

  cell_links.reserve(ed.m_clusters->number);

  size_t valid_clusters = 0;

  for (int i = 0; i < ed.m_clusters->number; ++i)
    {
      if (ed.m_clusters->seedCellID[i] >= 0)
        {
          cell_links.emplace_back(std::make_unique<CaloClusterCellLink>(cell_collection_link));
          cell_links.back()->reserve(256);
          //To be adjusted.
          ++valid_clusters;
        }
      else
        {
          cell_links.emplace_back(nullptr);
          //The excluded clusters don't have any cells.
        }
    }

  const auto after_creation = clock_type::now();
  
  
  //cell_index is the actual cell index in the full set of cells (identifier hash)
  //cell_count is the cell position in the cell collection (what we want for the weight)
  const auto process_cell = [&](const int cell_index, const int cell_count)
  {
    const ClusterTag this_tag = ed.m_cell_state->clusterTag[cell_index];
    if (this_tag.is_part_of_cluster())
      {
        const int this_index = this_tag.cluster_index();
        const int32_t weight_pattern = this_tag.secondary_cluster_weight();

        float tempf = 1.0f;

        std::memcpy(&tempf, &weight_pattern, sizeof(float));
        //C++20 would give us bit cast to do this more properly.
        //Still, given how the bit pattern is created,
        //it should be safe.

        const float reverse_weight = tempf;

        const float this_weight = 1.0f - reverse_weight;

        if (cell_links[this_index])
          {
            cell_links[this_index]->addCell(cell_count, this_weight);

            if (cell_index == ed.m_clusters->seedCellID[this_index] && cell_links[this_index]->size() > 1)
              //Seed cells aren't shared,
              //so no need to check this on the other case.
              {
                CaloClusterCellLink::iterator begin_it = cell_links[this_index]->begin();
                CaloClusterCellLink::iterator back_it  = std::prev(cell_links[this_index]->end());

                const unsigned int first_idx = begin_it.index();
                const double first_wgt = begin_it.weight();

                begin_it.reindex(back_it.index());
                begin_it.reweight(back_it.weight());

                back_it.reindex(first_idx);
                back_it.reweight(first_wgt);

                //Of course, this is to ensure the first cell is the seed cell,
                //in accordance to the way some cluster properties
                //(mostly phi-related) are calculated.
              }
          }

        if (this_tag.is_shared_between_clusters())
          {
            const int other_index = this_tag.secondary_cluster_index();
            if (cell_links[other_index])
              {
                cell_links[other_index]->addCell(cell_count, reverse_weight);
              }
          }
      }
  };

  if (cell_collection->isOrderedAndComplete())
    //Fast path: cell indices within the collection and identifierHashes match!
    {
      for (int cell_index = 0; cell_index < NCaloCells; ++cell_index)
        {
          process_cell(cell_index, cell_index);
        }
    }
  else if (cell_collection->isOrdered() && m_missingCellsToFill.size() > 0)
    {
      size_t missing_cell_count = 0;
      for (int cell_index = 0; cell_index < NCaloCells; ++cell_index)
        {
          if (missing_cell_count < m_missingCellsToFill.size() && cell_index == m_missingCellsToFill[missing_cell_count])
            {
              ++missing_cell_count;
              continue;
            }
          process_cell(cell_index, cell_index - missing_cell_count);
        }
    }
  else
    //Slow path: be careful.
    {
      CaloCellContainer::const_iterator iCells = cell_collection->begin();

      for (int cell_count = 0; iCells != cell_collection->end(); ++iCells, ++cell_count)
        {
          const CaloCell * cell = (*iCells);

          //const int cell_index = m_calo_id->calo_cell_hash(cell->ID());
          const int cell_index = cell->caloDDE()->calo_hash();
                 
          process_cell(cell_index, cell_count);
        }
    }
  const auto after_cells = clock_type::now();

  std::vector<int> cluster_order(ed.m_clusters->number);

  std::iota(cluster_order.begin(), cluster_order.end(), 0);

  std::sort(cluster_order.begin(), cluster_order.end(), [&](const int a, const int b) -> bool
  {
    const bool a_valid = ed.m_clusters->seedCellID[a] >= 0;
    const bool b_valid = ed.m_clusters->seedCellID[b] >= 0;
    if (a_valid && b_valid)
      {
        return ed.m_clusters->clusterEt[a]
        > ed.m_clusters->clusterEt[b];
      }
    else if (a_valid)
      {
        return true;
      }
    else if (b_valid)
      {
        return false;
      }
    else
      {
        return b > a;
      }
  } );

  //Ordered by Et as in the default algorithm...
  //The fact that some invalid clusters
  //(with possibly trash values for Et)
  //can crop up is irrelevant since
  //we don't add those anyway:
  //the rest is still ordered like we want it to be.

  const auto after_sort = clock_type::now();

  cluster_container->clear();
  cluster_container->reserve(cell_links.size());

  for (size_t i = 0; i < cluster_order.size(); ++i)
    {
      const int cluster_index = cluster_order[i];

      if (cell_links[cluster_index] != nullptr && cell_links[cluster_index]->size() > 0)
        {
          xAOD::CaloCluster * cluster = new xAOD::CaloCluster();
          cluster_container->push_back(cluster);

          cluster->addCellLink(cell_links[cluster_index].release());
          cluster->setClusterSize(m_clusterSize);
          if (m_useCPUPropertiesCalculation)
            {
              CaloClusterKineHelper::calculateKine(cluster, false, true, true);
            }
          else
            {
              cluster->setE(ed.m_clusters->clusterEnergy[cluster_index]);
              cluster->setEta(ed.m_clusters->clusterEta[cluster_index]);
              cluster->setPhi(ed.m_clusters->clusterPhi[cluster_index]);
            }

          if (m_saveUncalibrated)
            {
              cluster->setRawE(cluster->calE());
              cluster->setRawEta(cluster->calEta());
              cluster->setRawPhi(cluster->calPhi());
              cluster->setRawM(cluster->calM());
            }

        }

    }

  const auto after_fill = clock_type::now();

  if (m_measureTimes)
    {
      record_times(ctx.evt(), time_cast(start, after_send),
                   time_cast(after_send, after_creation),
                   time_cast(after_creation, after_cells),
                   time_cast(after_cells, after_sort),
                   time_cast(after_sort, after_fill)
                  );
    }

  return StatusCode::SUCCESS;

}


StatusCode BasicGPUToAthenaImporter::finalize()
{

  if (m_measureTimes)
    {
      print_times("Transfer_from_GPU Cluster_Creation Cell_Adding Sorting Collection_Filling", 5);
    }
  return StatusCode::SUCCESS;
}


BasicGPUToAthenaImporter::~BasicGPUToAthenaImporter()
{
  //Nothing!
}

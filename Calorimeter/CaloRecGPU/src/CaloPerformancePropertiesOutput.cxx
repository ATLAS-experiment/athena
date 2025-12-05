//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloPerformancePropertiesOutput.h"

#include "CaloIdentifier/LArNeighbours.h"

#include "CaloRecGPU/BaseDefinitions.h"

#include "StoreGate/DataHandle.h"

#include "MacroHelpers.h"

#include <fstream>

using namespace CaloRecGPU;

CaloPerformancePropertiesOutput::CaloPerformancePropertiesOutput(const std::string & type, const std::string & name, const IInterface * parent):
  base_class(type, name, parent)
{
}


StatusCode CaloPerformancePropertiesOutput::initialize()
{
  ATH_CHECK( m_cellsKey.initialize() );

  ATH_CHECK( m_noiseCDOKey.initialize() );

  ATH_CHECK( detStore()->retrieve(m_calo_id, "CaloCell_ID") );

  auto get_neighbour_option_from_string = [](const std::string & str, bool & failed)
  {
    failed = false;
    CRGPU_RECURSIVE_MACRO(
            CRGPU_CHEAP_STRING_TO_ENUM( str, LArNeighbours,
                                        prevInPhi,
                                        nextInPhi,
                                        prevInEta,
                                        nextInEta,
                                        faces2D,
                                        corners2D,
                                        all2D,
                                        prevInSamp,
                                        nextInSamp,
                                        upAndDown,
                                        prevSubDet,
                                        nextSubDet,
                                        all3D,
                                        corners3D,
                                        all3DwithCorners,
                                        prevSuperCalo,
                                        nextSuperCalo,
                                        super3D
                                      )
    )
    else
      {
        failed = true;
        return LArNeighbours::super3D;
      }
  };

  bool neigh_failed = false;
  m_growNeighborOption = get_neighbour_option_from_string(m_growNeighborOptionString, neigh_failed);

  if (neigh_failed)
    {
      ATH_MSG_ERROR("Invalid Grow Neighbour Option: " << m_growNeighborOptionString);
    }

  neigh_failed = false;
  m_growNeighborOption = get_neighbour_option_from_string(m_splitNeighborOptionString, neigh_failed);

  if (neigh_failed)
    {
      ATH_MSG_ERROR("Invalid Split Neighbour Option: " << m_splitNeighborOption);
    }

  return StatusCode::SUCCESS;
}

StatusCode CaloPerformancePropertiesOutput::execute(const EventContext & ctx, xAOD::CaloClusterContainer * cluster_collection) const
{
  SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);
  if ( !cell_collection.isValid() )
    {
      ATH_MSG_ERROR( " Cannot retrieve CaloCellContainer: " << cell_collection.name()  );
      return StatusCode::FAILURE;
    }

  SG::ReadCondHandle<CaloNoise> noise_handle(m_noiseCDOKey, ctx);
  const CaloNoise * noise_tool = *noise_handle;

  std::vector<unsigned char> cell_classification(NCaloCells, 0);
  //0 for invalid, 1 for terminal, 2 for growing, 3 for seed...


  EventPerformanceInfo ev_perf_info;

  ev_perf_info.num_clusters = cluster_collection->size();

  for (const auto & cell_ptr : *cell_collection)
    {
      const float energy = cell_ptr->energy();
      const float noise  = m_twoGaussianNoise ?
                           noise_tool->getEffectiveSigma(cell_ptr->ID(), cell_ptr->gain(), energy) :
                           noise_tool->getNoise(cell_ptr->ID(), cell_ptr->gain());
      const float s_n_r = (noise > 0 ? energy / noise : 0.00001f);
      const float abs_s_n_r = fabsf(s_n_r);

      const int this_hash_ID = cell_ptr->caloDDE()->calo_hash();

      if (s_n_r >= m_seedThreshold || (m_seedCutsInAbsE && abs_s_n_r >= m_seedThreshold))
        {
          cell_classification[this_hash_ID] = 3;
          ++ev_perf_info.total_seed;
        }
      else if (s_n_r >= m_growThreshold || (m_neighborCutsInAbsE && abs_s_n_r >= m_growThreshold))
        {
          cell_classification[this_hash_ID] = 2;
          ++ev_perf_info.total_grow;
        }
      else if (s_n_r >= m_cellThreshold || (m_cellCutsInAbsE && abs_s_n_r >= m_cellThreshold))
        {
          cell_classification[this_hash_ID] = 1;
          ++ev_perf_info.total_term;
        }
      else
        {
          cell_classification[this_hash_ID] = 0;
          ++ev_perf_info.total_invalid;
        }
    }

  auto accumulate_stats = [&](EventPerformanceInfo::Stats & s, const double v)
  {
    s.min = std::min(s.min, v);
    s.max = std::max(s.max, v);
    s.avg += v;
    s.stddev += v * v;
  };

  int cluster_count = 0;

  std::vector<int> cluster_assignment(NCaloCells, -1);

  constexpr int check_seed_flag = 0x40000000;
  constexpr int check_first_flag = 0x20000000;
  //Since we only have 187652 cells, this works...

  constexpr int check_mask = ~(check_seed_flag | check_first_flag);

  std::vector<int> cells_to_check;
  std::vector<int> new_cells_to_check;
  std::vector<IdentifierHash> neighs;
  
  auto check_for_restrict = [&] (const auto & hash_ID, const bool restrict_HECIWandFCal, const bool restrict_PS)
  {
    const Identifier cell_identifier = m_calo_id->cell_id(hash_ID);

    const auto sub_calo            = m_calo_id->sub_calo(cell_identifier);
    const auto region              = m_calo_id->region(cell_identifier);
    const auto intra_calo_sampling = m_calo_id->sampling(cell_identifier);

    const bool is_PS               =   (sub_calo == CaloCell_ID::LAREM   && intra_calo_sampling == 0);

    const bool is_HECIW_or_FCAL    = ( (sub_calo == CaloCell_ID::LARHEC  && region              == 1 ) ||
                                       (sub_calo == CaloCell_ID::LARFCAL && intra_calo_sampling >  1 )    );
              
    return (is_HECIW_or_FCAL && restrict_HECIWandFCal) || (is_PS && restrict_PS);
  };

  for (const auto & cluster_ptr : *cluster_collection)
    {
      int this_seed = 0, this_grow = 0, this_term = 0;

      cells_to_check.clear();
      
      bool first = true;

      for (const CaloCell * cell_ptr : *cluster_ptr)
        {
          const int this_hash_ID = cell_ptr->caloDDE()->calo_hash();

          cluster_assignment[this_hash_ID] = cluster_count | check_seed_flag | (first ? 0 : check_first_flag);

          const unsigned char this_classification = cell_classification[this_hash_ID];

          switch (this_classification)
            {
              case 3:
                ++this_seed;
                cluster_assignment[this_hash_ID] = cluster_count | (first ? 0 : check_first_flag);
                cells_to_check.push_back(this_hash_ID);
                break;
              case 2:
                ++this_grow;
                break;
              case 1:
                ++this_term;
                break;
              default:
                ATH_MSG_WARNING("Invalid cell " << this_hash_ID << " in cluster " << cluster_count << ".");
                break;
            }

          first = false;
        }

      ev_perf_info.seed_in_cluster += this_seed;
      ev_perf_info.grow_in_cluster += this_grow;
      ev_perf_info.term_in_cluster += this_term;

      accumulate_stats(ev_perf_info.cluster_size, cluster_ptr->size());
      accumulate_stats(ev_perf_info.cluster_num_seed, this_seed);
      accumulate_stats(ev_perf_info.cluster_num_grow, this_grow);
      accumulate_stats(ev_perf_info.cluster_num_term, this_term);

      //Now for the complicated part: the radius...


      int min_radius = -1, radius = -1;

      while (cells_to_check.size() > 0)
        {
          ++radius;

          new_cells_to_check.clear();

          for (const auto & hash_ID : cells_to_check)
            {
              if (min_radius < 0 && cell_classification[hash_ID] <= 1)
                {
                  min_radius = radius;
                  //We have reached a terminal cell: this is the minimum radius.
                }
              
              const bool restrict = check_for_restrict(hash_ID,
                                                       m_growRestrictHECIWandFCalNeighbors, 
                                                       m_growRestrictPSNeighbors);

              neighs.clear();

              if (restrict && (m_growNeighborOption & LArNeighbours::nextInSamp))
                {
                  m_calo_id->get_neighbours(hash_ID, LArNeighbours::nextInSamp, neighs);
                }
              else
                {
                  m_calo_id->get_neighbours(hash_ID, m_growNeighborOption, neighs);
                }

              for (const auto & neigh_hash : neighs)
                {
                  int & neigh_assignment = cluster_assignment[neigh_hash];
                  if ((neigh_assignment & check_seed_flag) && ((neigh_assignment & check_mask) == cluster_count))
                    {
                      new_cells_to_check.push_back(neigh_hash) ;
                      neigh_assignment = (neigh_assignment & ~check_seed_flag);
                    }
                }
            }

          cells_to_check.swap(new_cells_to_check);
        }

      accumulate_stats(ev_perf_info.cluster_seed_min_radius, min_radius);
      accumulate_stats(ev_perf_info.cluster_seed_max_radius, radius);

      //And now the radius from the first cell
      //(for split clusters)

      if (cluster_ptr->size() > 0)
        {
          cells_to_check.clear();
          cells_to_check.push_back(cluster_ptr->begin()->caloDDE()->calo_hash());

          radius = -1;
          min_radius = -1;

          while (cells_to_check.size() > 0)
            {
              ++radius;

              new_cells_to_check.clear();

              for (const auto & hash_ID : cells_to_check)
                {
              const bool restrict = check_for_restrict(hash_ID,
                                                       m_splitRestrictHECIWandFCalNeighbors, 
                                                       m_splitRestrictPSNeighbors);
                                                       

                  neighs.clear();

                  if (restrict && (m_splitNeighborOption & LArNeighbours::nextInSamp))
                    {
                      m_calo_id->get_neighbours(hash_ID, LArNeighbours::nextInSamp, neighs);
                    }
                  else
                    {
                      m_calo_id->get_neighbours(hash_ID, m_splitNeighborOption, neighs);
                    }

                  for (const auto & neigh_hash : neighs)
                    {
                      int & neigh_assignment = cluster_assignment[neigh_hash];
                      if ((neigh_assignment & check_first_flag) && ((neigh_assignment & check_mask) == cluster_count))
                        {
                          new_cells_to_check.push_back(neigh_hash) ;
                          neigh_assignment = (neigh_assignment & ~check_first_flag);
                        }
                    }
                }

              cells_to_check.swap(new_cells_to_check);
            }
            
            accumulate_stats(ev_perf_info.cluster_first_max_radius, radius);
        }

      ++cluster_count;
    }


  auto finalize_stats = [&](EventPerformanceInfo::Stats & s)
  {
    s.avg /= cluster_collection->size();
    s.stddev /= cluster_collection->size();
    s.stddev -= s.avg * s.avg;
    //Sure, floating point accuracy issues may ensue,
    //but not the most relevant anyway...
  };

  finalize_stats(ev_perf_info.cluster_size);
  finalize_stats(ev_perf_info.cluster_num_seed);
  finalize_stats(ev_perf_info.cluster_num_grow);
  finalize_stats(ev_perf_info.cluster_num_term);
  finalize_stats(ev_perf_info.cluster_seed_min_radius);
  finalize_stats(ev_perf_info.cluster_seed_max_radius);
  finalize_stats(ev_perf_info.cluster_first_max_radius);

  {
    std::lock_guard<std::mutex> lock_guard(m_mutex);

    m_eventInfo.push_back(ev_perf_info);
    m_eventNumbers.push_back(ctx.evt());
  }

  return StatusCode::SUCCESS;
}


StatusCode CaloPerformancePropertiesOutput::finalize()
{
  if (m_fileName.size() > 0)
    {
      std::ofstream out(m_fileName);

      std::vector<size_t> indices(m_eventNumbers.size());

      std::iota(indices.begin(), indices.end(), 0);
      std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b)
      {
        return m_eventNumbers[a] < m_eventNumbers[b];
      }
               );

      out << "Event_Number Number_Clusters "
          << "Total_Seed Total_Grow Total_Term Total_Invalid "
          << "Seed_In_Cluster Term_In_Cluster";

      auto print_stat_name = [&](const auto & name)
      {
        out << " " << name << "_Min " << name << "_Max " << name << "_Avg " << name << "_Stddev";
      };

      print_stat_name("Size");
      print_stat_name("Num_Seed");
      print_stat_name("Num_Grow");
      print_stat_name("Num_Term");
      print_stat_name("Radius_Seed_Min");
      print_stat_name("Radius_Seed_Max");
      print_stat_name("Radius_First");

      out << "\n";

      auto print_stat = [&](const EventPerformanceInfo::Stats & s)
      {
        out << " " << s.min << " " << s.max << " " << s.avg << " " << s.stddev;
      };

      for (const auto & idx : indices)
        {
          out << m_eventNumbers[idx] << " ";

          const auto & info = m_eventInfo[idx];

          out << info.num_clusters << " " << info.total_seed << " " << info.total_grow << " "
              << info.total_term << " " << info.total_invalid << " " << info.seed_in_cluster << " "
              << info.grow_in_cluster << " " << info.term_in_cluster;

          print_stat(info.cluster_size);
          print_stat(info.cluster_num_seed);
          print_stat(info.cluster_num_grow);
          print_stat(info.cluster_num_term);
          print_stat(info.cluster_seed_min_radius);
          print_stat(info.cluster_seed_max_radius);
          print_stat(info.cluster_first_max_radius);

          out << "\n";
        }

      out << std::endl;

      out.close();

    }

  return StatusCode::SUCCESS;
}

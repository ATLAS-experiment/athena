//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "TopoAutomatonClustering.h"
#include "TopoAutomatonClusteringImpl.h"


#include <string>
#include <climits> //For CHAR_BIT... though it's a slightly inefficient way of saying 8.

#include "CaloIdentifier/CaloCell_ID.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

#include "MacroHelpers.h"

using namespace CaloRecGPU;
using namespace TAGrowing;

TopoAutomatonClustering::TopoAutomatonClustering(const std::string & type, const std::string & name, const IInterface * parent):
  base_class(type, name, parent),
  CaloGPUTimed(this)
{
}

StatusCode TopoAutomatonClustering::initialize_non_CUDA()
{

  m_options.allocate();


  using PackType = decltype(m_options.m_options->valid_sampling_seed);

  static_assert(CaloCell_ID::getNumberOfSamplings() <= sizeof(PackType) * CHAR_BIT,  "We are assuming that we have fewer samplings that bits per int!");

  //Possibly more elegant alternative: compile-time packed bool vector (std::bitset?) with CUDA compat.
  //Been there, done that, overkill since number of samplings shouldn't change unexpectedly overnight
  //and it doesn't seem that likely to me that it'll reach anything that a 64-bit int wouldn't cover
  //(at least without implying such a major overhaul that the code will need a deeper redesign anyway...)

  auto get_sampling_from_string = [](const std::string & str, bool & failed)
  {
    failed = false;
    CRGPU_RECURSIVE_MACRO(
            CRGPU_CHEAP_STRING_TO_ENUM( str, CaloCell_ID,
                                        PreSamplerB,
                                        EMB1,
                                        EMB2,
                                        EMB3,
                                        PreSamplerE,
                                        EME1,
                                        EME2,
                                        EME3,
                                        HEC0,
                                        HEC1,
                                        HEC2,
                                        HEC3,
                                        TileBar0,
                                        TileBar1,
                                        TileBar2,
                                        TileGap1,
                                        TileGap2,
                                        TileGap3,
                                        TileExt0,
                                        TileExt1,
                                        TileExt2,
                                        FCAL0,
                                        FCAL1,
                                        FCAL2
                                      )
    )
    else
      {
        failed = true;
        return CaloCell_ID::Unknown;
      }
  };


  PackType & seed_samplings = m_options.m_options->valid_sampling_seed;

  seed_samplings = 0;
  
  for (const std::string & samp_name : m_samplingNames)
    {
      bool failed = false;
      const PackType sampling = (PackType) get_sampling_from_string(samp_name, failed);

      if (failed)
        {
          ATH_MSG_ERROR( "Calorimeter sampling" << samp_name
                         << " is not a valid Calorimeter sampling name and will be ignored! "
                         << "Valid names are: "
                         << "PreSamplerB, EMB1, EMB2, EMB3, "
                         << "PreSamplerE, EME1, EME2, EME3, "
                         << "HEC0, HEC1, HEC2, HEC3, "
                         << "TileBar0, TileBar1, TileBar2, "
                         << "TileGap1, TileGap2, TileGap3, "
                         << "TileExt0, TileExt1, TileExt2, "
                         << "FCAL0, FCAL1, FCAL2."  );
        }
      else
        {
          seed_samplings |= ((PackType) 1) << sampling;
        }
    }

  auto get_calo_from_string = [](const std::string & str, bool & failed)
  {
    failed = false;
    CRGPU_RECURSIVE_MACRO(
            CRGPU_CHEAP_STRING_TO_ENUM( str, CaloCell_ID,
                                        LAREM, LARHEC,
                                        LARFCAL, TILE,
                                        LARMINIFCAL
                                      )
    )
    else
      {
        failed = true;
        return CaloCell_ID::NOT_VALID;
      }
  };
  
  
  auto calo_to_sampling_mask = [](const CaloCell_ID::SUBCALO sc) -> PackType
  {
    switch (sc)
      {
        case CaloCell_ID::LAREM:
          return 0xFFU;
        //PreSamplerB=0, EMB1, EMB2, EMB3,
        //PreSamplerE, EME1, EME2, EME3=7,
        case CaloCell_ID::LARHEC:
          return 0xF00U;
        //HEC0=8, HEC1, HEC2, HEC3=11,
        case CaloCell_ID::TILE:
          return 0x1FF000U;
        //TileBar0=12, TileBar1, TileBar2,
        //TileGap1, TileGap2, TileGap3,
        //TileExt0, TileExt1, TileExt2=20,
        case CaloCell_ID::LARFCAL:
          return 0xE00000U;
        //FCAL0=21, FCAL1, FCAL2=23
        case CaloCell_ID::LARMINIFCAL:
          return 0xF000000U;
        //MINIFCAL0=24, MINIFCAL1, MINIFCAL2, MINIFCAL3=27,
        default:
          return 0;
      }
  };
  
  PackType & calo_samplings = m_options.m_options->valid_calorimeter_by_sampling;

  calo_samplings = 0;
  
  for (const std::string & calo_name : m_caloNames)
    {
      bool failed = false;
      const PackType sample_mask = calo_to_sampling_mask(get_calo_from_string(calo_name, failed));

      if (failed)
        {
          ATH_MSG_ERROR( "Calorimeter " << calo_name
                         << " is not a valid Calorimeter name and will be ignored! "
                         << "Valid names are: LAREM, LARHEC, LARFCAL, and TILE."  );
        }
      else
        {
          calo_samplings |= sample_mask;
        }
    }
  
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
  m_options.m_options->neighbour_options = (unsigned int) get_neighbour_option_from_string(m_neighborOptionString, neigh_failed);

  if (neigh_failed)
    {
      ATH_MSG_ERROR("Invalid Neighbour Option: " << m_neighborOptionString);
    }

  m_options.m_options->seed_threshold = m_seedThresholdOnEorAbsEinSigma;
  m_options.m_options->grow_threshold = m_neighborThresholdOnEorAbsEinSigma;
  m_options.m_options->terminal_threshold = m_cellThresholdOnEorAbsEinSigma;
  m_options.m_options->abs_seed = m_seedCutsInAbsE;
  m_options.m_options->abs_grow = m_neighborCutsInAbsE;
  m_options.m_options->abs_terminal = m_cellCutsInAbsE;
  m_options.m_options->use_two_gaussian = m_twoGaussianNoise;

  m_options.m_options->treat_L1_predicted_as_good = m_treatL1PredictedCellsAsGood;
  m_options.m_options->use_time_cut = m_cutCellsInTime;
  m_options.m_options->keep_significant_cells = m_keepSignificantCells;
  m_options.m_options->completely_exclude_cut_seeds = m_excludeCutSeedsFromClustering;
  m_options.m_options->time_threshold = m_timeThreshold;
  m_options.m_options->snr_threshold_for_keeping_cells = m_thresholdForKeeping;

  m_options.m_options->limit_HECIW_and_FCal_neighs = m_restrictHECIWandFCalNeighbors;
  m_options.m_options->limit_PS_neighs = m_restrictPSNeighbors;
  
  m_options.m_options->use_crosstalk = m_xtalkEM2;
  m_options.m_options->crosstalk_delta = m_xtalkDeltaT;

  ATH_CHECK( m_kernelSizeOptimizer.retrieve() );

  return StatusCode::SUCCESS;
}

StatusCode TopoAutomatonClustering::initialize_CUDA()
{
  m_options.sendToGPU();
  register_kernels( *(m_kernelSizeOptimizer.get()) );
  
  return StatusCode::SUCCESS;
}



StatusCode TopoAutomatonClustering::execute(const EventContext & ctx, const ConstantDataHolder & constant_data,
                                            EventDataHolder & event_data, void * /*temporary_buffer*/             ) const
{

  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  static_assert(sizeof(TopoAutomatonGrowingTemporaries) <= sizeof(ClusterMomentsArr), "We store the temporaries in the cluster moments, so the sizes must be compatible!");
  
  const auto start = clock_type::now();
  
  const auto before_snr = clock_type::now();

  signalToNoise(event_data, constant_data, m_options, *(m_kernelSizeOptimizer.get()), m_measureTimes);

  const auto before_pairs = clock_type::now();

  cellPairs(event_data, constant_data, m_options, *(m_kernelSizeOptimizer.get()), m_measureTimes);

  const auto before_growing = clock_type::now();

  clusterGrowing(event_data, constant_data, m_options, *(m_kernelSizeOptimizer.get()), m_measureTimes);

  const auto end = clock_type::now();


  if (m_measureTimes)
    {
      record_times(ctx.evt(),
                   time_cast(start, before_snr),
                   time_cast(before_snr, before_pairs),
                   time_cast(before_pairs, before_growing),
                   time_cast(before_growing, end)
                  );
    }

  return StatusCode::SUCCESS;


}


StatusCode TopoAutomatonClustering::finalize()
{
  if (m_measureTimes)
    {
      print_times("Preprocessing Signal-to-Noise_Ratio Cell_Pair_Creation Cluster_Growing", 4);
    }
  return StatusCode::SUCCESS;
}


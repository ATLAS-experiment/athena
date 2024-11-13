//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "TopoAutomatonSplitting.h"
#include "TopoAutomatonSplittingImpl.h"


#include <string>
#include <climits> //For CHAR_BIT... though it's a slightly inefficient way of saying 8.

#include "CaloIdentifier/CaloCell_ID.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

#include "MacroHelpers.h"

using namespace CaloRecGPU;
using namespace TASplitting;

TopoAutomatonSplitting::TopoAutomatonSplitting(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent),
  CaloGPUTimed(this)
{
  declareInterface<CaloClusterGPUProcessor> (this);

}

StatusCode TopoAutomatonSplitting::initialize_non_CUDA()
{
  m_options.allocate();

  using PackType = decltype(m_options.m_options->valid_sampling_primary);

  static_assert(CaloCell_ID::getNumberOfSamplings() <= sizeof(PackType) * CHAR_BIT,  "We are assuming that we have fewer samplings that bits per int!");

  auto get_option_from_string = [](const std::string & str, bool & failed)
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


  auto process_sampling = [&get_option_from_string](const std::vector<std::string> & sampling_names, std::string & invalid_names, PackType & sampling_option)
  {
    sampling_option = 0;
    for (const std::string & samp_name : sampling_names)
      {
        bool failed = false;
        const PackType sampling = (PackType) get_option_from_string(samp_name, failed);

        if (failed)
          {
            if (invalid_names.size() == 0)
              {
                invalid_names = "'" + samp_name + "'";
              }
            else
              {
                invalid_names += ", '" + samp_name + "'";
              }
          }
        else
          {
            sampling_option |= ((PackType) 1) << sampling;
          }
      }
  };

  std::string invalid_names;

  process_sampling(m_samplingNames, invalid_names, m_options.m_options->valid_sampling_primary);

  if (invalid_names.size() > 0)
    {
      ATH_MSG_ERROR( "Calorimeter samplings " << invalid_names
                     << " are not a valid Calorimeter sampling name and will be ignored! "
                     << "Valid names are: "
                     << "PreSamplerB, EMB1, EMB2, EMB3, "
                     << "PreSamplerE, EME1, EME2, EME3, "
                     << "HEC0, HEC1, HEC2, HEC3, "
                     << "TileBar0, TileBar1, TileBar2, "
                     << "TileGap1, TileGap2, TileGap3, "
                     << "TileExt0, TileExt1, TileExt2, "
                     << "FCAL0, FCAL1, FCAL2."  );
    }

  invalid_names.clear();

  process_sampling(m_secondarySamplingNames, invalid_names, m_options.m_options->valid_sampling_secondary);

  if (invalid_names.size() > 0)
    {
      ATH_MSG_ERROR( "Calorimeter samplings " << invalid_names
                     << " are not a valid Calorimeter sampling name and will be ignored! "
                     << "Valid names are: "
                     << "PreSamplerB, EMB1, EMB2, EMB3, "
                     << "PreSamplerE, EME1, EME2, EME3, "
                     << "HEC0, HEC1, HEC2, HEC3, "
                     << "TileBar0, TileBar1, TileBar2, "
                     << "TileGap1, TileGap2, TileGap3, "
                     << "TileExt0, TileExt1, TileExt2, "
                     << "FCAL0, FCAL1, FCAL2."  );
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
    //I know Topological Clustering only supports a subset of those,
    //but this is supposed to be a general data exporting tool...
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

  //We must repeat this printing part because ATH_MSG_ERROR
  //is a macro that apparently calls a this->msg(...) function.
  //Of course it won't work within a lambda...

  m_options.m_options->min_num_cells = m_nCells;
  m_options.m_options->min_maximum_energy = m_minEnergy;
  m_options.m_options->EM_shower_scale = m_emShowerScale;
  m_options.m_options->share_border_cells = m_shareBorderCells;
  m_options.m_options->use_absolute_energy = m_absOpt;
  m_options.m_options->treat_L1_predicted_as_good = m_treatL1PredictedCellsAsGood;

  m_options.m_options->limit_HECIW_and_FCal_neighs = m_restrictHECIWandFCalNeighbors;
  m_options.m_options->limit_PS_neighs = m_restrictPSNeighbors;
  
  ATH_CHECK( m_kernelSizeOptimizer.retrieve() );
  
  return StatusCode::SUCCESS;
}

StatusCode TopoAutomatonSplitting::initialize_CUDA()
{
  m_options.sendToGPU();
  register_kernels( *(m_kernelSizeOptimizer.get()) );
  
  return StatusCode::SUCCESS;
}

StatusCode TopoAutomatonSplitting::execute(const EventContext & ctx, const ConstantDataHolder & constant_data,
                                           EventDataHolder & event_data, void * /*temporary_buffer*/                ) const
{

  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  static_assert(sizeof(TopoAutomatonSplittingTemporaries) <= sizeof(ClusterMomentsArr), "We store the temporaries in the cluster moments, so the sizes must be compatible!");
  
  const auto start = clock_type::now();
  const auto preprocessing_end = clock_type::now();
  
  fillNeighbours(event_data, constant_data, m_options, *(m_kernelSizeOptimizer.get()), m_measureTimes);
  
  const auto after_neighs = clock_type::now();
  
  findLocalMaxima(event_data, constant_data, m_options, *(m_kernelSizeOptimizer.get()), m_measureTimes);
  
  const auto after_maxima = clock_type::now();
  
  excludeSecondaryMaxima(event_data, constant_data, m_options, *(m_kernelSizeOptimizer.get()), m_measureTimes);
  
  const auto after_secondary_maxima = clock_type::now();
  
  splitClusterGrowing(event_data, constant_data, m_options, *(m_kernelSizeOptimizer.get()), m_measureTimes);
  
  const auto after_growing = clock_type::now();
  
  cellWeightingAndFinalization(event_data, constant_data, m_options, *(m_kernelSizeOptimizer.get()), m_measureTimes);

  const auto end = clock_type::now();
  
  if (m_measureTimes)
    {
      record_times(ctx.evt(),
                   time_cast(start, preprocessing_end),
                   time_cast(preprocessing_end, after_neighs),
                   time_cast(after_neighs, after_maxima),
                   time_cast(after_maxima, after_secondary_maxima),
                   time_cast(after_secondary_maxima, after_growing),
                   time_cast(after_growing, end)
                  );
    }

  return StatusCode::SUCCESS;


}


StatusCode TopoAutomatonSplitting::finalize()
{
  if (m_measureTimes)
    {
      print_times("Preprocessing Fill_List_of_Intra-Cluster_Neighbours Find_Local_Maxima Find_Secondary_Maxima Splitter_Tag_Propagation Cell_Weighting_And_Finalization", 6);
    }
  return StatusCode::SUCCESS;
}

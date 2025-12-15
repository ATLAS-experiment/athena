//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
  base_class(type, name, parent),
  CaloGPUTimed(this)
{
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
  
  cluster_container->clear();

  const auto start = clock_type::now();

  SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);
  if ( !cell_collection.isValid() )
    {
      ATH_MSG_ERROR( " Cannot retrieve CaloCellContainer: " << cell_collection.name()  );
      return StatusCode::FAILURE;
    }
  const DataLink<CaloCellContainer> cell_collection_link (cell_collection.name(), ctx);

  ed.returnToCPU(MomentsOptionsArray::all(), false, false, true);

  const auto after_send = clock_type::now();

  size_t extra_times[5];

  ed.exportClusters(cluster_container,
                    cell_collection_link,
                    MomentsOptionsArray::all(),
                    true,
                    m_saveUncalibrated,
                    false,
                    m_missingCellsToFill,
                    m_measureTimes ? extra_times : nullptr);

  const auto after_export = clock_type::now();
  
  for (auto && cluster : *cluster_container)
    {
      cluster->setClusterSize(m_clusterSize);
    }

  const auto after_size = clock_type::now();

  if (m_measureTimes)
    {
      record_times(ctx.evt(),
                  time_cast(start, after_send),
                  extra_times[0],
                  extra_times[1],
                  extra_times[2],
                  extra_times[3],
                  extra_times[4],
                  time_cast(after_export, after_size)
                  );
    }

  return StatusCode::SUCCESS;

}


StatusCode BasicGPUToAthenaImporter::finalize()
{

  if (m_measureTimes)
    {
      print_times("Transfer_from_GPU Cell_Link_Creation Cell_Adding Sorting Filling_Collection Moments Cluster_Size", 7);
    }
  return StatusCode::SUCCESS;
}

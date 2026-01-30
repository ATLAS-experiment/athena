//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "BasicEventDataGPUExporter.h"

#include "CaloRecGPU/CUDAFriendlyClasses.h"

#include "AthenaKernel/errorcheck.h"
#include "StoreGate/DataHandle.h"

#include "MacroHelpers.h"

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
  ATH_CHECK( m_cellsKey.initialize() );

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

  SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);
  if ( !cell_collection.isValid() )
    {
      ATH_MSG_ERROR( " Cannot retrieve CaloCellContainer: " << cell_collection.name()  );
      return StatusCode::FAILURE;
    }

  if (cell_collection->isOrderedAndComplete())
    {
      ATH_MSG_DEBUG("Taking fast path on event " << ctx.evt());
    }
  else if (cell_collection->isOrdered() && m_missingCellsToFill.size() > 0)
    {
      ATH_MSG_DEBUG("Taking remediated fast path on event " << ctx.evt());
    }
  else
    {
      ATH_MSG_DEBUG("Taking slow path on event " << ctx.evt());
    }
    
  ed.importCells(static_cast<const CaloCellContainer *>(&(*cell_collection)), m_missingCellsToFill);

  const auto post_cells = clock_type::now();
  
  ed.importClusters(cluster_collection,
                    MomentsOptionsArray::all(),
                    m_outputCombinedTags,
                    m_considerSharedCells,
                    m_outputMoments,
                    m_outputExtraMoments,
                    m_missingCellsToFill);

  const auto post_clusters = clock_type::now();
  
  ed.sendToGPU(MomentsOptionsArray::all(), false, false, m_measureTimes);

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

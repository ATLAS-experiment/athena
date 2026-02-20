//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "CaloCPUOutput.h"
#include "CaloRecGPU/Helpers.h"
#include "CaloRecGPU/CUDAFriendlyClasses.h"
#include "CaloRecGPU/DataHolders.h"
#include "CaloRecGPU/StandaloneDataIO.h"
#include "StoreGate/DataHandle.h"


using namespace CaloRecGPU;

CaloCPUOutput::CaloCPUOutput(const std::string & type, const std::string & name, const IInterface * parent):
  base_class(type, name, parent)
{
}


StatusCode CaloCPUOutput::initialize()
{
  ATH_CHECK( m_cellsKey.initialize() );

  ATH_CHECK( detStore()->retrieve(m_calo_id, "CaloCell_ID") );

  return StatusCode::SUCCESS;
}

StatusCode CaloCPUOutput::execute (const EventContext & ctx, xAOD::CaloClusterContainer * cluster_collection) const
{
  SG::ReadHandle<CaloCellContainer> cell_collection(m_cellsKey, ctx);
  if ( !cell_collection.isValid() )
    {
      ATH_MSG_ERROR( " Cannot retrieve CaloCellContainer: " << cell_collection.name()  );
      return StatusCode::FAILURE;
    }
    
  EventDataHolder ed;
  
  ed.allocate(false);
  
  ed.importCells(static_cast<const CaloCellContainer *>(&(*cell_collection)));
  
  ed.importClusters(cluster_collection, MomentsOptionsArray::all(), m_outputTags, true, false, false);
  
  if (m_saveCellInfo)
    {
      const auto err =  StandaloneDataIO::save_event_to_folder(ctx.evt(), std::string(m_savePath),
                                                               ed.m_cell_info, ed.m_clusters,
                                                               m_filePrefix, m_fileSuffix, m_numWidth);

      if (err != StandaloneDataIO::ErrorState::OK)
        {
          return StatusCode::FAILURE;
        }
    }
  else
    {
      const auto err = StandaloneDataIO::save_clusters_to_folder(ctx.evt(), std::string(m_savePath),
                                                                 ed.m_clusters, m_filePrefix, m_fileSuffix, m_numWidth);

      if (err != StandaloneDataIO::ErrorState::OK)
        {
          return StatusCode::FAILURE;
        }
    }

  return StatusCode::SUCCESS;

}

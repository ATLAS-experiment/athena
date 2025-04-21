/*
  Copyright (C) 2002-2015 CERN for the benefit of the ATLAS collaboration
*/


/**
 * @file  CaloClusterProcessor.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date March, 2006
 * @brief Base class for cluster processing tools called from
 *        @c CaloClusterMaker that operate on individual clusters.
 */

#include "CaloUtils/CaloClusterProcessor.h"
#include "CaloEvent/CaloClusterContainer.h"
#include "GaudiKernel/ThreadLocalContext.h"


/**
 * @brief Constructor.
 * @param type The type of the tool.
 * @param name The name of the tool.
 * @param parent The parent algorithm of the tool.
 *
 * This just forwards on to the base class constructor.
 */
CaloClusterProcessor::CaloClusterProcessor (const std::string& type,
                                            const std::string& name,
                                            const IInterface* parent)
  : AthAlgTool (type, name, parent)
{
  declareInterface<CaloClusterCollectionProcessor> (this);
  declareInterface<CaloClusterProcessor> (this);
}


/**
 * @brief Execute on an entire collection of clusters.
 * @param collection The container of clusters.
 * @param ctx The event context.
 *
 * This will iterate over all the clusters in @c collection
 * and call @c execute on each one individually.
 */
StatusCode CaloClusterProcessor::execute (const EventContext& ctx,
                                          xAOD::CaloClusterContainer* collection) const
{
  for (xAOD::CaloCluster* clu : *collection) {
    ATH_CHECK( execute (ctx, clu) );
  }
  return StatusCode::SUCCESS;
}

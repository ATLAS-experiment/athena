// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  CaloClusterCollectionProcessor.h
 * @author scott snyder <snyder@bnl.gov>
 * @date March, 2006
 * @brief Base class for cluster processing tools called from
 *        @c CaloClusterMaker.
 *
 * This class defines an @c execute method that takes as an argument
 * a @c CaloClusterContainer.  Tools that operate on individual clusters
 * should derive from @c CaloClusterProcessor instead of this.
 */

#ifndef CALOREC_CALOCLUSTERCOLLECTIONPROCESSOR_H
#define CALOREC_CALOCLUSTERCOLLECTIONPROCESSOR_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "GaudiKernel/EventContext.h"


class CaloClusterCollectionProcessor
  : virtual public IAlgTool
{
public:
  
  DeclareInterfaceID(CaloClusterCollectionProcessor,1,0);

  /**
   * @brief Execute on an entire collection of clusters.
   * @param collection The container of clusters.
   * param ctx The event context.
   */
  virtual StatusCode execute (const EventContext& ctx,
                              xAOD::CaloClusterContainer* collection) const = 0;

};


#endif // not CALOREC_CALOCLUSTERCOLLECTIONPROCESSOR_H

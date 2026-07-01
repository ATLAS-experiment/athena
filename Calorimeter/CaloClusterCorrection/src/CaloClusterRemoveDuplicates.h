/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCLUSTERCORRECTION_CALOCLUSTERREMOVEDUPLICATES_H
#define CALOCLUSTERCORRECTION_CALOCLUSTERREMOVEDUPLICATES_H

/**************************************************************************

NAME:     CaloClusterRemoveDuplicates.h
AUTHORS:  M. Cooke

CREATED:  May, 2008

PURPOSE: Address duplicate cluster problem (pathology under investigation)
         Searches for pairs within a window and eliminates one w/ lower Et

Base class: CaloClusterCollectionProcessor

***************************************************************************/

// INCLUDE HEADER FILES:
#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "AthenaBaseComps/AthAlgTool.h"

class CaloClusterRemoveDuplicates :  public AthAlgTool, virtual public CaloClusterCollectionProcessor
{
  
 public:
  
  // Constructor
  CaloClusterRemoveDuplicates(const std::string& type,
                              const std::string& name,
                              const IInterface* parent);
  
  using CaloClusterCollectionProcessor::execute; // xxx temporary
  StatusCode execute(const EventContext& ctx,
                     xAOD::CaloClusterContainer*  clusColl) const override;
    
 private:
  
  int compare( xAOD::CaloCluster* clus1 , xAOD::CaloCluster* clus2 ) const;
  
  // If two cluster are within deta_cut and dphi_cut, lower ET one will be removed
  Gaudi::Property<float> m_deta_cut{this, "deta_cut", 0.05};
  Gaudi::Property<float> m_dphi_cut{this, "dphi_cut", 0.05};
};

#endif

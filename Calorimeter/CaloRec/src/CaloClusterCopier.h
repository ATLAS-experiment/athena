/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-
#ifndef CALOREC_CALOCLUSTERCOPIER_H
#define CALOREC_CALOCLUSTERCOPIER_H

/********************************************************************

NAME:     CaloClusterCopier.h 
PACKAGE:  offline/Calorimeter/CaloRec

AUTHORS:  D. Rousseau, copied from LArClusterCopier from Hong
CREATED:  September 1005

PURPOSE:  Copy a CaloClusterContainer, and its Clusters, into another
	  CaloClusterContainer

          Input: Input ClusterContainer 
          Output: new Cluster container 
          Parameters: 

Updated:  May 5, 2004    (Sven Menke)
	  base class changed from algo to tool

********************************************************************/

//  cluster builder with simple  Algorithm (fixed cone around seed)

#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"

class CaloClusterCopier : public AthAlgTool, virtual public CaloClusterCollectionProcessor {
 public:

  // constructor 
  CaloClusterCopier(const std::string& type, const std::string& name,
		    const IInterface* parent);
  // destructor 
  virtual ~CaloClusterCopier() override;
  
  // Algorithm virtual methods 
  using CaloClusterCollectionProcessor::execute;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx,
                             xAOD::CaloClusterContainer* ) const override;

  void getClusterSize();

 private:
  // properties 
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_clustersKey{this, "ClustersName", ""};
  Gaudi::Property<bool> m_copyCells{this, "CopyCells", true};
  Gaudi::Property<bool> m_useClusterPosition{this, "UseClusterPosition", false};
  Gaudi::Property<float> m_etCut{this, "etCut", -1.};
  xAOD::CaloCluster::ClusterSize m_clusterSize{xAOD::CaloCluster_v1::CSize_Unknown};
  
};

#endif










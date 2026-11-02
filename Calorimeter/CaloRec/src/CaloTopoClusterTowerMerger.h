// -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef CALOREC_CALOTOPOCLUSTERTOWERMERGER_H
#define CALOREC_CALOTOPOCLUSTERTOWERMERGER_H

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

#include "CaloEvent/CaloClusterCellLinkContainer.h"

#include <vector>
//#include <tuple>
#include <string>
#include <cmath>

class CaloTopoClusterTowerMerger : public AthReentrantAlgorithm
{
public:
  ///@brief Algorithm constructor
  CaloTopoClusterTowerMerger(const std::string& name,ISvcLocator* pSvcLocator);
  ///@brief Baseclass destructor 
  virtual ~CaloTopoClusterTowerMerger();

  ///@name Algorithm interface
  ///@{
  virtual StatusCode initialize() override;                            ///< Initialization sets up read and write handle keys
  virtual StatusCode execute(const EventContext& ctx) const override;  ///< Execution merges the container contents 
  ///@}

private:

  ///@name Internally used types
  ///@{
  typedef SG::ReadHandleKey<xAOD::CaloClusterContainer>    rhandlekey_t;   ///< Input data handle key type
  typedef SG::WriteHandleKey<xAOD::CaloClusterContainer>   whandlekey_t;   ///< Output data handle key type
  typedef SG::WriteHandleKey<CaloClusterCellLinkContainer> lhandlekey_t;   ///< Cell link container output data handle key
  typedef SG::ReadHandle<xAOD::CaloClusterContainer>       rhandle_t;      ///< Input data handle type
  typedef SG::WriteHandle<xAOD::CaloClusterContainer>      whandle_t;      ///< Output data handle type
  typedef SG::WriteHandle<CaloClusterCellLinkContainer>    lhandle_t;      ///< Cell link container output data handle
  ///@}

  ///@name Algorithm properties
  ///@{
  rhandlekey_t  m_clusterContainerKey{this, "TopoClusterContainerKey", "CaloCalTopoCluster", "Topo-cluster container key"}; ///< Input topo-cluster container
  rhandlekey_t  m_towerContainerKey{this, "TopoTowerContainerKey", "CaloCalFwdTopoTower", "Topo-tower container key"}; ///< Input topo-tower container
  whandlekey_t  m_topoSignalContainerKey{this, "TopoSignalContainerKey", "CaloCalTopoSignal", "Topo-signal container key"}; ///< Output merged container
  lhandlekey_t  m_cellLinkContainerKey{this, "TopoSignalCellLinksKey", "", "Topo-signal cell links key"}; ///< Output cell links for merged container
  Gaudi::Property<double> m_clusterRange{this, "TopoClusterRange", 3.2, "Rapidity range for using topo-clusters in combined signal mode"}; ///< Rapidity range for topo-clusters 
  ///@}

  ///@name Helpers
  ///@{
  static bool       makeDeepCopy(const xAOD::CaloCluster& rClus,xAOD::CaloClusterContainer* pClusCont) ; ///< Attaches a deep copy to container, returns @c true if successful. 
  //  bool       fillMoments(const xAOD::CaloCluster& rClus);                                          ///< Copies list of filled moments into lookup 
  bool       clusterFilter(const xAOD::CaloCluster& rClus) const;                                      ///< Filter topo-cluster
  bool       towerFilter(const xAOD::CaloCluster& rTowr) const;                                        ///< Filter topo-tower
  static StatusCode addContainerWriteHandle(whandle_t& signalHandle) ;                                   ///< Add a write handle for a container (in CaloClusterStoreHelper from r21.9)
  ///@}
};


inline bool CaloTopoClusterTowerMerger::clusterFilter(const xAOD::CaloCluster& rClus)   const { return std::abs(rClus.eta()) <= m_clusterRange; }
inline bool CaloTopoClusterTowerMerger::towerFilter(const xAOD::CaloCluster& /*rTowr*/) const { return true; }

///@class CaloTopoClusterTowerMerger
///
/// This algorithm merges objects from two @c xAOD::CaloClusterContainer. In the context of the
/// mixed topo-tower/topo-cluster output, the objects in the topo-cluster container are taken
/// up to a client-defined (symmetric) rapidity range. The rest of the phase space is then filled 
/// with the objects from the topo-tower container. It is assumed that the overlap resolution
/// is performed when the topo-tower container is filled. The mixed object container contains deep copies 
/// of the objects in the input containers.
/// 
#endif

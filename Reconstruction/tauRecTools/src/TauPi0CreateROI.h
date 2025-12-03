/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUPI0CREATEROI_H
#define TAURECTOOLS_TAUPI0CREATEROI_H

#include <string>
#include <vector>
#include <boost/dynamic_bitset.hpp>

#include "GaudiKernel/ToolHandle.h"
#include "tauRecTools/TauRecToolBase.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloEvent/CaloConstCellContainer.h"
#include "xAODTau/TauJet.h"

/**
 * @brief Find the cells used to create pi0 cluster
 * 
 * @author Will Davey <will.davey@cern.ch> 
 * @author Benedict Winter <benedict.tobias.winter@cern.ch> 
 * @author Stephanie Yuen <stephanie.yuen@cern.ch>
 */

class TauPi0CreateROI : public TauRecToolBase {

public:

  ASG_TOOL_CLASS2(TauPi0CreateROI, TauRecToolBase, ITauToolBase);
  
  TauPi0CreateROI(const std::string& name);
  virtual ~TauPi0CreateROI() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode executePi0CreateROI(xAOD::TauJet& pTau, CaloConstCellContainer& Pi0CellContainer, boost::dynamic_bitset<>& map) const override;

private:
  Gaudi::Property<bool> m_removeElectronCells {this, "RemoveElectronCells", false};
  Gaudi::Property<double> m_maxDeltaRTauCells {this, "MaxDeltaRTauCells", 0.4, "max DeltaR for cells-tau association"};

  SG::ReadHandleKey<CaloCellContainer>          m_caloCellInputContainer       {this,"Key_caloCellInputContainer",       "AllCalo",           "input calo cell container key"};
  SG::ReadCondHandleKey<CaloDetDescrManager>    m_caloMgrKey                   {this,"CaloDetDescrManager",              "CaloDetDescrManager"                               };
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_removedClusterInputContainer {this,"Key_RemovedClusterInputContainer", "",                   "input removed cluster key"   };
};
 

#endif	// TAURECTOOLS_TAUPI0CREATEROI_H

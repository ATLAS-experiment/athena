/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARDIGITIZATION_LArHitMerger_H
#define LARDIGITIZATION_LArHitMerger_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "Gaudi/Property.h"
#include <vector>
// Pile up
#include "PileUpTools/PileUpMergeSvc.h"


class LArEM_ID;
class LArHEC_ID;
class LArFCAL_ID;

class LArHitMerger : public AthAlgorithm
{
//
//
 public:
  LArHitMerger(const std::string& name, ISvcLocator* pSvcLocator);
  ~LArHitMerger() = default;
  virtual StatusCode initialize();
  virtual StatusCode execute();
  virtual StatusCode finalize();

 private:

  ServiceHandle<PileUpMergeSvc> m_mergeSvc{this, "PileUpMergeSvc", "PileUpMergeSvc"};

  std::vector<bool> m_SubDetFlag;
  std::vector <std::string> m_HitContainer; // hit container name list
  std::vector<int> m_CaloType;

  StringProperty m_SubDetectors{this, "SubDetectors", "LAr_All", "subdetector selection"};      // subdetectors
  StringProperty m_EmBarrelHitContainerName{this, "EmBarrelHitContainerName", "LArHitEMB", "Hit container name for EMB"};
  StringProperty m_EmEndCapHitContainerName{this, "EmEndCapHitContainerName", "LArHitEMEC", "Hit container name for EMEC"};
  StringProperty m_HecHitContainerName{this, "HecHitContainerName", "LArHitHEC", "Hit container name for HEC"};
  StringProperty m_ForWardHitContainerName{this, "ForWardHitContainerName", "LArHitFCAL", "Hit container name for FCAL"};

  const LArEM_ID*        m_larem_id{};
  const LArHEC_ID*       m_larhec_id{};
  const LArFCAL_ID*      m_larfcal_id{};
};

#endif

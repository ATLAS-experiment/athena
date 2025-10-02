/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARDIGITIZATION_LArHitFilter_H
#define LARDIGITIZATION_LArHitFilter_H

/**
 * @class LArHitFilter
 * @brief 
 *   algorithm to merge LArHit from different subevents from pileup service to a single
 *   event, assuming all subevents are at the same time and neglecting the individual hit times
 *   This is aimed to run at first stage for very large luminosity pileup
 */

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "Gaudi/Property.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "LArSimEvent/LArHitContainer.h"
#include <vector>

class LArEM_ID;
class LArHEC_ID;
class LArFCAL_ID;

class LArHitFilter : public AthReentrantAlgorithm
{
 public:
  LArHitFilter(const std::string& name, ISvcLocator* pSvcLocator);
  ~LArHitFilter() = default;
  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;

 private:
  StatusCode filterContainer(SG::ReadHandle<LArHitContainer>& inputContainer, SG::WriteHandle<LArHitContainer>& outputContainer, int ical) const;

  std::vector<bool> m_SubDetFlag;
  std::vector <std::string> m_HitContainer; // hit container name list
  std::vector<int> m_CaloType;

  const LArEM_ID*        m_larem_id{nullptr};
  const LArHEC_ID*       m_larhec_id{nullptr};
  const LArFCAL_ID*      m_larfcal_id{nullptr};

  double m_ecut[4][4]{};

  //Configurable Properties
  Gaudi::Property<std::string> m_SubDetectors{this, "SubDetectors", "LAr_All", "subdetector selection"};
  SG::ReadHandleKey<LArHitContainer>  m_inputEMBHitsKey{this, "EMBHitsInput", "LArHitEMBOLD"};
  SG::WriteHandleKey<LArHitContainer> m_outputEMBHitsKey{this, "EMBHitsOutput", "LArHitEMB"};
  SG::ReadHandleKey<LArHitContainer>  m_inputEMECHitsKey{this, "EMECHitsInput", "LArHitEMECOLD"};
  SG::WriteHandleKey<LArHitContainer> m_outputEMECHitsKey{this, "EMECHitsOutput", "LArHitEMEC"};
  SG::ReadHandleKey<LArHitContainer>  m_inputHECHitsKey{this, "HECHitsInput", "LArHitHECOLD"};
  SG::WriteHandleKey<LArHitContainer> m_outputHECHitsKey{this, "HECHitsOutput", "LArHitHEC"};
  SG::ReadHandleKey<LArHitContainer>  m_inputFCALHitsKey{this, "FCALHitsInput", "LArHitFCALOLD"};
  SG::WriteHandleKey<LArHitContainer> m_outputFCALHitsKey{this, "FCALHitsOutput", "LArHitFCAL"};
};

#endif

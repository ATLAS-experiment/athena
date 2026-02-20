/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#ifndef EVGENPRODTOOLS_COUNTHEPMC_H
#define EVGENPRODTOOLS_COUNTHEPMC_H

#include "GeneratorModules/GenBase.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "GaudiKernel/IIncidentListener.h"

#include <cstdint>

/**
 * @brief Count the number of events to pass all algorithms/filters
 *
 * CountHepMC counts the number of events to pass algs in the evgen
 * production chain. It signals to the AthenaEventLoopMgr service
 * when the requested number of events are produced.
 */

class CountHepMC : public GenBase,
                   public virtual IIncidentListener {
public:

  CountHepMC(const std::string& name, ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

  virtual void handle(const Incident& inc) override;

private:

  ServiceHandle< StoreGateSvc > m_metaDataStore{
    "StoreGateSvc/MetaDataStore", name()};
  uint64_t m_nPass{0};
  uint64_t m_nCount;

  uint64_t m_firstEv;
  uint32_t m_newRunNumber;

  bool m_corHepMC;
  bool m_corEvtID;
  bool m_corRunNumber;
  bool m_copyRunNumber;
  std::string m_inputKeyName;

  SG::ReadHandleKey<xAOD::EventInfo>  m_inputEvtInfoKey { this, "InputEventInfo",  "TMPEvtInfo", "ReadHandleKey for Input xAOD::EventInfo" };
  SG::WriteHandleKey<xAOD::EventInfo> m_outputEvtInfoKey{ this, "OutputEventInfo", "EventInfo", "WriteHandleKey for Output xAOD::EventInfo" };

  SG::ReadDecorHandleKey<xAOD::EventInfo> m_mcWeightsKey {this, "mcEventWeightsKey", m_inputEvtInfoKey, "mcEventWeights", "Decoration for MC Event Weights"};

};


#endif

#endif

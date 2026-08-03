/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTHIO_PRINTMC_H
#define TRUTHIO_PRINTMC_H

#include "GeneratorModules/GenBase.h"
#include "GeneratorModules/GenData.h"
#include "xAODEventInfo/EventInfo.h"

#include <memory>
#include <cstdint>
/// Print MC event details for a range of event numbers
class PrintMC : public GenBase {
public:

  PrintMC(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;

private:
  SG::ReadHandleKey<xAOD::EventInfo> m_evtInfoKey{this
      , "EventInfo"
      , "EventInfo"
      , "ReadHandleKey for xAOD::EventInfo" };
  
  std::string m_keyout;
  bool  m_VerboseOutput;
  std::string m_printsty;
  bool m_vertexinfo;
  uint64_t  m_firstEvt;
  uint64_t  m_lastEvt;
  bool m_trustHepMC;
  std::shared_ptr<GenData> m_gendata{nullptr};

};

#endif

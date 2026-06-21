/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTHIO_HEPMCREADFROMFILE_H
#define TRUTHIO_HEPMCREADFROMFILE_H

#include "GeneratorModules/GenBase.h"
#include "AtlasHepMC/IO_GenEvent.h"
#include <memory>

class StoreGateSvc;

class HepMCReadFromFile : public GenBase {
public:

  HepMCReadFromFile(const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;

private:

  std::string m_input_file;
  int m_event_number;
  double m_sum_xs;
  
  std::shared_ptr<HepMC3::Reader> m_hepmcio;
};

#endif

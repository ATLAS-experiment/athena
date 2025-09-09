/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTHIO_WRITEHEPMC_H
#define TRUTHIO_WRITEHEPMC_H

#include "GeneratorModules/GenBase.h"
#include "AtlasHepMC/IO_GenEvent.h"
#include <memory>


/// Write the MC event record to file in IO_GenEvent text format
class WriteHepMC : public GenBase {
public:

  WriteHepMC(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize();
  StatusCode execute();

  std::string m_outfile;
  int m_precision;
  std::string m_format;
  std::string m_units;

#ifdef HEPMC3
  std::unique_ptr<HepMC3::Writer> m_hepmcio;
  HepMC3::Units::MomentumUnit m_momentumunit;
  HepMC3::Units::LengthUnit m_lengthunit;
#else
  std::unique_ptr<HepMC::IO_GenEvent> m_hepmcio;
  HepMC3::Units::MomentumUnit m_momentumunit;
  HepMC3::Units::LengthUnit m_lengthunit;
#endif
};

#endif

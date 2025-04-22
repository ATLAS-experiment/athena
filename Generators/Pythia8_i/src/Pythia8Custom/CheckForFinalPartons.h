/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PYTHIA8_CHECK_FOR_FINAL_PARTONS_H
#define PYTHIA8_CHECK_FOR_FINAL_PARTONS_H

#include "Pythia8_i/IPythia8Custom.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "Pythia8/Pythia.h"

class CheckForFinalPartons: public extends<AthAlgTool, IPythia8Custom> {
  
  public:
  using base_class::base_class;

  StatusCode ModifyPythiaEvent(Pythia8::Pythia &pythia) const override;
  
  private:
  
  bool acceptEvent(Pythia8::Event &event) const;
  
  Gaudi::Property<int> m_maxFailures{this, "MaxFailures", 5};
  mutable int m_nFailures{0};
  
};

#endif

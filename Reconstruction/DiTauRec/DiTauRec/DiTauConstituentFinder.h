/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

#include "DiTauToolBase.h"
#include "GaudiKernel/ToolHandle.h"


class DiTauConstituentFinder : public DiTauToolBase {
 public:

  DiTauConstituentFinder(const std::string& type,
	     const std::string& name,
	     const IInterface * parent);

  virtual ~DiTauConstituentFinder();

  virtual StatusCode initialize() override;

  virtual StatusCode execute(DiTauCandidateData * data,
			     const EventContext& ctx) const override;


 private:
  float m_Rsubjet;

};


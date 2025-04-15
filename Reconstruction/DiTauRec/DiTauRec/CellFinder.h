/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUREC_CELLFINDER_H
#define DITAUREC_CELLFINDER_H

#include "DiTauToolBase.h"

#include "AsgTools/PropertyWrapper.h"

#include "GaudiKernel/ToolHandle.h"


class CellFinder : public DiTauToolBase {
 public:

  CellFinder(const std::string& type,
	     const std::string& name,
	     const IInterface * parent);

  virtual ~CellFinder();

  virtual StatusCode initialize() override;

  virtual StatusCode execute(DiTauCandidateData * data,
			     const EventContext& ctx) const override;


 private:

  Gaudi::Property<float> m_Rsubjet{this, "Rsubjet", 0.2}; 

};

#endif  // DITAUREC_CELLFINDER_H


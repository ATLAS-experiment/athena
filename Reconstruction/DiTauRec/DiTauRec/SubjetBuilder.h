/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUREC_SUBJETBUILDER_H
#define DITAUREC_SUBJETBUILDER_H

#include "DiTauToolBase.h"

#include "AsgTools/PropertyWrapper.h"

#include "fastjet/tools/Filter.hh"

class SubjetBuilder : public DiTauToolBase {
 public:

  //-------------------------------------------------------------
  //! Constructor
  //-------------------------------------------------------------
  SubjetBuilder(const std::string& type,
		const std::string& name,
		const IInterface * parent);

  //-------------------------------------------------------------
  //! Destructor
  //-------------------------------------------------------------
  virtual ~SubjetBuilder();

  virtual StatusCode initialize() override;

  virtual StatusCode execute(DiTauCandidateData * data,
			     const EventContext& ctx) const override;


 private:

  Gaudi::Property<float> m_Rsubjet{this, "Rsubjet", 0.2};
  Gaudi::Property<float> m_ptmin{this, "ptminsubjet", 10000};

};

#endif  // DITAUREC_SUBJETBUILDER_H

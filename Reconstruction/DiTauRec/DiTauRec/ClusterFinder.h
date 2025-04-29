/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DITAUREC_CLUSTERFINDER_H
#define DITAUREC_CLUSTERFINDER_H

#include "DiTauToolBase.h"

#include "AsgTools/PropertyWrapper.h"

#include "GaudiKernel/ToolHandle.h"


class ClusterFinder : public DiTauToolBase {
 public:

  ClusterFinder(const std::string& type,
	     const std::string& name,
	     const IInterface * parent);

  virtual ~ClusterFinder();

  virtual StatusCode initialize() override;

  virtual StatusCode execute(DiTauCandidateData * data,
			     const EventContext& ctx) const override;


 private:

  Gaudi::Property<float> m_Rsubjet{this, "Rsubjet", 0.2};

};

#endif  // DITAUREC_CLUSTERFINDER_H


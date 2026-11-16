/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBPRESCALERSTREAMERTOOL_H
#define TBREC_TBPRESCALERSTREAMERTOOL_H

#include "TBEventStreamerTool.h"

// This is needed for migration to CLHEP 1.9
// (Forward declarations of CLHEP classes will give rise to an error...)
// instead of ...
//class RanluxEngine;
 // use this:
#include "CLHEP/Random/RanluxEngine.h"

#include <string>
#include <map>
#include <vector>

class TBPreScalerStreamerTool : public TBEventStreamerTool
{
 public:

  TBPreScalerStreamerTool(const std::string& type,
			  const std::string& name,
			  const IInterface* parent);
  virtual ~TBPreScalerStreamerTool();

  virtual StatusCode initializeTool() override;

  virtual StatusCode accept() override;

 protected:

  ////////////////
  // Properties //
  ////////////////

  Gaudi::Property<double> m_acceptFraction{this, "AcceptEventFraction", 1., "fraction of accepted events"};

  /////////////////////
  // Internal Stores //
  /////////////////////

  CLHEP::RanluxEngine* m_randomizer{nullptr};

  //////////////////////
  // Internal Helpers //
  //////////////////////

};
#endif

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FORWARDTRANSPORT_FORWARD_TRANSPORT_MODEL_TOOL_H
#define FORWARDTRANSPORT_FORWARD_TRANSPORT_MODEL_TOOL_H

#include "G4AtlasTools/FastSimulationBase.h"

class G4VFastSimulationModel;

class ForwardTransportModelTool: public FastSimulationBase {

 public:

  using FastSimulationBase::FastSimulationBase;

protected:
  /** Method to make the actual fast simulation model itself, which
   will be owned by the tool.  Must be implemented in all concrete
   base classes. */
  virtual G4VFastSimulationModel* makeFastSimModel() override final;

private:
  Gaudi::Property<std::string> m_FwdTrSvcName{this, "ForwardTransportSvcName", "ForwardTransportSvc", ""};

};

#endif// FORWARDTRANSPORT_FORWARD_TRANSPORT_MODEL_TOOL_H

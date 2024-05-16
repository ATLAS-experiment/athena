/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FOWARDTRANSPORTSVC_FORWARDTRANSPORTSVC_H
#define FOWARDTRANSPORTSVC_FORWARDTRANSPORTSVC_H

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ToolHandle.h"
#include "ForwardTransportSvc/IForwardTransportSvc.h"

class ForwardTransportSvc: public extends<AthService, IForwardTransportSvc> {

 public:
  ForwardTransportSvc(const std::string& name, ISvcLocator* svc);

  virtual StatusCode initialize();

  virtual ForwardTracker::ConfigData getConfigData() const { return m_cData; }
  virtual bool getTransportFlag() const { return m_transportFlag; }
  virtual double getEtaCut() const { return m_etaCut; }
  virtual double getXiCut() const { return m_xiCut; }
  virtual bool selectedParticle(G4ThreeVector mom, int pid);

private:

  ForwardTracker::ConfigData m_cData;
  BooleanProperty m_transportFlag{this, "TransportFlag", false};     // Transport only neutrals (0) or charged (1)
  DoubleProperty m_etaCut{this, "EtaCut", 7.5};  // Minimum pseudorapidty of transported particles
  DoubleProperty m_xiCut{this, "XiCut", 0.8};  // Minimum momentum over beam energy of transported particles

};

#endif // FOWARDTRANSPORTSVC_FORWARDTRANSPORTSVC_H

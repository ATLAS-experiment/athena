/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FOWARDTRANSPORTSVC_FORWARDTRANSPORTSVC_H
#define FOWARDTRANSPORTSVC_FORWARDTRANSPORTSVC_H

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ToolHandle.h"
#include "ForwardTransportSvc/IForwardTransportSvc.h"

class ForwardTransportSvc: public extends<AthService, IForwardTransportSvc> {

 public:
  using base_class::base_class;

  virtual StatusCode initialize();

  virtual ForwardTracker::ConfigData getConfigData() const { return m_cData; }
  virtual bool getTransportFlag() const { return m_transportFlag; }
  virtual double getEtaCut() const { return m_etaCut; }
  virtual double getXiCut() const { return m_xiCut; }
  virtual bool selectedParticle(G4ThreeVector mom, int pid);

private:

  ForwardTracker::ConfigData m_cData;  // filled from the properties below in initialize()
  Gaudi::Property<std::string> m_twissFile1{this, "TwissFile1", m_cData.twissFile1};
  Gaudi::Property<std::string> m_twissFile2{this, "TwissFile2", m_cData.twissFile2};
  DoubleProperty m_positionC1{this, "PositionC1", m_cData.positionC1};
  DoubleProperty m_positionC2{this, "PositionC2", m_cData.positionC2};
  DoubleProperty m_apertureC1{this, "ApertureC1", m_cData.apertureC1};
  DoubleProperty m_apertureC2{this, "ApertureC2", m_cData.apertureC2};
  DoubleProperty m_endMarker{this, "EndMarker", m_cData.endMarker};
  BooleanProperty m_transportFlag{this, "TransportFlag", false};     // Transport only neutrals (0) or charged (1)
  DoubleProperty m_etaCut{this, "EtaCut", 7.5};  // Minimum pseudorapidty of transported particles
  DoubleProperty m_xiCut{this, "XiCut", 0.8};  // Minimum momentum over beam energy of transported particles

};

#endif // FOWARDTRANSPORTSVC_FORWARDTRANSPORTSVC_H

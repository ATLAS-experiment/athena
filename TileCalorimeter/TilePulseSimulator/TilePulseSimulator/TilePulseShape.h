/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEPULSESIMULATOR_TILEPULSESHAPE_H
#define TILEPULSESIMULATOR_TILEPULSESHAPE_H

#include "AthenaBaseComps/AthMessaging.h"
#include "TString.h"
#include <vector>

class TF1;
class TGraph;
class TSpline;
class IMessageSvc;

class TilePulseShape : public AthMessaging {
 public:
  TilePulseShape(IMessageSvc* msgSvc, const std::string& name);
  TilePulseShape(IMessageSvc* msgSvc, const std::string& name, const TString& fileName);
  TilePulseShape(IMessageSvc* msgSvc, const std::string& name, const std::vector<double>& shapevec);
  virtual ~TilePulseShape();
  void loadPulseShape(const TString& fileName);
  void setPulseShape(const std::vector<double>& shapevec);
  void setUndershootParameters(double amplitude, double decayTime);
  double undershootAmplitude() const { return m_undershootAmplitude; }
  double undershootDecayTime() const { return m_undershootDecayTime; }
  double effectiveStartTime() const;
  double effectiveEndTime() const;
  
  //=== access to the underlying graph
  TGraph* getGraph(double t0=0., double ped=0., double amp=1.);
  double  eval(double x, bool useSpline=true, bool useUndershoot=false);

  //=== modify the pulseshape
  void resetDeformation();
  int scalePulse(double leftSF=1., double rightSF=1.);
  
 private:
  TGraph* m_pulseShape{nullptr};
  TGraph* m_deformedShape{nullptr};
  TSpline* m_deformedSpline{nullptr};
  bool m_useConfiguredUndershoot{false};
  double m_undershootAmplitude{};
  double m_undershootDecayTime{};
};

#endif // TILEPULSESIMULATOR_TILEPULSESHAPE_H

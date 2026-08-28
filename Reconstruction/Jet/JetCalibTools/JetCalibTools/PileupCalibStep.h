/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETCALIBTOOLS_PILEUPCALIBSTEP_H
#define JETCALIBTOOLS_PILEUPCALIBSTEP_H 1

/* PileupAreaResidualCalibStep performs the first step of the jet calibration
 *
 *  - area subtraction in the form  pT_corr = pT - rho x pT_area 
 *  - residual correction (1D version)
 *
 */

#include "xAODTracking/VertexContainer.h"
#include "xAODEventShape/EventShape.h"
#include "xAODEventInfo/EventInfo.h"

#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgTools/PropertyWrapper.h"
#include "JetAnalysisInterfaces/IJetCalibStep.h"
#include "JetAnalysisInterfaces/IVarTool.h"

class PileupCalibStep   : public asg::AsgTool,
			  virtual public IJetCalibStep
{

  ASG_TOOL_CLASS(PileupCalibStep, IJetCalibStep) 

 public:
  PileupCalibStep(const std::string& name="PileupCalibStep");

  virtual StatusCode initialize() override;

  // Apply calibration to jet
  virtual StatusCode calibrate(xAOD::JetContainer& jetCont) const override;

 private:

  double getResidualOffset(const xAOD::Jet& jet, const JetHelper::JetContext& jc, double mu, double NPV) const;

  Gaudi::Property<bool> m_isData{this, "IsData", false, ""};
  Gaudi::Property<bool> m_doJetArea{this, "DoJetArea", false, "should the area correction be applied?"};
  Gaudi::Property<bool> m_doResidual{this, "DoResidual", true, "should the residual correction be applied?"};

  Gaudi::Property<bool> m_doSequentialResidual{this, "DoSequentialResidual", false, "apply residual correction sequentially"};
  Gaudi::Property<bool> m_doMuOnly{this, "ApplyOnlyMuResidual", false, "only mu residual correction will be applied"};
  Gaudi::Property<bool> m_doNPVOnly{this, "ApplyOnlyNPVResidual", false, "only NPV residual correction will be applied"};
  
  /// Event properties
  SG::ReadHandleKey<xAOD::EventShape> m_rhoKey{this, "RhoKey", "auto"};
  SG::ReadHandleKey<xAOD::VertexContainer> m_pvKey{this, "VertexContainer", "PrimaryVertices"};
  SG::ReadDecorHandleKey<xAOD::EventInfo> m_muKey {this, "averageInteractionsPerCrossingKey",
    "EventInfo.averageInteractionsPerCrossing","Decoration for Average Interaction Per Crossing"};

  /// Histograms with PU residual correction factors
  ToolHandle<JetHelper::IVarTool> m_histTool_mu = {this , "histTool_mu", "HistoInput1D", "mu histo reader" };
  ToolHandle<JetHelper::IVarTool> m_histTool_NPV = {this , "histTool_NPV", "HistoInput1D", "npv histo reader" };

  /// Properties
  Gaudi::Property<float> m_NPV_ref{this , "DefaultNPVRef", -99., ""};
  Gaudi::Property<float> m_mu_ref{this , "DefaultMuRef", -99., ""};
  Gaudi::Property<float> m_muSF{this , "MuScaleFactor", 1., ""};

  /// In and out scales
  Gaudi::Property<std::string> m_jetInScale {this, "InScale", "JetConstitScaleMomentum", "Starting jet scale" };
  Gaudi::Property<std::string> m_jetOutScale {this, "OutScale", "JetPileupScaleMomentum", "Ending jet scale" };
  Gaudi::Property<std::string> m_jetAreaOutScale {this, "OutScaleJetArea", "JetRhoAreaScaleMomentum", "Intermediate jet scale"};

  static constexpr float m_GeV = 1000;
  
};

#endif

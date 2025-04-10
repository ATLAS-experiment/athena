///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// InSituCalibStep.h 
// Header file for class InSituCalibStep
// Author: Fabrice Balli <fabrice.balli@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef JETCALIBTOOLS_INSITUCALIBSTEP_H
#define JETCALIBTOOLS_INSITUCALIBSTEP_H 1

#include <string.h>

#include <TString.h>
#include <TEnv.h>

#include "AsgTools/AsgTool.h"
#include "AsgTools/AsgToolMacros.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"

#include "xAODEventInfo/EventInfo.h"

#include "JetAnalysisInterfaces/IJetCalibTool.h"
#include "JetAnalysisInterfaces/IJetCalibStep.h"
#include "JetAnalysisInterfaces/IVarTool.h"
#include "JetToolHelpers/InputVariable.h"
#include "JetToolHelpers/HistoInputBase.h"

class InSituCalibStep
  : public asg::AsgTool,
    virtual public IJetCalibStep {
   ASG_TOOL_CLASS(InSituCalibStep, IJetCalibStep)

public:
  /// Constructor with parameters: 
  InSituCalibStep(const std::string& name = "InSituCalibStep");

  virtual StatusCode initialize() override;
  virtual StatusCode calibrate(xAOD::JetContainer&) const override;


private:
 
  Gaudi::Property<bool> m_CalibrateMC {this, "CalibrateMC", false, "force Insitu step for MC sample"};
  Gaudi::Property<bool> m_isMC {this, "isMC", false, "isMC"};
 
  Gaudi::Property<std::string> m_jetStartScale {this, "InSituStartingScale", "JetGSCScaleMomentum", "Starting jet scale"};
  Gaudi::Property<std::string> m_jetOutScale {this, "InSituOutScale", "JetInsituScaleMomentum", "Ending jet scale"};
  // retrieves in situ correction
  StatusCode getInsituCorr(const xAOD::Jet& jet,  JetHelper::JetContext& jc, unsigned int periodIndex, double &scale) const;
  // Relative calibration (derived with eta intercalibration)
  //ToolHandle<JetHelper::IVarTool> m_histTool_EtaInter{this, "HistoReaderEtaInter", "HistoInput2D", "Instance of HistoInput2D for reading histogram"};
  ToolHandleArray<JetHelper::IVarTool> m_histTool_EtaInter{this, "HistoReaderEtaInter", {}, "Instance of HistoInput2D for reading histogram"};
  // Absolute calibration (derived with |eta| < 0.8)
  //ToolHandle<JetHelper::IVarTool> m_histTool_Abs{this, "HistoReaderAbs", "HistoInput1D", "Instance of HistoInput1D for reading histogram"};
  ToolHandleArray<JetHelper::IVarTool> m_histTool_Abs{this, "HistoReaderAbs", {}, "Instance of HistoInput1D for reading histogram"};
  // needed for the combined histogram: pT
  ToolHandle<JetHelper::IVarTool> m_vartool1 {this, "vartool1", "VarTool", "InputVariable instance" };
  // needed for the combined histogram: eta
  ToolHandle<JetHelper::IVarTool> m_vartool2 {this, "vartool2", "VarTool", "InputVariable instance" };
  // vector of run numbers
  Gaudi::Property<std::vector<unsigned int> > m_RunNumBoundaries{this, "RunNumbers", {} ,""};
  // combine Relative and Absolute calibrations
  std::unique_ptr<const TH2> combineCalibration(const TH2* h2d, const TH1* h);
  // combined in situ correction histogram
  std::vector< std::unique_ptr<const TH2> > m_insituCorr_vec;
  // maximum eta of combined histogram
  std::vector<double> m_etaMax_vec;
  // minimum eta of combined histogram
  std::vector<double> m_etaMin_vec;
  // maximum pt of combined histogram
  std::vector<double> m_ptMax_vec;
  // minimum pt of combined histogram
  std::vector<double> m_ptMin_vec;
  //ReadHandleKey for event info
  SG::ReadHandleKey<xAOD::EventInfo> m_evtInfoKey{this, "EventInfoKey", "EventInfo"};
  // Method to get runNumber
  StatusCode retrieveEventInfo(unsigned int &r) const;


};
#endif

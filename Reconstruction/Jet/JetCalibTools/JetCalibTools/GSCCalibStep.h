///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// GSCCalibStep.h 
// Header file for class GSCCalibStep
/////////////////////////////////////////////////////////////////// 
#ifndef JETCALIBTOOLS_GSCCALIBSTEP_H
#define JETCALIBTOOLS_GSCCALIBSTEP_H 1

#include <string.h>

#include <TString.h>
#include <TEnv.h>

#include "AsgTools/AsgTool.h"
#include "AsgTools/AsgToolMacros.h"
#include "AsgTools/ToolHandle.h"
#include <AsgTools/PropertyWrapper.h>

#include "xAODEventInfo/EventInfo.h"

#include "JetAnalysisInterfaces/IJetCalibTool.h"
#include "JetAnalysisInterfaces/IJetCalibStep.h"
#include "JetAnalysisInterfaces/IVarTool.h"
#include "JetToolHelpers/InputVariable.h"

class GSCCalibStep
  : public asg::AsgTool,
    virtual public IJetCalibStep {

  ASG_TOOL_CLASS(GSCCalibStep, IJetCalibStep)

public:
  /// Constructor with parameters: 
  GSCCalibStep(const std::string& name = "GSCCalibStep");

  virtual StatusCode initialize() override;
  virtual StatusCode calibrate(xAOD::JetContainer&) const override;

private:
  Gaudi::Property<std::string> m_jetInScale {this, "InScale", "JetEtaJESScaleMomentum", "Starting jet scale"};
  Gaudi::Property<std::string> m_jetOutScale {this, "OutScale", "JetGSCScaleMomentum", "Ending jet scale"};

  ToolHandleArray<JetHelper::IVarTool> m_histTool_EM3 = {this , "histTool_EM3", {}, "EM3 histo reader" };
  ToolHandleArray<JetHelper::IVarTool> m_histTool_ChargedFraction = {this , "histTool_CharFrac", {}, "ChargedFraction histo reader" };
  ToolHandleArray<JetHelper::IVarTool> m_histTool_Tile0 = {this , "histTool_Tile0", {}, "Tile0 histo reader" };
  ToolHandleArray<JetHelper::IVarTool> m_histTool_PunchThrough = {this , "histTool_PunchThrough", {}, "PunchThrough histo reader" };
  ToolHandleArray<JetHelper::IVarTool> m_histTool_nTrk = {this , "histTool_nTrk", {}, "nTrk histo reader" };
  ToolHandleArray<JetHelper::IVarTool> m_histTool_trackWIDTH = {this , "histTool_trackWIDTH", {}, "trackWIDTH histo reader" };


  float getChargedFractionResponse(const xAOD::Jet& jet, const JetHelper::JetContext& jc, uint etabin) const;
  float getTile0Response(const xAOD::Jet& jet, const JetHelper::JetContext& jc, uint etabin) const;
  float getEM3Response(const xAOD::Jet& jet, const JetHelper::JetContext& jc, uint etabin) const;
  float getPunchThroughResponse(const xAOD::Jet& jet, const JetHelper::JetContext& jc, double eta_det) const;
  float getNTrkResponse(const xAOD::Jet& jet, const JetHelper::JetContext& jc, uint etabin) const;
  float getTrackWIDTHResponse(const xAOD::Jet& jet, const JetHelper::JetContext& jc, uint etabin) const;

}; 

#endif

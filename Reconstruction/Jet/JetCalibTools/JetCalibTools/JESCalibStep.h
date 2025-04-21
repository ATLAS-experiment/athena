///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// EtaMassJESCalibStep.h 
// Header file for class EtaMassJESCalibStep
// Author: Max Swiatlowski <mswiatlo@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef JETCALIBTOOLS_JESCALIBSTEP_H
#define JETCALIBTOOLS_JESCALIBSTEP_H 1

#include <string.h>

#include <TString.h>
#include <TEnv.h>

#include "AsgTools/AsgTool.h"
#include "AsgTools/AsgToolMacros.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/PropertyWrapper.h"

#include "xAODEventInfo/EventInfo.h"

#include "JetAnalysisInterfaces/IJetCalibTool.h"
#include "JetAnalysisInterfaces/IJetCalibStep.h"

#include "JetAnalysisInterfaces/IVarTool.h"

class EtaMassJESCalibStep
  : public asg::AsgTool,
    virtual public IJetCalibStep {

  ASG_TOOL_CLASS(EtaMassJESCalibStep, IJetCalibStep)

  public:
  EtaMassJESCalibStep(const std::string& name = "EtaMassJESCalibStep");

  virtual StatusCode initialize() override;
  virtual StatusCode calibrate(xAOD::JetContainer&) const override;

private:
  // support functions to extract information from text files
  double getLogPolN(const double *factors, double x) const;
  double getLogPolNSlope(const double *factors, double x) const ;
  int getEtaBin(double eta_det) const; 
  bool readMCJESFromText() ;
  bool readMCJESFromHists() ;
  void loadSplineHists(const std::string & fileName, const std::string &etajes_name) ;  

  /// return MCJES calibration factor
  double getJES(const double X, const double Y=0, const double Emax=-1) const;
  /// return Eta correction
  double getEtaCorr( double X,  double Y=0) const;
  /// return Emax
  double getEmaxJES(const double Y) const;
  /// deal with low pt jets
  double getLowPtJES(double E_uncorr, double eta_det) const ;

  double getSplineCorr(const int etaBin, double E) const;
  double getSplineSlope(const int ieta, const double minE) const;
  
  /// name of the text file
  Gaudi::Property< std::string > m_constantFileName { this, "CalibConstantFile", "/afs/cern.ch/work/s/stapiaar/JetDev4/athena/JetToolHelpers/data/file_JES.config", "text file containing constants" };
  /// jet collection to be calibrated
  Gaudi::Property< std::string > m_jetAlgo { this, "JetAlgo", "AntiKt4EMPFlow", "jet collection" };
  Gaudi::Property< float >  m_minPt_JES = {this, "MinPtForETAJES",15, "min pT"};
  Gaudi::Property< bool >  m_freezeJESatHighE = {this, "FreezeJEScorrectionatHighE",false, " freeze at high e"};
  Gaudi::Property< float > m_lowPtExtrap = {this, "LowPtJESExtrapolationMethod", 0, " low pt etrap"};
  Gaudi::Property< float > m_lowPtMinR = {this, "LowPtJESExtrapolationMinimumResponse", 0.25, " low pt etrap min"};

  Gaudi::Property< float > m_minPt_EtaCorr = {this, "MinPtForEtaCorr" ,8. , ""};
  Gaudi::Property< float > m_maxE_EtaCorr = {this, "MaxEForEtaCorr" ,2500. , ""};
  
  Gaudi::Property< std::string > m_histoFileName { this, "HistoFile", "none", "root file containing histos for spline calib" };
  Gaudi::Property< bool >  m_useSpline = {this, "UseSpline",false, " use spline"};
  
  
  ToolHandle<JetHelper::IVarTool> m_vartoolE{this, "VarToolE", "VarTool", "InputVariable instance E (or pT?)" };
  /// interface for xAOD::jet variable to be defined by user, this must correspond to jet Eta in currect version of jet calibration files
  ToolHandle<JetHelper::IVarTool> m_vartoolEta{this, "VarToolEta", "VarTool", "InputVariable instance eta (or rapididty?)" };

  // protected:
  unsigned int m_nPar{}; // number of parameters in config file
  const static unsigned int s_nEtaBins = 90;
  const static unsigned int s_nParMax = 9; 
  double m_JESFactors[s_nEtaBins][s_nParMax]{};
  double m_etaCorrFactors[s_nEtaBins][s_nParMax]{};
  double m_energyFreezeJES[s_nEtaBins]{};
  double m_JES_MinPt_Slopes[s_nEtaBins]={};
  double m_JES_MinPt_E[s_nEtaBins]={};
  double m_JES_MinPt_R[s_nEtaBins]={};


  TAxis * m_etaBinAxis{};

  std::vector<std::unique_ptr<TH1> > m_etajesFactors;

  
};

#endif


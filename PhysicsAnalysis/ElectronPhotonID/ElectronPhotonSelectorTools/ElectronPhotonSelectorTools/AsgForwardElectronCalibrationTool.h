/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASGFORWARDELECTRONCALIBRATIONTOOL_H
#define ASGFORWARDELECTRONCALIBRATIONTOOL_H

/**
    @class AsgForwardElectronCalibrationTool
    @brief Tool to apply DNN-based pT calibration to forward electrons for Run 4 using ITk and HGTD variables.
    Internal Note: https://cds.cern.ch/record/2922184

    Workflow:
      1. Extract 25 input variables from the forward electron container
      2. Run DNN via lwtnn (MinMax scaler already into JSON as a batch norm layer)
      3. Apply softplus manually (not supported natively by lwtnn)
      4. Undo MinMax pT scaling -> calibrated pT in MeV

    @author Mathis Dubau (LAPP)
    @date   Feb 2026

    Based on ElectronDNNCalculator and AsgForwardElectronLikelihoodTool

*/


#include "AsgTools/PropertyWrapper.h"
#include "AsgTools/AsgTool.h"
#include "xAODEgamma/ElectronFwd.h"

#include "EgammaAnalysisInterfaces/IForwardElectronCalib.h"

#include "lwtnn/LightweightGraph.hh"

#include <vector>
#include <string>
#include <memory>


class EventContext;


class AsgForwardElectronCalibrationTool 
  : public asg::AsgTool
  , virtual public IForwardElectronCalib
{
  ASG_TOOL_CLASS1(AsgForwardElectronCalibrationTool,
		  IForwardElectronCalib)



public:

  /** Standard constructor */
  AsgForwardElectronCalibrationTool(const std::string& myname);
  /** Standard destructor */
  virtual ~AsgForwardElectronCalibrationTool();


  /** Gaudi Service Interface method implementations */
  virtual StatusCode initialize() override;


  /** Return the DNN-calibrated pT in MeV
      Returns -999 on error. */
  double calibrate(const EventContext& ctx,
                   const xAOD::Electron* eg) const override;





private:

  /** Select the eta bin index for |eta|, three eta bins:
        Bin 0: 2.5 < |eta| <= 2.7
        Bin 1: 2.7 < |eta| <= 3.2
        Bin 2: 3.2 < |eta| <= 4.0
      Returns -1 if out of range. */
  static int getEtaBin(double absEta) ;

  /** Get 25 input variables
      Returns false on failure. */
  bool getInputs(const xAOD::Electron* eg,
                 std::vector<float>& inputs) const;

  /** Sftplus: log(1 + exp(x)) */
  static double softplus(double x) ;

  /** Undo MinMax pT scaling used during training */
  double unscalePt(double x) const;



  /** pT scaling range used [MeV] */
  Gaudi::Property<double> m_pTMin {this,"pTMin", 10000, "Lower bound of pT Min scaling [MeV]"};
  Gaudi::Property<double> m_pTMax {this,"pTMax", 255000, "Lower bound of pT Max scaling [MeV]"};

  /** One lwtnn JSON file / DNN  per eta bin */
  Gaudi::Property<std::vector<std::string>> m_modelFiles {this,"ModelFiles",{"","",""} , "lwtnn JSON files, one per eta bin (in eta order)"};
  std::vector<std::unique_ptr<lwt::LightweightGraph>> m_graphs;

  /** Input variable names */
  std::vector<std::string> m_variables;

};

#endif

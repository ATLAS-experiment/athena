/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASGFORWARDELECTRONSELECTORTOOL_H
#define ASGFORWARDELECTRONSELECTORTOOL_H

/**
    @class AsgForwardElectronSelectorTool
    @brief Tool to apply DNN-based ID to forward electrons for Run 4 using ITk and HGTD variables.
    Internal Note: https://cds.cern.ch/record/2922175

    Workflow:
      1. Call AsgForwardElectronCalibrationTool to get the calibrated pT in this forward region
      2. Apply Linear Regression to decorrelate the 7 shower shape moments from pT
      3. Run DNN via lwtnn (MinMax scaler already into JSON as a batch norm layer)
      4. Apply pT- and eta-dependent working point cut on the DNN score

    @author Mathis Dubau (LAPP)
    @date   Feb 2026

    Based on ElectronDNNCalculator and AsgForwardElectronLikelihoodTool

*/


#include "AsgTools/AsgTool.h"
#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"
#include "ElectronPhotonSelectorTools/AsgForwardElectronCalibrationTool.h"
#include "PATCore/AcceptInfo.h"
#include "PATCore/AcceptData.h"
#include "xAODEgamma/ElectronFwd.h"

#include "lwtnn/LightweightGraph.hh"

#include <vector>
#include <string>
#include <memory>


class EventContext;


class AsgForwardElectronSelectorTool
  : public asg::AsgTool
  , virtual public IAsgElectronLikelihoodTool
{
  ASG_TOOL_CLASS2(AsgForwardElectronSelectorTool,
                  IAsgElectronLikelihoodTool,
                  IAsgSelectionTool)



public:

  /** Standard constructor */
  AsgForwardElectronSelectorTool(const std::string& myname);
  /** Standard destructor */
  virtual ~AsgForwardElectronSelectorTool();


  /** Gaudi Service Interface method implementations */
  virtual StatusCode initialize() override final;


  /** Method to get the plain AcceptInfo.
      This is needed so that one can already get the AcceptInfo
      and query what cuts are defined before the first object
      is passed to the tool. */
  virtual const asg::AcceptInfo& getAcceptInfo() const override;




  /** IAsgSelectionTool interface */

  /** The main accept method: using the generic interface */
  virtual asg::AcceptData accept(const xAOD::IParticle* part) const override;
  virtual asg::AcceptData accept(const EventContext& ctx,
                                 const xAOD::IParticle* part) const override;

  /** The main accept method: the actual cuts are applied here */
  virtual asg::AcceptData accept(const EventContext& ctx,
                                 const xAOD::Electron* eg) const override;
  virtual asg::AcceptData accept(const EventContext& ctx,
                                 const xAOD::Electron* eg,
                                 double mu) const override;
  virtual asg::AcceptData accept(const EventContext& ctx,
                                 const xAOD::Egamma* eg) const override;
  virtual asg::AcceptData accept(const EventContext& ctx,
                                 const xAOD::Egamma* eg,
                                 double mu) const override;

  /** The main result method: the actual DNN is calculated here */
  virtual double calculate(const EventContext& ctx,
                           const xAOD::IParticle* part) const override;
  virtual double calculate(const EventContext& ctx,
                           const xAOD::Electron* eg) const override;
  virtual double calculate(const EventContext& ctx,
                           const xAOD::Electron* eg,
                           double mu) const override;
  virtual double calculate(const EventContext& ctx,
                           const xAOD::Egamma* eg) const override;
  virtual double calculate(const EventContext& ctx,
                           const xAOD::Egamma* eg,
                           double mu) const override;



  /** The result method for multiple outputs:
      only one output is used so return vector of that output */
  virtual std::vector<float> calculateMultipleOutputs(
      const EventContext& ctx,
      const xAOD::Electron* eg,
      double mu = -99) const override;


  virtual std::string getOperatingPointName() const override;





  private:

  /** Select the eta bin index for |eta|, three eta bins:
        Bin 0: 2.5 < |eta| <= 2.7
        Bin 1: 2.7 < |eta| <= 3.2
        Bin 2: 3.2 < |eta| <= 4.0
      Returns -1 if out of range. */
  int getEtaBin(double absEta) const;

  /** Select the pT bin index, 
        Bin 0: 5 GeV <= pT < 15 GeV
        Bin 1: 15 GeV <= pT < 20 GeV
        Bin 2: 20 GeV <= pT < 25 GeV
        Bin 3: 25 GeV <= pT < 30 GeV
        Bin 4: 30 GeV <= pT < 35 GeV
        Bin 5: 35 GeV <= pT < 40 GeV
        Bin 6: 40 GeV <= pT < 45 GeV
        Bin 7: 45 GeV <= pT < 50 GeV
        Bin 8: 50 GeV <= pT < 60 GeV
        Bin 9: 60 GeV <= pT < 80 GeV
        Bin 10: 80 GeV <= pT < 150 GeV
        Bin 11: 150 GeV <= pT < 500 GeV
      Returns -1 if outside valid range. */
  int getPtBin(double calibPt) const;

  /** Get 25 input variables
      Applies LR decorrelation
      Returns false on failure. */
  bool getInputs(const xAOD::Electron* eg,
                 double calibPt,
                 int etaBin,
                 std::vector<double>& inputs) const;

  /** Runs calibration + LR + DNN in a single pass.
      Returns false on error; on success fills score and calibPt. */
  bool calculateWithCalibPt(const EventContext& ctx,
                            const xAOD::Electron* eg,
                            double& score,
                            double& calibPt) const;

  /** One lwtnn JSON file / DNN  per eta bin */
  std::vector<std::string> m_modelFiles;
  std::vector<std::unique_ptr<lwt::LightweightGraph>> m_graphs;

  /** Input variable names */
  std::vector<std::string> m_variables;

  /** Working point: Loose = 90% | Medium = 80% | Tight = 70% */
  std::string m_workingPoint;

  /** Handle to the calibration tool */
  ToolHandle<AsgForwardElectronCalibrationTool> m_calibTool{
      this, "CalibrationTool", "",
      "Handle to the AsgForwardElectronCalibrationTool"};

  /** Working point index: 
        0 = Loose
        1 = Medium
        2 = Tight */
  int m_wpIndex{-1};

  /** AcceptInfo: defines the cut structure */
  asg::AcceptInfo m_acceptInfo;

};

#endif 
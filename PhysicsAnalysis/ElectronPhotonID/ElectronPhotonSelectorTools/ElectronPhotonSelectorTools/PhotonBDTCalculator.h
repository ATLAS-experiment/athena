/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
  @class PhotonBDTCalculator
  @brief Tool to compute and optionally decorate a photon identification BDT score.

  This tool computes the set of input variables required by the photon-ID BDT
  for a given xAOD::Photon, chooses the appropriate BDT model depending on the
  conversion status (converted vs unconverted), and evaluates the score via
  PhotonSingleBDTCalculator sub-tools.

  The score can be written as a decoration on the photon (configurable
  decoration name). If configured, the tool can also re-compute the score even
  if the decoration already exists.

  The tool is intended to be used as a building block by selection tools
  (e.g. AsgPhotonBDTSelector) but can also be called at analysis-level 

  @note The ordering and definition of input variables must match the training
        configuration used to build the model.

  @author Ruggero Turra, Elena Mazzeo
*/

#ifndef ELECTRONPHOTONSELECTORTOOLS_PHOTONBDTCALCULATOR_H
#define ELECTRONPHOTONSELECTORTOOLS_PHOTONBDTCALCULATOR_H

#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "EgammaAnalysisInterfaces/IPhotonObservableTool.h"
#include "xAODEgamma/Photon.h"
#include "PhotonSingleBDTCalculator.h"
#include <string>

namespace PhotonIDBDT {

class PhotonBDTCalculator : public asg::AsgTool, virtual public IPhotonObservableTool
{
ASG_TOOL_CLASS2(PhotonBDTCalculator, IPhotonObservableTool, asg::IAsgTool)

public:
  PhotonBDTCalculator(const std::string& name);
  virtual ~PhotonBDTCalculator() override;

  virtual StatusCode initialize() override;

  /// Evaluate the BDT score for a given photon (fails loudly if it cannot be computed)
  /// Inherited from IPhotonObservableTool
  using IPhotonObservableTool::evaluate;
  virtual double evaluate(const xAOD::Photon* photon) const override;

  /// Compute and decorate the photon with the BDT score.
  StatusCode decorate(const xAOD::Photon& ph) const;

  /// Return the score (computes if needed if m_forceRecompute is true)
  StatusCode getScore(const xAOD::Photon& ph, float& score) const;

private:
  // Helpers to compute input variables for converted and unconverted photons
  StatusCode fillVariablesConv(const xAOD::Photon& ph, std::vector<float>& vars) const;
  StatusCode fillVariablesUnconv(const xAOD::Photon& ph, std::vector<float>& vars) const;

  // Reserve size for input features vector for converted and unconverted photons
  Gaudi::Property<unsigned> m_reserveVarsConv   {this, "ReserveVarsConv", 16, "Reserve size for input vector (converted photons)"};
  Gaudi::Property<unsigned> m_reserveVarsUnconv {this, "ReserveVarsUnconv", 16, "Reserve size for input vector (unconverted photons)"};

  // Sub-tools for the evaluation of the BDT score for converted and unconverted photons. 
  ToolHandle<PhotonSingleBDTCalculator> m_toolConv {this, "ToolConv", "", "BDT calculator for converted photons"};
  ToolHandle<PhotonSingleBDTCalculator> m_toolUnconv {this, "ToolUnconv", "", "BDT calculator for unconverted photons"};

  // Properties
  Gaudi::Property<std::string> m_decorationName {this, "DecorationName", "PhotonBDTScore", "Aux decoration name for BDT score"};
  Gaudi::Property<bool> m_excludeTRT {this, "ExcludeTRT", true, "Exclude TRT conversions (for Run 3 conversion definition)"};
  Gaudi::Property<bool> m_forceRecompute {this, "ForceRecompute", false, "If true, recompute even if decoration exists"};

  bool isConverted(const xAOD::Photon& ph) const;
};

} // namespace PhotonIDBDT

#endif
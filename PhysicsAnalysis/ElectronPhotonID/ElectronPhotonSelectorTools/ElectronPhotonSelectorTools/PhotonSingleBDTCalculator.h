/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
  @class PhotonSingleBDTCalculator
  @brief Tool to evaluate a BDT score
         for photon identification.

  This tool loads a BDT model from a ROOT file (containing a TTree with the 
  trained model, produced using the MVAUtils converter script)
  and evaluates the classification score for a given set of input variables.

  The evaluation is done via MVAUtils::BDT.

  This class handles a single BDT instance (e.g. converted or unconverted
  photons, for Run 2 or Run 3). The selection between multiple BDTs, 
  the calculation of input variables, and the decoration of the
  photon with the resulting score is handled by PhotonBDTCalculator.

  @note The ordering and definition of input variables has to be
        consistent with the training configuration.

  @author Ruggero Turra, Elena Mazzeo
*/

#ifndef ELECTRONPHOTONSELECTORTOOLS_PHOTONSINGLEBDTCALCULATOR_H
#define ELECTRONPHOTONSELECTORTOOLS_PHOTONSINGLEBDTCALCULATOR_H

#include "AsgTools/PropertyWrapper.h"
#include "AsgTools/AsgTool.h"
#include <memory>
#include <string>
#include <vector>

namespace MVAUtils {
  class BDT;
}

class TFile;
class TTree;

namespace PhotonIDBDT {

class PhotonSingleBDTCalculator : public asg::AsgTool {
  ASG_TOOL_CLASS(PhotonSingleBDTCalculator, asg::IAsgTool)

public:
  PhotonSingleBDTCalculator(const std::string& name);
  virtual ~PhotonSingleBDTCalculator() override;

  virtual StatusCode initialize() override;

  /// Evaluate the BDT score for a photon
  StatusCode computeScore(const std::vector<float>& vars, float& score) const;

private:
  StatusCode loadBDT();

  Gaudi::Property<std::string> m_modelFile {
    this, "ModelFile", "", "ROOT file containing the BDT TTree"
  };

  Gaudi::Property<std::string> m_bdtTreeName {
    this, "BDTTreeName", "lgbm", "Name of the TTree containing the BDT (e.g. lgbm)"
  };

  // Private members
  std::unique_ptr<MVAUtils::BDT> m_bdt;

};

} // namespace PhotonIDBDT

#endif
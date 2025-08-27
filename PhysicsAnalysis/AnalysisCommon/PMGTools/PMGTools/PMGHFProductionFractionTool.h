/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PMGTOOLS_PMGHFPRODUCTIONFRACTIONTOOL_H
#define PMGTOOLS_PMGHFPRODUCTIONFRACTIONTOOL_H

// EDM include(s):
#include "AsgTools/AsgMetadataTool.h"
#include "PATInterfaces/SystematicsCache.h"

// Interface include(s):
#include "PMGAnalysisInterfaces/ISysTruthWeightTool.h"

#include <map>

namespace PMGTools
{
/// Implementation for xAOD Truth Weight Tool which retrieves
/// the charm / bottom hadron content of an event and derives
/// an event weight based on the HF production fractions.
///
/// @author Miha Muskinja
///
class PMGHFProductionFractionTool : public virtual ISysTruthWeightTool, public asg::AsgMetadataTool
{
  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS2(PMGHFProductionFractionTool, PMGTools::ISysTruthWeightTool, IReentrantSystematicsTool)

public:
  /// Create a constructor for standalone usage
  PMGHFProductionFractionTool(const std::string &name);

  /// @name Function(s) implementing the asg::IAsgTool interface
  /// @{

  /// Function initialising the tool
  virtual StatusCode initialize() override;

  /// @}

  /// @name Function(s) implementing the ISysTruthWeightTool interface
  /// @{

  /// Implements interface from ISysTruthWeightTool
  virtual float getSysWeight(const xAOD::TruthParticleContainer *truthParticles, const CP::SystematicSet &sys) const override;

  /// Which systematics have an effect on the tool's behaviour?
  virtual CP::SystematicSet affectingSystematics() const override;

  /// Copied from SystematicsTool.cxx
  virtual CP::SystematicSet recommendedSystematics() const override { return affectingSystematics(); };

  /// @}

private:
  /// Struct for the production fractions
  struct ParameterSet {
      std::map<unsigned int, float> charmWeights;
      std::map<unsigned int, float> bottomWeights;
  };

  /// Setup weights
  StatusCode setupProductionFractions();

  /// Read production fractions from input file
  StatusCode readProductionFractionsFile(std::string, std::map<CP::SystematicVariation, std::map<unsigned int, float>> *);

  /// Checks if a particle originates from a bottom decay
  bool fromBdecay(const xAOD::TruthParticle *particle) const;

  /// Loops back the decay chain through particles with the same pdgId (e.g. photon emission)
  const xAOD::TruthParticle *getInitialParticle(const xAOD::TruthParticle *tp) const;

  /// Print the current production fractions
  void printCurrentProdFractions() const;

  float getWeight(const xAOD::TruthParticleContainer *truthParticles, const ParameterSet &prod_fracs) const;

  /// Path to calibration area
  std::string m_calibrationAreaPath;

  /// MC Shower generator map
  std::map<std::string, std::string> m_showerGeneratorMap;

  /// MC Shower generator map file name
  std::string m_showerGeneratorMapFile;

  /// MC Shower generator software (valid options: Pythia8)
  std::string m_showerGenerator;

  /// Input file with charm production fractions
  std::string m_charmFilename;

  /// Input file with bottom production fractions
  std::string m_bottomFilename;

  /// The fiducial charm/bottom pT cut (in GeV)
  float m_fiducialPtCut;

  /// The fiducial charm/bottom eta cut
  float m_fiducialEtaCut;

  /// Charm production fraction weights
  std::map<CP::SystematicVariation, std::map<unsigned int, float>> m_charmProdFractionWeights;

  /// Bottom production fraction weights
  std::map<CP::SystematicVariation, std::map<unsigned int, float>> m_bottomProdFractionWeights;

  /// calculate the parameter set for the given systematic
  StatusCode setSystematicVariation(const CP::SystematicSet& systConfig, ParameterSet& param) const;

  /// The SystematicsCache object
  CP::SystematicsCache<ParameterSet> m_Parameters{this};
};
}  // namespace PMGTools

#endif  // PMGTOOLS_PMGHFProductionFractionTool_H

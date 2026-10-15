/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_TAURESOLUTIONTOOL_H
#define TAUANALYSISTOOLS_TAURESOLUTIONTOOL_H

// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgTools/AsgMetadataTool.h"
#include "AsgTools/PropertyWrapper.h"

#include "TauAnalysisTools/ITauResolutionTool.h"

#include "TGraph.h"

namespace TauAnalysisTools
{

class TauResolutionTool
  : public asg::AsgMetadataTool
  , public virtual ITauResolutionTool
{
  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS( TauResolutionTool, TauAnalysisTools::ITauResolutionTool )

public:
  /// Create a constructor for standalone usage
  TauResolutionTool( const std::string& sName );

  ~TauResolutionTool();

  /// Function initialising the tool
  virtual StatusCode initialize();

  /** Get MVA Et resolution, invoked by METSignificance */
  double getMvaEnergyResolution(const xAOD::TauJet& tau) const;

  /** Check if MVA TES and CaloTES are compatible */
  bool getTESCompatibility(const xAOD::TauJet& tau) const; 

private:

  Gaudi::Property<std::string> m_calFileName{this, "WeightFileName", "tauRecTools/R22_preprod/CombinedTES_R22_Round2.5_v2.root"};

  /// Binning in the calibraction graph/hist
  enum Binning {DecayModeBinning = 5, EtaBinning = 5};

  /// Decay mode binning in the calibration graph/hist
  const std::array<std::string, DecayModeBinning> m_decayModeNames = {"1p0n","1p1n","1pXn","3p0n","3pXn"}; //!
  
  /// Eta binning in the calibration graph
  const std::array<std::string, EtaBinning> m_etaBinNames = {"0", "1", "2", "3", "4"}; //!

  /** Get the index of eta in the calibration histogram */
  int getEtaIndex(float eta) const;

  /** Get the decay mode of the tau candidate */
  xAOD::TauJetParameters::DecayMode getDecayMode(const xAOD::TauJet& tau) const;

  /** Get the index of decay mode in the calibration histogram */
  int getDecayModeIndex(xAOD::TauJetParameters::DecayMode decayMode) const;

  /// Calibration graph: MVA TES resolution as a function of MVA pt
  std::array<std::array<std::unique_ptr<TGraph>, EtaBinning>, DecayModeBinning> m_mvaRes; //!
  
  /// Maximum Et of m_mvaRes
  std::array<std::array<double, EtaBinning>, DecayModeBinning> m_mvaResMaxEt; //! 

  /** Whether the tau candidate is valid for the calculation */
  bool isValid(const xAOD::TauJet& tau) const; 

  /** Get the resolution of Et at the calo TES */
  double getCaloResolution(double et, int decayModeIndex, int etaIndex) const;

  /// Calibration graph: resolution at Calo TES as a function of caloEt
  std::array<std::array<std::unique_ptr<TGraph>, EtaBinning>, DecayModeBinning> m_caloRes; //!

  /// Maximum Et of m_caloRes
  std::array<std::array<double, EtaBinning>, DecayModeBinning> m_caloResMaxEt; //!

}; // class TauResolutionTool

} // namespace TauAnalysisTools

#endif // TAUANALYSISTOOLS_TAURESOLUTIONTOOL_H

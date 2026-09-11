/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_COMMONDITAUSMEARINGTOOL_H
#define TAUANALYSISTOOLS_COMMONDITAUSMEARINGTOOL_H

/*
  author: David Kirchmeier
  mail: david.kirchmeier@cern.ch
  documentation in: ../doc/README-DiTauSmearingTool.rst
*/

// Framework include(s):
#include "AsgTools/AsgMetadataTool.h"
#include "AsgTools/AnaToolHandle.h"

// EDM include(s):
#include "xAODTau/DiTauJet.h"
#include "PATInterfaces/CorrectionCode.h"

// Local include(s):
#include "TauAnalysisTools/Enums.h"
#include "TauAnalysisTools/IDiTauSmearingTool.h"
#include "TauAnalysisTools/CommonDiTauEfficiencyTool.h"
#include "TauAnalysisTools/HelperFunctions.h"

#include "TH3.h"

namespace TauAnalysisTools
{

class CommonDiTauSmearingTool
  : public virtual IDiTauSmearingTool
  , public asg::AsgMetadataTool
{
  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS( CommonDiTauSmearingTool, TauAnalysisTools::IDiTauSmearingTool )

public:

  CommonDiTauSmearingTool(const std::string& sName);

  virtual StatusCode initialize();

  /// Apply the correction on a modifiable object
  virtual CP::CorrectionCode applyCorrection( xAOD::DiTauJet& xDiTau ) const;
  /// Create a corrected copy from a constant ditau
  virtual CP::CorrectionCode correctedCopy( const xAOD::DiTauJet& xDiTau,
      xAOD::DiTauJet*& xDiTauCopy) const;

  /// returns: whether this tool is affected by the given systematics
  virtual bool isAffectedBySystematic( const CP::SystematicVariation& systematic ) const;

  /// returns: the list of all systematics this tool can be affected by
  virtual CP::SystematicSet affectingSystematics() const;

  /// returns: the list of all systematics this tool recommends to use
  virtual CP::SystematicSet recommendedSystematics() const;

  /// configure this tool for the given list of systematic variations.  any
  /// requested systematics that are not affecting this tool will be silently ignored
  virtual StatusCode applySystematicVariation ( const CP::SystematicSet& sSystematicSet);

protected:

  std::map<std::string, TH3*> m_mDTSF;
  std::unordered_map < CP::SystematicSet, std::string > m_mSystematicSets;
  const CP::SystematicSet* m_sSystematicSet;
  std::map<std::string, int> m_mSystematics;
  std::map<std::string, std::string> m_mSystematicsHistNames;

  double (*m_fX)(const xAOD::DiTauJet& xDiTau);
  double (*m_fY)(const xAOD::DiTauJet& xDiTau);
  double (*m_fZ)(const xAOD::DiTauJet& xDiTau);

  template<class T>
  void ReadInputs(TFile* fFile, std::map<std::string, T>& mMap);
  virtual CP::CorrectionCode getValue(const std::string& sHistName,
                                      const xAOD::DiTauJet& xDiTau,
                                      double& dCorrectionFactor) const;
  void generateSystematicSets();

  Gaudi::Property<std::string> m_sInputFilePath{this, "InputFilePath", ""};
  Gaudi::Property<bool> m_bSkipTruthMatchCheck{this, "SkipTruthMatchCheck", false};

  bool m_bIsData;
  bool m_bIsConfigured; 

  TruthMatchedParticleType m_eCheckTruth;
  CP::SystematicSet m_sAffectingSystematics;
  CP::SystematicSet m_sRecommendedSystematics;

private:

  // Execute at each event
  virtual StatusCode beginEvent();

};
} // namespace TauAnalysisTools

#endif // TAUANALYSISTOOLS_COMMONEFFICIENCYTOOL_H




/**
 * @file CommonDiTauEfficiencyTool.h
 * @author David Kirchmeier
 * @author Guillermo Hamity (ghamity@cern.ch)
 * @brief 
 * @date 2021-02-18
 * 
 * @copyright Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 * 
 */


#ifndef TAUANALYSISTOOLS_COMMONDITAUEFFICIENCYTOOL_H
#define TAUANALYSISTOOLS_COMMONDITAUEFFICIENCYTOOL_H


// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"

// EDM include(s):
#include "xAODTau/DiTauJet.h"

// Local include(s):
#include "TauAnalysisTools/Enums.h"
#include "TauAnalysisTools/IDiTauEfficiencyCorrectionsTool.h"
#include "TauAnalysisTools/HelperFunctions.h"

// ROOT include(s):
#include "TFile.h"

class TKey;


namespace TauAnalysisTools
{
/** return the truth vis pT of the leading pT matched particle.*/
double TruthLeadPt(const xAOD::DiTauJet& xDiTau);
/** return the truth vis pT of the subleading pT matched particle.*/
double TruthSubleadPt(const xAOD::DiTauJet& xDiTau);
/** return the dR of between the leading and subleading pT matched particle.*/
double TruthDeltaR(const xAOD::DiTauJet& xDiTau);

class CommonDiTauEfficiencyTool
  : public virtual IDiTauEfficiencyCorrectionsTool
  , public asg::AsgTool	
{
  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS( CommonDiTauEfficiencyTool, TauAnalysisTools::IDiTauEfficiencyCorrectionsTool )

public:

  CommonDiTauEfficiencyTool(const std::string& sName);

  ~CommonDiTauEfficiencyTool();

  virtual StatusCode initialize();

  /**
   * @brief Get the Efficiency Scale Factor of ditau jet
   * 
   * @param xDiTau : reco DiTauJet
   * @param dEfficiencyScaleFactor : reference to output variable where efficiency is returned
   * @return CP::CorrectionCode 
   */
  virtual CP::CorrectionCode getEfficiencyScaleFactor(const xAOD::DiTauJet& xDiTau, double& dEfficiencyScaleFactor);

  /**
   * @brief Get the Efficiency Scale Factor of ditau jet
   * 
   * @param xDiTau 
   * @return CP::CorrectionCode 
   */
  virtual CP::CorrectionCode applyEfficiencyScaleFactor(const xAOD::DiTauJet& xDiTau);

  /// returns: whether this tool is affected by the given systematics
  virtual bool isAffectedBySystematic( const CP::SystematicVariation& systematic ) const;

  /// returns: the list of all systematics this tool can be affected by
  virtual CP::SystematicSet affectingSystematics() const;

  /// returns: the list of all systematics this tool recommends to use
  virtual CP::SystematicSet recommendedSystematics() const;

  /// configure this tool for the given list of systematic variations.  any
  /// requested systematics that are not affecting this tool will be silently ignored
  virtual StatusCode applySystematicVariation ( const CP::SystematicSet& sSystematicSet);

  std::function<double(const xAOD::DiTauJet& xDiTau)> m_fXDiTau;
  std::function<double(const xAOD::DiTauJet& xDiTau)> m_fYDiTau;
  std::function<double(const xAOD::DiTauJet& xDiTau)> m_fZDiTau; 

protected:

  void ReadInputs(std::unique_ptr<TFile> &fFile);
  void addHistogramToSFMap(TKey* kKey, const std::string& sKeyName);

  typedef std::tuple<TObject*,
          CP::CorrectionCode (*)(const TObject* oObject,
                                 double& dEfficiencyScaleFactor,
                                 double dVars[] ) > tTupleObjectFunc;
  typedef std::map<std::string, tTupleObjectFunc > tSFMAP; 
  // In gcc10 builds, cling gets confused by the type of m_mSF and produces
  // an ugly warning message.  Hide this from cling to suppress that
  // (substitute another unique_ptr so that the class layout remains the same).
#ifdef __CLING__
  std::unique_ptr<int> m_dummy;
#else
  std::unique_ptr< tSFMAP > m_mSF;
#endif

  std::unordered_map < CP::SystematicSet, std::string > m_mSystematicSets;
  const CP::SystematicSet* m_sSystematicSet;
  std::map<std::string, std::string> m_mSystematicsHistNames;

  /**
   * @brief Get the scale factor from a particular recommendations histogram.
   * 
   * @param sHistName 
   * @param xDiTau 
   * @param dEfficiencyScaleFactor 
   * @return CP::CorrectionCode 
   */
  virtual CP::CorrectionCode getValue(const std::string& sHistName,
                              const xAOD::DiTauJet& xDiTau,
                              double& dEfficiencyScaleFactor) const;

  static CP::CorrectionCode getValueTH1(const TObject* oObject,
                                        double& dEfficiencyScaleFactor,
                                        double dVars[]
                                        );
  static CP::CorrectionCode getValueTH2(const TObject* oObject,
                                        double& dEfficiencyScaleFactor,
                                        double dVars[]
                                        );

  /** generate a set of relevant systematic variations to be applied*/
  void generateSystematicSets();
  /** true if scale factor name is already decorated*/ 
  bool m_bSFIsAvailableDiTau;
  /** true if cale factor name is already decorated has already been checked*/
  bool m_bSFIsAvailableCheckedDiTau;

  CP::SystematicSet m_sAffectingSystematics;
  CP::SystematicSet m_sRecommendedSystematics;

  Gaudi::Property<bool> m_bSkipTruthMatchCheck{this, "SkipTruthMatchCheck", false};
  Gaudi::Property<std::string> m_sInputFilePath{this, "InputFilePath", ""};
  Gaudi::Property<std::string> m_sWP{this, "WP", ""};
  Gaudi::Property<std::string> m_sVarName{this, "VarName", ""};

  std::string m_sInputFileName;
  std::string m_sSFHistName;

  TruthMatchedParticleType m_eCheckTruth;

};
} // namespace TauAnalysisTools

#endif // TAUANALYSISTOOLS_COMMONDITAUEFFICIENCYTOOL_H

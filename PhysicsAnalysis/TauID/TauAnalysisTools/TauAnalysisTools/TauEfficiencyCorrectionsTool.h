/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_TAUEFFICIENCYCORRECTIONSTOOL_H
#define TAUANALYSISTOOLS_TAUEFFICIENCYCORRECTIONSTOOL_H

/*
  author: Dirk Duschinger
  maintainer: Guillermo Hamity
  mail: guillermo.nicolas.hamity@cern.ch
  documentation in: ../README.rst
                    https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/TauID/TauAnalysisTools/README.rst
*/

// Framework include(s):
#include "AsgTools/AsgMetadataTool.h"
#include "AsgTools/AnaToolHandle.h"
#include "AsgTools/PropertyWrapper.h"

// Local include(s):
#include "TauAnalysisTools/ITauEfficiencyCorrectionsTool.h"
#include "TauAnalysisTools/CommonEfficiencyTool.h"

// Tool include(s)
#include "AsgAnalysisInterfaces/IPileupReweightingTool.h"

namespace TauAnalysisTools
{

class TauEfficiencyCorrectionsTool
  : public virtual ITauEfficiencyCorrectionsTool
  , public asg::AsgMetadataTool
{
  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS( TauEfficiencyCorrectionsTool, TauAnalysisTools::ITauEfficiencyCorrectionsTool )

public:
  /// Create a constructor for standalone usage
  TauEfficiencyCorrectionsTool( const std::string& sName );

  ~TauEfficiencyCorrectionsTool();

  /// Function initialising the tool
  virtual StatusCode initialize();

  virtual StatusCode beginInputFile();

  /// Print tool configuration
  virtual void printConfig() const;

  /// Get the tau efficiency scale factor
  virtual CP::CorrectionCode getEfficiencyScaleFactor( const xAOD::TauJet& xTau,
      double& eff, unsigned int iRunNumber = 0);

  /// Decorate the tau with its efficiency scale factor
  virtual CP::CorrectionCode applyEfficiencyScaleFactor( const xAOD::TauJet& xTau,
      unsigned int iRunNumber = 0);

  /// returns: whether this tool is affected by the given systematics
  virtual bool isAffectedBySystematic( const CP::SystematicVariation& systematic ) const;

  /// returns: the list of all systematics this tool can be affected by
  virtual CP::SystematicSet affectingSystematics() const;

  /// returns: the list of all systematics this tool recommends to use
  virtual CP::SystematicSet recommendedSystematics() const;

  virtual StatusCode applySystematicVariation( const CP::SystematicSet& systConfig );

  virtual bool isSupportedRunNumber( int /*iRunNumber*/ ) const
  {
    return true;
  };

private:
  StatusCode firstEvent();
  StatusCode beginEvent();

  std::string ConvertJetIDToString(const int iLevel) const;

  std::string ConvertEleIDToString(const int iLevel) const;

  std::string ConvertTriggerIDToString(const int iLevel) const;

  std::string GetTriggerSFMeasurementString() const;

  StatusCode initializeTools_2022_prerec();
  StatusCode initializeTools_2025_prerec(); 

  StatusCode readRandomRunNumber();

private:

  Gaudi::Property<std::string> m_sInputFilePathRecoHadTau{this, "InputFilePathRecoHadTau", ""};
  Gaudi::Property<std::string> m_sInputFilePathEleIDHadTau{this, "InputFilePathEleIDHadTau", ""};
  Gaudi::Property<std::string> m_sInputFilePathEleIDElectron{this, "InputFilePathEleIDElectron", ""};
  Gaudi::Property<std::string> m_sInputFilePathJetIDHadTau{this, "InputFilePathJetIDHadTau", ""};  
  Gaudi::Property<std::string> m_sInputFilePathTriggerHadTau{this, "InputFilePathTriggerHadTau", ""};   
  Gaudi::Property<std::string> m_sRecommendationTag{this, "RecommendationTag", "2025-prerec"};
  Gaudi::Property<std::string> m_sTriggerName{this, "TriggerName", ""};
  Gaudi::Property<bool> m_bReadRandomRunNumber{this, "AutoTriggerYear", false}; 
  Gaudi::Property<std::string> m_sTriggerSFMeasurement{this, "TriggerSFMeasurement", "combined"}; 
  Gaudi::Property<bool> m_bUseTauSubstructure{this, "UseTauSubstructure", false}; 
  Gaudi::Property<int> m_iJetIDLevel{this, "JetIDLevel", static_cast<int>(JETIDNONE)}; 
  Gaudi::Property<int> m_iEleIDLevel{this, "EleIDLevel", static_cast<int>(ELEIDNONE)};
  Gaudi::Property<std::string> m_sCampaign{this, "Campaign", ""};
  Gaudi::Property<bool> m_useFastSim{this, "useFastSim", false};
  Gaudi::Property<bool> m_bSkipTruthMatchCheck{this, "SkipTruthMatchCheck", false};

  std::vector<int> m_vEfficiencyCorrectionTypes;
  std::vector< asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>* > m_vCommonEfficiencyTools;
  std::vector< asg::AnaToolHandle<ITauEfficiencyCorrectionsTool>* > m_vTriggerEfficiencyTools;
  std::string m_sInputFilePathDecayModeHadTau;
  std::string m_sVarName;
  bool m_bIsData;
  bool m_bIsConfigured;
  bool m_firstEvent = false;
  unsigned int m_iRunNumber;

}; // class TauEfficiencyCorrectionsTool

} // namespace TauAnalysisTools

#endif // TAUANALYSISTOOLS_TAUEFFICIENCYCORRECTIONSTOOL_H

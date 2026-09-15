/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_COMMONSMEARINGTOOL_H
#define TAUANALYSISTOOLS_COMMONSMEARINGTOOL_H

/*
  author: Dirk Duschinger
  mail: dirk.duschinger@cern.ch
  documentation in: ../README.rst
*/

// Framework include(s):
#include "AsgTools/AsgMetadataTool.h"
#include "AsgTools/AnaToolHandle.h"
#include "AsgTools/PropertyWrapper.h"

// EDM include(s):
#include "xAODTau/TauJet.h"
#include "xAODTruth/TruthParticle.h"
#include "PATInterfaces/CorrectionCode.h"

#include "tauRecTools/TauCombinedTES.h"

// Local include(s):
#include "TauAnalysisTools/Enums.h"
#include "TauAnalysisTools/ITauSmearingTool.h"
#include "TauAnalysisTools/HelperFunctions.h"

// ROOT include(s):
#include "TFile.h"
#include "TH1.h"

// tauRecTools include(s)
#include "tauRecTools/ITauToolBase.h"

#include <ColumnarEventInfo/EventInfoDef.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/LinkColumn.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/VectorColumn.h>
#include <ColumnarTau/TauJetDef.h>

namespace TauAnalysisTools
{

class CommonSmearingTool
  : public virtual ITauSmearingTool
  , public asg::AsgMetadataTool
  , public columnar::ColumnarTool<>	
{
  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS( CommonSmearingTool, TauAnalysisTools::ITauSmearingTool )

public:

  CommonSmearingTool(const std::string& sName);

  ~CommonSmearingTool();

  virtual StatusCode initialize() override;

  // CommonSmearingTool pure virtual public functionality
  //__________________________________________________________________________

  /// Apply the correction on a modifyable object
  virtual CP::CorrectionCode applyCorrection( xAOD::TauJet& xTau ) const override;
 
  CP::CorrectionCode applyCorrection( columnar::TauJetId tau ) const;

  /// Create a corrected copy from a constant tau
  virtual CP::CorrectionCode correctedCopy( const xAOD::TauJet& xTau,
      xAOD::TauJet*& xTauCopy) const override;

  /// returns: whether this tool is affected by the given systematics
  virtual bool isAffectedBySystematic( const CP::SystematicVariation& systematic ) const override;

  /// returns: the list of all systematics this tool can be affected by
  virtual CP::SystematicSet affectingSystematics() const override;

  /// returns: the list of all systematics this tool recommends to use
  virtual CP::SystematicSet recommendedSystematics() const override;

  /// configure this tool for the given list of systematic variations.  any
  /// requested systematics that are not affecting this tool will be silently ignored
  virtual StatusCode applySystematicVariation ( const CP::SystematicSet& sSystematicSet) override;

protected:

  std::string ConvertProngToString(const int iProngness) const;

  std::map<std::string, TH1*> m_mSF;
  std::unordered_map < CP::SystematicSet, std::string > m_mSystematicSets;
  const CP::SystematicSet* m_sSystematicSet;
  std::map<std::string, int> m_mSystematics;
  std::map<std::string, std::string> m_mSystematicsHistNames;

  // std::function<double> m_fX;
  double (*m_fX)(const xAOD::TauJet& xTau);
  double (*m_fY)(const xAOD::TauJet& xTau);

  template<class T>
  void ReadInputs(TFile* fFile, std::map<std::string, T>& mMap);

  virtual CP::CorrectionCode getValue(const std::string& sHistName,
                                      const xAOD::TauJet& xTau,
                                      double& dEfficiencyScaleFactor) const;

  void generateSystematicSets();

  Gaudi::Property<std::string> m_sInputFilePath{this, "InputFilePath", ""};
  Gaudi::Property<bool> m_bSkipTruthMatchCheck{this, "SkipTruthMatchCheck", false};
  Gaudi::Property<bool> m_bApplyFading{this, "ApplyFading", true};
  Gaudi::Property<bool> m_bMVATESQualityCheck{this, "MVATESQualityCheck", true};
  Gaudi::Property<bool> m_bApplyInsituCorrection{this, "ApplyInsituCorrection", true};  

  bool m_bIsData;
  bool m_bIsConfigured;

  asg::AnaToolHandle<ITauToolBase> m_tTauCombinedTES;

  TruthMatchedParticleType m_eCheckTruth;
  CP::SystematicSet m_sAffectingSystematics;
  CP::SystematicSet m_sRecommendedSystematics;
  
private:

  CxxUtils::CachedValue<bool> m_bIsTESCompatibilityCheckAvailable; 

  // Execute at each event
  virtual StatusCode beginEvent() override;

public:

  struct Accessors : public columnar::ColumnarTool<>
  {
    Accessors(CommonSmearingTool& tool) : columnar::ColumnarTool<>(&tool) {}

    columnar::EventInfoAccessor<columnar::ObjectColumn> m_eventInfo {*this, "EventInfo", {.addMTDependency=true}};
    columnar::EventInfoAccessor<uint32_t> randomrunnumber;

    columnar::TauJetAccessor<columnar::ObjectColumn> m_taus {*this, "TauJets"};
    //columnar::TauJetAccessor<float> m_eta{*this,"eta"};
    //columnar::TauJetAccessor<float> m_pt{*this,"pt"};
    columnar::TauJetDecorator<float> m_sfDec{*this,"sfOut"};
    columnar::TauJetDecorator<char> m_validDec{*this,"validOut"};
  };
  std::unique_ptr<Accessors> m_accessors;

  void callSingleEvent (columnar::TauJetRange taus) const;
  void callEvents (columnar::EventContextRange events) const override;

};
} // namespace TauAnalysisTools

#endif // TAUANALYSISTOOLS_COMMONSMEARINGTOOL_H

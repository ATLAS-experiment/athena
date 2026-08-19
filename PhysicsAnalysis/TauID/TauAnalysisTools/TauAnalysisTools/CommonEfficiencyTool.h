/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_COMMONEFFICIENCYTOOL_H
#define TAUANALYSISTOOLS_COMMONEFFICIENCYTOOL_H

/*
  author: Dirk Duschinger
  mail: dirk.duschinger@cern.ch
*/

// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"

// EDM include(s):
#include "xAODTau/TauJet.h"
#include "xAODTruth/TruthParticle.h"
#include "PATInterfaces/CorrectionCode.h"

// Local include(s):
#include "TauAnalysisTools/Enums.h"
#include "TauAnalysisTools/ITauEfficiencyCorrectionsTool.h"
#include "TauAnalysisTools/HelperFunctions.h"

#include <ColumnarEventInfo/EventInfoDef.h>
#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarCore/LinkColumn.h>
#include <ColumnarCore/ObjectColumn.h>
#include <ColumnarCore/VectorColumn.h>
#include <ColumnarTau/TauJetDef.h>
#include "TauAnalysisTools/ColumnarTauAccessors.h"

class TFile;
class TKey;

namespace TauAnalysisTools
{

// forward declaration
class TauEfficiencyCorrectionsTool;

class CommonEfficiencyTool
  : public virtual ITauEfficiencyCorrectionsTool
  , public asg::AsgTool
  , public columnar::ColumnarTool<CMode>	
{
  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS( CommonEfficiencyTool, TauAnalysisTools::ITauEfficiencyCorrectionsTool )

public:

  CommonEfficiencyTool(const std::string& sName);

  ~CommonEfficiencyTool();

  virtual StatusCode initialize() override;

  // CommonEfficiencyTool pure virtual public functionality
  //__________________________________________________________________________

  virtual CP::CorrectionCode getEfficiencyScaleFactor(const xAOD::TauJet& tau, double& dEfficiencyScaleFactor, 
    unsigned int iRunNumber = 0 ) override;

  CP::CorrectionCode getEfficiencyScaleFactor( columnar::TauJetId tau, double& dEfficiencyScaleFactor,
    unsigned int iRunNumber) const;

  virtual CP::CorrectionCode applyEfficiencyScaleFactor(const xAOD::TauJet& xTau, 
    unsigned int iRunNumber = 0 ) override;

  /// returns: whether this tool is affected by the given systematics
  virtual bool isAffectedBySystematic( const CP::SystematicVariation& systematic ) const override;

  /// returns: the list of all systematics this tool can be affected by
  virtual CP::SystematicSet affectingSystematics() const override;

  /// returns: the list of all systematics this tool recommends to use
  virtual CP::SystematicSet recommendedSystematics() const override;

  /// configure this tool for the given list of systematic variations.  any
  /// requested systematics that are not affecting this tool will be silently ignored
  virtual StatusCode applySystematicVariation ( const CP::SystematicSet& sSystematicSet) override;

  virtual bool isSupportedRunNumber( int /*iRunNumber*/ ) const override
  {
    return true;
  };

protected:

  std::string ConvertProngToString(const int iProngness) const;
  std::string ConvertDecayModeToString(const int iDecayMode) const;

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
  std::map<std::string, int> m_mSystematics;
  std::map<std::string, std::string> m_mSystematicsHistNames;

  std::function<double(const xAOD::TauJet& xTau)> m_fX;
  std::function<double(const xAOD::TauJet& xTau)> m_fY;

  void ReadInputs(const TFile& fFile);
  void addHistogramToSFMap(TKey* kKey, const std::string& sKeyName);

  virtual CP::CorrectionCode getValue(const std::string& sHistName,
                                      columnar::TauJetId tau,
                                      double& dEfficiencyScaleFactor) const;

  static CP::CorrectionCode getValueTH1(const TObject* oObject,
                                        double& dEfficiencyScaleFactor,
                                        double dVars[]
                                        );
  static CP::CorrectionCode getValueTH2(const TObject* oObject,
                                        double& dEfficiencyScaleFactor,
                                        double dVars[]
                                        );
  static CP::CorrectionCode getValueTF1(const TObject* oObject,
                                        double& dEfficiencyScaleFactor,
                                        double dVars[]
                                       );

  void generateSystematicSets();

protected:

  CP::SystematicSet m_sAffectingSystematics;
  CP::SystematicSet m_sRecommendedSystematics;

  Gaudi::Property<std::string> m_sInputFilePath{this, "InputFilePath", ""};
  Gaudi::Property<std::string> m_sVarName{this, "VarName", ""};
  Gaudi::Property<std::string> m_sWP{this, "WP", ""};
  Gaudi::Property<bool> m_bSkipTruthMatchCheck{this, "SkipTruthMatchCheck", false};
  Gaudi::Property<int> m_iJetIDLevel{this, "JetIDLevel", static_cast<int>(JETIDNONE)};
  Gaudi::Property<int> m_iEleIDLevel{this, "EleIDLevel", static_cast<int>(ELEIDNONE)};
  Gaudi::Property<bool> m_bUseTauSubstructure{this, "UseTauSubstructure", false};
  Gaudi::Property<bool> m_doTauTrig{this, "DoTauTrig", false};
  Gaudi::Property<bool> m_applyToData{this, "ApplyToData", false}; // for experts only

  std::string m_sInputFileName;
  std::string m_sSFHistName;
  bool m_bNoMultiprong;

  TruthMatchedParticleType m_eCheckTruth;

  bool m_bSFIsAvailable;
  bool m_bSFIsAvailableChecked;


public:

  struct Accessors : public columnar::ColumnarTool<CMode>
  {
    Accessors(CommonEfficiencyTool& tool) : columnar::ColumnarTool<CMode>(&tool) {}

    columnar::EventInfoAccessor<columnar::ObjectColumn,CMode> m_eventInfo {*this, "EventInfo", {.addMTDependency=true}};
    columnar::EventInfoAccessor<uint32_t,CMode> randomrunnumber;

    // Associated truth particles and jets. These are picked up by truth
    // links on the tau itself.
    columnar::TruthParticleAccessor<columnar::ObjectColumn,CMode> m_truthParticles {*this, "TruthTaus"};
    columnar::JetAccessor<columnar::ObjectColumn> m_jets {*this, "AntiKt4TruthDressedWZJets"};

    columnar::TauJetAccessor<columnar::ObjectColumn> m_taus {*this, "TauJets"};
    //columnar::TauJetAccessor<int> m_nTracks{*this, "nChargedTracks", {.isOptional=true}}; to be used when 'nChargedTracks' will be in physlite
    //columnar::TauJetAccessor<float> m_eta{*this,"eta"};
    //columnar::TauJetAccessor<float> m_pt{*this,"pt"};
    columnar::TauJetAccessor<int> m_decayMode{*this,"PanTau_DecayMode"};
    TruthParticleTypeAccessor<> m_truthParticleType{*this};
    columnar::TauJetDecorator<float> m_sfDec{*this,"sfOut"};
    columnar::TauJetDecorator<char> m_validDec{*this,"validOut"};
  };
  std::unique_ptr<Accessors> m_accessors;

  void callSingleEvent (columnar::TauJetRange taus, columnar::EventInfoId<CMode> event) const;
  void callEvents (columnar::EventContextRange<CMode> events) const override;


};
} // namespace TauAnalysisTools

#endif // TAUANALYSISTOOLS_COMMONEFFICIENCYTOOL_H

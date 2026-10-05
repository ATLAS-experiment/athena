/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CPBTAGGINGSELECTIONJSONTOOL_H
#define CPBTAGGINGSELECTIONJSONTOOL_H

#include "FTagAnalysisInterfaces/IBTaggingSelectionJsonTool.h"
#include "AsgTools/AsgTool.h"
#include <AsgTools/PropertyWrapper.h>
#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;

class BTaggingSelectionJsonTool: public asg::AsgTool,
                                 public virtual IBTaggingSelectionJsonTool {

  ASG_TOOL_CLASS( BTaggingSelectionJsonTool, IBTaggingSelectionJsonTool )

public:
  BTaggingSelectionJsonTool(const std::string& name);
  StatusCode initialize() override;

  virtual int accept(const xAOD::Jet& jet) const override;
  // Following function is for Xbb calibration purposes only
  virtual int acceptOnlyForXbbCalibrationUsage(double pt, double eta, double mass, double tagger_discriminant) const override;

  virtual double getTaggerDiscriminant(const xAOD::Jet& jet) const override;
  virtual double getVetoDiscriminant(const xAOD::Jet& jet) const;

private:
  bool m_initialised = false;
  Gaudi::Property<float> m_minPt {this, "MinPt", -1 /*MeV*/, "Minimum jet pT cut (in MeV)"};
  Gaudi::Property<float> m_maxEta {this, "MaxEta", 2.5, "Maximum jet eta cut"};
  
  
  Gaudi::Property<std::string> m_outputName {this, "OutputName", "", "output name of the tagger"};
  Gaudi::Property<std::string> m_jetAuthor {this, "JetAuthor", "", "jet collection"};
  Gaudi::Property<std::string> m_OP {this, "OperatingPoint", "", "operating point"};
  Gaudi::Property<std::string> m_json_config_path {this, "JsonConfigFile", "", "Path to JSON config file"};
  Gaudi::Property<bool> m_allowBinCountMismatch {this, "AllowBinCountMismatch", false,
    "If true, downgrade inconsistent pT/mass bin counts in loadBinConfig() from an ERROR "
    "(initialisation failure) to a WARNING and continue. Default false (fail on inconsistency)."};

  std::unique_ptr<SG::ConstAccessor<float>> m_massAcc;
  std::unique_ptr<SG::ConstAccessor<float>> m_ptAcc;

  json m_json_config;

  struct BinConfig {
    std::vector<float> pTbins;
    std::vector<std::vector<float>> massbins;
    std::vector<std::vector<float>> OPCutValues;
  };

  struct FractionAccessor {
    float fraction;
    SG::ConstAccessor<float> accessor;
    bool isTarget;

    FractionAccessor(float fraction, const SG::ConstAccessor<float>& accessor, bool isTarget)
      : fraction(fraction), accessor(accessor), isTarget(isTarget) {}
  };

  BinConfig m_BinConfig;
  BinConfig m_VetoBinConfig;

  std::vector<FractionAccessor> m_fractionAccessors;
  std::vector<FractionAccessor> m_vetoFractionAccessors;

  bool m_veto = false;

  std::string m_vetoTagger;
  std::string m_vetoOP;

  StatusCode loadBinConfig(const json& pT_mass_2d_cutvalue, BinConfig& config) const;
  std::vector<FractionAccessor> loadFractionValues(const json &meta) const;
  double getTaggerDiscriminantInternal(const xAOD::Jet& jet, const std::vector<FractionAccessor>& fractionAccessors) const;
  int findBin(const std::vector<float>& bins, float value) const;
  float getJetMass(const xAOD::Jet& jet) const;
  float getJetPt(const xAOD::Jet& jet) const;
};

#endif // CPBTAGGINGSELECTIONJSONTOOL_H

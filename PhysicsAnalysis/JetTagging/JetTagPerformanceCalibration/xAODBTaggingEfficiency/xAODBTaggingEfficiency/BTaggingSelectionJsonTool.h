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
  BTaggingSelectionJsonTool( const std::string& name );
  StatusCode initialize() override;

  virtual int accept(const xAOD::Jet& jet) const override;
  // the following funciton is only for Xbb calibration team, for physics analyses, please use the one above.
  virtual int accept(double pt, double eta, double mass, double tagger_discriminant) const override;

  virtual double getTaggerDiscriminant( const xAOD::Jet& jet ) const override;
  
private:
  bool m_initialised = false;
  Gaudi::Property<float> m_minPt {this, "MinPt", -1 /*MeV*/, "Minimum jet pT cut (in MeV)"};
  Gaudi::Property<float> m_maxEta {this, "MaxEta", 2.5, "Maximum jet eta cut"};
  
  std::string m_taggerName;
  std::string m_target;
  
  Gaudi::Property<std::string> m_outputName {this, "OutputName", "", "output name of the tagger"};
  Gaudi::Property<std::string> m_jetAuthor {this, "JetAuthor", "", "jet collection"};
  Gaudi::Property<std::string> m_OP {this, "OperatingPoint", "", "operating point"};
  Gaudi::Property<std::string> m_json_config_path {this, "JsonConfigFile", "", "Path to JSON config file"};
  
  std::unique_ptr<SG::AuxElement::ConstAccessor<float>> m_massAcc;
  std::unique_ptr<SG::AuxElement::ConstAccessor<float>> m_ptAcc;

  json m_json_config;

  struct FractionAccessor {
    float fraction;
    SG::AuxElement::ConstAccessor<float> accessor;
    bool isTarget;

    FractionAccessor(float fraction, const SG::AuxElement::ConstAccessor<float>& accessor, bool isTarget)
      : fraction(fraction), accessor(accessor), isTarget(isTarget) {}
  };
  std::vector<FractionAccessor> m_fractionAccessors;
  std::vector<float> m_pTbins;
  std::vector<std::vector<float>> m_massbins;
  std::vector<std::vector<float>> m_OPCutValues;

  int findBin(const std::vector<float>& bins, float value) const;
  float getJetMass( const xAOD::Jet& jet ) const;
  float getJetPt( const xAOD::Jet& jet ) const;
};

#endif // CPBTAGGINGSELECTIONJSONTOOL_H

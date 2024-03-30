/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CPBTAGGINGSELECTIONJSONTOOL_H
#define CPBTAGGINGSELECTIONJSONTOOL_H

#include "FTagAnalysisInterfaces/IBTaggingSelectionJsonTool.h"
#include "AsgTools/AsgTool.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class BTaggingSelectionJsonTool: public asg::AsgTool,
			     public virtual IBTaggingSelectionJsonTool {

  public:
  BTaggingSelectionJsonTool( const std::string& name );
  StatusCode initialize() override;

  virtual int accept(const xAOD::Jet& jet) const override;
  virtual double getTaggerDiscriminant( const xAOD::Jet& jet) const override;

private:
  bool m_initialised = false;

  double m_maxEta;
  double m_minPt;

  std::string m_taggerName;
  std::string m_OP;
  std::string m_jetAuthor;
  std::string m_json_config_path;
  std::string m_target;

  json m_json_config;

  struct FractionAccessor {
    float fraction;
    SG::AuxElement::ConstAccessor<float> accessor;
    bool isTarget;

    FractionAccessor(float fraction, const SG::AuxElement::ConstAccessor<float>& accessor, bool isTarget)
      : fraction(fraction), accessor(accessor), isTarget(isTarget) {}
  };
  std::vector<FractionAccessor> m_fractionAccessors;
  std::vector<std::pair<std::string, float>> m_OPCutValues;

};

#endif // CPBTAGGINGSELECTIONJSONTOOL_H

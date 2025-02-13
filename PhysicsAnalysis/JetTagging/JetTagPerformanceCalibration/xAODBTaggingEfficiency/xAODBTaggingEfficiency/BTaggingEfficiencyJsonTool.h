/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CPBTAGGINGEFFICIENCYJSONTOOL_H
#define CPBTAGGINGEFFICIENCYJSONTOOL_H

#include "FTagAnalysisInterfaces/IBTaggingEfficiencyJsonTool.h"
#include "AsgTools/AsgTool.h"
#include "PATInterfaces/SystematicsCache.h"
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class BTaggingEfficiencyJsonTool: public asg::AsgTool,
                                  virtual public IBTaggingEfficiencyJsonTool 
{
  // creates a proper constructor for athena
  ASG_TOOL_CLASS2 (BTaggingEfficiencyJsonTool, IBTaggingEfficiencyJsonTool, CP::IReentrantSystematicsTool )

  public:
  BTaggingEfficiencyJsonTool( const std::string& name );
  virtual ~BTaggingEfficiencyJsonTool();
  StatusCode initialize() override;

  virtual CP::CorrectionCode getScaleFactor( const xAOD::Jet& jet, float& scalefactor, const CP::SystematicSet& sys) const override;

  // systematic stuff
  virtual CP::SystematicSet affectingSystematics() const override;
  virtual CP::SystematicSet recommendedSystematics() const override;

  private:
  bool m_initialised;

  std::string m_taggerName;
  std::string m_OP;
  std::string m_jetAuthor;
  std::string m_truthlabel;
  float m_minPt;
  float m_maxEta;

  std::string m_json_config_path;
  json m_json_config;
  std::map<int, std::string> m_labelMap;
  std::map<std::string, std::vector<float>> m_ptMap;
  std::map<std::string, std::vector<float>> m_sfMap;
  std::map<std::string, std::map<std::string, std::vector<float>>> m_sysMap;

  struct sysData {
    float xbb_syst {0};
  };
  CP::SystematicsCache<sysData> m_sysCache{this};
  const sysData* m_currentSys{nullptr};
  StatusCode calcSystematicVariation(const CP::SystematicSet& systConfig, sysData& mySys ) const;
  float getSFSys ( const std::string& label, size_t bin_index) const;

};

#endif

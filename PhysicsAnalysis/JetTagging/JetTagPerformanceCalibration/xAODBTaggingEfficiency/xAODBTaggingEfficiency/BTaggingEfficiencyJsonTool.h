/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CPBTAGGINGEFFICIENCYJSONTOOL_H
#define CPBTAGGINGEFFICIENCYJSONTOOL_H

#include "FTagAnalysisInterfaces/IBTaggingEfficiencyJsonTool.h"
#include "AsgTools/AsgTool.h"
#include <AsgTools/PropertyWrapper.h>
#include "PATInterfaces/SystematicsCache.h"
#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;

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

  Gaudi::Property<std::string> m_outputName {this, "OutputName", "", "Output name of the tagger"};
  Gaudi::Property<std::string> m_OP {this, "OperatingPoint", "", "Operating point"};
  Gaudi::Property<std::string> m_jetAuthor {this, "JetAuthor", "", "Jet collection"};
  Gaudi::Property<std::string> m_json_config_path {this, "JsonConfigFile", "", "Path to JSON config file"};
  Gaudi::Property<std::string> m_mcGenerator {this, "MCGenerator", "", "Name of the MC generator of the sample being processed (required for MC-MC scale factor)"};

  Gaudi::Property<float> m_minPt {this, "MinPt", -1 /*MeV*/, "Minimum jet pT cut (in MeV)"};
  Gaudi::Property<float> m_maxEta {this, "MaxEta", 2.5, "Maximum jet eta cut"};
 
  json m_json_config;
  std::map<int, std::string> m_labelMap;
  std::map<int, std::string> m_labelMapMCMC;

  class MCMCHandler {
    public: 
      struct varBounds {
        float lowerBound = std::numeric_limits<float>::lowest();
        float upperBound = std::numeric_limits<float>::max();
      };

      MCMCHandler(std::map<std::string, varBounds> bounds, float sf) 
        : m_varBinBounds(std::move(bounds)), m_MCMCSF(sf) {}

      bool isJetWithinBounds(const xAOD::Jet& jet,
                            const BTaggingEfficiencyJsonTool& tool) const;
      float getFactor() const { return m_MCMCSF; }
 
    private: 
      std::map<std::string,  varBounds> m_varBinBounds; 
      float m_MCMCSF;
  };
  
  // SF maps
  std::map<std::string, std::vector<float>> m_sfMap;
  std::map<std::string, std::vector<float>> m_sfPtMap;
  std::map<std::string, std::map<std::string, std::vector<float>>> m_sfSysMap;
  
  // mc-to-mc correction maps
  std::map<std::string, std::string> m_mcReference;
  std::map<std::string, std::vector<MCMCHandler>> m_mcmcHandlers;

  std::unique_ptr<SG::AuxElement::ConstAccessor<int>> m_truthLabelAcc;
  std::unique_ptr<SG::AuxElement::ConstAccessor<float>> m_massAcc;
  std::unique_ptr<SG::AuxElement::ConstAccessor<float>> m_ptAcc;

  struct sysData {
    float xbb_syst {0};
  };
  CP::SystematicsCache<sysData> m_sysCache{this};
  const sysData* m_currentSys{nullptr};
  StatusCode calcSystematicVariation(const CP::SystematicSet& systConfig, sysData& mySys ) const;
  float getSFSys ( const std::string& label, size_t bin_index) const;
  virtual CP::CorrectionCode getMCToMCCorr( const xAOD::Jet& jet, float& corr) const;
  float getJetPt( const xAOD::Jet& jet ) const;
  float getJetMass( const xAOD::Jet& jet ) const;
  float getJetQuantity( const xAOD::Jet& jet, const std::string &varName ) const;
};

#endif

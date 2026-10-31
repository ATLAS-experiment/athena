/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CPBTAGGINGLARGEREFFICIENCYJSONTOOL_H
#define CPBTAGGINGLARGEREFFICIENCYJSONTOOL_H

#include "FTagAnalysisInterfaces/IBTaggingLargeREfficiencyJsonTool.h"
#include "FTagAnalysisInterfaces/IBTaggingLargeRSelectionJsonTool.h"
#include "AsgTools/AsgTool.h"
#include <AsgTools/PropertyWrapper.h>
#include "AsgTools/ToolHandle.h"
#include "PATInterfaces/SystematicsCache.h"
#include "xAODBTaggingEfficiency/BTaggingToolUtil.h"
#include "xAODBTaggingEfficiency/BTaggingBoundsHandler.h"
#include <nlohmann/json.hpp>
#include <cmath>
#include <limits>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>



class BTaggingLargeREfficiencyJsonTool: public asg::AsgTool,
                                        virtual public IBTaggingLargeREfficiencyJsonTool
{
  // creates a proper constructor for athena
  ASG_TOOL_CLASS2 (BTaggingLargeREfficiencyJsonTool, IBTaggingLargeREfficiencyJsonTool, CP::IReentrantSystematicsTool )

  public:
  BTaggingLargeREfficiencyJsonTool( const std::string& name );
  virtual ~BTaggingLargeREfficiencyJsonTool();
  StatusCode initialize() override;

  virtual CP::CorrectionCode getScaleFactor( const xAOD::Jet& jet, float& scalefactor, const CP::SystematicSet& sys) const override;

  // systematic stuff
  virtual CP::SystematicSet affectingSystematics() const override;
  virtual CP::SystematicSet recommendedSystematics() const override;

  private:
  bool m_initialised = false;

  Gaudi::Property<std::string> m_outputName {this, "OutputName", "", "Output name of the tagger"};
  Gaudi::Property<std::string> m_OP {this, "OperatingPoint", "", "Operating point"};
  Gaudi::Property<std::string> m_jetAuthor {this, "JetAuthor", "", "Jet collection"};
  Gaudi::Property<std::string> m_json_config_path {this, "JsonConfigFile", "", "Path to JSON config file"};
  Gaudi::Property<std::string> m_mcGenerator {this, "MCGenerator", "", "Name of the MC generator of the sample being processed (required for MC-MC scale factor)"};

  // In the initialize function the min pT is converted into GeV as in the JSON files numbers are in GeV 
  Gaudi::Property<float> m_minPt {this, "MinPt", -1 /*MeV*/, "Minimum jet pT cut (in MeV)"};
  Gaudi::Property<float> m_maxEta {this, "MaxEta", 2.5, "Maximum jet eta cut"};
 
  nlohmann::ordered_json m_json_config;
  std::map<int, std::string> m_labelMap;
  std::map<int, std::string> m_labelMapMCMC;

  /// Access to a BTaggingLargeRSelectionJsonTool 
  ToolHandle<IBTaggingLargeRSelectionJsonTool> m_selTool;
  
  class LargeRMCMCHandler : public BTaggingBoundsHandler {
    public:
      LargeRMCMCHandler(const nlohmann::ordered_json& jsonConfig, JetQuantityGetter funcGetJetQuantity);
      // Prevent handler from being copied forcing passing them by (const) references or using pointers.
      // It ensures that the code is efficient memory wise as no duplicate can be created 
      // The handlers should only be stored in vectors or maps and used directly basically 
      LargeRMCMCHandler& operator=(LargeRMCMCHandler&&) noexcept = default;
      LargeRMCMCHandler(LargeRMCMCHandler&&) noexcept = default;
      LargeRMCMCHandler(const LargeRMCMCHandler&) = delete;
      LargeRMCMCHandler& operator=(const LargeRMCMCHandler&) = delete;
      virtual ~LargeRMCMCHandler() = default;

      float getScaleFactor() const { return m_MCMCSF; }

      // For printing the content of a handler
      friend std::ostream& operator<<(std::ostream& os, const LargeRMCMCHandler& handler);
      virtual std::string to_string() const override {
        std::ostringstream oss;
        oss << *this;
        return oss.str();
      }
    private:
      float m_MCMCSF = std::numeric_limits<float>::lowest();
  };

  // Declare it also here so one can use the operator<< within the BTaggingLargeREfficiencyJsonTool
  friend std::ostream& operator<<(std::ostream& os, const LargeRMCMCHandler& handler);

  // SF maps
  std::map<std::string, std::vector<float>> m_sfMap;
  std::map<std::string, std::vector<float>> m_sfPtMap;
  std::map<std::string, std::map<std::string, std::vector<float>>> m_sfSysMap;
  
  // mc-to-mc correction maps
  std::map<std::string, std::string> m_mcReference;
  std::map<std::string, std::vector<LargeRMCMCHandler>> m_mcmcHandlers;

  std::unique_ptr<SG::ConstAccessor<int>> m_truthLabelAcc;
  std::unique_ptr<SG::ConstAccessor<float>> m_massAcc;
  std::unique_ptr<SG::ConstAccessor<float>> m_ptAcc;

  struct sysData {
    float xbb_syst {0};
  };
  CP::SystematicsCache<sysData> m_sysCache{this};
  const sysData* m_currentSys{nullptr};
  StatusCode calcSystematicVariation(const CP::SystematicSet& systConfig, sysData& mySys ) const;
  float getSFSys ( const std::string& label, size_t bin_index) const;
  CP::CorrectionCode getMCToMCCorr( const xAOD::Jet& jet, float& corr) const;
  float getJetPtInGeV( const xAOD::Jet& jet ) const;
  float getJetMassInGeV( const xAOD::Jet& jet ) const;
  float getJetQuantity( const xAOD::Jet& jet, const BTaggingToolUtil::varType vType ) const;
};

#endif // CPBTAGGINGLARGEREFFICIENCYJSONTOOL_H

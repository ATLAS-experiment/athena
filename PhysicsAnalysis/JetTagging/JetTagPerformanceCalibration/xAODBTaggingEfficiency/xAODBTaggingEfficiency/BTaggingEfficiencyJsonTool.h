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
#include <ostream>
#include <sstream>
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
  bool m_initialised = false;

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

  // Parent class of all handlers defined by an N-dimensional bins
  // i.e. a set of [low, high) bounds for a list of jet variables (e.g. pT, mass, abseta)
  // It handles everything related to the bounds: parsing them from the JSON entry,
  // checking if a jet falls into the bin, ordering and overlap of bins, printing of bounds.
  // Child classes e.g. MCMCHandler only deal with the extra values stored after the bounds.
  class BoundsHandler {
    public:
      // Only child handlers can be built.
      // The entry should be a list starting with one [low, high] range per variable in varNames
      // followed by nExtraValues values handled by the child class
      // NB: it's possible to pass all additional information with only one extra value that is itself 
      // a JSON list and can hold several fields e.g. the SF and its uncertainties.
      // varNames={"pT", "mass"}, nExtraValues=1 and entry = [[250, 500], [50, 100], {"SF": 0.9, "SF_uncert_1": 0.1, "SF_uncert_2": 0.03}]
      BoundsHandler(const json& jsonConfig, const std::vector<std::string>& varNames, const size_t nExtraValues=1);
      // Prevent handler from being copied forcing passing them by (const) references or using pointers
      // It ensures that the code is efficient memory wise as no duplicate can be created
      // The handlers should only be stored in vectors or maps and used directly basically
      BoundsHandler& operator=(BoundsHandler&&) noexcept = default;
      BoundsHandler(BoundsHandler&&) noexcept = default;
      BoundsHandler(const BoundsHandler&) = delete;
      BoundsHandler& operator=(const BoundsHandler&) = delete;
      ~BoundsHandler() = default;

      bool isJetWithinBounds(const xAOD::Jet& jet,
                             const BTaggingEfficiencyJsonTool& tool) const;
      
      // Two handlers i.e. N-dimensional bins overlap if their bounds overlap
      // for every variable. As soon as one variable has non overlapping bounds the bins are disjoint
      bool overlaps(const BoundsHandler& o) const;

      // For printing the list of variables with lower and upper bounds
      // e.g. "pT: [250, 500], mass: [50, 100]"
      friend std::ostream& operator<<(std::ostream& os, const BoundsHandler& handler);
      // To get the content of the handler as a string
      // Use the operator << function
      virtual std::string to_string() const {
        std::ostringstream oss;
        oss << *this;
        return oss.str();
      }

      protected:
        // Structure for storing a min and max value for a given variable
        // The varBounds structure also defines an ordering
        // which allows to sort bins in increasing or decreasing order basically
        // Finally an overlap function allows to check if two varBounds have some overlap
        struct varBounds {
          float lowerBound = std::numeric_limits<float>::lowest();
          float upperBound = std::numeric_limits<float>::max();

          // Epsilon value to compare lowerBound and upperBound values
          // This value can be changed if needed when initializing the structure
          float varEps = 1.e-6f;
          static bool approxEqual(const float a, const float b, const float eps) {
            return std::abs(a - b) <= eps * std::max(std::abs(a), std::abs(b));
          }

          // Two half-open intervals [low, high) overlap if each one starts before the other ends.
          // Touching edges (upper = other lower) are not considered overlapping.
          bool overlaps(const varBounds& o) const;

          // Define an ordering of varBounds compare first lowerBound then upperBound
          // within tolerance
          bool operator<(const varBounds& o) const;
          bool operator>(const varBounds& o) const { return o < *this; }
          // Define also equal and not equal operator within tolerance
          bool operator==(const varBounds& o) const;
          bool operator!=(const varBounds& o) const { return !(*this == o); }
        };

        // Ordering of handlers loop over variables as stored in the map
        // and let the first variable whose bounds differ decide of the ordering
        bool operator<(const BoundsHandler& o) const;
        bool operator>(const BoundsHandler& o) const { return o < *this; }
        // Define also equal and not equal operator
        // if neither is less than the other then handlers are equal
        bool operator==(const BoundsHandler& o) const { return !(*this < o) && !(*this > o); }
        bool operator!=(const BoundsHandler& o) const { return !(*this == o); }

        std::map<std::string,  varBounds> m_varBinBounds;
  };

  class MCMCHandler : public BoundsHandler {
    public:
      MCMCHandler(const json& jsonConfig, const std::vector<std::string>& varNames);
      virtual ~MCMCHandler() = default;
      // Prevent handler from being copied forcing passing them by (const) references or using pointers.
      // It ensures that the code is efficient memory wise as no duplicate can be created 
      // The handlers should only be stored in vectors or maps and used directly basically 
      MCMCHandler& operator=(MCMCHandler&&) noexcept = default;
      MCMCHandler(MCMCHandler&&) noexcept = default;
      MCMCHandler(const MCMCHandler&) = delete;
      MCMCHandler& operator=(const MCMCHandler&) = delete;

      
      float getScaleFactor() const { return m_MCMCSF; }

      // For printing the content of a handler
      friend std::ostream& operator<<(std::ostream& os, const MCMCHandler& handler);
      virtual std::string to_string() const override {
        std::ostringstream oss;
        oss << *this;
        return oss.str();
      }
    private:
      float m_MCMCSF = std::numeric_limits<float>::lowest();
  };

  // Declare them also here so one can use the operator<< within the BTaggingEfficiencyJsonTool
  friend std::ostream& operator<<(std::ostream& os, const BoundsHandler& handler);
  friend std::ostream& operator<<(std::ostream& os, const MCMCHandler& handler);

  // SF maps
  std::map<std::string, std::vector<float>> m_sfMap;
  std::map<std::string, std::vector<float>> m_sfPtMap;
  std::map<std::string, std::map<std::string, std::vector<float>>> m_sfSysMap;
  
  // mc-to-mc correction maps
  std::map<std::string, std::string> m_mcReference;
  std::map<std::string, std::vector<MCMCHandler>> m_mcmcHandlers;

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
  float getJetPt( const xAOD::Jet& jet ) const;
  float getJetMass( const xAOD::Jet& jet ) const;
  float getJetQuantity( const xAOD::Jet& jet, const std::string &varName ) const;
};

#endif

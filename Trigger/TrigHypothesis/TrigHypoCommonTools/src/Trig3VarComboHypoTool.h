/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DECISIONHANDLING_TRIG3VARCOMBOHYPOTOOL_H
#define DECISIONHANDLING_TRIG3VARCOMBOHYPOTOOL_H

#include "DecisionHandling/ComboHypoToolBase.h"


#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include <vector>
#include <cstdint> //for unit32_t etc
#include <tuple>
#include <limits>

class Trig3VarComboHypoTool:  public ComboHypoToolBase {

 public:
  enum ComboHypoVars { UNDEF=-1, MASSWISO=0};

  Trig3VarComboHypoTool(const std::string& type,
                        const std::string& name,
                        const IInterface* parent);

  virtual StatusCode initialize() override;
 private:

  static constexpr float FLOATDEFAULT = std::numeric_limits<float>::lowest();

  /// Organise info per var selection in a struct
  struct VarInfo {
    std::string varTag{""};
    ComboHypoVars var{UNDEF};
    size_t index{0};
    std::string monToolName{""};

    bool useMin{false};
    float varMin{FLOATDEFAULT};
    bool useMax{false};
    float varMax{FLOATDEFAULT};

    bool legA_is_MET{false};
    uint32_t legA{0};
    bool legB_is_MET{false};
    uint32_t legB{0};
    bool legC_is_MET{false};
    uint32_t legC{0};

    /// Check consistency of single var config
    bool validate(std::string& errmsg) const;
    /// Generate range string for printing
    std::string rangeStr() const {
      return (useMin ? std::to_string(varMin) + " < " : "") + varTag + (useMax ? " < " + std::to_string(varMax): "");
    }
    bool test(float value) const {
      return (useMin ? value > varMin : true) && (useMax ? value < varMax : true);
    }

  };

  /// alias for convenience, will contain eta/phi/pt info
  using KineInfo = std::tuple<float,float,float>;
  using Combination = std::vector<Combo::LegDecision>;

  /// Override the ComboHypoToolBase::decide in order to optimise combination generation
  /// This is to avoid excessive combinatorics for complex multileg chains
  StatusCode decide(Combo::LegDecisionsMap& passingLegs, const EventContext& /*ctx*/) const final;

  /// Implementation of selection on individual variables
  bool executeAlgStep(const Combination& combination, const VarInfo&, std::vector<float>& values) const;
  /// Computation of the variables from the specified kinematics
  float compute(const std::tuple<KineInfo,KineInfo,KineInfo>& kinetrio, ComboHypoVars var) const;

  bool fillLegDecisions_diffLeg(std::tuple<Combo::LegDecision,Combo::LegDecision,Combo::LegDecision>& legtrio, const Combination& combination, uint32_t legA, uint32_t legB, uint32_t legC) const;
  bool fillTrioKinematics(std::tuple<KineInfo,KineInfo,KineInfo>& kinetrio, const Combination& combination, const VarInfo& varInfo) const;
  bool fillKineInfo(KineInfo& kinematics, Combo::LegDecision decision, bool isMET) const;

  /// Gaudi configuration hooks
  // flags
  Gaudi::Property<std::vector<std::string>> m_varTag_vec     {this, "Variables"  , {""}, "Variables to cut on"};
  Gaudi::Property<std::vector<bool> >       m_useMin_vec     {this, "UseMinVec"  , {false}, "Array with the apply_min_cut setting"};
  Gaudi::Property<std::vector<bool> >       m_useMax_vec     {this, "UseMaxVec"  , {false}, "Array with the apply_max_cut setting"};

  //legs
  Gaudi::Property<std::vector<uint32_t>>    m_legA_vec       {this, "LegAVec"      , {0}, "Array with the first Leg ID"};
  Gaudi::Property<std::vector<uint32_t>>    m_legB_vec       {this, "LegBVec"      , {0}, "Array with the second Leg ID"};
  Gaudi::Property<std::vector<uint32_t>>    m_legC_vec       {this, "LegCVec"      , {0}, "Array with the third Leg ID"};
  Gaudi::Property<std::vector< bool >>      m_isLegA_MET_vec {this, "IsLegA_METVec", {false}, "Array with the first Leg MET identifier"};
  Gaudi::Property<std::vector< bool >>      m_isLegB_MET_vec {this, "IsLegB_METVec", {false}, "Array with the second Leg MET identifier"};
  Gaudi::Property<std::vector< bool >>      m_isLegC_MET_vec {this, "IsLegC_METVec", {false}, "Array with the third Leg MET identifier"};

  // cuts
  Gaudi::Property<std::vector<float>>       m_varMin_vec     {this, "LowerCutVec", {FLOATDEFAULT}, "Array with the lower cut for legs pair"};
  Gaudi::Property<std::vector<float>>       m_varMax_vec     {this, "UpperCutVec", {FLOATDEFAULT}, "Array with the upper cut for legs pair"};

  // monitoring
  ToolHandleArray<GenericMonitoringTool>    m_monTool_vec    {this, "MonTools", {}, "Monitoring tools" };

  /// Internal variables for more efficient config lookup
  std::vector<VarInfo>                      m_varInfo_vec;

}; // DECISIONHANDLING_TRIG3VARCOMBOHYPOTOOL_H
#endif

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "Trig3VarComboHypoTool.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "TrigCompositeUtils/Combinators.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"

#include <Math/Vector4Dfwd.h> // PtEtaPhiM typedef
#include <Math/Vector2D.h>    // for XYVectorF

#include "xAODTrigMissingET/TrigMissingETContainer.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <stdexcept>

constexpr float invGeV = 1. / Gaudi::Units::GeV;

using namespace TrigCompositeUtils;

// Translate strings into enum values
const std::map<std::string, Trig3VarComboHypoTool::ComboHypoVars> VarMap = {
  {"masswiso", Trig3VarComboHypoTool::ComboHypoVars::MASSWISO}
};

Trig3VarComboHypoTool::Trig3VarComboHypoTool(const std::string& type,
                                             const std::string& name,
                                             const IInterface* parent)
  : ComboHypoToolBase(type, name, parent)
{ }

bool Trig3VarComboHypoTool::VarInfo::validate(std::string& errmsg) const {
  if (legA==0){
    errmsg = "legA ID not set!";
    return false;
  }
  if (legB==0){
    errmsg="legB ID not set!";
    return false;
  }
  if (legC==0){
    errmsg="legC ID not set!";
    return false;
  }
  if ((!useMin) && (!useMax)){
    errmsg="Trying to configure the Tool without setting at least one of UseMin or UseMax!";
    return false;
  }
  if (legA==legB || legA==legC || legB==legC) {
    errmsg = "Currently all legs must be different";
    return false;
  }
  return true;
}

StatusCode Trig3VarComboHypoTool::initialize() {
  ATH_MSG_DEBUG("Variable   = " << m_varTag_vec );
  ATH_MSG_DEBUG("UseCut min = " << m_useMin_vec );
  ATH_MSG_DEBUG("UseCut max = " << m_useMax_vec );
  ATH_MSG_DEBUG("varCut min = " << m_varMin_vec );
  ATH_MSG_DEBUG("varCut max = " << m_varMax_vec );
  ATH_MSG_DEBUG("LegA       = " << m_legA_vec );
  ATH_MSG_DEBUG("LegB       = " << m_legB_vec );
  ATH_MSG_DEBUG("LegC       = " << m_legC_vec );

  ATH_CHECK( m_monTool_vec.retrieve() );

  if (m_legA_vec.size() != m_legB_vec.size() || m_legA_vec.size() != m_legC_vec.size()) {
    ATH_MSG_ERROR("Trying to configure the Tool with legA/legB/legC vectors of different size!");
    return StatusCode::FAILURE;
  }
  if (m_useMin_vec.size() != m_useMax_vec.size()) {
    ATH_MSG_ERROR("Trying to configure the Tool with UseMin and UseMax vectors of different size!");
    return StatusCode::FAILURE;
  }
  if (m_legA_vec.size() != m_useMax_vec.size()) {
    ATH_MSG_ERROR("Trying to configure the Tool with legA/B and UseMax/Min vectors of different size!");
    return StatusCode::FAILURE;
  }
  if (m_varTag_vec.size() != m_useMax_vec.size()) {
    ATH_MSG_ERROR("Trying to configure the Tool with varTag and UseMax/Min(LegA/B/C) vectors of different size!");
    return StatusCode::FAILURE;
  }

  for (size_t i=0; i<m_varTag_vec.size(); ++i){
    VarInfo info;
    info.index = i;
    if(!m_monTool_vec.empty()) {
      info.monToolName = m_monTool_vec[i].name();
    }
    if (VarMap.find(m_varTag_vec[i]) == VarMap.end()){
      ATH_MSG_ERROR("The variable is not present in the ComboHypoVars list");
      return StatusCode::FAILURE;
    }
    info.varTag = (m_varTag_vec[i]);
    info.var = VarMap.at(m_varTag_vec[i]);
    //
    info.useMin = m_useMin_vec[i];
    if(info.useMin) {info.varMin=m_varMin_vec[i];}
    info.useMax = m_useMax_vec[i];
    if(info.useMax) {info.varMax=m_varMax_vec[i];}
    //
    info.legA = m_legA_vec[i];
    info.legA_is_MET = m_isLegA_MET_vec[i];
    info.legB = m_legB_vec[i];
    info.legB_is_MET = m_isLegB_MET_vec[i];
    info.legC = m_legC_vec[i];
    info.legC_is_MET = m_isLegC_MET_vec[i];

    std::string validmsg{""};
    if(!info.validate(validmsg)) {
      ATH_MSG_ERROR(validmsg);
      return StatusCode::FAILURE;
    }

    m_varInfo_vec.push_back(std::move(info));
  }
  ATH_MSG_DEBUG("Initialization completed successfully");

  return StatusCode::SUCCESS;
}

StatusCode Trig3VarComboHypoTool::decide(Combo::LegDecisionsMap& passingLegs, const EventContext& /*context*/) const {

  // if no combinations passed, then exit 
  if (passingLegs.empty()) {
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG("Looking for legs from " << decisionId() << " in the map. Map contains features for " << passingLegs.size() << " legs, which may be data for many chains.");
  for(const auto& legpair : passingLegs) {
    ATH_MSG_DEBUG("  Leg " << legpair.first << " has " << legpair.second.size() << " features");
  }

  // select the leg decisions from the map with this ID:
  std::vector<Combination> legDecisions;
  ATH_CHECK(selectLegs(passingLegs, legDecisions));

  // Track if we have at least 3 objects on the target legs that can be used for variable computation
  bool hasViableLegs{true};
  // Determine the functional leg multiplicities for combinations to generate
  std::vector<size_t> legMultiplicityForComputation(legMultiplicity().size(),0);
  for (const VarInfo& varInfo : m_varInfo_vec){
    ATH_MSG_DEBUG("Var " << varInfo.varTag << " needs legs " << varInfo.legA << ", " << varInfo.legB << ", " << varInfo.legC);

    // Assess the leg decisions and extract the relevant ones
    if (passingLegs.contains(varInfo.legA) && passingLegs.contains(varInfo.legB) && passingLegs.contains(varInfo.legC)) {
      bool goodLegA{false}, goodLegB{false}, goodLegC{false};
      int32_t iLegA = getIndexFromLeg(varInfo.legA);
      int32_t iLegB = getIndexFromLeg(varInfo.legB);
      int32_t iLegC = getIndexFromLeg(varInfo.legC);
      if ((iLegA<0) or (iLegB<0) or (iLegC<0)){
        ATH_MSG_ERROR("Trig3VarComboHypoTool::decide: Index into array is negative");
        return StatusCode::FAILURE;
      }
      goodLegA = !passingLegs[varInfo.legA].empty();
      legMultiplicityForComputation[iLegA] = std::max<size_t>(1,legMultiplicityForComputation[iLegA]);
      ATH_MSG_DEBUG("Leg " << varInfo.legA << " has " << passingLegs[varInfo.legA].size() << " features --> " << (goodLegA ? "pass" : "fail"));

      if(varInfo.legB == varInfo.legA) {
        ATH_MSG_ERROR("Trig3VarComboHypoTool::decide: legB has to be different from legA");
        return StatusCode::FAILURE;
      }
      goodLegB = !passingLegs[varInfo.legB].empty();
      ATH_MSG_DEBUG("Leg " << varInfo.legB << " has " << passingLegs[varInfo.legB].size() << " features --> " << (goodLegB ? "pass" : "fail"));
      legMultiplicityForComputation[iLegB] = std::max<size_t>(1,legMultiplicityForComputation[iLegB]);

      if(varInfo.legC == varInfo.legA) {
        ATH_MSG_ERROR("Trig3VarComboHypoTool::decide: legC has to be different from legA");
        return StatusCode::FAILURE;
      }
      if(varInfo.legC == varInfo.legB) {
        ATH_MSG_ERROR("Trig3VarComboHypoTool::decide: legC has to be different from legB");
        return StatusCode::FAILURE;
      }
      goodLegC = !passingLegs[varInfo.legC].empty();
      ATH_MSG_DEBUG("Leg " << varInfo.legC << " has " << passingLegs[varInfo.legC].size() << " features --> " << (goodLegC ? "pass" : "fail"));
      legMultiplicityForComputation[iLegC] = std::max<size_t>(1,legMultiplicityForComputation[iLegC]);

      hasViableLegs &= (goodLegA && goodLegB && goodLegC);
      if (!hasViableLegs) {
        ATH_MSG_DEBUG("Did not find at least 3 features on the target legs to compute " << varInfo.varTag);
      }
    } else {
      ATH_MSG_DEBUG(
                    "Insufficient passing legs to compute " << varInfo.varTag
                    << ", intended on (" << varInfo.legA << ", " << varInfo.legB << ", " << varInfo.legC << ")"
                    );
      hasViableLegs = false;
    }

  }

  if (!hasViableLegs) {
    ATH_MSG_DEBUG("This Trig3VarComboHypoTool cannot run in this event, this chain **REJECTS** this event.");
    eraseFromLegDecisionsMap(passingLegs);
    if (msgLvl(MSG::DEBUG)) printDebugInformation(passingLegs);
    return StatusCode::SUCCESS;
  }

  // Create and initialise the combinations generator for the requirements of this chain, given the objects available in this event.
  // Extract the features on legs not used for the decision, so they stay in the navigation
  Combination extraLegs;
  HLT::NestedUniqueCombinationGenerator nucg;
  for (size_t legindex = 0; size_t legmult : legMultiplicityForComputation){
    size_t out_of = legDecisions[legindex].size();
    if(legmult==0) {
      extraLegs.insert(extraLegs.end(),legDecisions[legindex].cbegin(),legDecisions[legindex].cend());
    } else {
      nucg.add({out_of, legmult});
      ATH_MSG_DEBUG("For leg index " << legindex << " we will be choosing any " << legmult << " Decision Objects out of " << out_of);
    }
    ++legindex;
  }

  // Iterate over all variable computations
  std::vector<Combination> passingCombinations;
  std::vector<float> values;
  values.reserve(m_varInfo_vec.size());
  size_t warnings = 0, iterations = 0;
  // Correct for the legs on which we compute with 3 features
  auto get_index_offset = [&legMultiplicityForComputation](size_t legindex) {
    size_t offset{0};
    for (auto iLeg=legMultiplicityForComputation.cbegin(); iLeg!=legMultiplicityForComputation.cbegin()+legindex; ++iLeg) {
      offset += (*iLeg)-1;
    }
    return offset;
  };
  do {
    bool lastDecision(true);
    const std::vector<size_t> combination = nucg();
    ++nucg;
    ++iterations;
    values.clear();

    // This collects all the features contributing to any variable computation
    Combination combinationToRecord;
    for (auto iVarInfo = m_varInfo_vec.cbegin(); iVarInfo!=m_varInfo_vec.cend() && lastDecision; ++iVarInfo){
      // Just the features for the current variable evaluation
      Combination combinationToCheck;

      size_t legA_index{0};
      size_t legB_index{0};
      size_t legC_index{0};

      legA_index = getIndexFromLeg(iVarInfo->legA);
      legB_index = getIndexFromLeg(iVarInfo->legB);
      legC_index = getIndexFromLeg(iVarInfo->legC);

      ATH_MSG_DEBUG(
                    "Computing " << iVarInfo->varTag << " on legs "
                    << iVarInfo->legA << " (" << legA_index << "), "
                    << iVarInfo->legB << " (" << legB_index << "), "
                    << iVarInfo->legC << " (" << legC_index << ")"
                    );
      if(iVarInfo->legA==iVarInfo->legB) {
        ATH_MSG_DEBUG(" legA == legB is not allowed");
        break;
      }
      if(iVarInfo->legA==iVarInfo->legC) {
        ATH_MSG_DEBUG(" legA == legC is not allowed");
        break;
      }
      if(iVarInfo->legB==iVarInfo->legC) {
        ATH_MSG_DEBUG(" legB == legC is not allowed");
        break;
      }

      // 1 object each on 3 legs
      Combination featureTrio = {legDecisions[legA_index][combination.at(legA_index+get_index_offset(legA_index))],
        legDecisions[legB_index][combination.at(legB_index+get_index_offset(legB_index))],
        legDecisions[legC_index][combination.at(legC_index+get_index_offset(legC_index))]};
      combinationToCheck.insert(combinationToCheck.end(),featureTrio.cbegin(),featureTrio.cend());
      combinationToRecord.insert(combinationToRecord.end(),featureTrio.cbegin(),featureTrio.cend());

      try {
        lastDecision = executeAlgStep(combinationToCheck, *iVarInfo, values);
        ATH_MSG_DEBUG("Combination " << (iterations - 1) << " decided to be " <<  (lastDecision ? "passing" : "failing") << " " << iVarInfo->varTag);
      } catch (std::exception& e) {
        ATH_MSG_ERROR(e.what());
        return StatusCode::FAILURE;
      }

      if ((iterations >= m_combinationsThresholdWarn && warnings == 0) or (iterations >= m_combinationsThresholdBreak)) {
        ATH_MSG_WARNING("Have so far processed " << iterations << " combinations for " << decisionId() << " in this event, " << passingCombinations.size() << " passing.");
        ++warnings;
        if (iterations >= m_combinationsThresholdBreak) {
          ATH_MSG_WARNING("Too many combinations! Breaking the loop at this point.");
          break;
        }
      }
    }

    // Assess the collective decision on the combination
    if (lastDecision) {
      combinationToRecord.insert(combinationToRecord.end(),extraLegs.cbegin(),extraLegs.cend());
      passingCombinations.push_back(std::move(combinationToRecord));
      if (m_modeOR == true and m_enableOverride) {
        break;
      }
    } else { // the combination failed
      if (m_modeOR == false and m_enableOverride) {
        break;
      }
    }

    // Monitoring of variables for only accepted events
    if(lastDecision && !m_monTool_vec.empty()) {
      for (const VarInfo& varInfo : m_varInfo_vec) {
        float value = values[varInfo.index];
        auto varOfAccepted  = Monitored::Scalar(m_varTag_vec[varInfo.index]+"OfAccepted", value );//varInfo->monToolName+"OfAccepted", value );
        auto monitorIt      = Monitored::Group (m_monTool_vec[varInfo.index], varOfAccepted);
        ATH_MSG_VERBOSE( varInfo.varTag << " = " << value << " is in range " << varInfo.rangeStr() << ".");
        ATH_MSG_VERBOSE("m_varTag_vec = "<< m_varTag_vec<<", values = "<<values << ", valIndex = "<< varInfo.index <<", monToolName = " << varInfo.monToolName << ", monToolVec = "<< m_monTool_vec);
      }
    }

  } while (nucg);

  if (m_modeOR) {

    ATH_MSG_DEBUG("Passing " << passingCombinations.size() << " combinations out of " << iterations << ", " 
                  << decisionId() << (passingCombinations.size() ? " **ACCEPTS**" : " **REJECTS**") << " this event based on OR logic.");

    if (m_enableOverride) {
      ATH_MSG_DEBUG("Note: stopped after the first successful combination due to the EnableOverride flag.");  
    }

  } else {  // modeAND

    const bool passAll = (passingCombinations.size() == iterations);

    ATH_MSG_DEBUG("Passing " << passingCombinations.size() << " combinations out of " << iterations << ", " 
                  << decisionId() << (passAll ? " **ACCEPTS**" : " **REJECTS**") << " this event based on AND logic.");

    if (m_enableOverride) {
      ATH_MSG_DEBUG("Note: stopped after the first failed combination due to the EnableOverride flag.");  
    }

    if (not passAll) {
      passingCombinations.clear();
    }

  }

  if (not passingCombinations.empty()) { // need partial erasure of the decsions (only those not present in any combination)
    updateLegDecisionsMap(passingCombinations, passingLegs);
  } else { // need complete erasure of input decisions
    eraseFromLegDecisionsMap(passingLegs);
  }

  if (msgLvl(MSG::DEBUG)) printDebugInformation(passingLegs);
  return StatusCode::SUCCESS;
}

bool Trig3VarComboHypoTool::executeAlgStep(const Combination& combination, const VarInfo& varInfo, std::vector<float> &vals) const {
  ATH_MSG_DEBUG("Executing selection " << varInfo.index << " of " << m_varInfo_vec.size() << ": " << varInfo.rangeStr());

  std::tuple<KineInfo,KineInfo,KineInfo> kinetrio;
  if(!fillTrioKinematics(kinetrio, combination, varInfo)) {
    ATH_MSG_ERROR("Failed to extract kinematics of feature trio!");
    return false;
  }

  if(msgLvl(MSG::VERBOSE)) {
    float etaCheck, phiCheck, ptCheck;
    std::tie(etaCheck,phiCheck,ptCheck) = std::get<0>(kinetrio);
    msg() << MSG::VERBOSE << "  Test filled legA kinematics: pt " << ptCheck*invGeV << ", eta " << etaCheck << ", phi " << phiCheck << endmsg;

    std::tie(etaCheck,phiCheck,ptCheck) = std::get<1>(kinetrio);
    msg() << MSG::VERBOSE << "  Test filled legB kinematics: pt " << ptCheck*invGeV << ", eta " << etaCheck << ", phi " << phiCheck << endmsg;

    std::tie(etaCheck,phiCheck,ptCheck) = std::get<2>(kinetrio);
    msg() << MSG::VERBOSE << "  Test filled legC kinematics: pt " << ptCheck*invGeV << ", eta " << etaCheck << ", phi " << phiCheck << endmsg;
  }

  // apply the cut
  float value = compute(kinetrio,varInfo.var);
  if(!m_monTool_vec.empty()) {
    auto varOfProcessed = Monitored::Scalar(m_varTag_vec[varInfo.index]+"OfProcessed"  , value );
    auto monitorIt      = Monitored::Group (m_monTool_vec[varInfo.index], varOfProcessed);
  }
  vals.push_back(value);
  bool pass = varInfo.test(value);

  ATH_MSG_DEBUG("  Found a combination with " << value);
  if(!pass) {
    ATH_MSG_DEBUG("  Combination failed var cut: " << varInfo.varTag << " = " << value << " not in range " << varInfo.rangeStr());
  }
  return pass;
}

/// Test function to compare decision ID with the legs to be used in var computation
bool testLegId3varcombo(const Combo::LegDecision& d, uint32_t targetleg) {
  auto combId = HLT::Identifier(d.first);
  if(!TrigCompositeUtils::isLegId(combId)) return false;
  return combId.numeric() == targetleg;
}

bool Trig3VarComboHypoTool::fillLegDecisions_diffLeg(std::tuple<Combo::LegDecision,Combo::LegDecision,Combo::LegDecision>& legtrio, const Combination& combination, uint32_t legA, uint32_t legB, uint32_t legC) const {
  // Extract the features matching the legs
  // We take all of them, so as to be able to check if there is any ambiguity
  auto isLegA = [&legA](const Combo::LegDecision& d) { return testLegId3varcombo(d,legA); };
  auto isLegB = [&legB](const Combo::LegDecision& d) { return testLegId3varcombo(d,legB); };
  auto isLegC = [&legC](const Combo::LegDecision& d) { return testLegId3varcombo(d,legC); };
  Combination legA_features, legB_features, legC_features;

  std::copy_if(combination.begin(),combination.end(),std::back_inserter(legA_features),isLegA);
  if(legA_features.size()!=1) {
    ATH_MSG_ERROR(legA_features.size() << " Decision Objects supplied on leg " << legA
                  << ", must be 1 for different-leg topo selection!");
    return false;
  }

  std::copy_if(combination.begin(),combination.end(),std::back_inserter(legB_features),isLegB);
  if (legB_features.size()!=1) {
    ATH_MSG_ERROR(legB_features.size() << " Decision Objects supplied on leg " << legB
                  << ", must be 1 for different-leg topo selection!");
    return false;
  }

  std::copy_if(combination.begin(),combination.end(),std::back_inserter(legC_features),isLegC);
  if (legC_features.size()!=1) {
    ATH_MSG_ERROR(legB_features.size() << " Decision Objects supplied on leg " << legC
                  << ", must be 1 for different-leg topo selection!");
    return false;
  }

  std::get<0>(legtrio) = legA_features[0];
  std::get<1>(legtrio) = legB_features[0];
  std::get<2>(legtrio) = legC_features[0];

  return true;
}

bool Trig3VarComboHypoTool::fillTrioKinematics(std::tuple<KineInfo,KineInfo,KineInfo>& kinetrio, const Combination& combination, const VarInfo& varInfo) const {
  ATH_MSG_VERBOSE("  Decision objects available = "<< combination);
  // Check that there are enough features
  size_t nFeatures(combination.size());
  if (nFeatures < 3){
    ATH_MSG_ERROR("Number of Decision Objects passed is less than 3! Sum over decision objects on all legs = " << combination.size() );
    return false;
  }
  std::tuple<Combo::LegDecision,Combo::LegDecision,Combo::LegDecision> legtrio;
  fillLegDecisions_diffLeg(legtrio,combination,varInfo.legA,varInfo.legB,varInfo.legC);
  ATH_MSG_VERBOSE("    Fill leg A kinematics");
  if(!fillKineInfo(std::get<0>(kinetrio),std::get<0>(legtrio),varInfo.legA_is_MET)) {
    ATH_MSG_ERROR("Failed to extract requisite kinematic info from leg " << varInfo.legA << "!");
    return false;
  }
  ATH_MSG_VERBOSE("    Fill leg B kinematics");
  if(!fillKineInfo(std::get<1>(kinetrio),std::get<1>(legtrio),varInfo.legB_is_MET)) {
    ATH_MSG_ERROR("Failed to extract requisite kinematic info from leg " << varInfo.legB << "!");
    return false;
  }
  ATH_MSG_VERBOSE("    Fill leg C kinematics");
  if(!fillKineInfo(std::get<2>(kinetrio),std::get<2>(legtrio),varInfo.legC_is_MET)) {
    ATH_MSG_ERROR("Failed to extract requisite kinematic info from leg " << varInfo.legC << "!");
    return false;
  }
  return true;
}

bool Trig3VarComboHypoTool::fillKineInfo(Trig3VarComboHypoTool::KineInfo& kinematics, Combo::LegDecision decision, bool isMET) const {
  float eta, phi, pt;
  if (isMET) {
    auto pLink = TrigCompositeUtils::findLink<xAOD::TrigMissingETContainer>( *decision.second, featureString() ).link;
    if (!pLink.isValid()){
      ATH_MSG_ERROR("link for MET not valid");
      return false;
    }
    ROOT::Math::XYVectorF metv((*pLink)->ex(),(*pLink)->ey());
    eta = FLOATDEFAULT;
    phi = metv.phi();
    pt  = metv.r();
  } else {
    auto pLink = TrigCompositeUtils::findLink<xAOD::IParticleContainer>( *decision.second, featureString() ).link;
    if (!pLink.isValid()){
      ATH_MSG_ERROR("link for IParticle not valid");
      return false;
    }
    eta = (*pLink)->p4().Eta();
    phi = (*pLink)->p4().Phi();
    pt  = (*pLink)->p4().Pt();
  }
  ATH_MSG_VERBOSE("      Filled kinematics with pt " << pt*invGeV << ", eta " << eta << ", phi " << phi);
  kinematics = std::make_tuple(eta,phi,pt);
  return true;
}

float Trig3VarComboHypoTool::compute(const std::tuple<KineInfo,KineInfo,KineInfo>& kinetrio, ComboHypoVars var) const {
  const auto& [legA_kine,legB_kine,legC_kine] = kinetrio;
  const auto& [eta1,phi1,pt1] = legA_kine;
  const auto& [eta2,phi2,pt2] = legB_kine;
  const auto& [eta3,phi3,pt3] = legC_kine;

  ATH_MSG_DEBUG("    Leg A has pt " << pt1*invGeV << ", eta " << eta1 << ", phi " << phi1);
  ATH_MSG_DEBUG("    Leg B has pt " << pt2*invGeV << ", eta " << eta2 << ", phi " << phi2);
  ATH_MSG_DEBUG("    Leg C has pt " << pt3*invGeV << ", eta " << eta3 << ", phi " << phi3);

  float value{0.0f};
  switch(var) {
    case ComboHypoVars::MASSWISO:
      {
        ROOT::Math::PtEtaPhiMVector p1(pt1,eta1,phi1,0.), p2(pt2,eta2,phi2,0.);
        value = (p1+p2).M()*invGeV; // Convert to GeV

        // require legC objects are isolated from legA and legB
        float dRAC = xAOD::P4Helpers::deltaR(eta1,phi1,eta3,phi3);
        float dRBC = xAOD::P4Helpers::deltaR(eta2,phi2,eta3,phi3);

        ATH_MSG_DEBUG("    invmAB = " << value << ", dRAC = " << dRAC << ", dRBC = " << dRBC);

        if(dRAC < 0.2 || dRBC < 0.2)
          value = -1.;

        break;
      }
    default:
      {
        ATH_MSG_ERROR("Undefined variable requested -- should never happen!");
      }
  }
  return value;
}

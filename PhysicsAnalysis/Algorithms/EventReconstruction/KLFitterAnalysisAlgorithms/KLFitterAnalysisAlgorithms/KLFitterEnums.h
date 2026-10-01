/*
    Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Oliver Majersky
/// @author Baptiste Ravina

#ifndef KLFITTERANALYSISALGORITHMS_KLFITTERENUMS_H_
#define KLFITTERANALYSISALGORITHMS_KLFITTERENUMS_H_

#include <map>
#include <sstream>

#include "KLFitter/LikelihoodBase.h"

namespace EventReco {
namespace KLFEnums {
enum class Likelihood {
  ttbar,
  ttbar_AllHad,
  ttbar_JetAngles,
  ttbar_Angular,
  ttbar_BoostedLJets,
  ttH,
  ttZTrilepton
};
enum class LeptonType { kNoLepton, kElectron, kMuon, kTriElectron, kTriMuon };

enum class JetSelectionMode {
  kLeadingThree,
  kLeadingFour,
  kLeadingFive,
  kLeadingSix,
  kLeadingSeven,
  kLeadingEight,
  // keep btag priority and non-btag priority enum values separate
  kBtagPriorityThreeJets,
  kBtagPriorityFourJets,
  kBtagPriorityFiveJets,
  kBtagPrioritySixJets,
  kBtagPrioritySevenJets,
  kBtagPriorityEightJets
};

inline const std::map<std::string, LeptonType> strToLeptonType{
    {"kNoLepton", LeptonType::kNoLepton},
    {"kElectron", LeptonType::kElectron},
    {"kMuon", LeptonType::kMuon},
    {"kTriElectron", LeptonType::kTriElectron},
    {"kTriMuon", LeptonType::kTriMuon}};

inline const std::map<std::string, Likelihood> strToLikelihood{
    {"ttbar", Likelihood::ttbar},
    {"ttbar_AllHad", Likelihood::ttbar_AllHad},
    {"ttbar_JetAngles", Likelihood::ttbar_JetAngles},
    {"ttbar_Angular", Likelihood::ttbar_Angular},
    {"ttbar_BoostedLJets", Likelihood::ttbar_BoostedLJets},
    {"ttH", Likelihood::ttH},
    {"ttZTrilepton", Likelihood::ttZTrilepton}};

inline const std::map<std::string, JetSelectionMode> strToJetSelection{
    {"kLeadingThree", JetSelectionMode::kLeadingThree},
    {"kLeadingFour", JetSelectionMode::kLeadingFour},
    {"kLeadingFive", JetSelectionMode::kLeadingFive},
    {"kLeadingSix", JetSelectionMode::kLeadingSix},
    {"kLeadingSeven", JetSelectionMode::kLeadingSeven},
    {"kLeadingEight", JetSelectionMode::kLeadingEight},
    {"kBtagPriorityThreeJets", JetSelectionMode::kBtagPriorityThreeJets},
    {"kBtagPriorityFourJets", JetSelectionMode::kBtagPriorityFourJets},
    {"kBtagPriorityFiveJets", JetSelectionMode::kBtagPriorityFiveJets},
    {"kBtagPrioritySixJets", JetSelectionMode::kBtagPrioritySixJets},
    {"kBtagPrioritySevenJets", JetSelectionMode::kBtagPrioritySevenJets},
    {"kBtagPriorityEightJets", JetSelectionMode::kBtagPriorityEightJets}};

using KLFitter::LikelihoodBase;
inline const std::map<std::string, LikelihoodBase::BtaggingMethod>
    strToBtagMethod{
        {"kNotag", LikelihoodBase::BtaggingMethod::kNotag},
        {"kVetoNoFit", LikelihoodBase::BtaggingMethod::kVetoNoFit},
        {"kVetoNoFitLight", LikelihoodBase::BtaggingMethod::kVetoNoFitLight},
        {"kVetoNoFitBoth", LikelihoodBase::BtaggingMethod::kVetoNoFitBoth},
        {"kVetoHybridNoFit", LikelihoodBase::BtaggingMethod::kVetoHybridNoFit},
        {"kWorkingPoint", LikelihoodBase::BtaggingMethod::kWorkingPoint},
        {"kVeto", LikelihoodBase::BtaggingMethod::kVeto},
        {"kVetoLight", LikelihoodBase::BtaggingMethod::kVetoLight},
        {"kVetoBoth", LikelihoodBase::BtaggingMethod::kVetoBoth}};

inline const std::map<JetSelectionMode, size_t> jetSelToNumber{
    {JetSelectionMode::kLeadingThree, 3},
    {JetSelectionMode::kLeadingFour, 4},
    {JetSelectionMode::kLeadingFive, 5},
    {JetSelectionMode::kLeadingSix, 6},
    {JetSelectionMode::kLeadingSeven, 7},
    {JetSelectionMode::kLeadingEight, 8},
    {JetSelectionMode::kBtagPriorityThreeJets, 3},
    {JetSelectionMode::kBtagPriorityFourJets, 4},
    {JetSelectionMode::kBtagPriorityFiveJets, 5},
    {JetSelectionMode::kBtagPrioritySixJets, 6},
    {JetSelectionMode::kBtagPrioritySevenJets, 7},
    {JetSelectionMode::kBtagPriorityEightJets, 8}};

template <class T>
std::string printEnumOptions(const std::map<std::string, T>& availOpts) {
  std::stringstream sstream;
  for (const auto& elem : availOpts) {
    sstream << elem.first << " ";
  }
  return sstream.str();
}
}  // namespace KLFEnums
}  // namespace EventReco

#endif

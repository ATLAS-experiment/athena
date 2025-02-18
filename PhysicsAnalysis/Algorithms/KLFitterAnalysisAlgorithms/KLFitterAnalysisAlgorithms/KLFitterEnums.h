/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Oliver Majersky
/// @author Baptiste Ravina

#ifndef KLFITTERNANALYSISALGORITHMS_KLFITTERENUMS_H_
#define KLFITTERNANALYSISALGORITHMS_KLFITTERENUMS_H_

#include <map>
#include <sstream>

#include "KLFitter/LikelihoodBase.h"

namespace EventReco {
namespace KLFEnums {
enum Likelihood {
  ttbar,
  ttbar_AllHad,
  ttbar_JetAngles,
  ttbar_Angular,
  ttbar_BoostedLJets,
  ttH,
  ttZTrilepton
};
enum LeptonType { kNoLepton, kElectron, kMuon, kTriElectron, kTriMuon };

enum JetSelectionMode {
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

static const std::map<std::string, LeptonType> strToLeptonType{
    {"kNoLepton", kNoLepton},
    {"kElectron", kElectron},
    {"kMuon", kMuon},
    {"kTriElectron", kTriElectron},
    {"kTriMuon", kTriMuon}};

static const std::map<std::string, Likelihood> strToLikelihood{
    {"ttbar", ttbar},
    {"ttbar_AllHad", ttbar_AllHad},
    {"ttbar_JetAngles", ttbar_JetAngles},
    {"ttbar_Angular", ttbar_Angular},
    {"ttbar_BoostedLJets", ttbar_BoostedLJets},
    {"ttH", ttH},
    {"ttZTrilepton", ttZTrilepton}};

static const std::map<std::string, JetSelectionMode> strToJetSelection{
    {"kLeadingThree", kLeadingThree},
    {"kLeadingFour", kLeadingFour},
    {"kLeadingFive", kLeadingFive},
    {"kLeadingSix", kLeadingSix},
    {"kLeadingSeven", kLeadingSeven},
    {"kLeadingEight", kLeadingEight},
    {"kBtagPriorityThreeJets", kBtagPriorityThreeJets},
    {"kBtagPriorityFourJets", kBtagPriorityFourJets},
    {"kBtagPriorityFiveJets", kBtagPriorityFiveJets},
    {"kBtagPrioritySixJets", kBtagPrioritySixJets},
    {"kBtagPrioritySevenJets", kBtagPrioritySevenJets},
    {"kBtagPriorityEightJets", kBtagPriorityEightJets}};

using KLFitter::LikelihoodBase;
static const std::map<std::string, LikelihoodBase::BtaggingMethod>
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

static const std::map<JetSelectionMode, size_t> jetSelToNumber{
    {kLeadingThree, 3},          {kLeadingFour, 4},
    {kLeadingFive, 5},           {kLeadingSix, 6},
    {kLeadingSeven, 7},          {kLeadingEight, 8},
    {kBtagPriorityThreeJets, 3}, {kBtagPriorityFourJets, 4},
    {kBtagPriorityFiveJets, 5},  {kBtagPrioritySixJets, 6},
    {kBtagPrioritySevenJets, 7}, {kBtagPriorityEightJets, 8}};

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

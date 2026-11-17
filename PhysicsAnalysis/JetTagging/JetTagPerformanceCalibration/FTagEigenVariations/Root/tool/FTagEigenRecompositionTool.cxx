/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FTagEigenVariations/FTagEigenRecompositionTool.h"

#include "FTagEigenVariations/CDIInput.h"
#include "FTagEigenVariations/EigenDecomposition.h"
#include "FTagEigenVariations/EigenRecomposition.h"
#include "FTagEigenVariations/EigenReduction.h"
#include "FTagEigenVariations/SystematicNames.h"

#include "CalibrationDataInterface/CalibrationDataContainer.h"
#include "CxxUtils/StringUtils.h"
#include "PathResolver/PathResolver.h"

#include "TFile.h"

#include <algorithm>
#include <format>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace {

  const std::vector<std::string> labels = {"B", "C", "T", "Light"};
  // Label of the global eigenvariations, which act on all flavours
  const std::string globalLabel = "Global";

  std::vector<std::string> splitList(const std::string& list) {
    std::vector<std::string> out;
    for (const std::string& item : CxxUtils::tokenize(list, ';')) {
      std::string name(CxxUtils::trimWhiteSpaces(item));
      if (!name.empty()) out.push_back(std::move(name));
    }
    return out;
  }

  std::string normalisedSuffix(const std::string& suffix) {
    std::string out(CxxUtils::trimWhiteSpaces(suffix));
    if (!out.empty() && !out.starts_with('_')) out.insert(0, "_");
    return out;
  }

}

StatusCode ftag::ev::FTagEigenRecompositionTool::initialize() {
  ATH_MSG_WARNING("FTagEigenRecompositionTool is not recommended until the BTaggingEfficiencyTool uses the same "
                  "eigenvector decomposition");
  if (m_strategy != "SFEigen" && m_strategy != "SFGlobalEigen") {
    ATH_MSG_ERROR("unknown SystematicsStrategy " << m_strategy.value() << ", expected SFEigen or SFGlobalEigen");
    return StatusCode::FAILURE;
  }

  const std::string path = PathResolverFindCalibFile(m_cdiFile.value());
  std::unique_ptr<TFile> file(TFile::Open(path.c_str(), "READ"));
  if (!file || file->IsZombie()) {
    ATH_MSG_ERROR("cannot open " << m_cdiFile.value());
    return StatusCode::FAILURE;
  }

  const std::map<std::string, std::string> calibrations = {{"B", m_calibrationB.value()},
                                                           {"C", m_calibrationC.value()},
                                                           {"T", m_calibrationT.value()},
                                                           {"Light", m_calibrationLight.value()}};
  const std::map<std::string, std::string> suffixes = {{"B", m_suffixB.value()},
                                                       {"C", m_suffixC.value()},
                                                       {"T", m_suffixT.value()},
                                                       {"Light", m_suffixLight.value()}};

  try {
    cdi::Settings settings;
    settings.excludeRecommended = m_excludeRecommended.value();
    const std::map<std::string, std::string> perFlavour = {
      {"B", m_excludeB.value()}, {"C", m_excludeC.value()}, {"Light", m_excludeLight.value()}};
    for (const auto& [label, list] : perFlavour) {
      std::vector<std::string>& exclusions = settings.exclusions[label];
      exclusions = splitList(m_exclude.value());
      for (std::string& name : splitList(list)) exclusions.push_back(std::move(name));
    }
    settings.reductions = {{"B", cdi::parseReduction(m_reductionB.value())},
                           {"C", cdi::parseReduction(m_reductionC.value())},
                           {"Light", cdi::parseReduction(m_reductionLight.value())}};
    // Tau jets take the charm settings, until there is a dedicated tau calibration
    settings.exclusions["T"] = settings.exclusions.at("C");
    settings.reductions["T"] = settings.reductions.at("C");
    // As in the BTaggingEfficiencyTool, the high-pT extrapolation is not decomposed for continuous working points
    if (m_operatingPoint.value().find("Continuous") != std::string::npos) {
      const std::map<std::string, std::string> highPt = {{"B", "extrapolation_pt_b_Eigen*"},
                                                         {"C", "extrapolation_pt_c_Eigen*"},
                                                         {"T", "extrapolation_pt_c_Eigen*"},
                                                         {"Light", "extrapolation_pt_l_Eigen*"}};
      for (const auto& [label, pattern] : highPt) settings.exclusions[label].push_back(pattern);
    }

    std::map<std::string, cdi::Container> containers;
    for (const std::string& label : labels) {
      const std::string name = std::format("{}/{}/{}/{}/{}_SF", m_tagger.value(), m_jetAuthor.value(),
                                           m_operatingPoint.value(), label, calibrations.at(label));
      Analysis::CalibrationDataHistogramContainer* raw = nullptr;
      file->GetObject(name.c_str(), raw);
      std::unique_ptr<Analysis::CalibrationDataHistogramContainer> container(raw);
      if (!container) {
        ATH_MSG_ERROR("no container " << name << " in " << m_cdiFile.value());
        return StatusCode::FAILURE;
      }
      containers.emplace(label, cdi::readContainer(*container, label));
    }

    // An exact name must match an uncertainty of a flavour it applies to; tau jets take the charm list
    const std::vector<std::pair<std::string, std::vector<std::string>>> userExclusions = {
      {m_exclude.value(), labels}, {m_excludeB.value(), {"B"}}, {m_excludeC.value(), {"C", "T"}},
      {m_excludeLight.value(), {"Light"}}};
    for (const auto& [list, appliesTo] : userExclusions) {
      for (const std::string& name : splitList(list)) {
        if (name.ends_with('*')) continue;
        const bool found = std::ranges::any_of(appliesTo, [&](const std::string& label) {
          const std::vector<Source>& sources = containers.at(label).sources;
          return std::ranges::find(sources, name, &Source::name) != sources.end();
        });
        if (!found) {
          ATH_MSG_ERROR("exclusion " << name << " matches no uncertainty in " << m_cdiFile.value());
          return StatusCode::FAILURE;
        }
      }
    }

    // Tau jets share the c-jet nuisance parameters, or the charm part of the global ones, unless split
    std::vector<std::string> named;
    std::ranges::copy_if(labels, std::back_inserter(named),
                         [&](const std::string& label) { return label != "T" || m_splitTauCharm; });
    const auto store = [&](const std::string& label, const std::string& suffix, const Decomposition& decomposition,
                           const Eigen::MatrixXd& variations) {
      const Recomposition recomposition = recompose(decomposition, variations);
      Entry& entry = m_entries[label];
      entry.cdiSources = decomposition.sourceNames;
      for (const std::string& name : decomposition.sourceNames) entry.table.sources.push_back(sourceName(name));
      for (Eigen::Index k = 0; k < variations.cols(); ++k) {
        entry.table.nuisanceParameters.push_back(eigenName(label, k) + suffix);
        const Eigen::VectorXd c = recomposition.coefficients.col(k);
        entry.table.coefficients.emplace_back(c.data(), c.data() + c.size());
        entry.table.describedFraction.push_back(recomposition.describedFraction(k));
      }
    };

    if (m_strategy == "SFGlobalEigen") {
      for (const auto& [label, reduction] : settings.reductions) {
        if (reduction != cdi::Reduction::Loose) {
          ATH_MSG_ERROR("SFGlobalEigen supports only the Loose reduction, " << label << " is not Loose");
          return StatusCode::FAILURE;
        }
      }
      std::vector<cdi::Container> blocks;
      for (const std::string& label : named) blocks.push_back(containers.at(label));
      const Decomposition decomposition = decompose(cdi::globalInput(blocks, settings));
      // Each global eigenvariation acts on all flavours, so it has one name for all of them, without suffix
      store(globalLabel, "", decomposition, decomposition.variations);
      ATH_MSG_INFO("SFGlobalEigen: " << decomposition.variations.cols() << " eigenvector nuisance parameters");
      return StatusCode::SUCCESS;
    }

    for (const std::string& label : named) {
      const cdi::Container& container = containers.at(label);
      const Decomposition decomposition = decompose(cdi::perFlavourInput(container, settings));
      const std::optional<Eigen::Index> nKeep = cdi::eigenvariationsToKeep(container, settings);
      const Eigen::MatrixXd variations = nKeep ? reduce(decomposition, *nKeep) : decomposition.variations;
      store(label, normalisedSuffix(suffixes.at(label)), decomposition, variations);
      ATH_MSG_INFO(label << ": " << variations.cols() << " eigenvector nuisance parameters, largest correlation change "
                         << reductionQuality(decomposition.covariance, variations).maxCorrelationChange);
    }
  } catch (const std::invalid_argument& e) {
    ATH_MSG_ERROR(e.what());
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

const ftag::ev::FTagEigenRecompositionTool::Entry&
ftag::ev::FTagEigenRecompositionTool::entry(const std::string& label) const {
  if (std::ranges::find(labels, label) == labels.end()) {
    throw std::invalid_argument(std::format("FTagEigenRecompositionTool: no recomposition for label {}", label));
  }
  if (m_strategy == "SFGlobalEigen") return m_entries.at(globalLabel);
  return m_entries.at((label == "T" && !m_splitTauCharm) ? "C" : label);
}

const ftag::ev::RecompositionTable&
ftag::ev::FTagEigenRecompositionTool::recomposition(const std::string& label) const {
  return entry(label).table;
}

std::vector<std::string>
ftag::ev::FTagEigenRecompositionTool::getListOfOriginalNuisanceParameters(const std::string& label) const {
  return entry(label).cdiSources;
}

std::map<std::string, std::map<std::string, float>>
ftag::ev::FTagEigenRecompositionTool::getCoefficientMap(const std::string& label,
                                                        const std::vector<unsigned int>& eigenIdxList) const {
  const Entry& e = entry(label);
  // An empty list means all eigenvectors
  std::vector<unsigned int> indices = eigenIdxList;
  if (indices.empty()) {
    indices.resize(e.table.nuisanceParameters.size());
    std::iota(indices.begin(), indices.end(), 0u);
  }
  std::map<std::string, std::map<std::string, float>> out;
  for (unsigned int k : indices) {
    const std::vector<float> coefficients = getCoefficients(label, k);
    const std::string& keyLabel = m_strategy == "SFGlobalEigen" ? globalLabel : label;
    std::map<std::string, float>& row = out[std::format("Eigen_{}_{}", keyLabel, k)];
    for (std::size_t s = 0; s < coefficients.size(); ++s) row[e.cdiSources[s]] = coefficients[s];
  }
  return out;
}

std::vector<float> ftag::ev::FTagEigenRecompositionTool::getCoefficients(const std::string& label,
                                                                         const unsigned int evIdx) const {
  const RecompositionTable& table = recomposition(label);
  if (evIdx >= table.coefficients.size()) {
    throw std::invalid_argument(
      std::format("FTagEigenRecompositionTool: label {} has no eigenvector {}, only {}", label, evIdx,
                  table.coefficients.size()));
  }
  return std::vector<float>(table.coefficients[evIdx].begin(), table.coefficients[evIdx].end());
}

int ftag::ev::FTagEigenRecompositionTool::getNumEigenVectors(const std::string& label) const {
  return static_cast<int>(recomposition(label).nuisanceParameters.size());
}

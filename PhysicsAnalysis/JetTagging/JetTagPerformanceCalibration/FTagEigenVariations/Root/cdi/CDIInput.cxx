/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FTagEigenVariations/CDIInput.h"

#include "CalibrationDataInterface/CalibrationDataContainer.h"
#include "CxxUtils/StringUtils.h"

#include "TH1.h"
#include "TMatrixDSym.h"
#include "TVectorD.h"

#include <algorithm>
#include <array>
#include <set>
#include <stdexcept>
#include <string_view>

namespace {

  // Not decomposed: bookkeeping entries, and "extrapolation from charm", which tau jets get as its own systematic
  const std::set<std::string> reserved = {"comment",       "result",       "combined",        "statistics",
                                          "systematics",   "MCreference",  "MChadronisation", "extrapolation",
                                          "ReducedSets",   "excluded_set", "extrapolation from charm"};

  [[noreturn]] void fail(const std::string& message) { throw std::invalid_argument("ftag::ev::cdi: " + message); }

  std::vector<std::string> exclusionPatterns(const ftag::ev::cdi::Container& container,
                                             const ftag::ev::cdi::Settings& settings) {
    std::vector<std::string> patterns;
    if (auto it = settings.exclusions.find(container.block.label); it != settings.exclusions.end()) {
      patterns = it->second;
    }
    if (settings.excludeRecommended) {
      patterns.insert(patterns.end(), container.recommendedExclusions.begin(), container.recommendedExclusions.end());
    }
    return patterns;
  }

  // A trailing "*" matches by prefix; patterns that match no source are ignored
  std::set<std::string> excludedSources(const ftag::ev::cdi::Container& container,
                                        const ftag::ev::cdi::Settings& settings) {
    std::set<std::string> excluded;
    for (const std::string& pattern : exclusionPatterns(container, settings)) {
      const bool prefix = pattern.ends_with('*');
      const std::string_view stem = std::string_view(pattern).substr(0, pattern.size() - (prefix ? 1 : 0));
      for (const ftag::ev::Source& source : container.sources) {
        if (prefix ? source.name.starts_with(stem) : source.name == stem) excluded.insert(source.name);
      }
    }
    return excluded;
  }

  // Calibrations differ in whether their source names carry this prefix
  constexpr std::string_view prefix = "FT_EFF_";

  std::string withoutPrefix(const std::string& name) {
    return name.starts_with(prefix) ? name.substr(prefix.size()) : name;
  }

}

ftag::ev::cdi::Reduction ftag::ev::cdi::parseReduction(const std::string& scheme) {
  if (scheme == "Loose") return Reduction::Loose;
  if (scheme == "Medium") return Reduction::Medium;
  if (scheme == "Tight") return Reduction::Tight;
  fail("unknown reduction scheme " + scheme);
}

ftag::ev::cdi::Container ftag::ev::cdi::readContainer(Analysis::CalibrationDataHistogramContainer& container,
                                                      const std::string& label) {
  const TH1* result = dynamic_cast<const TH1*>(container.GetValue("result"));
  if (!result) fail("container of " + label + " has no result histogram");
  const int tagWeightAxis = container.getTagWeightAxis();

  // Inner bins in ROOT's global bin order; with a tag weight axis, the bins that differ only in tag weight share a key
  Container out;
  std::vector<std::array<int, 3>> keys;
  for (int z = 1; z <= result->GetNbinsZ(); ++z) {
    for (int y = 1; y <= result->GetNbinsY(); ++y) {
      for (int x = 1; x <= result->GetNbinsX(); ++x) {
        out.histogramBins.push_back(result->GetBin(x, y, z));
        std::array<int, 3> key = {x, y, z};
        if (tagWeightAxis >= 0) key.at(tagWeightAxis) = 0;
        keys.push_back(key);
      }
    }
  }
  const auto nBins = static_cast<Eigen::Index>(out.histogramBins.size());

  // Bin groups of the sources that are not correlated between bins
  std::vector<std::size_t> uncorrelatedGroups;
  std::map<std::array<int, 3>, std::size_t> groupOfKey;
  for (const std::array<int, 3>& key : keys) {
    uncorrelatedGroups.push_back(groupOfKey.try_emplace(key, groupOfKey.size()).first->second);
  }

  bool statAsVariations = false;
  for (const std::string& name : container.listUncertainties()) {
    if (name.find("stat_np") != std::string::npos) statAsVariations = true;
    if (reserved.contains(name)) continue;
    const TH1* hist = dynamic_cast<const TH1*>(container.GetValue(name.c_str()));
    if (!hist) fail("entry " + name + " of " + label + " is not a histogram");
    if (hist->GetDimension() != result->GetDimension() || hist->GetNcells() != result->GetNcells()) {
      fail("binning of " + name + " of " + label + " differs from the result histogram");
    }
    Eigen::VectorXd shifts(nBins);
    for (Eigen::Index i = 0; i < nBins; ++i) shifts(i) = hist->GetBinContent(out.histogramBins.at(i));
    Source source{name, {shifts}, {}};
    if (!container.isBinCorrelated(name)) source.binGroups = {uncorrelatedGroups};
    out.sources.push_back(std::move(source));
  }

  out.block.label = label;
  out.block.statCovariance = Eigen::MatrixXd::Zero(nBins, nBins);
  if (const auto* stat = dynamic_cast<const TMatrixDSym*>(container.GetValue("statistics"))) {
    if (stat->GetNrows() != result->GetNcells()) fail("statistics matrix of " + label + " does not match the binning");
    for (Eigen::Index i = 0; i < nBins; ++i) {
      for (Eigen::Index j = 0; j < nBins; ++j) {
        out.block.statCovariance(i, j) = (*stat)(out.histogramBins.at(i), out.histogramBins.at(j));
      }
    }
  } else if (!statAsVariations) {
    for (Eigen::Index i = 0; i < nBins; ++i) {
      const double error = result->GetBinError(out.histogramBins.at(i));
      out.block.statCovariance(i, i) = error * error;
    }
  }

  for (const std::string& name : CxxUtils::tokenize(container.getExcludedUncertainties(), ';')) {
    const std::string_view trimmed = CxxUtils::trimWhiteSpaces(name);
    if (!trimmed.empty()) out.recommendedExclusions.emplace_back(trimmed);
  }

  if (const auto* sets = dynamic_cast<const TVectorD*>(container.GetValue("ReducedSets"))) {
    for (int i = 0; i < sets->GetNoElements(); ++i) out.reducedSets.push_back(static_cast<int>((*sets)[i]));
  }
  return out;
}

ftag::ev::DecompositionInput ftag::ev::cdi::perFlavourInput(const Container& container, const Settings& settings) {
  return {{container.block}, container.sources, excludedSources(container, settings)};
}

ftag::ev::DecompositionInput ftag::ev::cdi::globalInput(const std::vector<Container>& containers,
                                                        const Settings& settings) {
  DecompositionInput in;
  // Sources that differ only by the prefix are one source, named with the prefix
  std::map<std::string, std::size_t> indexOfSource;
  std::set<std::string> excluded;
  for (std::size_t b = 0; b < containers.size(); ++b) {
    const Container& container = containers.at(b);
    in.blocks.push_back(container.block);
    for (const Source& source : container.sources) {
      auto [it, added] = indexOfSource.try_emplace(withoutPrefix(source.name), in.sources.size());
      if (added) in.sources.push_back({source.name, std::vector<Eigen::VectorXd>(containers.size()), {}});
      Source& merged = in.sources.at(it->second);
      if (merged.shifts.at(b).size() != 0) {
        fail("container of " + container.block.label + " has " + source.name + " under two names");
      }
      if (source.name.starts_with(prefix)) merged.name = source.name;
      merged.shifts.at(b) = source.shifts.front();
    }
    for (const std::string& name : excludedSources(container, settings)) excluded.insert(withoutPrefix(name));
  }
  for (const std::string& name : excluded) in.excluded.insert(in.sources.at(indexOfSource.at(name)).name);

  // A source uncorrelated between bins in any container gets distinct groups in every block
  for (Source& source : in.sources) {
    std::vector<const std::vector<std::size_t>*> groups(containers.size(), nullptr);
    for (std::size_t b = 0; b < containers.size(); ++b) {
      for (const Source& own : containers.at(b).sources) {
        if (withoutPrefix(own.name) == withoutPrefix(source.name) && !own.binGroups.empty()) {
          groups.at(b) = &own.binGroups.front();
        }
      }
    }
    if (std::ranges::all_of(groups, [](const auto* g) { return g == nullptr; })) continue;
    source.binGroups.resize(containers.size());
    std::size_t offset = 0;
    for (std::size_t b = 0; b < containers.size(); ++b) {
      const auto nBins = static_cast<std::size_t>(source.shifts.at(b).size());
      if (nBins == 0) continue;
      std::vector<std::size_t>& ids = source.binGroups.at(b);
      ids = groups.at(b) ? *groups.at(b) : std::vector<std::size_t>(nBins, 0);
      for (std::size_t& id : ids) id += offset;
      offset = *std::ranges::max_element(ids) + 1;
    }
  }
  return in;
}

std::optional<Eigen::Index> ftag::ev::cdi::eigenvariationsToKeep(const Container& container,
                                                                const Settings& settings) {
  const std::string& label = container.block.label;
  const auto it = settings.reductions.find(label);
  const Reduction reduction = it == settings.reductions.end() ? Reduction::Loose : it->second;
  // Like the current implementation, Loose also reduces if the container stores a number for it
  const auto index = static_cast<std::size_t>(reduction);
  if (index >= container.reducedSets.size()) {
    if (reduction == Reduction::Loose) return std::nullopt;
    fail("container of " + label + " stores no number of eigenvariations for the requested reduction");
  }
  if (container.reducedSets.at(index) < 1) fail("container of " + label + " stores a number of eigenvariations below 1");
  return container.reducedSets.at(index);
}

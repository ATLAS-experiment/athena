/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Runs the current (CalibrationDataEigenVariations) and the new eigenvector decomposition side by side on the
// CDI files used in CI. Asserted where the two should agree, printed where they are expected to differ.

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "FTagEigenVariations/CDIInput.h"
#include "FTagEigenVariations/EigenDecomposition.h"
#include "FTagEigenVariations/EigenReduction.h"
#include "FTagEigenVariations/EigenRecomposition.h"

#include "CalibrationDataInterface/CalibrationDataContainer.h"
#include "CalibrationDataInterface/CalibrationDataEigenVariations.h"
#include "PathResolver/PathResolver.h"

#include "TFile.h"
#include "TH1.h"
#include "TMatrixDSym.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>
#include <format>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

using Analysis::CalibrationDataEigenVariations;
using Analysis::CalibrationDataHistogramContainer;
using ftag::ev::Decomposition;
using ftag::ev::Recomposition;
using ftag::ev::ReductionQuality;
using ftag::ev::decompose;
using ftag::ev::recompose;
using ftag::ev::reduce;
using ftag::ev::reductionQuality;
namespace cdi = ftag::ev::cdi;

namespace {

  const std::string tagger = "GN2v01";
  const std::string jetCollection = "AntiKt4EMPFlowJets";
  const std::vector<std::string> files = {"xAODBTaggingEfficiency/13TeV/MC20_2025-06-17_GN2v01_v4.root",
                                          "xAODBTaggingEfficiency/13p6TeV/MC23_2026_05_13_GN2v01_v0.root"};
  const std::vector<std::string> operatingPoints = {"Continuous", "Continuous2D"};
  const std::vector<std::string> labels = {"B", "C", "T", "Light"};
  // Relative eigenvalue gap below which eigenvectors are not unique
  constexpr double degenerate = 1e-8;

  // The exclusions BTaggingEfficiencyTool passes for continuous working points
  const std::map<std::string, std::string> highPtExclusion = {{"B", "extrapolation_pt_b_Eigen*"},
                                                              {"C", "extrapolation_pt_c_Eigen*"},
                                                              {"T", "extrapolation_pt_c_Eigen*"},
                                                              {"Light", "extrapolation_pt_l_Eigen*"}};

  struct Case {
    std::string file;
    std::string operatingPoint;
    std::string label;
    std::string name() const { return std::format("{} {} {}", file.substr(file.rfind('/') + 1), operatingPoint, label); }
  };

  std::vector<Case> cases() {
    std::vector<Case> out;
    for (const std::string& file : files) {
      for (const std::string& op : operatingPoints) {
        for (const std::string& label : labels) out.push_back({file, op, label});
      }
    }
    return out;
  }

  // The global decomposition, per file and operating point
  std::vector<Case> globalCases() {
    std::vector<Case> out;
    for (const std::string& file : files) {
      for (const std::string& op : operatingPoints) out.push_back({file, op, "Global"});
    }
    return out;
  }

  std::string testName(const ::testing::TestParamInfo<Case>& info) {
    std::string name = info.param.name();
    std::ranges::replace_if(name, [](char ch) { return !std::isalnum(ch); }, '_');
    return name;
  }

  class CompareCurrentImplementation : public ::testing::TestWithParam<Case> {
  protected:
    void SetUp() override {
      const Case& c = GetParam();
      m_path = PathResolverFindCalibFile(c.file);
      ASSERT_FALSE(m_path.empty()) << "cannot resolve " << c.file;
      m_file.reset(TFile::Open(m_path.c_str(), "READ"));
      ASSERT_TRUE(m_file && !m_file->IsZombie());
      // As in the CalibrationDataInterface tests, the container is not deleted: its histograms belong to the file
      m_file->GetObject(std::format("{}/{}/{}/{}/default_SF", tagger, jetCollection, c.operatingPoint, c.label).c_str(),
                        m_cnt);
      ASSERT_NE(m_cnt, nullptr);
      m_result = dynamic_cast<const TH1*>(m_cnt->GetValue("result"));
      ASSERT_NE(m_result, nullptr);
      m_container = cdi::readContainer(*m_cnt, c.label);
      m_settings.exclusions[c.label] = {highPtExclusion.at(c.label)};
    }

    std::unique_ptr<CalibrationDataEigenVariations> current() {
      const Case& c = GetParam();
      auto eigen = std::make_unique<CalibrationDataEigenVariations>(m_path, tagger, c.operatingPoint, jetCollection,
                                                                    m_cnt, false);
      eigen->excludeNamedUncertainty(highPtExclusion.at(c.label), m_cnt);
      return eigen;
    }

    Eigen::MatrixXd variations(CalibrationDataEigenVariations& eigen) const {
      const auto nBins = static_cast<Eigen::Index>(m_container.histogramBins.size());
      Eigen::MatrixXd out(nBins, eigen.getNumberOfEigenVariations());
      for (Eigen::Index k = 0; k < out.cols(); ++k) {
        TH1* up = nullptr;
        TH1* down = nullptr;
        eigen.getEigenvectorVariation(static_cast<unsigned int>(k), up, down);
        for (Eigen::Index i = 0; i < nBins; ++i) {
          const int bin = m_container.histogramBins.at(i);
          out(i, k) = up->GetBinContent(bin) - m_result->GetBinContent(bin);
        }
      }
      return out;
    }

    std::string m_path;
    std::unique_ptr<TFile> m_file;
    CalibrationDataHistogramContainer* m_cnt = nullptr;
    const TH1* m_result = nullptr;
    cdi::Container m_container;
    cdi::Settings m_settings;
  };

  std::string summary(const ReductionQuality& quality) {
    std::vector<double> ratio(quality.sigmaRatio.begin(), quality.sigmaRatio.end());
    std::ranges::sort(ratio);
    return std::format("sigma ratio min {:.3f} median {:.3f}, max |dcorr| {:.3f}", ratio.front(),
                       ratio.at(ratio.size() / 2), quality.maxCorrelationChange);
  }

}

TEST_P(CompareCurrentImplementation, LooseMatches) {
  const Decomposition decomposition = decompose(cdi::perFlavourInput(m_container, m_settings));
  const std::unique_ptr<CalibrationDataEigenVariations> eigen = current();

  const TMatrixDSym oldCovariance = eigen->getEigenCovarianceMatrix();
  const Eigen::Index nBins = decomposition.covariance.rows();
  double maxCovarianceDifference = 0.0;
  for (Eigen::Index i = 0; i < nBins; ++i) {
    for (Eigen::Index j = 0; j < nBins; ++j) {
      const double old = oldCovariance(m_container.histogramBins.at(i), m_container.histogramBins.at(j));
      maxCovarianceDifference = std::max(maxCovarianceDifference, std::abs(old - decomposition.covariance(i, j)));
    }
  }
  EXPECT_LE(maxCovarianceDifference, 1e-12 * decomposition.covariance.cwiseAbs().maxCoeff());

  eigen->initialize();
  const Eigen::MatrixXd oldVariations = variations(*eigen);
  const Eigen::MatrixXd& newVariations = decomposition.variations;
  const Eigen::Index nNew = newVariations.cols();
  ASSERT_GE(oldVariations.cols(), nNew);

  // The current implementation stores SF + variation as float, so variations agree to the float precision of the SF
  const double precision = 4 * std::numeric_limits<float>::epsilon() * std::max(1.0, m_result->GetMaximum());
  // Equal up to the sign, apart from (numerically) degenerate eigenvalues, where only the summed covariance is unique
  double maxDifference = 0.0;
  std::size_t nDegenerate = 0;
  std::vector<bool> unique(nNew, false);
  for (Eigen::Index first = 0, last = 0; first < nNew; first = last) {
    last = first + 1;
    while (last < nNew && decomposition.eigenvalues(last - 1) - decomposition.eigenvalues(last) <=
                            degenerate * decomposition.eigenvalues(0)) {
      ++last;
    }
    const auto oldBlock = oldVariations.middleCols(first, last - first);
    const auto newBlock = newVariations.middleCols(first, last - first);
    if (last - first == 1) {
      unique.at(first) = true;
      const double sign = oldBlock.col(0).dot(newBlock.col(0)) < 0 ? -1.0 : 1.0;
      maxDifference = std::max(maxDifference, (oldBlock - sign * newBlock).cwiseAbs().maxCoeff());
      EXPECT_LE((oldBlock - sign * newBlock).cwiseAbs().maxCoeff(), precision) << "eigenvariation " << first;
    } else {
      ++nDegenerate;
      const double scale = 2 * newBlock.cwiseAbs().maxCoeff() * (last - first);
      EXPECT_LE((oldBlock * oldBlock.transpose() - newBlock * newBlock.transpose()).cwiseAbs().maxCoeff(),
                precision * scale) << "eigenvariations " << first << " to " << last - 1;
    }
  }
  double maxExtra = 0.0;
  for (Eigen::Index k = nNew; k < oldVariations.cols(); ++k) {
    maxExtra = std::max(maxExtra, oldVariations.col(k).cwiseAbs().maxCoeff());
  }
  EXPECT_LE(maxExtra, 1e-6 * newVariations.cwiseAbs().maxCoeff());

  // Recomposition, of the unique eigenvariations; limited by the float histograms and coefficients of the current one
  std::map<std::string, std::map<std::string, float>> oldCoefficients;
  ASSERT_TRUE(eigen->EigenVectorRecomposition(GetParam().label, oldCoefficients));
  const Recomposition recomposition = recompose(decomposition, newVariations);
  double maxCoefficientDifference = 0.0;
  for (Eigen::Index k = 0; k < nNew; ++k) {
    if (!unique.at(k)) continue;
    const double sign = oldVariations.col(k).dot(newVariations.col(k)) < 0 ? -1.0 : 1.0;
    const auto& old = oldCoefficients.at(std::format("Eigen_{}_{}", GetParam().label, k));
    for (std::size_t s = 0; s < decomposition.sourceNames.size(); ++s) {
      const auto it = old.find(decomposition.sourceNames.at(s));
      if (it == old.end()) continue;
      maxCoefficientDifference =
        std::max(maxCoefficientDifference, std::abs(it->second - sign * recomposition.coefficients(s, k)));
    }
  }
  EXPECT_LE(maxCoefficientDifference, 1e-3);
  std::cout << std::format("{}: {} bins, {} sources, eigenvariations old {} new {} ({} degenerate groups), "
                           "max difference {:.1e}, largest extra old {:.1e}, max recomposition coefficient difference "
                           "{:.1e}\n", GetParam().name(), nBins, decomposition.sourceNames.size(), oldVariations.cols(),
                           nNew, nDegenerate, maxDifference, maxExtra, maxCoefficientDifference);
}

TEST_P(CompareCurrentImplementation, PrintMergeQuality) {
  const Decomposition decomposition = decompose(cdi::perFlavourInput(m_container, m_settings));
  for (const cdi::Reduction reduction : {cdi::Reduction::Medium, cdi::Reduction::Tight}) {
    m_settings.reductions[GetParam().label] = reduction;
    const Eigen::Index nKeep = cdi::eigenvariationsToKeep(m_container, m_settings).value();

    const std::unique_ptr<CalibrationDataEigenVariations> eigen = current();
    eigen->initialize();
    eigen->mergeVariationsFrom(static_cast<std::size_t>(nKeep - 1));
    const Eigen::MatrixXd oldReduced = variations(*eigen);
    const Eigen::MatrixXd newReduced = reduce(decomposition, nKeep);
    std::cout << std::format("{} {} (keep {}):\n  old {}\n  new {}\n", GetParam().name(),
                             reduction == cdi::Reduction::Medium ? "Medium" : "Tight", nKeep,
                             summary(reductionQuality(decomposition.covariance, oldReduced)),
                             summary(reductionQuality(decomposition.covariance, newReduced)));
  }
}

INSTANTIATE_TEST_SUITE_P(CIFiles, CompareCurrentImplementation, ::testing::ValuesIn(cases()), testName);

// The current global implementation is not a reference, so the global decomposition is checked against the
// covariance built directly from the sources of each flavour
class GlobalDecomposition : public ::testing::TestWithParam<Case> {};

TEST_P(GlobalDecomposition, CoversCrossFlavourCovariance) {
  const Case& c = GetParam();
  const std::string path = PathResolverFindCalibFile(c.file);
  std::unique_ptr<TFile> file(TFile::Open(path.c_str(), "READ"));
  ASSERT_TRUE(file && !file->IsZombie());
  // As the recomposition tool does without a dedicated tau decomposition: tau jets take the charm variations
  const std::vector<std::string> globalLabels = {"B", "C", "Light"};
  std::vector<cdi::Container> containers;
  cdi::Settings settings;
  for (const std::string& label : globalLabels) {
    CalibrationDataHistogramContainer* cnt = nullptr;
    file->GetObject(std::format("{}/{}/{}/{}/default_SF", tagger, jetCollection, c.operatingPoint, label).c_str(), cnt);
    ASSERT_NE(cnt, nullptr);
    containers.push_back(cdi::readContainer(*cnt, label));
    settings.exclusions[label] = {highPtExclusion.at(label)};
  }
  const Decomposition global = decompose(cdi::globalInput(containers, settings));
  const double scale = global.covariance.cwiseAbs().maxCoeff();
  EXPECT_LE((global.variations * global.variations.transpose() - global.covariance).cwiseAbs().maxCoeff(),
            1e-10 * scale);

  // Within a flavour the per-flavour covariance; between flavours only the sources both share, matched ignoring a
  // leading "FT_EFF_", while stat and sources uncorrelated between bins do not contribute
  constexpr std::string_view prefix = "FT_EFF_";
  const auto stem = [&](const std::string& name) {
    return name.starts_with(prefix) ? name.substr(prefix.size()) : name;
  };
  std::string shared;
  double maxCross = 0.0;
  for (std::size_t a = 0; a < containers.size(); ++a) {
    const cdi::Container& ca = containers.at(a);
    const auto na = static_cast<Eigen::Index>(ca.histogramBins.size());
    const std::set<std::string> excludedA = cdi::perFlavourInput(ca, settings).excluded;
    for (std::size_t b = a; b < containers.size(); ++b) {
      const cdi::Container& cb = containers.at(b);
      const auto nb = static_cast<Eigen::Index>(cb.histogramBins.size());
      Eigen::MatrixXd expected = Eigen::MatrixXd::Zero(na, nb);
      std::size_t nShared = 0;
      if (a == b) {
        expected = decompose(cdi::perFlavourInput(ca, settings)).covariance;
      } else {
        const std::set<std::string> excludedB = cdi::perFlavourInput(cb, settings).excluded;
        for (const ftag::ev::Source& sa : ca.sources) {
          if (excludedA.contains(sa.name) || !sa.binGroups.empty()) continue;
          for (const ftag::ev::Source& sb : cb.sources) {
            if (excludedB.contains(sb.name) || !sb.binGroups.empty() || stem(sb.name) != stem(sa.name)) continue;
            expected += sa.shifts.front() * sb.shifts.front().transpose();
            ++nShared;
          }
        }
        shared += std::format(" {}-{} {}", ca.block.label, cb.block.label, nShared);
        maxCross = std::max(maxCross, expected.cwiseAbs().maxCoeff());
      }
      const auto block = global.covariance.block(global.blockOffsets.at(a), global.blockOffsets.at(b), na, nb);
      EXPECT_LE((block - expected).cwiseAbs().maxCoeff(), 1e-12 * scale)
        << ca.block.label << "-" << cb.block.label;
    }
  }
  std::cout << std::format("{} {} global: {} bins, {} eigenvariations, shared sources{}, largest cross-flavour "
                           "covariance {:.1e} of {:.1e}\n", c.file.substr(c.file.rfind('/') + 1), c.operatingPoint,
                           global.covariance.rows(), global.variations.cols(), shared, maxCross, scale);
}

INSTANTIATE_TEST_SUITE_P(CIFiles, GlobalDecomposition, ::testing::ValuesIn(globalCases()), testName);

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

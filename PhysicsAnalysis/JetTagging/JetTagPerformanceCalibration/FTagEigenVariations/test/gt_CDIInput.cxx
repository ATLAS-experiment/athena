/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "FTagEigenVariations/CDIInput.h"

#include "CalibrationDataInterface/CalibrationDataContainer.h"
#include "CalibrationDataInterface/CalibrationDataEigenVariations.h"

#include "TH2D.h"
#include "TMap.h"
#include "TMatrixDSym.h"
#include "TVectorD.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

using Analysis::CalibrationDataHistogramContainer;
using ftag::ev::Decomposition;
using ftag::ev::DecompositionInput;
using ftag::ev::Source;
using ftag::ev::decompose;
using ftag::ev::cdi::Container;
using ftag::ev::cdi::Reduction;
using ftag::ev::cdi::Settings;

namespace {

  constexpr double tolerance = 1e-12;

  // A 3 (pt) x 2 (second axis) container that owns its entries, as replacing an entry deletes it anyway
  class SyntheticContainer : public ::testing::Test {
  protected:
    SyntheticContainer() { m_container.SetOwnerKeyValue(true, true); }

    void build(const std::string& secondAxis) {
      m_secondAxis = secondAxis;
      m_result = histogram("result", [](int x, int y) { return 1.0 + 0.01 * x + 0.02 * y; });
      for (int x = 1; x <= 3; ++x) {
        for (int y = 1; y <= 2; ++y) m_result->SetBinError(x, y, 0.01 * x * y);
      }
      m_container.setResult(m_result);
      add("corr", [](int x, int y) { return 0.02 * x - 0.01 * y; });
      add("uncorr", [](int x, int y) { return 0.03 / x + 0.005 * y; });
      m_container.setUncorrelated("uncorr");
      add("extrapolation", [](int, int) { return 0.5; });
      m_container.setComment("synthetic");
      m_container.setHadronisation("Pythia8EvtGen");
      m_container.setExcludedUncertainties(" corr ;");
      auto* sets = new TVectorD(2);
      (*sets)[0] = 3;
      (*sets)[1] = 2;
      m_container.setUncertainty("ReducedSets", sets);
    }

    template <typename F>
    TH2D* histogram(const std::string& name, F values) const {
      auto* hist = new TH2D(name.c_str(), name.c_str(), 3, 20.0, 80.0, 2, 0.0, 2.0);
      hist->SetDirectory(nullptr);
      hist->GetXaxis()->SetTitle("pt");
      hist->GetYaxis()->SetTitle(m_secondAxis.c_str());
      for (int x = 1; x <= 3; ++x) {
        for (int y = 1; y <= 2; ++y) hist->SetBinContent(x, y, values(x, y));
      }
      return hist;
    }

    template <typename F>
    void add(const std::string& name, F values) {
      m_container.setUncertainty(name, histogram(name, values));
    }

    double content(const std::string& name, int bin) const {
      return dynamic_cast<const TH1*>(m_container.GetValue(name.c_str()))->GetBinContent(bin);
    }

    static Source source(const Container& container, const std::string& name) {
      const auto it = std::ranges::find(container.sources, name, &Source::name);
      if (it == container.sources.end()) throw std::out_of_range(name);
      return *it;
    }

    std::string m_secondAxis;
    TH2D* m_result = nullptr;
    CalibrationDataHistogramContainer m_container{"synthetic"};
  };

  void expectMatrixNear(const Eigen::MatrixXd& actual, const Eigen::MatrixXd& expected) {
    ASSERT_EQ(actual.rows(), expected.rows());
    ASSERT_EQ(actual.cols(), expected.cols());
    if (expected.size() == 0) return;
    const double scale = std::max(1.0, expected.cwiseAbs().maxCoeff());
    EXPECT_LE((actual - expected).cwiseAbs().maxCoeff(), tolerance * scale) << "actual\n" << actual
                                                                            << "\nexpected\n" << expected;
  }

  // Covariance of the current implementation, restricted to the bins of the container
  Eigen::MatrixXd currentCovariance(CalibrationDataHistogramContainer& cnt, const Container& container,
                                    const std::vector<std::string>& excluded) {
    Analysis::CalibrationDataEigenVariations current("", "", "", "", &cnt, false);
    for (const std::string& name : excluded) current.excludeNamedUncertainty(name, &cnt);
    const TMatrixDSym cov = current.getEigenCovarianceMatrix();
    const auto n = static_cast<Eigen::Index>(container.histogramBins.size());
    Eigen::MatrixXd out(n, n);
    for (Eigen::Index i = 0; i < n; ++i) {
      for (Eigen::Index j = 0; j < n; ++j) out(i, j) = cov(container.histogramBins.at(i), container.histogramBins.at(j));
    }
    return out;
  }

}

TEST_F(SyntheticContainer, ReadsBinsAndSources) {
  build("tagweight");
  add("extrapolation from charm", [](int, int) { return 0.2; });
  const Container container = ftag::ev::cdi::readContainer(m_container, "B");

  std::vector<int> bins;
  for (int y = 1; y <= 2; ++y) {
    for (int x = 1; x <= 3; ++x) bins.push_back(m_result->GetBin(x, y));
  }
  EXPECT_EQ(container.histogramBins, bins);
  EXPECT_EQ(container.block.label, "B");
  ASSERT_EQ(container.sources.size(), 2u);

  const Source corr = source(container, "corr");
  ASSERT_EQ(corr.shifts.size(), 1u);
  for (std::size_t i = 0; i < bins.size(); ++i) {
    EXPECT_DOUBLE_EQ(corr.shifts.front()(i), content("corr", bins.at(i)));
  }
  EXPECT_TRUE(corr.binGroups.empty());
  // Uncorrelated between pt bins, correlated between tag weight bins
  const std::vector<std::vector<std::size_t>> groups = {{0, 1, 2, 0, 1, 2}};
  EXPECT_EQ(source(container, "uncorr").binGroups, groups);

  Eigen::VectorXd statVariance(6);
  for (std::size_t i = 0; i < bins.size(); ++i) statVariance(i) = std::pow(m_result->GetBinError(bins.at(i)), 2);
  expectMatrixNear(container.block.statCovariance, statVariance.asDiagonal().toDenseMatrix());

  EXPECT_EQ(container.recommendedExclusions, std::vector<std::string>{"corr"});
  EXPECT_EQ(container.reducedSets, (std::vector<int>{3, 2}));
}

TEST_F(SyntheticContainer, CovarianceMatchesCurrentImplementation) {
  build("tagweight");
  const Container container = ftag::ev::cdi::readContainer(m_container, "B");
  expectMatrixNear(decompose(ftag::ev::cdi::perFlavourInput(container, {})).covariance,
                   currentCovariance(m_container, container, {}));
}

TEST_F(SyntheticContainer, WithoutTagWeightAxisUncorrelatedMeansPerBin) {
  build("abseta");
  const Container container = ftag::ev::cdi::readContainer(m_container, "B");
  const std::vector<std::vector<std::size_t>> groups = {{0, 1, 2, 3, 4, 5}};
  EXPECT_EQ(source(container, "uncorr").binGroups, groups);
  expectMatrixNear(decompose(ftag::ev::cdi::perFlavourInput(container, {})).covariance,
                   currentCovariance(m_container, container, {}));
}

TEST_F(SyntheticContainer, StatisticsFromMatrixOrVariations) {
  build("tagweight");
  auto* stat = new TMatrixDSym(m_result->GetNcells());
  for (int i = 0; i < stat->GetNrows(); ++i) {
    for (int j = 0; j < stat->GetNrows(); ++j) (*stat)(i, j) = 1e-4 * (i == j ? 2.0 : 0.5) + 1e-6 * (i + j);
  }
  m_container.setUncertainty("statistics", stat);
  Container container = ftag::ev::cdi::readContainer(m_container, "B");
  for (std::size_t i = 0; i < container.histogramBins.size(); ++i) {
    for (std::size_t j = 0; j < container.histogramBins.size(); ++j) {
      EXPECT_DOUBLE_EQ(container.block.statCovariance(i, j),
                       (*stat)(container.histogramBins.at(i), container.histogramBins.at(j)));
    }
  }
  expectMatrixNear(decompose(ftag::ev::cdi::perFlavourInput(container, {})).covariance,
                   currentCovariance(m_container, container, {}));

  // Without a matrix, stat variations replace the bin errors
  m_container.DeleteEntry(static_cast<TPair*>(m_container.FindObject("statistics"))->Key());
  add("FT_EFF_stat_np_1", [](int x, int) { return 0.001 * x; });
  container = ftag::ev::cdi::readContainer(m_container, "B");
  expectMatrixNear(container.block.statCovariance, Eigen::MatrixXd::Zero(6, 6));
  expectMatrixNear(decompose(ftag::ev::cdi::perFlavourInput(container, {})).covariance,
                   currentCovariance(m_container, container, {}));

  // A matrix of the wrong size is an error
  m_container.setUncertainty("statistics", new TMatrixDSym(4));
  EXPECT_THROW(ftag::ev::cdi::readContainer(m_container, "B"), std::invalid_argument);
}

TEST_F(SyntheticContainer, InconsistentContainerThrows) {
  build("tagweight");
  auto* coarse = new TH2D("coarse", "coarse", 2, 20.0, 80.0, 2, 0.0, 2.0);
  coarse->SetDirectory(nullptr);
  m_container.setUncertainty("coarse", coarse);
  EXPECT_THROW(ftag::ev::cdi::readContainer(m_container, "B"), std::invalid_argument);

  m_container.setUncertainty("coarse", new TVectorD(2));
  EXPECT_THROW(ftag::ev::cdi::readContainer(m_container, "B"), std::invalid_argument);

  CalibrationDataHistogramContainer empty("empty");
  EXPECT_THROW(ftag::ev::cdi::readContainer(empty, "B"), std::invalid_argument);
}

TEST_F(SyntheticContainer, Exclusions) {
  build("tagweight");
  add("extrapolation_pt_b_Eigen_1", [](int, int) { return 0.1; });
  const Container b = ftag::ev::cdi::readContainer(m_container, "B");

  Settings settings;
  EXPECT_TRUE(ftag::ev::cdi::perFlavourInput(b, settings).excluded.empty());

  // Per flavour label; names ending in "*" match by prefix, and names that match nothing are ignored
  settings.exclusions["B"] = {"extrapolation_pt_b_Eigen*", "unc*", "nothing*", "nothing"};
  settings.exclusions["C"] = {"corr"};
  EXPECT_EQ(ftag::ev::cdi::perFlavourInput(b, settings).excluded,
            (std::set<std::string>{"extrapolation_pt_b_Eigen_1", "uncorr"}));

  settings.exclusions["B"] = {"extrapolation_pt_b_Eigen*"};
  settings.excludeRecommended = true;
  EXPECT_EQ(ftag::ev::cdi::perFlavourInput(b, settings).excluded,
            (std::set<std::string>{"corr", "extrapolation_pt_b_Eigen_1"}));
  // The current implementation ignores exclusions by exact name, hence "corr*"
  expectMatrixNear(decompose(ftag::ev::cdi::perFlavourInput(b, settings)).covariance,
                   currentCovariance(m_container, b, {"corr*", "extrapolation_pt_b_Eigen*"}));
}

TEST_F(SyntheticContainer, ReductionFromReducedSets) {
  build("tagweight");
  Container c = ftag::ev::cdi::readContainer(m_container, "C");
  Container t = ftag::ev::cdi::readContainer(m_container, "T");
  t.reducedSets = {5, 4};

  EXPECT_EQ(ftag::ev::cdi::parseReduction("Medium"), Reduction::Medium);
  EXPECT_THROW(ftag::ev::cdi::parseReduction("medium"), std::invalid_argument);

  Settings settings;
  EXPECT_EQ(ftag::ev::cdi::eigenvariationsToKeep(c, settings), std::nullopt);
  settings.reductions["C"] = Reduction::Tight;
  EXPECT_EQ(ftag::ev::cdi::eigenvariationsToKeep(c, settings), 3);
  // Per flavour label, with the numbers of the container
  settings.reductions["T"] = Reduction::Medium;
  EXPECT_EQ(ftag::ev::cdi::eigenvariationsToKeep(t, settings), 4);

  // Loose reduces only if a number is stored for it
  settings.reductions["C"] = Reduction::Loose;
  c.reducedSets = {3, 2, 6};
  EXPECT_EQ(ftag::ev::cdi::eigenvariationsToKeep(c, settings), 6);

  settings.reductions["C"] = Reduction::Medium;
  c.reducedSets = {3};
  EXPECT_THROW(ftag::ev::cdi::eigenvariationsToKeep(c, settings), std::invalid_argument);
  c.reducedSets = {3, 0};
  EXPECT_THROW(ftag::ev::cdi::eigenvariationsToKeep(c, settings), std::invalid_argument);
}

TEST_F(SyntheticContainer, GlobalInputCombinesFlavours) {
  build("tagweight");
  const Container b = ftag::ev::cdi::readContainer(m_container, "B");
  add("cOnly", [](int, int y) { return 0.01 * y; });
  Container c = ftag::ev::cdi::readContainer(m_container, "C");
  // In charm, "uncorr" is correlated between bins
  std::ranges::find(c.sources, std::string("uncorr"), &Source::name)->binGroups.clear();

  Settings settings;
  settings.exclusions["C"] = {"cOnly"};
  const DecompositionInput in = ftag::ev::cdi::globalInput({b, c}, settings);

  ASSERT_EQ(in.blocks.size(), 2u);
  EXPECT_EQ(in.blocks.at(1).label, "C");
  ASSERT_EQ(in.sources.size(), 3u);
  EXPECT_EQ(in.excluded, std::set<std::string>{"cOnly"});
  for (const Source& s : in.sources) {
    ASSERT_EQ(s.shifts.size(), 2u);
    EXPECT_EQ(s.shifts.at(0).size(), s.name == "cOnly" ? 0 : 6) << s.name;
    EXPECT_EQ(s.shifts.at(1).size(), 6) << s.name;
    if (s.name == "corr") {
      EXPECT_TRUE(s.binGroups.empty());
    }
  }
  // Uncorrelated in one flavour means uncorrelated between flavours
  const Source& uncorr = *std::ranges::find(in.sources, std::string("uncorr"), &Source::name);
  const std::vector<std::vector<std::size_t>> groups = {{0, 1, 2, 0, 1, 2}, {3, 3, 3, 3, 3, 3}};
  EXPECT_EQ(uncorr.binGroups, groups);

  const Decomposition decomposition = decompose(in);
  EXPECT_EQ(std::ranges::count(decomposition.sourceNames, std::string("uncorr_bin3")), 1);
  EXPECT_EQ(std::ranges::count(decomposition.sourceNames, std::string("corr")), 1);

  // An exclusion of one flavour removes the source from all, one that matches nothing is ignored
  settings.exclusions = {{"B", {"corr", "cOnly"}}};
  EXPECT_EQ(ftag::ev::cdi::globalInput({b, c}, settings).excluded, std::set<std::string>{"corr"});
}

TEST_F(SyntheticContainer, GlobalInputMatchesPrefixedNames) {
  build("tagweight");
  const Container c = ftag::ev::cdi::readContainer(m_container, "C");
  Container light = ftag::ev::cdi::readContainer(m_container, "Light");
  for (Source& s : light.sources) s.name.insert(0, "FT_EFF_");

  Settings settings;
  settings.exclusions["C"] = {"corr"};
  const DecompositionInput in = ftag::ev::cdi::globalInput({c, light}, settings);

  ASSERT_EQ(in.sources.size(), 2u);
  for (const Source& s : in.sources) {
    EXPECT_TRUE(s.name.starts_with("FT_EFF_")) << s.name;
    EXPECT_EQ(s.shifts.at(0).size(), 6) << s.name;
    EXPECT_EQ(s.shifts.at(1).size(), 6) << s.name;
  }
  EXPECT_EQ(in.excluded, std::set<std::string>{"FT_EFF_corr"});
  const Source& uncorr = *std::ranges::find(in.sources, std::string("FT_EFF_uncorr"), &Source::name);
  const std::vector<std::vector<std::size_t>> groups = {{0, 1, 2, 0, 1, 2}, {3, 4, 5, 3, 4, 5}};
  EXPECT_EQ(uncorr.binGroups, groups);

  // The same source under both names in one container is ambiguous
  light.sources.push_back(source(c, "corr"));
  EXPECT_THROW(ftag::ev::cdi::globalInput({c, light}, settings), std::invalid_argument);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

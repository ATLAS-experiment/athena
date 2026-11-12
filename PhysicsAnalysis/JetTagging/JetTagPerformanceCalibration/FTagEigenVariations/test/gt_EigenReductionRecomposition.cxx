/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "FTagEigenVariations/EigenDecomposition.h"
#include "FTagEigenVariations/EigenReduction.h"
#include "FTagEigenVariations/EigenRecomposition.h"
#include "FTagEigenVariations/SystematicNames.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

using ftag::ev::Decomposition;
using ftag::ev::DecompositionInput;
using ftag::ev::FlavourBlock;
using ftag::ev::Recomposition;
using ftag::ev::ReductionQuality;
using ftag::ev::decompose;
using ftag::ev::recompose;
using ftag::ev::reduce;
using ftag::ev::reductionQuality;

namespace {

  constexpr double tolerance = 1e-12;

  Eigen::VectorXd vec(const std::vector<double>& values) {
    return Eigen::Map<const Eigen::VectorXd>(values.data(), static_cast<Eigen::Index>(values.size()));
  }

  FlavourBlock block(const std::string& label, const std::vector<double>& statErrors) {
    return {label, vec(statErrors).array().square().matrix().asDiagonal()};
  }

  void expectMatrixNear(const Eigen::MatrixXd& actual, const Eigen::MatrixXd& expected) {
    ASSERT_EQ(actual.rows(), expected.rows());
    ASSERT_EQ(actual.cols(), expected.cols());
    if (expected.size() == 0) return;
    const double scale = std::max(1.0, expected.cwiseAbs().maxCoeff());
    EXPECT_LE((actual - expected).cwiseAbs().maxCoeff(), tolerance * scale) << "actual\n" << actual
                                                                            << "\nexpected\n" << expected;
  }

  // Five bins over two flavours, with a correlated, a grouped and a single-flavour source
  DecompositionInput input(double statScale) {
    DecompositionInput in;
    in.blocks = {block("B", {statScale * 0.10, statScale * 0.20, statScale * 0.15}),
                 block("C", {statScale * 0.25, statScale * 0.05})};
    in.sources = {
      {"shared", {vec({0.3, 0.2, -0.1}), vec({0.1, 0.4})}, {}},
      {"grouped", {vec({0.2, 0.1, 0.3}), vec({0.15, 0.05})}, {{0, 0, 1}, {0, 1}}},
      {"bOnly", {vec({0.02, -0.3, 0.1}), {}}, {}},
    };
    return in;
  }

}

TEST(EigenReduction, MergeKeepsBinVariance) {
  const Decomposition decomposition = decompose(input(1.0));
  const Eigen::Index nAll = decomposition.variations.cols();
  ASSERT_EQ(nAll, 5);

  for (Eigen::Index nKeep = 1; nKeep < nAll; ++nKeep) {
    const Eigen::MatrixXd reduced = reduce(decomposition, nKeep);
    ASSERT_EQ(reduced.cols(), nKeep);
    expectMatrixNear(reduced.leftCols(nKeep - 1), decomposition.variations.leftCols(nKeep - 1));
    expectMatrixNear((reduced * reduced.transpose()).diagonal(), decomposition.covariance.diagonal());
    const ReductionQuality quality = reductionQuality(decomposition.covariance, reduced);
    expectMatrixNear(quality.sigmaRatio, Eigen::VectorXd::Ones(5));
    // The merged variation follows the sign of the leading merged eigenvariation in every bin
    const Eigen::VectorXd leading = decomposition.variations.col(nKeep - 1);
    ASSERT_LT(leading.minCoeff(), 0.0);
    for (Eigen::Index u = 0; u < leading.size(); ++u) {
      EXPECT_EQ(std::signbit(reduced(u, nKeep - 1)), std::signbit(leading(u))) << "nKeep " << nKeep << " bin " << u;
    }
  }
}

TEST(EigenReduction, KnownAnswerMerge) {
  // Two uncorrelated bins: eigenvariations (0, 4) and (3, 0) merge into the fully correlated (3, 4)
  DecompositionInput in;
  in.blocks = {block("B", {0.0, 0.0})};
  in.sources = {{"uncorrelated", {vec({3.0, 4.0})}, {{0, 1}}}};
  const Decomposition decomposition = decompose(in);
  const Eigen::MatrixXd reduced = reduce(decomposition, 1);

  expectMatrixNear(reduced, vec({3.0, 4.0}));
  const ReductionQuality quality = reductionQuality(decomposition.covariance, reduced);
  expectMatrixNear(quality.sigmaRatio, vec({1.0, 1.0}));
  EXPECT_NEAR(quality.maxCorrelationChange, 1.0, tolerance);
}

TEST(EigenReduction, KeepingAllChangesNothing) {
  const Decomposition decomposition = decompose(input(1.0));
  for (const Eigen::Index nKeep : {Eigen::Index{5}, Eigen::Index{8}}) {
    EXPECT_EQ(reduce(decomposition, nKeep), decomposition.variations);
  }
  const ReductionQuality quality = reductionQuality(decomposition.covariance, decomposition.variations);
  expectMatrixNear(quality.sigmaRatio, Eigen::VectorXd::Ones(5));
  EXPECT_LE(quality.maxCorrelationChange, tolerance);
}

TEST(EigenReduction, QualityShowsLostVariance) {
  // Dropping the smallest eigenvariation, instead of merging it, loses uncertainty
  const Decomposition decomposition = decompose(input(1.0));
  const ReductionQuality quality = reductionQuality(decomposition.covariance, decomposition.variations.leftCols(4));
  EXPECT_LT(quality.sigmaRatio.minCoeff(), 1.0 - 1e-6);
  EXPECT_LE(quality.sigmaRatio.maxCoeff(), 1.0 + tolerance);
}

TEST(EigenReduction, InvalidArgumentsThrow) {
  const Decomposition decomposition = decompose(input(1.0));
  EXPECT_THROW(reduce(decomposition, 0), std::invalid_argument);
  EXPECT_THROW(reductionQuality(decomposition.covariance, Eigen::MatrixXd::Zero(4, 2)), std::invalid_argument);
}

TEST(EigenRecomposition, KnownAnswer) {
  DecompositionInput in;
  in.blocks = {block("B", {0.0, 0.0})};
  in.sources = {{"a", {vec({3.0, 0.0})}, {}}, {"b", {vec({0.0, 4.0})}, {}}};
  const Decomposition decomposition = decompose(in);
  const Recomposition result = recompose(decomposition, decomposition.variations);

  // Eigenvariations (0, 4) and (3, 0): a is the second, b the first
  Eigen::MatrixXd expected(2, 2);
  expected << 0.0, 1.0,
              1.0, 0.0;
  expectMatrixNear(result.coefficients, expected);
  expectMatrixNear(result.describedFraction, vec({1.0, 1.0}));
}

TEST(EigenRecomposition, ExactWithoutStat) {
  const Decomposition decomposition = decompose(input(0.0));
  const Recomposition result = recompose(decomposition, decomposition.variations);

  ASSERT_EQ(result.coefficients.rows(), static_cast<Eigen::Index>(decomposition.sourceNames.size()));
  expectMatrixNear(decomposition.variations * result.coefficients.transpose(), decomposition.shifts);
  expectMatrixNear(result.describedFraction, Eigen::VectorXd::Ones(decomposition.variations.cols()));
}

TEST(EigenRecomposition, StatIsNotDescribed) {
  const Decomposition decomposition = decompose(input(1.0));
  const Recomposition result = recompose(decomposition, decomposition.variations);

  // Without reduction, the fraction is 1 - u^T V_stat u / lambda for eigenvector u
  const Eigen::MatrixXd stat = decomposition.covariance - decomposition.shifts * decomposition.shifts.transpose();
  for (Eigen::Index k = 0; k < decomposition.variations.cols(); ++k) {
    const Eigen::VectorXd e = decomposition.variations.col(k);
    const double lambda = decomposition.eigenvalues(k);
    EXPECT_NEAR(result.describedFraction(k), 1.0 - e.dot(stat * e) / (lambda * lambda), tolerance);
  }
}

TEST(EigenRecomposition, LeastSquaresAfterMerge) {
  const Decomposition decomposition = decompose(input(1.0));
  const Eigen::MatrixXd reduced = reduce(decomposition, 2);
  const Recomposition result = recompose(decomposition, reduced);

  // The part of the shifts the reduced variations do not describe is orthogonal to them
  const Eigen::MatrixXd residual = decomposition.shifts - reduced * result.coefficients.transpose();
  expectMatrixNear(reduced.transpose() * residual, Eigen::MatrixXd::Zero(2, residual.cols()));

  // Keeping all is the same as no reduction
  const Recomposition all = recompose(decomposition, reduce(decomposition, 5));
  expectMatrixNear(all.coefficients, recompose(decomposition, decomposition.variations).coefficients);

  EXPECT_THROW(recompose(decomposition, Eigen::MatrixXd::Zero(4, 2)), std::invalid_argument);
}

TEST(SystematicNames, MatchBTaggingEfficiencyTool) {
  EXPECT_EQ(ftag::ev::eigenName("B", 3), "FT_EFF_Eigen_B_3");
  EXPECT_EQ(ftag::ev::sourceName("ttbar PS"), "FT_EFF_ttbar_PS");
  EXPECT_EQ(ftag::ev::sourceName("ttbar PS_bin3"), "FT_EFF_ttbar_PS_bin3");
  EXPECT_EQ(ftag::ev::sourceName("extrapolation"), "FT_EFF_extrapolation");
  EXPECT_EQ(ftag::ev::sourceName("FT_EFF_already prefixed"), "FT_EFF_already_prefixed");
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

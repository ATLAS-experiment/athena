/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "FTagEigenVariations/EigenDecomposition.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

using ftag::ev::Decomposition;
using ftag::ev::DecompositionInput;
using ftag::ev::FlavourBlock;
using ftag::ev::Source;

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
    const double scale = std::max(1.0, expected.cwiseAbs().maxCoeff());
    EXPECT_LE((actual - expected).cwiseAbs().maxCoeff(), tolerance * scale) << "actual\n" << actual
                                                                            << "\nexpected\n" << expected;
  }

  // Two flavours with a shared source, a single-flavour source and a source grouped across both flavours
  DecompositionInput twoFlavourInput() {
    DecompositionInput input;
    input.blocks = {block("B", {0.10, 0.20, 0.15, 0.30}), block("C", {0.25, 0.05, 0.40})};
    input.sources = {
      {"shared", {vec({0.3, 0.2, -0.1, 0.05}), vec({0.1, 0.4, 0.2})}, {}},
      {"bOnly", {vec({0.02, -0.3, 0.1, 0.2}), {}}, {}},
      {"grouped", {vec({0.2, 0.1, 0.3, -0.2}), vec({0.15, 0.05, 0.1})}, {{0, 0, 1, 1}, {0, 1, 1}}},
    };
    return input;
  }

}

TEST(EigenDecomposition, KnownAnswerTwoBins) {
  DecompositionInput input;
  input.blocks = {block("B", {1.0, 1.0})};
  input.sources = {{"syst", {vec({3.0, 4.0})}, {}}};
  const Decomposition result = decompose(input);

  // Covariance [[10, 12], [12, 17]] has eigenvectors (3, 4)/5 and (4, -3)/5
  expectMatrixNear(result.eigenvalues, vec({26.0, 1.0}));
  expectMatrixNear(result.variations.col(0), std::sqrt(26.0) * vec({0.6, 0.8}));
  expectMatrixNear(result.variations.col(1), vec({0.8, -0.6}));
}

TEST(EigenDecomposition, JointBinsAndSourceNames) {
  const Decomposition result = decompose(twoFlavourInput());

  EXPECT_EQ(result.blockOffsets, (std::vector<Eigen::Index>{0, 4}));
  EXPECT_EQ(result.sourceNames, (std::vector<std::string>{"shared", "bOnly", "grouped_bin0", "grouped_bin1"}));
  Eigen::MatrixXd expectedShifts(7, 4);
  expectedShifts << 0.3, 0.02, 0.2, 0.0,
                    0.2, -0.3, 0.1, 0.0,
                    -0.1, 0.1, 0.0, 0.3,
                    0.05, 0.2, 0.0, -0.2,
                    0.1, 0.0, 0.15, 0.0,
                    0.4, 0.0, 0.0, 0.05,
                    0.2, 0.0, 0.0, 0.1;
  expectMatrixNear(result.shifts, expectedShifts);

  const Eigen::VectorXd statErrors = vec({0.10, 0.20, 0.15, 0.30, 0.25, 0.05, 0.40});
  const Eigen::MatrixXd stat = statErrors.array().square().matrix().asDiagonal();
  expectMatrixNear(result.covariance, stat + expectedShifts * expectedShifts.transpose());
}

TEST(EigenDecomposition, VariationsCloseAndAreOrthogonal) {
  const Decomposition result = decompose(twoFlavourInput());

  ASSERT_EQ(result.eigenvalues.size(), 7);
  expectMatrixNear(result.variations * result.variations.transpose(), result.covariance);
  const Eigen::MatrixXd gram = result.variations.transpose() * result.variations;
  expectMatrixNear(gram, result.eigenvalues.asDiagonal().toDenseMatrix());
}

TEST(EigenDecomposition, OrderAndSignConvention) {
  const Decomposition result = decompose(twoFlavourInput());

  for (Eigen::Index k = 1; k < result.eigenvalues.size(); ++k) {
    EXPECT_GT(result.eigenvalues(k - 1), result.eigenvalues(k));
  }
  for (Eigen::Index k = 0; k < result.variations.cols(); ++k) {
    Eigen::Index largest = 0;
    result.variations.col(k).cwiseAbs().maxCoeff(&largest);
    EXPECT_GT(result.variations(largest, k), 0.0);
  }

  // Flipping the sign of every source leaves the eigenvariations unchanged
  DecompositionInput flipped = twoFlavourInput();
  for (Source& source : flipped.sources) {
    for (Eigen::VectorXd& shift : source.shifts) shift = -shift;
  }
  expectMatrixNear(decompose(flipped).variations, result.variations);
}

TEST(EigenDecomposition, BinGroupsAreUncorrelated) {
  DecompositionInput input;
  input.blocks = {block("B", {0.0, 0.0})};
  input.sources = {{"uncorrelated", {vec({3.0, 4.0})}, {{0, 1}}}};
  const Decomposition result = decompose(input);

  EXPECT_EQ(result.sourceNames, (std::vector<std::string>{"uncorrelated_bin0", "uncorrelated_bin1"}));
  expectMatrixNear(result.covariance, vec({9.0, 16.0}).asDiagonal().toDenseMatrix());
  expectMatrixNear(result.eigenvalues, vec({16.0, 9.0}));
  expectMatrixNear(result.variations.col(0), vec({0.0, 4.0}));
  expectMatrixNear(result.variations.col(1), vec({3.0, 0.0}));
}

TEST(EigenDecomposition, SharedSourceCorrelatesFlavours) {
  DecompositionInput input;
  input.blocks = {block("B", {0.0, 0.0}), block("C", {0.0})};
  input.sources = {{"shared", {vec({1.0, 2.0}), vec({-3.0})}, {}}};
  const Decomposition result = decompose(input);

  expectMatrixNear(result.eigenvalues, vec({14.0}));
  expectMatrixNear(result.variations.col(0), vec({-1.0, -2.0, 3.0}));
}

TEST(EigenDecomposition, IndependentFlavoursMatchSeparateDecompositions) {
  DecompositionInput joint;
  joint.blocks = {block("B", {0.1, 0.2, 0.15}), block("C", {0.3, 0.05})};
  joint.sources = {
    {"b1", {vec({0.3, 0.2, -0.1}), {}}, {}},
    {"b2", {vec({0.05, -0.25, 0.4}), {}}, {}},
    {"c1", {{}, vec({0.5, 0.35})}, {}},
  };
  DecompositionInput onlyB;
  onlyB.blocks = {joint.blocks[0]};
  onlyB.sources = {{"b1", {joint.sources[0].shifts[0]}, {}}, {"b2", {joint.sources[1].shifts[0]}, {}}};
  DecompositionInput onlyC;
  onlyC.blocks = {joint.blocks[1]};
  onlyC.sources = {{"c1", {joint.sources[2].shifts[1]}, {}}};

  const Decomposition result = decompose(joint);
  const Decomposition resultB = decompose(onlyB);
  const Decomposition resultC = decompose(onlyC);

  // Match each separate eigenvariation to the joint one with the same eigenvalue
  ASSERT_EQ(result.eigenvalues.size(), resultB.eigenvalues.size() + resultC.eigenvalues.size());
  const auto expectEmbedded = [&result](const Decomposition& separate, Eigen::Index offset) {
    for (Eigen::Index k = 0; k < separate.eigenvalues.size(); ++k) {
      Eigen::Index match = 0;
      (result.eigenvalues.array() - separate.eigenvalues(k)).abs().minCoeff(&match);
      EXPECT_NEAR(result.eigenvalues(match), separate.eigenvalues(k), tolerance);
      Eigen::VectorXd expected = Eigen::VectorXd::Zero(result.covariance.rows());
      expected.segment(offset, separate.variations.rows()) = separate.variations.col(k);
      expectMatrixNear(result.variations.col(match), expected);
    }
  };
  expectEmbedded(resultB, 0);
  expectEmbedded(resultC, 3);
}

TEST(EigenDecomposition, ThresholdIsRelative) {
  // Two sources in three bins without stat give rank two at any overall scale
  for (const double scale : {1.0, 1e-4, 1e4}) {
    DecompositionInput input;
    input.blocks = {block("B", {0.0, 0.0, 0.0})};
    input.sources = {
      {"a", {scale * vec({0.3, 0.2, -0.1})}, {}},
      {"b", {scale * vec({0.05, -0.25, 0.4})}, {}},
    };
    const Decomposition result = decompose(input);
    EXPECT_EQ(result.eigenvalues.size(), 2) << "scale " << scale;
    expectMatrixNear(result.variations * result.variations.transpose(), result.covariance);
  }
}

TEST(EigenDecomposition, EmptyBinsStayZero) {
  DecompositionInput input;
  input.blocks = {block("B", {0.1, 0.0, 0.2})};
  input.sources = {{"syst", {vec({0.3, 0.0, 0.1})}, {}}};
  const Decomposition result = decompose(input);

  ASSERT_EQ(result.eigenvalues.size(), 2);
  for (Eigen::Index k = 0; k < result.variations.cols(); ++k) EXPECT_EQ(result.variations(1, k), 0.0);
}

TEST(EigenDecomposition, ExcludedSourcesStayOut) {
  DecompositionInput input = twoFlavourInput();
  input.excluded = {"bOnly"};
  const Decomposition result = decompose(input);

  EXPECT_EQ(result.sourceNames, (std::vector<std::string>{"shared", "grouped_bin0", "grouped_bin1"}));
  expectMatrixNear(result.variations * result.variations.transpose(), result.covariance);
}

TEST(EigenDecomposition, InconsistentInputThrows) {
  const auto expectThrow = [](const auto& modify) {
    DecompositionInput input = twoFlavourInput();
    modify(input);
    EXPECT_THROW(decompose(input), std::invalid_argument);
  };
  expectThrow([](DecompositionInput& in) { in.blocks.clear(); });
  expectThrow([](DecompositionInput& in) { in.blocks[0].statCovariance = Eigen::MatrixXd::Zero(4, 3); });
  expectThrow([](DecompositionInput& in) { in.sources[1].name = "shared"; });
  expectThrow([](DecompositionInput& in) { in.sources[0].shifts.pop_back(); });
  expectThrow([](DecompositionInput& in) { in.sources[0].shifts[1] = vec({0.1, 0.2}); });
  expectThrow([](DecompositionInput& in) { in.sources[1].shifts[0] = Eigen::VectorXd(); });
  expectThrow([](DecompositionInput& in) { in.sources[2].binGroups.pop_back(); });
  expectThrow([](DecompositionInput& in) { in.sources[2].binGroups[1] = {0, 1}; });
  expectThrow([](DecompositionInput& in) { in.excluded = {"unknown"}; });
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

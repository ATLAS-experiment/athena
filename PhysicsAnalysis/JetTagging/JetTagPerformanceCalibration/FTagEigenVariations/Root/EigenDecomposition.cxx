/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FTagEigenVariations/EigenDecomposition.h"

#include <cmath>
#include <format>
#include <map>
#include <stdexcept>
#include <utility>

namespace {

  using ftag::ev::DecompositionInput;
  using ftag::ev::FlavourBlock;
  using ftag::ev::Source;

  [[noreturn]] void fail(const std::string& message) {
    throw std::invalid_argument("ftag::ev::decompose: " + message);
  }

  void validate(const DecompositionInput& input) {
    if (input.blocks.empty()) fail("no flavour blocks");
    for (const FlavourBlock& block : input.blocks) {
      const Eigen::MatrixXd& stat = block.statCovariance;
      if (stat.rows() == 0 || stat.rows() != stat.cols()) {
        fail(std::format("block {}: stat covariance is not square and non-empty", block.label));
      }
    }

    const std::size_t nBlocks = input.blocks.size();
    std::set<std::string> names;
    for (const Source& source : input.sources) {
      if (!names.insert(source.name).second) fail(std::format("duplicate source {}", source.name));
      if (source.shifts.size() != nBlocks) {
        fail(std::format("source {}: {} shifts for {} blocks", source.name, source.shifts.size(), nBlocks));
      }
      if (!source.binGroups.empty() && source.binGroups.size() != nBlocks) {
        fail(std::format("source {}: {} bin groups for {} blocks", source.name, source.binGroups.size(), nBlocks));
      }
      bool affectsAnyBlock = false;
      for (std::size_t b = 0; b < nBlocks; ++b) {
        const Eigen::Index nShift = source.shifts[b].size();
        const FlavourBlock& block = input.blocks[b];
        if (nShift != 0 && nShift != block.statCovariance.rows()) {
          fail(std::format("source {}, block {}: {} shifts for {} bins",
                           source.name, block.label, nShift, block.statCovariance.rows()));
        }
        if (!source.binGroups.empty() && static_cast<Eigen::Index>(source.binGroups[b].size()) != nShift) {
          fail(std::format("source {}, block {}: {} group ids for {} shifts",
                           source.name, block.label, source.binGroups[b].size(), nShift));
        }
        affectsAnyBlock = affectsAnyBlock || nShift != 0;
      }
      if (!affectsAnyBlock) fail(std::format("source {} affects no block", source.name));
    }

    for (const std::string& name : input.excluded) {
      if (!names.contains(name)) fail(std::format("excluded source {} is not an input source", name));
    }
  }

}

ftag::ev::Decomposition ftag::ev::decompose(const DecompositionInput& input) {
  validate(input);

  Decomposition result;
  Eigen::Index nBins = 0;
  for (const FlavourBlock& block : input.blocks) {
    result.blockOffsets.push_back(nBins);
    nBins += block.statCovariance.rows();
  }

  // One column per source, or per bin group of a grouped source
  std::vector<Eigen::VectorXd> columns;
  for (const Source& source : input.sources) {
    if (input.excluded.contains(source.name)) continue;
    std::map<std::size_t, Eigen::VectorXd> groups;
    for (std::size_t b = 0; b < input.blocks.size(); ++b) {
      const Eigen::VectorXd& shift = source.shifts[b];
      for (Eigen::Index i = 0; i < shift.size(); ++i) {
        const std::size_t id = source.binGroups.empty() ? 0 : source.binGroups[b][i];
        auto [group, inserted] = groups.try_emplace(id, Eigen::VectorXd::Zero(nBins));
        group->second(result.blockOffsets[b] + i) = shift(i);
      }
    }
    for (auto& [id, column] : groups) {
      result.sourceNames.push_back(source.binGroups.empty() ? source.name : std::format("{}_bin{}", source.name, id));
      columns.push_back(std::move(column));
    }
  }

  result.shifts.resize(nBins, static_cast<Eigen::Index>(columns.size()));
  for (Eigen::Index c = 0; c < result.shifts.cols(); ++c) result.shifts.col(c) = columns[c];

  result.covariance = result.shifts * result.shifts.transpose();
  for (std::size_t b = 0; b < input.blocks.size(); ++b) {
    const Eigen::MatrixXd& stat = input.blocks[b].statCovariance;
    result.covariance.block(result.blockOffsets[b], result.blockOffsets[b], stat.rows(), stat.cols()) += stat;
  }

  // Bins without any uncertainty stay out of the solve, so their entries are exactly zero
  std::vector<Eigen::Index> active;
  for (Eigen::Index i = 0; i < nBins; ++i) {
    if (result.covariance(i, i) > 0.0) active.push_back(i);
  }
  result.eigenvalues.resize(0);
  result.variations.resize(nBins, 0);
  if (active.empty()) return result;

  const Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(result.covariance(active, active));
  if (solver.info() != Eigen::Success) throw std::runtime_error("ftag::ev::decompose: eigen solver failed");

  // Eigen sorts eigenvalues ascending
  const Eigen::VectorXd& lambda = solver.eigenvalues();
  const Eigen::Index nActive = lambda.size();
  const double threshold = relativeEigenvalueThreshold * lambda(nActive - 1);
  Eigen::Index nKept = 0;
  while (nKept < nActive && lambda(nActive - 1 - nKept) > threshold) ++nKept;

  result.eigenvalues.resize(nKept);
  result.variations.setZero(nBins, nKept);
  for (Eigen::Index k = 0; k < nKept; ++k) {
    const Eigen::Index j = nActive - 1 - k;
    Eigen::VectorXd u = solver.eigenvectors().col(j);
    Eigen::Index largest = 0;
    u.cwiseAbs().maxCoeff(&largest);
    if (u(largest) < 0.0) u = -u;
    result.eigenvalues(k) = lambda(j);
    result.variations(active, k) = std::sqrt(lambda(j)) * u;
  }
  return result;
}

// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"

// Project include(s).
#include "AthAllocators/DataPool.h"

// Google benchmark include(s).
#include <benchmark/benchmark.h>

// System include(s).
#include <memory>

/// Benchmarks for the xAOD EDM
namespace xAOD::Benchmark {

/// Code performing operations in a slow / basic way
namespace Basic {

std::pair<std::unique_ptr<xAOD::PixelClusterContainer>,
          std::unique_ptr<xAOD::PixelClusterAuxContainer>>
makeContainer(std::size_t nElements) {

  // Create the objects.
  auto cont = std::make_unique<xAOD::PixelClusterContainer>();
  auto aux = std::make_unique<xAOD::PixelClusterAuxContainer>();
  cont->setStore(aux.get());

  // Add the requested number of elements.
  for (std::size_t i = 0; i < nElements; ++i) {
    cont->push_back(std::make_unique<xAOD::PixelCluster>());
  }

  // Return the created objects.
  return {std::move(cont), std::move(aux)};
}

void fillContainer(xAOD::PixelClusterContainer& cont) {

  for (xAOD::PixelCluster* cluster : cont) {
    cluster->setIdentifierHash(123);
    cluster->setIdentifier(456);
    cluster->setMeasurement<2>(123, xAOD::MeasVector<2>{1.0, 2.0},
                               xAOD::MeasMatrix<2>{{1.0, 0.5}, {0.5, 2.0}});
    cluster->setLVL1A(42);
  }
}

void readContainer(const xAOD::PixelClusterContainer& cont) {

  for (const xAOD::PixelCluster* cluster : cont) {
    benchmark::DoNotOptimize(cluster->identifierHash());
    benchmark::DoNotOptimize(cluster->identifier());
    benchmark::DoNotOptimize(cluster->localPosition<2>());
    benchmark::DoNotOptimize(cluster->localCovariance<2>());
    benchmark::DoNotOptimize(cluster->lvl1a());
  }
}

}  // namespace Basic

/// Code performing operations in a more advanced / fast way
namespace Fast {

std::pair<std::unique_ptr<xAOD::PixelClusterContainer>,
          std::unique_ptr<xAOD::PixelClusterAuxContainer>>
makeContainer(std::size_t nElements) {

  // Create the objects.
  auto cont = std::make_unique<xAOD::PixelClusterContainer>(
      SG::VIEW_ELEMENTS, SG::ALWAYS_TRACK_INDICES);
  auto aux = std::make_unique<xAOD::PixelClusterAuxContainer>();

  // Create a data pool.
  DataPool<xAOD::PixelCluster> pool;
  pool.reserve(nElements);

  // Fill the interface container.
  cont->push_new(nElements, [&pool]() { return pool.nextElementPtr(); });

  // Set up the auxiliary store.
  aux->resize(nElements);
  cont->setStore(aux.get());

  // Return the created objects.
  return {std::move(cont), std::move(aux)};
}

void fillContainer(xAOD::PixelClusterContainer& cont) {

  xAOD::PixelCluster::ClusterVars vars{cont};
  for (std::size_t i = 0; i < cont.size(); ++i) {
    vars.identifierHash[i] = 123;
    vars.identifier[i] = 456;
    vars.localPositionDim2[i] = {1.0, 2.0};
    vars.localCovarianceDim2[i] = {1.0, 0.5, 0.5, 2.0};
    vars.lvl1a[i] = 42;
  }
}

void readContainer(xAOD::PixelClusterContainer& cont) {

  xAOD::PixelCluster::ClusterVars vars{cont};
  for (std::size_t i = 0; i < cont.size(); ++i) {
    benchmark::DoNotOptimize(vars.identifierHash[i]);
    benchmark::DoNotOptimize(vars.identifier[i]);
    benchmark::DoNotOptimize(vars.localPositionDim2[i]);
    benchmark::DoNotOptimize(vars.localCovarianceDim2[i]);
    benchmark::DoNotOptimize(vars.lvl1a[i]);
  }
}

}  // namespace Fast

/// Code performing operations on a custom type
namespace Custom {

/// Dummy custom type that should be as fast/simple as possible
struct PixelClusterContainer {
  std::vector<xAOD::DetectorIDHashType> identifierHash;
  std::vector<xAOD::DetectorIdentType> identifier;
  std::vector<xAOD::PosAccessor<2>::element_type> localPositionDim2;
  std::vector<xAOD::CovAccessor<2>::element_type> localCovarianceDim2;
  std::vector<int> lvl1a;
};  // struct PixelClusterContainer

std::unique_ptr<PixelClusterContainer> makeContainer(std::size_t nElements) {

  auto cont = std::make_unique<PixelClusterContainer>();
  cont->identifierHash.resize(nElements);
  cont->identifier.resize(nElements);
  cont->localPositionDim2.resize(nElements);
  cont->localCovarianceDim2.resize(nElements);
  cont->lvl1a.resize(nElements);
  return cont;
}

void fillContainer(PixelClusterContainer& cont) {

  for (std::size_t i = 0; i < cont.identifierHash.size(); ++i) {
    cont.identifierHash[i] = 123;
    cont.identifier[i] = 456;
    cont.localPositionDim2[i] = {1.0, 2.0};
    cont.localCovarianceDim2[i] = {1.0, 0.5, 0.5, 2.0};
    cont.lvl1a[i] = 42;
  }
}

void readContainer(PixelClusterContainer& cont) {

  for (std::size_t i = 0; i < cont.identifierHash.size(); ++i) {
    benchmark::DoNotOptimize(cont.identifierHash[i]);
    benchmark::DoNotOptimize(cont.identifier[i]);
    benchmark::DoNotOptimize(cont.localPositionDim2[i]);
    benchmark::DoNotOptimize(cont.localCovarianceDim2[i]);
    benchmark::DoNotOptimize(cont.lvl1a[i]);
  }
}

}  // namespace Custom

/// @name Benchmarks for the PixelCluster class
/// @{

void xAODBasicPixelClusterCreation(benchmark::State& state) {

  // Get the number of elements to create.
  const auto nElements = state.range(0);

  for (auto _ : state) {
    auto [cont, aux] = Basic::makeContainer(nElements);
    state.PauseTiming();
    cont.reset();
    aux.reset();
    state.ResumeTiming();
  }
}
BENCHMARK(xAODBasicPixelClusterCreation)->Range(1, 1'000'000);

void xAODBasicPixelClusterFilling(benchmark::State& state) {

  // Get the number of elements to create.
  const auto nElements = state.range(0);

  auto [cont, aux] = Basic::makeContainer(nElements);
  for (auto _ : state) {
    Basic::fillContainer(*cont);
  }
}
BENCHMARK(xAODBasicPixelClusterFilling)->Range(1, 1'000'000);

void xAODBasicPixelClusterReading(benchmark::State& state) {

  // Get the number of elements to create.
  const auto nElements = state.range(0);

  auto [cont, aux] = Basic::makeContainer(nElements);
  Basic::fillContainer(*cont);
  for (auto _ : state) {
    Basic::readContainer(*cont);
  }
}
BENCHMARK(xAODBasicPixelClusterReading)->Range(1, 1'000'000);

void xAODFastPixelClusterCreation(benchmark::State& state) {

  // Get the number of elements to create.
  const auto nElements = state.range(0);

  for (auto _ : state) {
    auto [cont, aux] = Fast::makeContainer(nElements);
    state.PauseTiming();
    cont.reset();
    aux.reset();
    state.ResumeTiming();
  }
}
BENCHMARK(xAODFastPixelClusterCreation)->Range(1, 1'000'000);

void xAODFastPixelClusterFilling(benchmark::State& state) {

  // Get the number of elements to create.
  const auto nElements = state.range(0);

  auto [cont, aux] = Fast::makeContainer(nElements);
  for (auto _ : state) {
    Fast::fillContainer(*cont);
  }
}
BENCHMARK(xAODFastPixelClusterFilling)->Range(1, 1'000'000);

void xAODFastPixelClusterReading(benchmark::State& state) {

  // Get the number of elements to create.
  const auto nElements = state.range(0);

  auto [cont, aux] = Fast::makeContainer(nElements);
  Fast::fillContainer(*cont);
  for (auto _ : state) {
    Fast::readContainer(*cont);
  }
}
BENCHMARK(xAODFastPixelClusterReading)->Range(1, 1'000'000);

void CustomPixelClusterCreation(benchmark::State& state) {

  // Get the number of elements to create.
  const auto nElements = state.range(0);

  for (auto _ : state) {
    auto cont = Custom::makeContainer(nElements);
    state.PauseTiming();
    cont.reset();
    state.ResumeTiming();
  }
}
BENCHMARK(CustomPixelClusterCreation)->Range(1, 1'000'000);

void CustomPixelClusterFilling(benchmark::State& state) {

  // Get the number of elements to create.
  const auto nElements = state.range(0);

  auto cont = Custom::makeContainer(nElements);
  for (auto _ : state) {
    Custom::fillContainer(*cont);
  }
}
BENCHMARK(CustomPixelClusterFilling)->Range(1, 1'000'000);

void CustomPixelClusterReading(benchmark::State& state) {

  // Get the number of elements to create.
  const auto nElements = state.range(0);

  auto cont = Custom::makeContainer(nElements);
  Custom::fillContainer(*cont);
  for (auto _ : state) {
    Custom::readContainer(*cont);
  }
}
BENCHMARK(CustomPixelClusterReading)->Range(1, 1'000'000);

/// @}

}  // namespace xAOD::Benchmark

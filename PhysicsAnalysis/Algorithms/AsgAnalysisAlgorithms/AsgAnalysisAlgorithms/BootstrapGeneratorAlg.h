/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#ifndef ASG_ANALYSIS_ALGORITHMS__BOOTSTRAP_GENERATOR_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__BOOTSTRAP_GENERATOR_ALG_H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <xAODEventInfo/EventInfo.h>
#include <AsgTools/PropertyWrapper.h>
#include <TRandomGen.h>
#include <vector>

namespace CP
{
  /// \brief a class to generate random numbers with a unique seed
  class BootstrapGenerator
  {
    /// \brief the standard constructor
  public:
    BootstrapGenerator() {};

    /// \brief implementation of the hash function from https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
  public:
    std::uint64_t fnv1a_64(const void *buffer, size_t size, std::uint64_t offset_basis);

    /// \brief set the seed of the random number generator based on event properties
  public:
    void setSeed(std::uint64_t eventNumber, std::uint32_t runNumber, std::uint32_t mcChannelNumber);

    /// \brief generate a unique seed based on event identifiers
  public:
    std::uint64_t generateSeed(std::uint64_t eventNumber, std::uint32_t runNumber, std::uint32_t mcChannelNumber);

    /// \brief get the next bootstrap weight
  public:
    std::uint8_t getBootstrap() { return m_rng.Poisson(1); };

    /// \brief constants for seed generation
  private:
    static constexpr std::uint64_t m_offset = 14695981039346656037u;
    static constexpr std::uint64_t m_prime = 1099511628211u;

    /// \brief the random number generator (Ranlux++)
  private:
    TRandomRanluxpp m_rng;
  };


  /// \brief an algorithm to compute per-event bootstrap replica weights
  class BootstrapGeneratorAlg final : public EL::AnaReentrantAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;

  public:
    StatusCode initialize() override;

  public:
    StatusCode execute(const EventContext& ctx) const override;

    /// \brief the EventInfo container
  private:
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{
      this, "eventInfo", "EventInfo", "the EventInfo container"};

    /// \brief the number of bootstrap replicas
  private:
    Gaudi::Property<int> m_nReplicas {this, "nReplicas", 1000, "number of bootstrapped weights (toys) to generate"};

    /// \brief flag whether we are running on data
  private:
    Gaudi::Property<bool> m_data {this, "isData", false, "whether we are running on data"};

    /// \brief the output decoration
  private:
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_decorationKey{
      this, "decorationName", m_eventInfoKey, "bootstrapWeights", "decoration name for the vector of bootstrapped weights"};
  };
} // namespace CP

#endif

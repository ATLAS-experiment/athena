//
// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHDEVICECOMPS_POOLMEMORYRESOURCESVC_H
#define ATHDEVICECOMPS_POOLMEMORYRESOURCESVC_H

// Framework include(s).
#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ToolHandle.h"

// AthDevice include(s).
#include "AthDeviceInterfaces/IMemoryResourceSvc.h"
#include "AthDeviceInterfaces/IMemoryResourceTool.h"

// VecMem include(s).
#include <vecmem/memory/pool_memory_resource.hpp>

namespace AthDevice {

/// Service implementing "pooled" caching on top of another memory resource
///
/// Making use of @c vecmem::pool_memory_resource.
///
class PoolMemoryResourceSvc : public extends<AthService, IMemoryResourceSvc> {

 public:
  // Inherit the base class's constructor(s).
  using extends::extends;

  /// @name Function(s) inherited from @c AthService
  /// @{

  /// Initialize the tool
  virtual StatusCode initialize() override;

  /// @}

  /// @name Function(s) inherited from @c IMemoryResourceSvc
  /// @{

  /// Get the provided @c std::pmr::memory_resource object
  virtual std::pmr::memory_resource& mr() const override;

  /// @}

 private:
  /// Handle to the tool providing the underlying memory resource
  ToolHandle<IMemoryResourceTool> m_mrTool{
      this, "MRTool", "", "Tool providing the memory resource to be cached"};
  /// The memory resource that this tool uses for caching
  std::unique_ptr<std::pmr::memory_resource> m_cachedMR;
  /// The memory resource that this tool uses for synchronization
  std::unique_ptr<std::pmr::memory_resource> m_syncedMR;

  /// @name Service properties
  /// @{

  /// Options object for the underlying pool memory resource
  vecmem::pool_memory_resource::options m_opts;

  Gaudi::Property<std::size_t> m_minBlocksPerChunk{
      this, "MinBlocksPerChunk", m_opts.min_blocks_per_chunk,
      [this](Gaudi::Details::PropertyBase&) {
        m_opts.min_blocks_per_chunk = m_minBlocksPerChunk;
      }};
  Gaudi::Property<std::size_t> m_maxBlocksPerChunk{
      this, "MaxBlocksPerChunk", m_opts.max_blocks_per_chunk,
      [this](Gaudi::Details::PropertyBase&) {
        m_opts.max_blocks_per_chunk = m_maxBlocksPerChunk;
      }};
  Gaudi::Property<std::size_t> m_minBytesPerChunk{
      this, "MinBytesPerChunk", m_opts.min_bytes_per_chunk,
      [this](Gaudi::Details::PropertyBase&) {
        m_opts.min_bytes_per_chunk = m_minBytesPerChunk;
      }};
  Gaudi::Property<std::size_t> m_maxBytesPerChunk{
      this, "MaxBytesPerChunk", m_opts.max_bytes_per_chunk,
      [this](Gaudi::Details::PropertyBase&) {
        m_opts.max_bytes_per_chunk = m_maxBytesPerChunk;
      }};

  Gaudi::Property<std::size_t> m_smallestBlockSize{
      this, "SmallestBlockSize", m_opts.smallest_block_size,
      [this](Gaudi::Details::PropertyBase&) {
        m_opts.smallest_block_size = m_smallestBlockSize;
      }};
  Gaudi::Property<std::size_t> m_largestBlockSize{
      this, "LargestBlockSize", m_opts.largest_block_size,
      [this](Gaudi::Details::PropertyBase&) {
        m_opts.largest_block_size = m_largestBlockSize;
      }};

  Gaudi::Property<std::size_t> m_alignment{
      this, "Alignment", m_opts.alignment,
      [this](Gaudi::Details::PropertyBase&) {
        m_opts.alignment = m_alignment;
      }};

  Gaudi::Property<bool> m_cacheOversized{
      this, "CacheOversized", m_opts.cache_oversized,
      [this](Gaudi::Details::PropertyBase&) {
        m_opts.cache_oversized = m_cacheOversized;
      }};
  Gaudi::Property<std::size_t> m_cachedSizeCutoffFactor{
      this, "CachedSizeCutoffFactor", m_opts.cached_size_cutoff_factor,
      [this](Gaudi::Details::PropertyBase&) {
        m_opts.cached_size_cutoff_factor = m_cachedSizeCutoffFactor;
      }};
  Gaudi::Property<std::size_t> m_cachedAlignmentCutoffFactor{
      this, "CachedAlignmentCutoffFactor",
      m_opts.cached_alignment_cutoff_factor,
      [this](Gaudi::Details::PropertyBase&) {
        m_opts.cached_alignment_cutoff_factor = m_cachedAlignmentCutoffFactor;
      }};

  /// @}

};  // class PoolMemoryResourceSvc

}  // namespace AthDevice

#endif  // ATHDEVICECOMPS_POOLMEMORYRESOURCESVC_H

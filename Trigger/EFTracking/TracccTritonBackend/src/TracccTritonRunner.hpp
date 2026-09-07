// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef TRACCC_TRITON_RUNNER_H
#define TRACCC_TRITON_RUNNER_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace triton { namespace backend { namespace traccc {

class TracccTritonInitializer;

/// @class TracccTritonRunner
///
/// @brief Drives the device reconstruction chain for a single Triton request.
///
/// Per call to @c run() the following happens:
///
///   1. the raw @c CELLS byte buffer is deserialized into a host
///      @c traccc::edm::silicon_cell_collection;
///   2. it is copied to the device and recorded in StoreGate under the key the
///      clusterization algorithm reads;
///   3. the three device algorithms (clusterization -> spacepoint formation ->
///      triplet seeding) are executed in order on a fresh @c EventContext;
///   4. the resulting seeds and the spacepoints they point at are copied back
///      to the host and packed into the single UINT8 output tensor expected
///      by @c TracccTritonClient: an 8-byte seed count followed by 9 column
///      blocks of N floats each (the global x/y/z of each seed's bottom,
///      middle and top spacepoint).
///
/// The runner holds no GPU state of its own: memory resources, copies and
/// streams all come from the embedded Gaudi kernel owned by
/// @c TracccTritonInitializer.
class TracccTritonRunner {
public:
    /// @brief The packed result, ready to hand to BackendOutputResponder.
    struct Output {
        /// Serialized seed collection for the single UINT8 output tensor:
        /// uint64 N, then 9 column blocks of N floats
        /// (bottom.x/y/z, middle.x/y/z, top.x/y/z), matching the CELLS
        /// input's SoA layout. See README.md's Wire format section.
        std::vector<uint8_t> buffer;
        /// Number of seeds produced (for logging / statistics).
        std::size_t nSeeds = 0;
    };

    /// Construct a runner bound to an already-initialized kernel.
    /// @throws std::runtime_error if the initializer is not ready.
    explicit TracccTritonRunner(TracccTritonInitializer& initializer);

    ~TracccTritonRunner();

    TracccTritonRunner(const TracccTritonRunner&) = delete;
    TracccTritonRunner& operator=(const TracccTritonRunner&) = delete;

    /// Run the full chain for one request.
    ///
    /// The @p buffer is the raw @c CELLS tensor gathered by
    /// BackendInputCollector. Its byte layout (native endianness, length
    /// 8 + 20*N) is:
    ///   offset 0      : uint64_t N              (cell count)
    ///   then 5 column blocks of N entries each:
    ///     channel0 (u32), channel1 (u32), activation (f32),
    ///     time (f32), module_index (u32)
    /// which mirrors @c TritonTracccTrackMaker::serializeCells on the client.
    ///
    /// @param buffer     pointer to the gathered CELLS byte buffer.
    /// @param byteSize   size of @p buffer in bytes.
    /// @param printStats emit per-stage timing to stdout.
    /// @throws std::runtime_error on any deserialization, algorithm or copy
    ///         failure.
    Output run(const uint8_t* buffer, std::size_t byteSize, bool printStats);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}}}  // namespace triton::backend::traccc

#endif  // TRACCC_TRITON_RUNNER_H

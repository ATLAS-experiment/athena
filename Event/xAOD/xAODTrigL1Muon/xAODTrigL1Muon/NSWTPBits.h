#ifndef XAODTRIGL1MUON_NSWTPBITS_H
#define XAODTRIGL1MUON_NSWTPBITS_H

#include <cstdint>
#include <numbers>

namespace L1Muon {
  namespace NSWTPBits {
    // Shifts and masks for Run 4 NSW-TP segment packed words

    /// Segment position in eta. [-2.7:2.7] is mapped.
    /// step = (2.7+2.7)/2^14 -> 1 bit~0.00033. (14 bits, bit order [13:0])
    constexpr uint32_t RUN4_NSWTP_ETA_SHIFT  = 0;
    constexpr uint32_t RUN4_NSWTP_ETA_MASK   = 0x3FFF; // 14 bits
    constexpr float ETA_MAX_RAW  = 16383.0f; // 2^14 - 1
    constexpr float ETA_MIN      = -2.7f;
    constexpr float ETA_MAX      = 2.7f;

    /// Segment position in phi. [0:2pi] is mapped.
    /// step = (2*6.28318)/2^10 -> 1 bit~0.00614 radians. (10 bits, bit order [23:14])
    constexpr uint32_t RUN4_NSWTP_PHI_SHIFT  = 14;
    constexpr uint32_t RUN4_NSWTP_PHI_MASK   = 0x3FF;  // 10 bits
    constexpr float PHI_MAX_RAW  = 1023.0f;  // 2^10 - 1
    constexpr float PHI_MAX      = static_cast<float>(std::numbers::pi * 2.0);

    /// Angular deviation of the locally defined segment from the infinite momentum track.
    /// [4]: Sign (0:+, 1:-), [3:0]: Absolute value.
    /// '+0' used for 0, and '-0' is not-used. (5 bits, bit order [28:24])
    constexpr uint32_t RUN4_NSWTP_DTH_SHIFT  = 24;
    constexpr uint32_t RUN4_NSWTP_DTH_MASK   = 0x1F;   // 5 bits

    /// Valid flag and detector information.
    /// 0: Invalid/no segment, 1: sTGC pad, 2: sTGC strip,
    /// 3: MM, 4: Combination. (3 bits, bit order [31:29])
    constexpr uint32_t RUN4_NSWTP_QUAL_SHIFT = 29;
    constexpr uint32_t RUN4_NSWTP_QUAL_MASK  = 0x7;    // 3 bits
  }
}

#endif // XAODTRIGL1MUON_NSWTPBITS_H

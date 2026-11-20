// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
// Device-compatible adaptation of the itksw RD53C stream decoder
// (itksw/pix/endec: DecStream2.hpp, DecStreamDefs.hpp, MemStreamProvider.hpp,
// BinaryTree.hpp, intf/{options,errors}.hpp, defs/{ItkChip,BitUtils,ChipUtils}.hpp).
//
// The decoding logic is copied unchanged from the release headers; only the
// pieces on the CUDA decoding call chain are vendored, with host/device
// annotations added. Keep this file synchronised with upstream itksw.
//
// Differences from upstream (none change the decoded output):
//  - everything lives in namespace itksw_cuda so it cannot clash with itksw;
//  - std::array / std::optional / std::min / std::tuple / std::pair are replaced
//    by plain arrays and small structs, since they are not device-callable;
//  - the C++20 concepts are dropped (plain templates);
//  - the BinaryTreeLUT lookup tables are __device__ arrays, filled on the host
//    from the release tables (see RD53CDecodingCUDA.cu).
//
// Defines __device__ variables: include from exactly one .cu translation unit.
//
// \authors Roxani Lazaridou <roxani.lazaridou@cern.ch>
//          Fabrice Le Goff <fabrice.le.goff@cern.ch>
#pragma once

#include "itksw/pix/endec/intf/Errors.hpp"
#include "itksw/pix/defs/ItkChip.hpp"
#include "itksw/pix/endec/DecStreamDefs.hpp"
#include "itksw/pix/endec/DecStream.hpp"
#include "itksw/pix/endec/MemStreamProvider.hpp"

#ifdef __CUDACC__
#define ITKSW_CUDA_D __device__
#else
#define ITKSW_CUDA_D
#endif

namespace itksw_cuda {

using itksw::pix::endec::intf::EventError;
using itksw::pix::endec::intf::EventErrorCode;
using itksw::pix::defs::ItkChip;
using itksw::pix::endec::DecState;
using itksw::pix::endec::TokenType;
using itksw::pix::endec::MAX_QCOL;
using itksw::pix::endec::MAX_QROW;
using itksw::pix::endec::MemStreamState;

namespace defs {

template <typename T>
ITKSW_CUDA_D inline size_t N_bits() {
    return sizeof(T) * 8;
}

ITKSW_CUDA_D inline uint64_t mask(size_t N) {
    assert(N <= 64);
    if (N == 64) {
        return ~0;
    } else {
        return (1UL << N) - 1;
    }
}

// Extract bits from from to downto, including the positions.
template <typename T>
ITKSW_CUDA_D inline T bit_extract(T val, size_t from, size_t downto) {
    assert(from <= N_bits<T>());
    assert(downto <= N_bits<T>());
    assert(from >= downto);
    return (val >> downto) & static_cast<T>(mask(from - downto + 1));
}

// Helper to read chip id position in frame
ITKSW_CUDA_D inline uint8_t read_chip_id(uint64_t frame) {
    return static_cast<uint8_t>(bit_extract<uint64_t>(frame, 62, 61));
}

// Check if frame has chip ID according to allowed mask (Nth bit = chip ID N allowed)
ITKSW_CUDA_D inline bool check_chip_id(uint64_t frame, uint8_t allowed_mask) {
    return ((allowed_mask >> read_chip_id(frame)) & 1) == 1;
}

}  // namespace defs

namespace endec {

// Stands in for std::optional<EventError::error_t>.
struct MaybeError {
    bool has_value;
    EventErrorCode value;
};

class DebugEventVoid {
public:
    ITKSW_CUDA_D inline void token_out(uint64_t, size_t, TokenType) {}
};

// BinaryTreeLUT lookup tables (same indexing and values as the release tables).
struct HitLutEntry {
    uint8_t first;
    uint8_t second;
};
struct HitLutRow {
    HitLutEntry e[16];
};
struct LutTopEntry {
    uint16_t decoded;
    uint8_t offset;
    uint8_t length;
};
struct LutBottomEntry {
    uint8_t hitmap;
    uint8_t length;
};

static constexpr uint32_t LUT_LEN = 16;
static constexpr size_t HIT_LUT_SIZE = 1 << 16;
static constexpr size_t LUT_TOP_SIZE = 1 << LUT_LEN;
static constexpr size_t LUT_BOTTOM_SIZE = 1 << (30 - LUT_LEN);

namespace tables {
    ITKSW_CUDA_D HitLutRow hit_lut[HIT_LUT_SIZE];
    ITKSW_CUDA_D uint8_t hit_lut_length[HIT_LUT_SIZE];
    ITKSW_CUDA_D LutTopEntry lut_top[LUT_TOP_SIZE];
    ITKSW_CUDA_D LutBottomEntry lut_bottom[LUT_BOTTOM_SIZE];
}  // namespace tables

// BinaryTreeLUT<true>::decode
ITKSW_CUDA_D inline uint32_t binary_tree_decode(uint32_t encoded, uint32_t& decoded) {
    auto _encoded = encoded & 0x3fffffff;
    const LutTopEntry top = tables::lut_top[_encoded >> (30 - LUT_LEN)];
    uint32_t hitmap = top.decoded;
    decoded = hitmap;
    if (top.offset == 0xff) return top.length;
    _encoded = (_encoded >> top.offset) & 0x3fff;
    const LutBottomEntry bottom = tables::lut_bottom[_encoded];
    decoded &= 0xff;
    decoded |= uint16_t(bottom.hitmap) << 8;
    return top.length + bottom.length;
}

class MemStreamProvider {
public:
    ITKSW_CUDA_D MemStreamProvider(const uint64_t* data, size_t data_size, size_t frame_size,
                                    uint8_t chip_id_filter = 0b1111)
        : state_(MemStreamState{0, frame_size}),
          frame_size_(frame_size),
          data_(data),
          data_size_(data_size),
          chip_id_filter_(chip_id_filter) {
        assert(frame_size > 1 && frame_size <= 64);
    }

    // Returns true on success (FetchResult::Okay upstream).
    ITKSW_CUDA_D inline bool fetch(size_t N_bits, uint64_t* out) {
        assert(N_bits <= 64);
        int N_bits_int = static_cast<int>(N_bits);
        MemStreamState copy_state = state_;
        bool res = read(&copy_state, &N_bits_int, out);
        if (res) {
            state_ = copy_state;
        }
        return res;
    }

    ITKSW_CUDA_D inline bool peek(size_t N_bits, uint64_t* out, bool pad = false) {
        assert(N_bits <= 64);
        int N_bits_int = static_cast<int>(N_bits);
        MemStreamState copy_state = state_;

        bool res = read(&copy_state, &N_bits_int, out);
        if (!res) {
            if (pad) {
                // We have some unread bits, need to shift by that amount
                *out = *out << N_bits_int;
                return true;
            } else {
                return false;
            }
        } else {
            return true;
        }
    }

    ITKSW_CUDA_D inline MaybeError reached_eos() {
        // Check current frame
        if (defs::bit_extract(data_[state_.index], state_.bit, 0) != 0) {
            return MaybeError{true, EventErrorCode::DATA_AFTER_EOS};
        }
        // Check following frames (matching chip ID)
        for (size_t idx = state_.index + 1; idx < data_size_; idx++) {
            if (defs::bit_extract(data_[idx], frame_size_ - 1, 0) != 0 &&
                defs::check_chip_id(data_[idx], chip_id_filter_)) {
                return MaybeError{true, EventErrorCode::DATA_AFTER_EOS};
            }
        }
        return MaybeError{false, EventErrorCode::UNDEFINED};
    }

private:
    // Returns true for FetchResult::Okay, false for FetchResult::EndOfData.
    ITKSW_CUDA_D inline bool read(MemStreamState* state_ref, int* N_bits_ref, uint64_t* out) {
        assert(*N_bits_ref <= 64);

        auto check_id_mismatch = [&]() -> bool {
            if (state_ref->index >= data_size_) {
                return true;
            }
            return !defs::check_chip_id(data_[state_ref->index], chip_id_filter_);
        };

        // Now build the output without further checks
        *out = 0;
        while (*N_bits_ref > 0) {
            // check if we need to get a new frame
            if (state_ref->bit == 0 || check_id_mismatch()) {
                do {
                    if (state_ref->index + 1 >= data_size_) {
                        return false;
                    }

                    // Fetch new word
                    state_ref->index += 1;
                    state_ref->bit = frame_size_;

                    // Repeat if the new frame has a chip ID that we don't care about
                } while (check_id_mismatch());
            }

            int cur_bit = static_cast<int>(state_ref->bit);
            int bits_to_read = (cur_bit < *N_bits_ref) ? cur_bit : *N_bits_ref;  // std::min
            int top_bit = static_cast<int>(state_ref->bit) - 1;
            int bot_bit = static_cast<int>(state_ref->bit) - bits_to_read;
            uint64_t data_from_cur_frame = defs::bit_extract(data_[state_ref->index], top_bit, bot_bit);
            *out = (*out << bits_to_read) | data_from_cur_frame;
            *N_bits_ref -= bits_to_read;
            state_ref->bit -= bits_to_read;
        }

        return true;
    }

    MemStreamState state_;

    const size_t frame_size_;
    const uint64_t* data_;
    const size_t data_size_;
    const uint8_t chip_id_filter_;
};

struct DecStreamState {
    bool started_evt = false;
    DecState state = DecState::TAG;
    bool internal_tag = false;
    int cur_qcol = 0;
    int cur_qrow = 0;
    int last_q_row[ItkChip::N_C_COL] = {};
    bool cur_is_last = true;
    bool cur_is_neighbour = false;
    uint32_t cur_hitmap = 0;
    int cur_hits = 0;

    ITKSW_CUDA_D void reset() {
        started_evt = false;
        state = DecState::TAG;
        internal_tag = false;
        cur_qcol = 0;
        cur_qrow = 0;
        for (int i = 0; i < ItkChip::N_C_COL; ++i) last_q_row[i] = -1;
        cur_is_last = true;
        cur_is_neighbour = false;
        cur_hitmap = 0;
        cur_hits = 0;
    }
};

template <typename TCallback>
ITKSW_CUDA_D inline void throw_error(DecStreamState& state, TCallback& cb,
    EventErrorCode code)
{
    EventError error{code, (uint8_t)state.cur_qcol, (uint8_t)state.cur_qrow};
    int retval = cb.on_error(error);
    if (retval == 1) {
        if (state.started_evt) {
            cb.evt_done();
            state.started_evt = false;
        }
        state.state = DecState::ERROR;
    }
}

template <typename TStreamProvider, typename TDebugEventCallback>
ITKSW_CUDA_D inline bool decode_hitmap(DecStreamState& state, TStreamProvider& source,
                                       TDebugEventCallback& debug_cb) {
    uint64_t out;
    if (!source.peek(30, &out, true)) return false;

    auto len = binary_tree_decode(out, state.cur_hitmap);
    if (!source.fetch(len, &out)) {
        return false;
    }
    debug_cb.token_out(out, len, TokenType::HITMAP);
    state.cur_hits = tables::hit_lut_length[state.cur_hitmap];

    return true;
}

template <typename TCallback, typename TStreamProvider, typename TDebugEventCallback>
ITKSW_CUDA_D inline bool decode_tots(DecStreamState& state, TCallback& cb, TStreamProvider& source,
                                     TDebugEventCallback& debug_cb, bool hitmap_only) {
    uint64_t out = 0;

    if (!hitmap_only) {
        if (!source.fetch(state.cur_hits * 4, &out)) {
            return false;
        }
        debug_cb.token_out(out, 4 * state.cur_hits, TokenType::TOT);
    }

    if (state.cur_qcol <= MAX_QCOL && state.cur_qrow <= MAX_QROW) {
        int hit_count = state.cur_hits;
        const HitLutRow& h = tables::hit_lut[state.cur_hitmap];
        uint16_t col_base = state.cur_qcol * ItkChip::N_COL_IN_C_COL;
        uint16_t row_base = state.cur_qrow * ItkChip::N_ROW_IN_Q_ROW;
        for (int i = 0; i < state.cur_hits; ++i) {
            uint16_t col = col_base + uint16_t(h.e[i].first);
            uint16_t row = row_base + uint16_t(h.e[i].second);
            hit_count--;

            uint16_t tot = 0;
            if (!hitmap_only) {
                tot = ((out >> (hit_count * 4)) & 0xf);
            }
            cb.add_hit(col, row, tot);
        }
    }

    return true;
}

template <typename TStreamProvider, typename TCallback, typename TDebugEventCallback>
ITKSW_CUDA_D inline void decode_stream_end(
    const itksw::pix::endec::intf::Options& opt,
    TCallback& cb,
    TStreamProvider&,
    TDebugEventCallback&,
    DecStreamState& state)
{
    if (state.started_evt) {
        bool valid_end_state = false;
        if (opt.en_eos) {
            valid_end_state = (state.state == DecState::EOS || state.state == DecState::DONE);
        } else {
            valid_end_state = (state.state == DecState::EOS || state.state == DecState::DONE ||
                               state.state == DecState::COL1 || state.state == DecState::COL2);
        }
        if (!valid_end_state) {
            throw_error(state, cb, EventErrorCode::TRUNCATED_STREAM);
        }

        cb.evt_done();
    }

    state.started_evt = false;
}

// Decode until the input blocks or the stream reaches a terminal state.
template <typename TStreamProvider, typename TCallback, typename TDebugEventCallback>
ITKSW_CUDA_D inline void decode_stream(
    const itksw::pix::endec::intf::Options& opt,
    TCallback& cb,
    TStreamProvider& source,
    TDebugEventCallback& debug_cb,
    DecStreamState& state)
{
    uint64_t out = 0;

    switch (state.state) {
    case DecState::TAG:
        goto read_tag;
    case DecState::COL1:
        goto read_col1;
    case DecState::COL2:
        goto read_col2;
    case DecState::CTRL_BITS:
        goto read_ctrl_bits;
    case DecState::ROW:
        goto read_row;
    case DecState::HMAP:
        goto read_hitmap;
    case DecState::TOTS:
        goto read_tots;
    case DecState::EOS:
        goto finish_eos;
    case DecState::DONE:
    case DecState::ERROR:
        return;
    }

read_tag:
    if (!source.fetch(8, &out)) {
        state.state = DecState::TAG;
        return;
    }
    if (state.internal_tag) {
        cb.evt_next(out & 0xff);
        state.started_evt = true;
        debug_cb.token_out(0b111 << 8 | out, 11, TokenType::INT_TAG);
    } else {
        cb.evt_init(out & 0xff);
        state.started_evt = true;
        debug_cb.token_out(out, 8, TokenType::TAG);
    }

read_col1:
    if (!state.cur_is_last) {
        goto read_ctrl_bits;
    }

    if (!source.fetch(3, &out)) {
        state.state = DecState::COL1;
        return;
    }

    if (out == 0b111) {
        state.reset();
        state.internal_tag = true;
        cb.evt_done();
        state.started_evt = false;
        goto read_tag;
    }

    state.cur_qcol = out << 3;

read_col2:
    if (!source.fetch(3, &out)) {
        state.state = DecState::COL2;
        return;
    }

    state.cur_qcol |= out;
    if (state.cur_qcol == 0) {
        goto finish_eos;
    }

    state.cur_qcol--;
    debug_cb.token_out(state.cur_qcol, 6, TokenType::C_COL);
    if (state.cur_qcol > MAX_QCOL) {
        throw_error(state, cb, EventErrorCode::QCOL_RANGE_ERROR);
        if (state.state == DecState::ERROR) return;
        state.cur_qcol = 0;
    }

read_ctrl_bits:
    if (!source.fetch(2, &out)) {
        state.state = DecState::CTRL_BITS;
        return;
    }

    debug_cb.token_out(out, 2, TokenType::CTRL);
    state.cur_is_last = (out & 0b10) != 0;
    state.cur_is_neighbour = (out & 0b01) != 0;

read_row:
    if (state.cur_is_neighbour) {
        int last = state.last_q_row[state.cur_qcol];
        if (last < 0) {
            throw_error(state, cb, EventErrorCode::QROW_ORDER_ERROR);
            if (state.state == DecState::ERROR) return;
            last = 0;
        }
        state.cur_qrow = last + 1;
        state.last_q_row[state.cur_qcol]++;
    } else {
        if (!source.fetch(8, &out)) {
            state.state = DecState::ROW;
            return;
        }

        if (out > MAX_QROW) {
            throw_error(state, cb, EventErrorCode::QROW_RANGE_ERROR);
            if (state.state == DecState::ERROR) return;
        }
        debug_cb.token_out(out, 8, TokenType::Q_ROW);
        state.cur_qrow = out;
        state.last_q_row[state.cur_qcol] = state.cur_qrow;
    }

read_hitmap:
    if (!decode_hitmap(state, source, debug_cb)) {
        state.state = DecState::HMAP;
        return;
    }

    if (opt.drop_tot) {
        bool tot_success = decode_tots(state, cb, source, debug_cb, true);
        assert(tot_success);
        (void)tot_success;
        if (state.state == DecState::ERROR) return;
    } else {
read_tots:
        if (!decode_tots(state, cb, source, debug_cb, false)) {
            state.state = DecState::TOTS;
            return;
        }
        if (state.state == DecState::ERROR) return;
    }

    if (state.cur_is_last) {
        goto read_col1;
    }
    goto read_ctrl_bits;

finish_eos:
    state.state = DecState::EOS;
    MaybeError maybe_error = source.reached_eos();
    if (maybe_error.has_value) {
        throw_error(state, cb, maybe_error.value);
        if (state.state == DecState::ERROR) return;
    }
    cb.evt_done();
    state.started_evt = false;
    state.state = DecState::DONE;
}

}  // namespace endec
}  // namespace itksw_cuda

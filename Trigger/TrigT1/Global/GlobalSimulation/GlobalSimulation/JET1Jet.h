/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_JET1JET_H
#define GLOBALSIM_JET1JET_H

#include "BitSpec.h"
#include "Object.h"

namespace GlobalSim {

    class JET1Jet : public BitSpec<JET1Jet, 128> {
        /** BitSpec for the output jets of the JET1 algorithm */

        // Example of a custom encoding/decoding for one of the bitfields
        static constexpr float s_ET_UNIT = 0.25f; // suppose 0.25 GeV per count
        static std::bitset<13> et_encoder(float value) {
            return static_cast<unsigned long long>(value / s_ET_UNIT);
        }
        static float et_decoder(std::bitset<13> bits) {
            return (bits.to_ullong() * s_ET_UNIT);
        }

    public:
        static inline const BitField<0, 12, float> ptt{"ptt","et", "Transverse energy", et_encoder, et_decoder};
        static inline const BitField<13, 22, float> eta{"eta","eta", "Eta coordinate"};
        static inline const BitField<23, 31, float> phi{"phi","phi", "Phi coordinate"};
        static inline const BitField<32, 32, uint8_t, bool> flag_et_overflow{"flag_et_overflow","flags[0]", "flag description placeholder"};
        static inline const BitField<33, 33, uint8_t, bool> flag_error{"flag_error","flags[1]", "flag description placeholder"};
        static inline const BitField<34, 34, uint8_t, bool> flag_next_tob_same_et{"flag_next_tob_same_et","flags[2]", "flag description placeholder"};
        static inline const BitField<35, 35, uint8_t, bool> flag_truncate{"flag_truncate","flags[3]", "flag description"};
        static inline const BitField<36, 38, uint8_t> ring_four_tobs{"ring_four_tobs","ring_four_tobs", "Number of TOBs in the fourth ring"};
        static inline const BitField<39, 43, uint8_t> ring_three_tobs{"ring_three_tobs","ring_three_tobs", "Number of TOBs in the third ring"};
        static inline const BitField<44, 48, uint8_t> ring_two_tobs{"ring_two_tobs","ring_two_tobs", "Number of TOBs in the second ring"};
        static inline const BitField<49, 52, uint8_t> ring_one_tobs{"ring_one_tobs","ring_one_tobs", "Number of TOBs in the first ring"};
        static inline const BitField<53, 58, uint8_t> total_tobs{"total_tobs","total_tobs", "Number of TOBs in the jet"};
        static inline const BitField<59, 70, uint16_t> ring_four_ptt{"ring_four_ptt","ring_four_ptt", "Energy in the fourth ring"};
        static inline const BitField<71, 85, uint16_t> ring_three_ptt{"ring_three_ptt","ring_three_ptt", "Energy in the third ring"};
        static inline const BitField<86, 100, uint16_t> ring_two_ptt{"ring_two_ptt","ring_two_ptt", "Energy in the second ring"};
        static inline const BitField<101, 115, uint16_t> ring_one_ptt{"ring_one_ptt","ring_one_ptt", "Energy in the first ring"};
        static inline const BitField<116, 127, uint16_t> ring_zero_ptt{"ring_zero_ptt","ring_zero_ptt", "Energy in the zeroth ring"};

      DECLARE_FIELDS(ptt, eta, phi, flag_et_overflow, flag_error, flag_next_tob_same_et, flag_truncate,
		     ring_four_tobs, ring_three_tobs, ring_two_tobs, ring_one_tobs, total_tobs,
		     ring_four_ptt, ring_three_ptt, ring_two_ptt, ring_one_ptt, ring_zero_ptt);

    };

}

#endif

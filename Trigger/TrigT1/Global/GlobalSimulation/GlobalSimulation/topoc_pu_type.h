/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_TOPOCPUTYPE_H
#define GLOBALSIM_TOPOCPUTYPE_H

#include "BitSpec.h"
#include "Object.h"

namespace GlobalSim {

    class topoc_pu_type : public BitSpec<topoc_pu_type, 64> {

        // Example of a custom encoding/decoding for one of the bitfields
        static constexpr float s_ET_UNIT = 0.25f; // suppose 0.25 GeV per count
        static std::bitset<13> et_encoder(float value) {
            return static_cast<unsigned long long>(value / s_ET_UNIT);
        }
        static std::pair<float,float> et_decoder(std::bitset<13> bits) {
            const double v = bits.to_ullong() * s_ET_UNIT;
            return {v-0.5*s_ET_UNIT,v+0.5*s_ET_UNIT};
        }

    public:
        // syntax is:
        // BitField<Lo,Hi, AuxType [, ValueType=AuxType]> name{"name","auxvar", "description" [, encoder, decoder]}
        // auxvar can optionally include bitpacking-specifiers, in form of either auxvar[i] or auxvar[a:b]
        // this will map the bitfield into subbits of the auxvar. AuxType will need to be an integral type,
        // e.g. you cannot use bitpacking-specifiers on AuxType=float fields
        static inline const BitField<0, 12, float> ptt{"ptt","et", "Transverse energy", et_encoder, et_decoder};
        static inline const BitField<13, 22, float> eta{"eta","eta", "Eta coordinate"};
        static inline const BitField<23, 31, float> phi{"phi","phi", "Phi coordinate"};
        static inline const BitField<32, 32, uint8_t, bool> flag_et_overflow{"flag_et_overflow","flags[0]", "flag description placeholder"};
        static inline const BitField<33, 33, uint8_t, bool> flag_error{"flag_error","flags[1]", "flag description placeholder"};
        static inline const BitField<34, 34, uint8_t, bool> flag_next_tob_same_et{"flag_next_tob_same_et","flags[2]", "flag description placeholder"};
        static inline const BitField<35, 35, uint8_t, bool> flag_truncate{"flag_truncate","flags[3]", "flag description placeholder"};

        DECLARE_FIELDS(ptt, eta, phi, flag_et_overflow, flag_error, flag_next_tob_same_et, flag_truncate);

    };

}

#endif
/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_JETTAGOUTPUTTYPE_H
#define GLOBALSIM_JETTAGOUTPUTTYPE_H

#include "BitSpec.h"
#include "Object.h"

namespace GlobalSim {

    /*
      Output TOB of the large-R jet tagger (Jet_Tag), basic algorithm (v2).

      The word layout is the same as HLS implementation & generic TOB format, MSB -> LSB is
      phi | eta | ptt, so ptt occupies the least significant bits and the top
      32 bits are padding.

        [0:12]  ptt   13b   0.25 GeV per count (et_max_ 2048 / 2^13)
        [13:22] eta   10b   tower index, 98 used of 1024
        [23:31] phi    9b   tower index, 64 used of 512
        [32:63] --          padded zeroes

    */
    class jet_tag_output_type : public BitSpec<jet_tag_output_type, 64> {

      // Example of a custom encoding/decoding for one of the bitfields
        static constexpr float s_ET_UNIT = 0.25f; // suppose 0.25 GeV per count
        static std::bitset<13> et_encoder(float value) {
            return static_cast<unsigned long long>(value / s_ET_UNIT);
        }
        static float et_decoder(std::bitset<13> bits) {
            return (bits.to_ullong() * s_ET_UNIT);
        }

    public:
        // syntax is:
        // BitField<Lo,Hi, AuxType [, ValueType=AuxType]> name{"name","auxvar", "description" [, encoder, decoder]}
        static inline const BitField<0, 12, float> ptt{"ptt","et", "Transverse energy", et_encoder, et_decoder};
        static inline const BitField<13, 22, float> eta{"eta","eta", "Eta coordinate"};
        static inline const BitField<23, 31, float> phi{"phi","phi", "Phi coordinate"};

        DECLARE_FIELDS(ptt, eta, phi);

    };

}

#endif

/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_TOWER_H
#define GLOBALSIM_TOWER_H

#include "BitSpec.h"

namespace GlobalSim {

    namespace v1 {
        class Tower : public BitSpec<Tower, 64> {


        public:
            // syntax is:
            // BitField<Lo,Hi, AuxType [, ValueType=AuxType]> name{"name","auxvar", "description" [, encoder, decoder]}
            // auxvar can optionally include bitpacking-specifiers, in form of either auxvar[i] or auxvar[a:b]
            // this will map the bitfield into subbits of the auxvar. AuxType will need to be an integral type,
            // e.g. you cannot use bitpacking-specifiers on AuxType=float fields

            static inline const BitField<0, 12, float> ptt{{.name="ptt", .auxvar="et", .description="Transverse energy", .scale=0.1/*GeV*/, .centre=0}};
            static inline const SignedBitField<13, 22, float> eta{{.name="eta", .description="Eta coordinate",.scale=0.1}};
            static inline const UnclampedBitField<23, 31, float> phi{{.name="phi", .description="Phi coordinate",.scale=M_PI/32}};

            static inline const BitField<32, 32, uint8_t, bool> flag_et_overflow{"flag_et_overflow", "flags[0]","flag description placeholder"};
            static inline const BitField<33, 33, uint8_t, bool> flag_error{"flag_error", "flags[1]","flag description placeholder"};
            static inline const BitField<34, 34, uint8_t, bool> flag_next_tob_same_et{"flag_next_tob_same_et","flags[2]","flag description placeholder"};
            static inline const BitField<35, 35, uint8_t, bool> flag_truncate{"flag_truncate", "flags[3]","flag description placeholder"};

        DECLARE_FIELDS(ptt, eta, phi, flag_et_overflow, flag_error, flag_next_tob_same_et, flag_truncate);

        };
    }

    typedef v1::Tower Tower;

}

#endif
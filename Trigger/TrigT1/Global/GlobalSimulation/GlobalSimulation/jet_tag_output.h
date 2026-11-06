/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_JETTAGOUTPUTTYPE_H
#define GLOBALSIM_JETTAGOUTPUTTYPE_H

#include "GlobalSimulation/BitSpec.h"

namespace GlobalSim {

    /*
      Output TOB of the large-R jet tagger (Jet_Tag), basic algorithm (v2).

      The word layout is the same as HLS implementation & generic TOB format, MSB -> LSB is
      phi | eta | ptt, so ptt occupies the least significant bits and the top
      32 bits are padding.

        [0:12]  ptt   13b   0.25 GeV per count (et_max_ 2048 / 2^13)
        [13:22] eta   10b   two's complement, 0.0125 per count, code k = [k, k+1) * 0.0125
        [23:31] phi    9b   2pi/512 per count from phi = 0; the tower index << 3 (3 LSBs zero)
        [32:63] --          padded zeroes

    */
    class jet_tag_output_type : public BitSpec<jet_tag_output_type, 64> {

    public:
        // syntax is:
        // BitField<Lo,Hi, AuxType [, ValueType=AuxType]> name{"name","auxvar", "description" [, encoder, decoder]}
        // Codes are assigned through the bits overloads; a physical value is stored as the decoded
        // bin (lower edge for ptt, so 0.25 GeV counts round-trip exactly; midpoint for eta/phi).
        static inline const BitField<0, 12, float> ptt{{.name="ptt", .auxvar="et", .description="Transverse energy", .scale=0.25/*GeV*/, .centre=0}};
        static inline const SignedBitField<13, 22, float> eta{{.name="eta", .description="Eta coordinate", .scale=0.0125}};
        static inline const UnclampedBitField<23, 31, float> phi{{.name="phi", .description="Phi coordinate", .scale=M_PI/256}};

        DECLARE_FIELDS(ptt, eta, phi);

    };

}

#endif

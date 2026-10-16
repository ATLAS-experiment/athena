/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_GENERICTOB_H
#define GLOBALSIM_GENERICTOB_H

#include "GlobalSimulation/BitSpec.h"

namespace GlobalSim {

    namespace v1 {
        class GenericTOB : public BitSpec<GenericTOB, 64> {
        public:
            static inline const BitField<0, 12, float> et{{.name="et", .auxvar="et", .description="Transverse energy", .scale=0.1/*GeV*/, .centre=0}};
            static inline const SignedBitField<13, 22, float> eta{{.name="eta", .auxvar="eta", .description="Eta coordinate", .scale=0.0125}};
            static inline const UnclampedBitField<23, 31, float> phi{{.name="phi", .auxvar="phi", .description="Phi coordinate", .scale=M_PI / 256}};

            DECLARE_FIELDS(et, eta, phi);
        };
    }


    typedef v1::GenericTOB GenericTOB;

}
#endif

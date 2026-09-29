/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_GENERICTOB_H
#define GLOBALSIM_GENERICTOB_H

#include "GlobalSimulation/BitSpec.h"

namespace GlobalSim {


    class GenericTOB : public BitSpec<GenericTOB, 64> {
        /** BitSpec "base class" for inputs to hypothesis algorithm */

    public:
        static inline const BitField<0, 12, float> ptt{{.name="ptt", .auxvar="et", .description="Transverse energy", .scale=0.1/*GeV*/}};
        static inline const SignedBitField<13, 22, float> eta{{.name="eta", .auxvar="eta", .description="Eta coordinate", .scale=0.0125}};
        static inline const BitField<23, 31, float> phi{"phi", "phi", "Phi coordinate", nullptr, nullptr, M_PI / 256};

        DECLARE_FIELDS(ptt, eta, phi);
    };

}
#endif

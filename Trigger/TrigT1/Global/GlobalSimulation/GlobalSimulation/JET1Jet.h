/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_JET1JET_H
#define GLOBALSIM_JET1JET_H

#include "GlobalSimulation/BitSpec.h"
#include "GlobalSimulation/Tower.h"

namespace GlobalSim {


    // TEMPORARY: JET1Jet inherits the v3 tower encoding (0.25 GeV E_T, unsigned tower-grid eta/phi
    // indices) because the JET1 firmware still outputs tower-grid coordinates. Need to move to 
    // GenericTOB format once the JET1 firmware is updated to that encoding.
    class JET1Jet : public BitSpec<JET1Jet, 128, v3::Tower> {
        /** BitSpec for the output jets of the JET1 algorithm. ptt, eta, phi and the flags come from v3::Tower */


    public:
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

      DECLARE_FIELDS(ring_four_tobs, ring_three_tobs, ring_two_tobs, ring_one_tobs, total_tobs,
		     ring_four_ptt, ring_three_ptt, ring_two_ptt, ring_one_ptt, ring_zero_ptt);

    };

}

#endif

/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_METOUTPUTTYPE_H
#define GLOBALSIM_METOUTPUTTYPE_H

#include "GlobalSimulation/BitSpec.h"

namespace GlobalSim {

    /*
      Output TOB of the GEP MET algorithm (Total MET).

      This is the 64-bit MET TOB Data Format, identical to the MET emulation's
      (metEmulation.cc), MSB -> LSB:

        [63]    sum_et overflow
        [62]    upstream overflow  an accepted input object had already saturated its E_T
        [61]    met overflow
        [60:48] sum_et        13b  unsigned scalar energy, 0.25 GeV per count, saturated
        [47:35] ey_miss       13b  two's complement, saturated at +-4095
        [34:22] ex_miss       13b  two's complement, saturated at +-4095
        [21:13] phi_miss       9b  {phi6, 3'b000}: the 6-bit azimuth index in the top bits
        [12:0]  et_miss       13b  unsigned magnitude, 0.25 GeV per count, saturated

      Ex/Ey still saturate at +-4095, but their overflow conditions are not in the word.

      Things to note:

        * et_miss is NOT exactly sqrt(ex^2 + ey^2). It comes from the normalized-LUT square root
          in MET_LUT_SQRT.v, which deviates from the exact integer root by up to 3 counts
          (0.75 GeV).
        * phi_miss is NOT exactly atan2(ey, ex) rounded. It comes from the divider-free comparator
          ladder in MET_PHI_COMPARATOR.v, accurate to about half a bin.

      The magnitude fields SATURATE rather than wrap; MET and SumET raise their flag when they do.
    */
    class met_output_type : public BitSpec<met_output_type, 64> {

    public:
        // 0.25 GeV per count (et_max 2048 GeV over 13 bits), stored at the bin's lower edge so counts
        // round-trip exactly. Unsigned fields saturate at 8191; the signed (two's complement) fields
        // clamp at [-4096, 4095], and METAlg hands them components already saturated at +-4095.
        static inline const BitField<0, 12, float> et_miss{{.name="et_miss", .description="Missing transverse energy magnitude",
            .scale=0.25, .centre=0}};
        // Carried as the raw code rather than an angle: the firmware has no angle, and converting
        // here would invent precision the 6-bit index does not have. METAlg writes index << 3.
        static inline const BitField<13, 21, uint16_t> phi_miss{
            "phi_miss", "phi_miss", "Missing-energy azimuth, {6-bit index from +x, 3'b000}"};
        static inline const SignedBitField<22, 34, float> ex_miss{{.name="ex_miss", .description="Missing-energy x component",
            .scale=0.25, .centre=0}};
        static inline const SignedBitField<35, 47, float> ey_miss{{.name="ey_miss", .description="Missing-energy y component",
            .scale=0.25, .centre=0}};
        static inline const BitField<48, 60, float> sum_et{{.name="sum_et", .description="Scalar sum of transverse energy",
            .scale=0.25, .centre=0}};

        static inline const BitField<61, 61, uint8_t, bool> flag_met_overflow{
            "flag_met_overflow", "flags[0]", "Missing-energy magnitude exceeded the output range"};
        static inline const BitField<62, 62, uint8_t, bool> flag_upstream_overflow{
            "flag_upstream_overflow", "flags[1]",
            "An accepted input object had already saturated its E_T field"};
        static inline const BitField<63, 63, uint8_t, bool> flag_sum_et_overflow{
            "flag_sum_et_overflow", "flags[2]", "Energy Sum exceeded the output range"};

        DECLARE_FIELDS(et_miss, phi_miss, ex_miss, ey_miss, sum_et,
                       flag_met_overflow, flag_upstream_overflow, flag_sum_et_overflow);

    };

}

#endif

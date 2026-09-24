/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef GLOBALSIM_METOUTPUTTYPE_H
#define GLOBALSIM_METOUTPUTTYPE_H

#include "BitSpec.h"
#include "Object.h"

namespace GlobalSim {

    /*
      Output TOB of the GEP MET algorithm (Total MET).

      This is the 64-bit MET TOB Data Format of Table 4.5, and it is also exactly what
      MET_Engine.v packs into formatted_met_word. LSB -> MSB:

        [12:0]  et_miss       13b  unsigned magnitude, 0.25 GeV per count, saturated
        [18:13] phi_miss       6b  azimuth INDEX, 64 bins
        [31:19] ex_miss       13b  sign-magnitude {sign, magnitude[11:0]}
        [44:32] ey_miss       13b  sign-magnitude {sign, magnitude[11:0]}
        [57:45] sum_et        13b  unsigned scalar energy, saturated
        [58]    ex overflow
        [59]    ey overflow
        [60]    sum_et overflow
        [61]    met overflow
        [62]    upstream overflow
        [63]    reserved, driven to zero (not declared below, like the padding in
                jet_tag_output_type)

      Two things about this word are NOT what a reader might assume, and both are
      deliberate -- they are properties of the firmware, reproduced rather than corrected:

        * et_miss is NOT sqrt(ex^2 + ey^2). It comes from the normalized-LUT square root
          in MET_LUT_SQRT.v, which deviates from the exact integer root by up to 3 counts
          (0.75 GeV).
        * phi_miss is NOT atan2(ey, ex) rounded. It comes from the divider-free comparator
          ladder in MET_PHI_COMPARATOR.v, accurate to about half a bin.

      The three magnitude fields SATURATE rather than wrap, each raising its flag, which
      is why the flags are part of the word rather than bookkeeping.
    */
    class met_output_type : public BitSpec<met_output_type, 64> {

        /// 0.25 GeV per count: et_max 2048 GeV over the 13-bit unsigned field.
        static constexpr float s_ET_UNIT = 0.25f;

        /// Count: magnitude bits of a sign-magnitude field (13 total, 1 of them sign).
        static constexpr unsigned long long s_signed_mag_max = (1ULL << 12) - 1ULL;
        static constexpr unsigned long long s_signed_sign_bit = 1ULL << 12;
        /// Count: largest value the unsigned 13-bit fields can carry.
        static constexpr unsigned long long s_unsigned_max = (1ULL << 13) - 1ULL;

        static std::bitset<13> et_encoder(float value) {
            if (value < 0.f) return 0;
            unsigned long long counts = static_cast<unsigned long long>(value / s_ET_UNIT);
            if (counts > s_unsigned_max) counts = s_unsigned_max; // saturate, never wrap
            return counts;
        }
        static float et_decoder(std::bitset<13> bits) {
            return (bits.to_ullong() * s_ET_UNIT);
        }

        // Sign-magnitude, matching ex_sign_magnitude / ey_sign_magnitude in MET_Engine.v.
        // The sign bit is forced low at zero magnitude, so there is no negative zero --
        // a pattern the firmware can never emit.
        static std::bitset<13> signed_et_encoder(float value) {
            const bool negative = (value < 0.f);
            const float absValue = negative ? -value : value;
            unsigned long long magnitude = static_cast<unsigned long long>(absValue / s_ET_UNIT);
            if (magnitude > s_signed_mag_max) magnitude = s_signed_mag_max; // saturate
            if (magnitude == 0ULL) return 0;
            return (negative ? s_signed_sign_bit : 0ULL) | magnitude;
        }
        static float signed_et_decoder(std::bitset<13> bits) {
            const unsigned long long raw = bits.to_ullong();
            const float magnitude = static_cast<float>(raw & s_signed_mag_max) * s_ET_UNIT;
            return (raw & s_signed_sign_bit) ? -magnitude : magnitude;
        }

    public:
        static inline const BitField<0, 12, float> et_miss{
            "et_miss", "et_miss", "Missing transverse energy magnitude",
            et_encoder, et_decoder};
        // Carried as the raw index rather than an angle: the firmware has no angle, and
        // converting here would invent precision the 6-bit field does not have.
        static inline const BitField<13, 18, uint8_t> phi_miss{
            "phi_miss", "phi_miss", "Missing-energy azimuth index, 64 bins from +x"};
        static inline const BitField<19, 31, float> ex_miss{
            "ex_miss", "ex_miss", "Missing-energy x component",
            signed_et_encoder, signed_et_decoder};
        static inline const BitField<32, 44, float> ey_miss{
            "ey_miss", "ey_miss", "Missing-energy y component",
            signed_et_encoder, signed_et_decoder};
        static inline const BitField<45, 57, float> sum_et{
            "sum_et", "sum_et", "Scalar sum of transverse energy",
            et_encoder, et_decoder};

        static inline const BitField<58, 58, uint8_t, bool> flag_ex_overflow{
            "flag_ex_overflow", "flags[0]", "|Ex| exceeded the sign-magnitude output range"};
        static inline const BitField<59, 59, uint8_t, bool> flag_ey_overflow{
            "flag_ey_overflow", "flags[1]", "|Ey| exceeded the sign-magnitude output range"};
        static inline const BitField<60, 60, uint8_t, bool> flag_sum_et_overflow{
            "flag_sum_et_overflow", "flags[2]", "Energy Sum exceeded the output range"};
        static inline const BitField<61, 61, uint8_t, bool> flag_met_overflow{
            "flag_met_overflow", "flags[3]", "Missing-energy magnitude exceeded the output range"};
        static inline const BitField<62, 62, uint8_t, bool> flag_upstream_overflow{
            "flag_upstream_overflow", "flags[4]",
            "An accepted input object had already saturated its E_T field"};

        DECLARE_FIELDS(et_miss, phi_miss, ex_miss, ey_miss, sum_et,
                       flag_ex_overflow, flag_ey_overflow, flag_sum_et_overflow,
                       flag_met_overflow, flag_upstream_overflow);

    };

}

#endif

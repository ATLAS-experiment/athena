/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILERECUTILS_TILENNEMULATOR_H
#define TILERECUTILS_TILENNEMULATOR_H
#include <cstdint>
#include <string>
#include <vector>

/**
 * @class TileNNEmulator
 * @brief Bit-exact software emulator of the TilePPr neural-network energy reconstruction
 */
class TileNNEmulator {
  public:

    /**
     * @param jsonText the JSON document itself (not a file name, so the
     *                 payload can later come from the conditions database)
     * @param error    filled with a diagnostic on failure
     * @return true on success, on any problem (malformed JSON, unknown op,
     *         size or type inconsistency) returns false and leaves the
     *         emulator unusable
     */
    bool load(const std::string& jsonText, std::string& error);
    int nSamples() const { return m_nSamples; }

    /** Combined dual-gain input sample S = (hg + lgScale*lg) / denominator. */
    float sValue(float hg, float lg) const;

    /**
     * @return the raw integer code of the output fixed-point word
     */
    int64_t run(const float* s) const;

    /** Fractional bits of the output word (code * 2^-outFracBits() = value). */
    int outFracBits() const { return m_layers.back().outT.frac(); }
    double amplitudeScale() const { return m_ampScale; }

  private:
    enum class Round { RND_CONV, TRN };
    enum class Sat { SAT, SAT_SYM, WRAP };

    struct FixSpec {
      int w = 0;
      int i = 0;
      Round round = Round::TRN;
      Sat sat = Sat::WRAP;
      int frac() const { return w - i; }
    };

    enum class Op { Dense, LeakyRelu };

    struct Layer {
      Op op = Op::Dense;
      int nIn = 0;
      int nOut = 0;
      int alphaShift = 0;
      FixSpec weightT, accumT, biasT, outT;
      std::vector<int64_t> w, b;
    };

    static int64_t rshiftRoundHalfEven(int64_t v, int k);
    static int64_t castTo(int64_t v, int fracFrom, const FixSpec& spec);
    int m_nSamples = 0;
    double m_lgScale = 0.0;
    double m_denominator = 0.0;
    double m_ampScale = 0.0;
    FixSpec m_inT, m_castT;
    std::vector<Layer> m_layers;
};

#endif // TILERECUTILS_TILENNEMULATOR_H

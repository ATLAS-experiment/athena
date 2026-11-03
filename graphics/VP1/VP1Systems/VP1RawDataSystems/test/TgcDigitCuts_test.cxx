/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "../src/TgcDigitCuts.h"
#include <array>
#include <iostream>

int main()
{
  // Every combination of channel and BC selections, including unknown raw tags.
  const std::array<bool TgcDigitCuts::*, 8> fields = {
    &TgcDigitCuts::showStrips, &TgcDigitCuts::showWires,
    &TgcDigitCuts::showPrevious, &TgcDigitCuts::showCurrent,
    &TgcDigitCuts::showNext, &TgcDigitCuts::showNextNext,
    &TgcDigitCuts::showUndefined, &TgcDigitCuts::showUnknown
  };
  const std::array<unsigned, 8> tags = {
    TgcDigit::BC_UNDEFINED, TgcDigit::BC_PREVIOUS, TgcDigit::BC_CURRENT,
    TgcDigit::BC_NEXT, TgcDigit::BC_NEXTNEXT, 5, 32768, 65535
  };
  const std::array<unsigned, 8> bcBits = {6, 2, 3, 4, 5, 7, 7, 7};
  TgcDigitCuts defaults;
  for (bool strip : {false, true}) {
    for (unsigned tag : tags) {
      if (!defaults.accepts(strip, tag)) {
        std::cerr << "Default selection must accept every digit\n";
        return 1;
      }
    }
  }
  for (unsigned mask = 0; mask < 256; ++mask) {
    TgcDigitCuts cuts;
    for (unsigned bit = 0; bit < fields.size(); ++bit)
      cuts.*fields[bit] = mask & (1U << bit);
    for (bool strip : {false, true}) {
      for (unsigned index = 0; index < tags.size(); ++index) {
        const bool expected = (mask & (1U << (strip ? 0 : 1))) &&
                              (mask & (1U << bcBits[index]));
        if (cuts.accepts(strip, tags[index]) != expected) {
          std::cerr << "Wrong TGC cut: mask=" << mask << " strip=" << strip
                    << " BC=" << tags[index] << '\n';
          return 1;
        }
      }
    }
  }
  return 0;
}

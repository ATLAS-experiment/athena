/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef PADEMULATORCOINCIDENCES_H
#define PADEMULATORCOINCIDENCES_H

#include <algorithm>
#include <vector>

/**
 * @publicsection PadEmulatorCoincidences
 * @brief List of functions to evaluate sTGC Pad Trigger coincidences
 *
 * A basic lists of booleand functions to check trigger coincidences,
 * according to the chosen trigger logic
 **/

namespace NSWL1 {

  inline bool trigger_1over4(const bool in0, const bool in1, const bool in2, const bool in3) {
    return (in0 or in1 or in2 or in3);
  }

  inline bool trigger_2over4(const bool in0, const bool in1, const bool in2, const bool in3) {
    return (in0 + in1 + in2 + in3 >= 2);
  }

  inline bool trigger_specific2over4(const bool in0, const bool in1, const bool in2, const bool in3) {
    return (in0 or in1) and (in2 or in3);
  }

  inline bool trigger_3over4(const bool in0, const bool in1, const bool in2, const bool in3) {
    return (in0 + in1 + in2 + in3 >= 3);
  }

  inline bool trigger_4over4(const bool in0, const bool in1, const bool in2, const bool in3) {
    return (in0 and in1 and in2 and in3);
  }

  inline bool trigger_3and1(const bool in0, const bool in1, const bool in2, const bool in3,
                            const bool in4, const bool in5, const bool in6, const bool in7) {
    if(trigger_1over4(in0, in1, in2, in3) and trigger_3over4(in4, in5, in6, in7)) return true;
    if(trigger_3over4(in0, in1, in2, in3) and trigger_1over4(in4, in5, in6, in7)) return true;
    return false;
  }

  inline bool trigger_2and2(const bool in0, const bool in1, const bool in2, const bool in3,
                            const bool in4, const bool in5, const bool in6, const bool in7) {
    return (trigger_2over4(in0, in1, in2, in3) and trigger_2over4(in4, in5, in6, in7));
  }

  inline bool trigger_4over8(const bool in0, const bool in1, const bool in2, const bool in3,
                             const bool in4, const bool in5, const bool in6, const bool in7) {
    if(trigger_2and2(in0, in1, in2, in3, in4, in5, in6, in7)) return true;
    if(trigger_3and1(in0, in1, in2, in3, in4, in5, in6, in7)) return true;
    return false;
  }

  inline bool trigger_specific4over8(const bool in0, const bool in1, const bool in2, const bool in3,
                                     const bool in4, const bool in5, const bool in6, const bool in7) {
    return trigger_specific2over4(in0, in1, in2, in3) and trigger_specific2over4(in4, in5, in6, in7);
  }

  inline bool trigger_5over8(const bool in0, const bool in1, const bool in2, const bool in3,
                             const bool in4, const bool in5, const bool in6, const bool in7) {
    return (in0 + in1 + in2 + in3 + in4 + in5 + in6 + in7 >= 5);
  }

  inline bool trigger_specific5over8(const bool in0, const bool in1, const bool in2, const bool in3,
                                     const bool in4, const bool in5, const bool in6, const bool in7) {
    if(not trigger_3over4(in0, in1, in2, in3) and not trigger_3over4(in4, in5, in6, in7)) return false;

    return (in0 + in1 + in2 + in3 >= 2) and (in4 + in5 + in6 + in7 >= 2) and ((in0 + in1 + in2 + in3 >= 3) or (in4 + in5 + in6 + in7 >= 3));
  }

  inline bool trigger_superspecific5over8(const bool in0, const bool in1, const bool in2, const bool in3,
                                          const bool in4, const bool in5, const bool in6, const bool in7) {
    bool IP3over4 = trigger_3over4(in0, in1, in2, in3);
    bool HO3over4 = trigger_3over4(in4, in5, in6, in7);
    if(not IP3over4 and not HO3over4) return false;
    return ((IP3over4 and trigger_specific2over4(in4, in5, in6, in7)) or (HO3over4 and trigger_specific2over4(in0, in1, in2, in3)));
  }

  inline bool trigger_6over8(const bool in0, const bool in1, const bool in2, const bool in3,
                             const bool in4, const bool in5, const bool in6, const bool in7) {
    return (in0 + in1 + in2 + in3 + in4 + in5 + in6 + in7 >= 6);
  }

  inline bool trigger_2X_3over4(const bool in0, const bool in1, const bool in2, const bool in3,
                                const bool in4, const bool in5, const bool in6, const bool in7) {
    return trigger_3over4(in0, in1, in2, in3) and trigger_3over4(in4, in5, in6, in7);
  }

  inline bool trigger_8over8(const bool in0, const bool in1, const bool in2, const bool in3,
                             const bool in4, const bool in5, const bool in6, const bool in7) {
    return trigger_4over4(in0, in1, in2, in3) and trigger_4over4(in4, in5, in6, in7);
  }
}
#endif

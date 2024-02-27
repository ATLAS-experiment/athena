/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONMDT_CABLING_MICROMEGA_ZEBRADATA_H
#define MUONMDT_CABLING_MICROMEGA_ZEBRADATA_H

/*
 * @brief: Helper struct containing the information about which strip range in
 * the MM requires a shift. The connectors in the MMs are called ZEBRA
 * connectors. Since it is assumed at the moment that zebra connector is the
 * smallest shiftable object in the MMs, this gives the name to this struct. But
 * in principle it can handle any arbitrary channel range.
 */

#include <set>
#include <cstdint>

struct NswZebraData {
   public:
    int16_t firstChannel{0};
    int16_t lastChannel{0};
    int16_t shiftChannel{0};
};

// overriding operators for NswZebraData struct to allow finding them in
// maps and to sort them.
inline bool operator<(const NswZebraData& a,
                      const NswZebraData& b) {
    return a.lastChannel < b.firstChannel;
}
inline bool operator<(const NswZebraData& a, const int b) {
    return a.lastChannel < b;
}
inline bool operator<(const int a, const NswZebraData& b) {
    return a < b.firstChannel;
}

using NswZebraSet = std::set<NswZebraData, std::less<>>;

#endif
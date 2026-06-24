/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONHISTUTILS_MUONENUMDEFS_H
#define MUONHISTUTILS_MUONENUMDEFS_H

#include "xAODMuon/Muon.h"
#include <string_view>

namespace Muon {


enum class DetRegion: std::uint8_t { GLOBAL, BA, BC, EA, EC, nDetRegions };
static inline std::string_view toString(DetRegion reg) {
    switch (reg) {
        using enum DetRegion;
        case GLOBAL:
            return "Global";
        case BA:
            return "BA";
        case BC:
            return "BC";
        case EA:
            return "EA";
        case EC:
            return "EC";
        default:
            return "UnknownDetRegion";
    }
}

}  // namespace Muon

#endif

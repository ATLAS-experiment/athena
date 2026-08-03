// Dear emacs, this is -*- c++ -*-
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef PARTICLEJETTOOLS_SMALLRJETPILEUPLABELENUM_H
#define PARTICLEJETTOOLS_SMALLRJETPILEUPLABELENUM_H

// ROOT include(s).
#include <TString.h>

namespace SmallRJetPileupLabel
{
    enum TypeEnum : int
    {
        HS = 0,    // HS match
        MixHS,     // multiple HS matches
        HSPU,      // HS and PU matches
        ITPU,      // In time PU match
        MixPU,     // Multiple PU matches
        OOTPU,     // Out of time PU match
        Unknown,   // Other
    };

    inline int enumToInt(const TypeEnum type)
    {
        return static_cast<std::underlying_type_t< TypeEnum >>(type);
    }

    inline TypeEnum stringToEnum(const TString& name)
    {
#define TRY(STRING) if (name.EqualTo(#STRING, TString::kIgnoreCase)) return STRING
        TRY(HS);
        TRY(MixHS);
        TRY(HSPU);
        TRY(ITPU);
        TRY(MixPU);
        TRY(OOTPU);
        TRY(Unknown);
#undef TRY
        return Unknown;
    }
}

#endif
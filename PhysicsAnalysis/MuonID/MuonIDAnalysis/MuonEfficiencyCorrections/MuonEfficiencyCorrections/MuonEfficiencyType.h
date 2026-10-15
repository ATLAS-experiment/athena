/*
 Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 */

#ifndef MUONEFFICIENCYCORRECTIONS_MUONEFFICIENCYTYPE_H
#define MUONEFFICIENCYCORRECTIONS_MUONEFFICIENCYTYPE_H

#include <string>
#include <functional>
#include <ColumnarMode/ColumnarMode.h>

namespace ADD_CMODE(CP) {
    enum MuonEfficiencyType {
        Undefined = 0,
        Reco = 1,
        TTVA = 2,
        Iso = 3,
        BadMuonVeto = 4
    };
    inline static std::string EfficiencyTypeName(MuonEfficiencyType M) {
        if (M == MuonEfficiencyType::Reco) return "RECO";
        if (M == MuonEfficiencyType::TTVA) return "TTVA";
        if (M == MuonEfficiencyType::Iso) return "ISO";
        if (M == MuonEfficiencyType::BadMuonVeto) return "BADMUON";
        return "Unknown";
    }
    inline static MuonEfficiencyType EfficiencyType(const std::string &S) {
        if (S == "RECO") return MuonEfficiencyType::Reco;
        if (S == "TTVA") return MuonEfficiencyType::TTVA;
        if (S == "ISO") return MuonEfficiencyType::Iso;
        if (S == "BADMUON") return MuonEfficiencyType::BadMuonVeto;
        return MuonEfficiencyType::Undefined;
    }
}

#endif // MUONEFFICIENCYCORRECTIONS_MUONEFFICIENCYTYPE_H

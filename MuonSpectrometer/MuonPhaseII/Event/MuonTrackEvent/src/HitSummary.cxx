/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackEvent/HitSummary.h"

#include <cassert>
namespace{
    std::string whiteSpaces(std::size_t n) {
        std::string s{};
        s.assign(n, ' ');
        return s;
    }
}

namespace MuonR4{
    std::string HitSummary::toString(const HitCategory c) {
        switch (c) {
            using enum HitCategory;
            case Precision: return "Precision";
            case TriggerEta: return "TriggerEta";
            case TriggerPhi: return "TriggerPhi";
            case sTgcPad: return "sTgcPad";
            case nCategories: return "nCategories";
        }
        return "Unknown";
    }
    std::string HitSummary::toString(const Status s) {
        switch(s) {
            using enum Status;
            case OnTrack: return "OnTrack";
            case Outlier: return "Outlier";
            case Hole: return "Hole";
            case MaxValue: return "MaxValue";
        }
        return "Unknown";
    }
    unsigned HitSummary::translate(const HitCategory cat, const Status status,
                                    LayerIndex layer, const bool isSmall) const {
       using namespace Muon::MuonStationIndex;
       /// Map the barrel extended ->  extended index
       if (layer == LayerIndex::BarrelExtended) {
            layer = LayerIndex::Extended;
       }
       constexpr unsigned A = toInt(Status::MaxValue);
       constexpr unsigned AxB = A * toInt(HitCategory::nCategories);
       constexpr unsigned AxBxC = 2 *AxB;
       const unsigned idx = AxBxC*toInt(layer) + AxB*toInt(isSmall) + A*toInt(cat)  + toInt(status);
       assert(idx < m_counts.size());
       return idx;
    }
    HitSummary::value_type 
        HitSummary::value(const HitCategory cat, const Status status,
                          const LayerIndex layer, const bool isSmall) const {
        return m_counts[translate(cat, status, layer, isSmall)];
    }       
    HitSummary::value_type&
        HitSummary::value(const HitCategory cat, const Status status,
                          const LayerIndex layer, const bool isSmall) {
        return m_counts[translate(cat, status, layer, isSmall)];
    }
    void HitSummary::print(std::ostream& ostr) const {
        using ColumnArray_t = std::array<std::string, 6>;
        std::vector<ColumnArray_t> summaryTable{ColumnArray_t{"layer", "sector", "type",
                                                            "on-track", "outlier", "hole"}};

        for (const auto lay: {LayerIndex::Inner, LayerIndex::Middle, LayerIndex::Extended,
                              LayerIndex::Outer}){            
            for (const bool small: {false, true}) {
                for (const auto cat : {HitCategory::Precision, HitCategory::TriggerEta,
                                       HitCategory::TriggerPhi, HitCategory::sTgcPad}){
                    const unsigned onTrk = value(cat, Status::OnTrack, lay, small);
                    const unsigned outlier = value(cat, Status::Outlier, lay, small);
                    const unsigned hole = value(cat, Status::Hole, lay, small);
                    if (onTrk + outlier + hole == 0u) {
                        continue;
                    }
                    summaryTable.emplace_back(ColumnArray_t{Muon::MuonStationIndex::layerName(lay),
                                                            (small ? "small" : "large"),
                                                            toString(cat), std::to_string(onTrk),
                                                            std::to_string(outlier), std::to_string(hole)});
                }
            }
        }
        std::array<std::size_t, 6> widths{};
        for (const ColumnArray_t& row : summaryTable) {
            for (std::size_t c = 0 ; c < row.size(); ++c) {
                widths[c] = std::max(widths[c], row[c].size());
            }
        }
        for (const ColumnArray_t& row : summaryTable) {
            ostr<<"|";
            for (std::size_t c = 0; c < row.size(); ++c) {
                const std::size_t W = widths[c] - row[c].size(); 
                const std::size_t nWL = (W - W % 2) / 2;
                const std::size_t nWR = (W - W % 2) / 2 + W%2;
                ostr<<" "<<whiteSpaces(nWL)<<row[c]<<whiteSpaces(nWR)<<" |";
            }
            ostr<<std::endl;
        }
    }
}
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonTrackEvent/HitSummary.h>
#include <stdlib.h>
#include <iostream>

int main() {
    using namespace Muon::MuonStationIndex;
    using namespace MuonR4;
    using Cat_t = HitSummary::HitCategory;
    using Stat_t = HitSummary::Status;

    HitSummary summary{};
    std::cout<<summary<<std::endl;
   
    std::uint8_t c = 0;
     for (const auto lay : {LayerIndex::Inner, LayerIndex::Middle, LayerIndex::Extended, LayerIndex::Outer}) {
        for (const auto small: {false, true}){
            for (const auto cat : {Cat_t::Precision, Cat_t::TriggerEta, Cat_t::TriggerPhi}){
                for (const auto stat: { Stat_t::OnTrack, Stat_t::Outlier, Stat_t::Hole}){
                    summary.value(cat,stat,lay, small) = (++c);
                }
            }
        }
    }
    std::cout<<summary<<std::endl;
    c = 0;
    for (const auto lay : {LayerIndex::Inner, LayerIndex::Middle, LayerIndex::Extended, LayerIndex::Outer}) {
        for (const auto small: {false, true}){
            for (const auto cat : {Cat_t::Precision, Cat_t::TriggerEta, Cat_t::TriggerPhi}){
                for (const auto stat: { Stat_t::OnTrack, Stat_t::Outlier, Stat_t::Hole}){
                    if (summary.value(cat,stat,lay, small) != (++c)){
                        return EXIT_FAILURE;
                    }
                }
            }
        }
    }
  
    return EXIT_SUCCESS;
}
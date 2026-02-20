/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonPRDTestR4/TrackSummaryModule.h"

#include "MuonTrackEvent/MsTrackSeed.h"

#include <format>
using namespace MuonR4;
using namespace Muon::MuonStationIndex;

namespace MuonValR4{

    using Category = MuonR4::HitSummary::HitCategory;
    using Status = MuonR4::HitSummary::Status;
    inline std::string nameBr(Category cat, Status status,
                              LayerIndex layer, bool isSmall) {
        std::string lName = layerName(layer);
        lName[0] = std::tolower(lName[0]);
        return std::format("{:}{:}{:}{:}",
                    lName, (isSmall ? "Small" :"Large"),
                    HitSummary::toString(cat),
                    (status == Status::OnTrack ? std::string{"Hits"} 
                                               : HitSummary::toString(status) +"s"));
    }
    
    TrackSummaryValueBranch::TrackSummaryValueBranch(TTree* tree,
                                                     const std::string& collName,
                                                     Category cat, Status status,
                                                     LayerIndex layer, bool isSmall):
        VectorBranch<std::uint8_t>(tree,
                                   std::format("{:}_{:}",
                                        collName, nameBr(cat, status, layer, isSmall))),
        m_cat{cat}, m_status{status}, m_layer{layer}, m_isSmall{isSmall}{}
    void TrackSummaryValueBranch::push_back(const MuonR4::HitSummary& summary){
        push_back(summary.value(m_cat, m_status, m_layer, m_isSmall));
    }


    TrackSummaryModule::TrackSummaryModule(MuonVal::MuonTesterTree& parent,
                                            const std::string& collName,
                                            const MuonR4::ITrackSummaryTool* summaryTool):
        MuonTesterBranch{parent, " track summary " + collName},
        m_summaryTool{summaryTool} {
        for (const auto layer : {LayerIndex::Inner, LayerIndex::Middle,
                                 LayerIndex::Outer, LayerIndex::Extended}) {
            for (const auto cat : {Category::Precision, Category::TriggerEta, Category::TriggerPhi}){
                /// Extended chambers don't have any complementary trigger chambers
                if ( (layer == LayerIndex::BarrelExtended || layer == LayerIndex::Extended) &&
                      (cat != Category::Precision)){
                    continue;
                }
                for (const auto status : {Status::OnTrack, Status::Outlier, Status::Hole}) {
                    for (const bool small : {false, true}) {
                        if (layer == LayerIndex::BarrelExtended && !small){
                            continue;
                        }
                        m_values.emplace_back(std::make_shared<TrackSummaryValueBranch>(parent.tree(), collName,
                                                                                        cat, status, layer,
                                                                                        small));
                    }
                }
            }
        }
        for (const auto& br : m_values){
            parent.addBranch(br);
        }
    }
    bool TrackSummaryModule::fill(const EventContext& /*ctx*/) {
        return true;
    }   
    bool TrackSummaryModule::init() {
        return true;
    }
    
    void TrackSummaryModule::push_back(const EventContext& ctx, const ConstTrack_t track) {
        push_back(m_summaryTool->makeSummary(ctx, track));
    }
    void TrackSummaryModule::push_back(const EventContext& ctx, const MsTrackSeed& seed) {
        push_back(m_summaryTool->makeSummary(ctx, seed.segments()));
    }
    void TrackSummaryModule::push_back(const HitSummary& summary) {
        for(const auto& br : m_values){
            br->push_back(summary);
        }
    }
}
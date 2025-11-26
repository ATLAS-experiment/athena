/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONPRDTESTR4_TRACKSUMMARYMODULE_H
#define MUONPRDTESTR4_TRACKSUMMARYMODULE_H

#include "MuonPRDTestR4/TesterModuleBase.h"
#include "MuonRecToolInterfacesR4/ITrackSummaryTool.h"
#include "ActsEvent/TrackContainer.h"
#include "MuonTrackEvent/HitSummary.h"


namespace MuonValR4{
    /** @brief Helper branch class to dump a pick a single hit summary value from the
     *         overall summary & to dump it in the tree */
    class TrackSummaryValueBranch : public MuonVal::VectorBranch<std::uint8_t> {
        public:
            using Category = MuonR4::HitSummary::HitCategory;
            using Status = MuonR4::HitSummary::Status;
            using LayerIndex = MuonR4::HitSummary::LayerIndex;
            /** @brief Constructor taking the reference to the tree & the summary
             *         values together with an overall collection name
             *  @param tree: TTree to which the branch is appended
             *  @param collName: Name of the hit summary collection
             *  @param cat: HitCategory of the summary to be picked by this instance
             *  @param status: HitStatus of the summary to be picked by this instance
             *  @param layer: Spectrometer lay to be picked by this instance
             *  @param isSmall: Pick the summary from the large or the small chambers*/
            TrackSummaryValueBranch(TTree* tree,
                                    const std::string& collName,
                                    Category cat,
                                    Status status,
                                    LayerIndex layer,
                                    bool isSmall);
            using VectorBranch<std::uint8_t>::push_back;
            /** @brief push back the assigned hit summary value */
            void push_back(const MuonR4::HitSummary& summary);
        private:
            Category m_cat{Category::nCategories};
            Status m_status{Status::MaxValue};
            LayerIndex m_layer{LayerIndex::LayerIndexMax};
            bool m_isSmall{false};
    };

    class TrackSummaryModule: public MuonVal::MuonTesterBranch {
        public:
            /** @brief Constructor */
            TrackSummaryModule(MuonVal::MuonTesterTree& parent,
                                const std::string& collName,
                                const MuonR4::ITrackSummaryTool* summaryTool);
            /// Clears vector in cases that it has not been updated in this event
            /// Returns false if the vector has not been initialized yet
            bool fill(const EventContext& ctx) override;
            /// Initialized the Branch
            bool init() override;

            using ConstTrack_t = MuonR4::ITrackSummaryTool::ConstTrack_t;

            void push_back(const EventContext& ctx, const ConstTrack_t track);
            void push_back(const EventContext& ctx, const MuonR4::MsTrackSeed& seed);
            void push_back(const MuonR4::HitSummary& summary);


        private:
            using ValuePtr_t = std::shared_ptr<TrackSummaryValueBranch>;
            std::vector<ValuePtr_t> m_values{};
            const MuonR4::ITrackSummaryTool* m_summaryTool{nullptr};
            
    };
}

#endif
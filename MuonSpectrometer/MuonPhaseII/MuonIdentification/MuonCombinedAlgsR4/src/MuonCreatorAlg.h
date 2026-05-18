/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCOMBINEDALGSR4_MUONCREATORALG_H
#define MUONCOMBINEDALGSR4_MUONCREATORALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"


#include "xAODMuonViews/FillContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonAuxContainer.h"

#include "MuonTrackEvent/MuonTag.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "MuonRecToolInterfacesR4/ITrackSummaryTool.h"
namespace MuonCombinedR4 {
    /** @brief Algorithm to create the xAOD::Muon analysis objects from
     *         the various muon reconstruction chains. The configured MuonTag
     *         containers are grouped by the commonly shared ID track particle
     *         and the tags are then ordered to define the muon's momentum
     *         and author. The parameters and track summaries are copied onto
     *         the muon as well as the links to the associated track particles */
    class MuonCreatorAlg : public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;

            
            /** @brief Helper struct to ship the data containers across the methods*/
            struct DataShip {
                /** @brief Define the muon container type */
                using MuonCont_t = xAOD::FillContainer<xAOD::MuonContainer,
                                                       xAOD::MuonAuxContainer>;
                
                /** @brief The output muon container handle */
                MuonCont_t muons{};
                /** @brief Abrivation a vector of tags */
                using TagVec_t = std::vector<const MuonR4::MuonTag*>;
                /** @brief Group the muon tags by the associated ID track
                 *         Later apply an ordering according to the author */
                using TagMap_t = std::map<const xAOD::TrackParticle*,
                                          TagVec_t, MuonR4::ParticleSorter>;

                /** @brief Collect all Muon tags with an ID track to form
                 *         proper combined reconstruction */
                TagMap_t combinedTags{};
                /** @brief The vector of all muon tags without an associated
                 *         ID track (I.e. the standalone MS reconstruction chain) */
                TagVec_t standaloneTags{};
            };
            /** @brief Abrivate the ElementLink to the track particles */
            using TrackLink_t = ElementLink<xAOD::TrackParticleContainer>;
        private:
            /** @brief The Muon selection tool to assess the muon quality */
            ToolHandle<CP::IMuonSelectionTool> m_selectionTool{this, "SelectionTool", ""};
            /** @brief Key name to store the primary muon container  */
            SG::WriteHandleKey<xAOD::MuonContainer> m_muonKey{this, "MuonKey", "Muons"};
            /** @brief Key name under which the input tags can be found*/
            SG::ReadHandleKeyArray<MuonR4::MuonTagContainer> m_tagKeys{this, "TagKeys", {}};
           /** @brief Handle to the muon summary tool */
            ToolHandle<MuonR4::ITrackSummaryTool> m_summaryTool{this, "SummaryTool" , ""};
            /** @brief Load all containers from store gate and setup the output containers  */
            StatusCode setupDataShip(const EventContext& ctx, DataShip& ship) const;
        
            /** @brief Create the xAOD::Muon from the list of muon tags. The first tag
             *         in the list seves as primary author and to define the muon's 
             *         momentum. The remaining ones just copy the parameters and
             *         additional track particle links onto the muon. 
             *  @param ctx: EventContext to establish the ElementLink
             *  @param muTags: Sorted list of muon tags to be transformed into a 
             *                 muon object
             *  @param ship: The data holder in the event to whose containers
             *               the muon is appended to */
            void createMuon(const EventContext& ctx,
                            const std::span<const MuonR4::MuonTag* const>& muTags,
                            DataShip& ship) const;

            /** @brief Constructs an element link to the passed track particle
             *         If a nullptr is passed, an empty link is returned
             *  @param ctx: The current event context
             *  @param trk: Pointer to the track particle for which the link is to be
             *              constructed */
            TrackLink_t linkParticle(const EventContext& ctx,
                                     const xAOD::TrackParticle* trk) const;
    };

}
#endif
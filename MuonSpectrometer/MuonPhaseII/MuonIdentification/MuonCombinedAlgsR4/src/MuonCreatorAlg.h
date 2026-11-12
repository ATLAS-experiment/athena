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
#include "xAODMuon/MuonAuxContainerR4.h"

#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"
#include "CaloEvent/CaloClusterCellLinkContainer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloConditions/CaloNoise.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "TrackToCalo/CaloCellCollector.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "ActsEvent/ContextUtility.h"
#include "ActsEvent/CaloExtension.h"

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
                                                       xAOD::MuonAuxContainerR4>;
                
                /** @brief The output muon container handle */
                MuonCont_t muons{};

                /** @brief The output calo cluster container handle */
                xAOD::FillContainer<xAOD::CaloClusterContainer, xAOD::CaloClusterAuxContainer> caloClusters{};
                /** @brief The output calo cluster cell link container handle */
                std::unique_ptr<CaloClusterCellLinkContainer> caloClusterLinks{std::make_unique<CaloClusterCellLinkContainer>()};
                /** @brief The calo cells used to build the muon clusters. Nullptr if the
                 *         cluster building is switched off */
                const CaloCellContainer* caloCells{nullptr};
                /** @brief The calorimeter detector description */
                const CaloDetDescrManager* caloMgr{nullptr};
                /** @brief Optional calo noise object to compute the ET core */
                const CaloNoise* caloNoise{nullptr};


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

            /** @brief Calo clusters built from the cells along the calo extension of the muon's ID track.
             *         An empty key switches the cluster building off */
            SG::WriteHandleKey<xAOD::CaloClusterContainer> m_clusterContainerName{this, "ClusterContainerName", "MuonClusterCollection",
                                                                          "Clusters"};
            /** @brief Cell links of the clusters. Set to <ClusterContainerName>_links in the python configuration */
            SG::WriteHandleKey<CaloClusterCellLinkContainer> m_clusterContainerLinkName{this, "CaloClusterCellLinkName", "MuonClusterCollection_links",
                                                                                "Cluster links"};

            /** @brief Switch the cluster building on or off */
            Gaudi::Property<bool> m_useCaloCells{this, "UseCaloCells", true};
            /** @brief Clusters are built: UseCaloCells is set and the cluster container key is not empty */
            bool m_useCells{false};
            /** @brief Calo cell container used to build the clusters */
            SG::ReadHandleKey<CaloCellContainer> m_cellContainerName{this, "CaloCellContainer", "AllCalo", "calo cells"};
            /** @brief Calorimeter detector description */
            SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this, "CaloDetDescrManager", "CaloDetDescrManager"};
            /** @brief CaloNoise object to apply the noise cut in the ET core, or blank */
            SG::ReadCondHandleKey<CaloNoise> m_caloNoiseKey{this, "CaloNoise", "", "CaloNoise object to use, or blank."};
            /** @brief Noise threshold in sigmas applied when computing the ET core */
            Gaudi::Property<float> m_sigmaCaloNoiseCut{this, "SigmaCaloNoiseCut", 3.4};
            /** @brief Collector of the calo cells and the core energy */
            Rec::CaloCellCollector m_cellCollector{};
            /** @brief Provider of the Acts geometry context */
            ActsTrk::ContextUtility m_ctxProvider{this};

           /** @brief Handle to the muon summary tool */
            ToolHandle<MuonR4::ITrackSummaryTool> m_summaryTool{this, "TrackSummaryTool" , ""};
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

            /** @brief Builds the calo cluster from the cells along the calo extension of the
             *         muon's ID track, links it to the muon and fills the core energy
             *         decorations (ET_Core, ET_EMCore, ET_TileCore, ET_HECCore).
             *  @param ctx: EventContext to establish the ElementLink
             *  @param muon: Muon to which the cluster is attached
             *  @param ship: The data holder providing the cluster container and cell inputs */
            void collectCells(const EventContext& ctx,
                              xAOD::Muon& muon,
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
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  ** @brief This alg receives the container of truth muons and decorates them 
  *         with hit counts and coincidences in different positions of the detector,
  *         as well as hits'ID classified by chamber technology.
*/

#pragma once

#include <GaudiKernel/StatusCode.h>
#include <map>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "StoreGate/WriteDecorHandle.h"

namespace Muon {

    class MuonTruthHitCountsAlg : public AthReentrantAlgorithm {
    public:

        // Constructor with parameters:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;

        // Basic algorithm methods:
        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;

        /** @brief This map contains all the hits corresponding to truth muons classified by chamber layer that
        *          recorded them. ChIndex is the chamber layer  index (e.g. BMS, BOL,...), while Identifier is the hit ID,
        *          which represents the channel where the hit has been recorded.     
        */
        using ChamberIdMap = std::map<Muon::MuonStationIndex::ChIndex, std::vector<Identifier>>;

    private:
        /** @brief Struct object encapsulating all write-decorators. See the keys declaration below for the full list of
        *          decorators. As WriteDecor do not have default empty constructor, we cannot decouple declaration and population 
        *          of this object. Therefore we define a constuctor taking @param parent alg instance, needed for the decorator keys,
        *          and @param ctx event context. In this way when we build a struct instance we automatically retrieve all 
        *          write-decorators for that event context.
        */
        using WriteDecor_uint8_t =  SG::WriteDecorHandle<xAOD::TruthParticleContainer, uint8_t>;
        using WriteDecor_llvec =  SG::WriteDecorHandle<xAOD::TruthParticleContainer, std::vector<unsigned long long>>;
        friend struct summaryDecors;
        struct summaryDecors{
            
            explicit summaryDecors(const MuonTruthHitCountsAlg* parent,
                          const EventContext& ctx) :
                m_parent{parent},
                m_ctx{ctx} {}
            
            const MuonTruthHitCountsAlg* m_parent{};
            const EventContext& m_ctx;

            WriteDecor_uint8_t nprecLayersDecor{m_parent->m_nprecLayersKey, m_ctx};
            WriteDecor_uint8_t nphiLayersDecor{m_parent->m_nphiLayersKey, m_ctx};
            WriteDecor_uint8_t ntrigEtaLayersDecor{m_parent->m_ntrigEtaLayersKey, m_ctx};
            WriteDecor_uint8_t innerSmallHitsDecor{m_parent->m_innerSmallHitsKey, m_ctx};
            WriteDecor_uint8_t innerLargeHitsDecor{m_parent->m_innerLargeHitsKey, m_ctx};
            WriteDecor_uint8_t middleSmallHitsDecor{m_parent->m_middleSmallHitsKey, m_ctx};
            WriteDecor_uint8_t middleLargeHitsDecor{m_parent->m_middleLargeHitsKey, m_ctx};
            WriteDecor_uint8_t outerSmallHitsDecor{m_parent->m_outerSmallHitsKey, m_ctx};
            WriteDecor_uint8_t outerLargeHitsDecor{m_parent->m_outerLargeHitsKey, m_ctx};
            WriteDecor_uint8_t extendedSmallHitsDecor{m_parent->m_extendedSmallHitsKey, m_ctx};
            WriteDecor_uint8_t extendedLargeHitsDecor{m_parent->m_extendedLargeHitsKey, m_ctx};
            WriteDecor_uint8_t phiLayer1HitsDecor{m_parent->m_phiLayer1HitsKey, m_ctx};
            WriteDecor_uint8_t phiLayer2HitsDecor{m_parent->m_phiLayer2HitsKey, m_ctx};
            WriteDecor_uint8_t phiLayer3HitsDecor{m_parent->m_phiLayer3HitsKey, m_ctx};
            WriteDecor_uint8_t phiLayer4HitsDecor{m_parent->m_phiLayer4HitsKey, m_ctx};
            WriteDecor_uint8_t etaLayer1HitsDecor{m_parent->m_etaLayer1HitsKey, m_ctx};
            WriteDecor_uint8_t etaLayer2HitsDecor{m_parent->m_etaLayer2HitsKey, m_ctx};
            WriteDecor_uint8_t etaLayer3HitsDecor{m_parent->m_etaLayer3HitsKey, m_ctx};
            WriteDecor_uint8_t etaLayer4HitsDecor{m_parent->m_etaLayer4HitsKey, m_ctx};

            std::unique_ptr<WriteDecor_llvec> truthMdtHitsDecor{m_parent->m_idHelperSvc->hasMDT() ? std::make_unique<WriteDecor_llvec>(m_parent->m_truthMdtHitsKey, m_ctx) : nullptr};
            std::unique_ptr<WriteDecor_llvec> truthTgcHitsDecor{m_parent->m_idHelperSvc->hasTGC() ? std::make_unique<WriteDecor_llvec>(m_parent->m_truthTgcHitsKey, m_ctx) : nullptr};
            std::unique_ptr<WriteDecor_llvec> truthRpcHitsDecor{m_parent->m_idHelperSvc->hasRPC() ? std::make_unique<WriteDecor_llvec>(m_parent->m_truthRpcHitsKey, m_ctx) : nullptr};
            std::unique_ptr<WriteDecor_llvec> truthCscHitsDecor{m_parent->m_idHelperSvc->hasCSC() ? std::make_unique<WriteDecor_llvec>(m_parent->m_truthCscHitsKey, m_ctx) : nullptr};
            std::unique_ptr<WriteDecor_llvec> truthStgcHitsDecor{m_parent->m_idHelperSvc->hasSTGC() ? std::make_unique<WriteDecor_llvec>(m_parent->m_truthStgcHitsKey, m_ctx) : nullptr};
            std::unique_ptr<WriteDecor_llvec> truthMMHitsDecor{m_parent->m_idHelperSvc->hasMM() ? std::make_unique<WriteDecor_llvec>(m_parent->m_truthMMHitsKey, m_ctx) : nullptr};

        };

        /** @brief This is the actual function that decorates truth muons with hit counts. For each truth muon, we loop over 
        *          the detector technologies and over the recorded hits. If a hit originated by that muon is found, we store 
        *          the hit ID into a map (=ids), which contains all hits corresponding to truth muons classified by the chamber
        *          that recorded them. In addition, we update the following counters: 
        *          -* nprecHitsPerChamberLayer[] counts the hits recorded by precision chambers (MDTs, MM, sTGCs with no phi  
        *          measurement), classified by chamber layer (eg. BIS, BML, ...); 
        *          -* nphiHitsPerChamberLayer[] counts the hits recorded by chambers providing phi-measurement 
        *          (sSTGs+trigger+others chambers with phi measurement), classified by phi chamber layer (e.g. BM1, BM2, BO1, ...); 
        *          -* ntrigEtaHitsPerChamberLayer[] counts the hits recorded by trigger chambers (RPC, TGC) with only eta measurement, 
        *          classified by phi chamber layer.
        *          At the end, we use this information to perfom further classification and save results (i.e. resulting counters) to decorators:
        *          -- hits from precision chambers classified by inner/middle/outer/extended stations and small/large sectors
        *          -- hits with phi-measurment classified by 1/2/3/4 phi chamber layers
        *          -- hits from trigger chambers with only-eta measurement classified by 1/2/3/4 phi chamber layers
        *          -- coincidences between chamber layers (> 2-3) considering hits from precision chambers
        *          -- coincidences between phi chamber layers (> 0-2-3) considering hits with phi-measurement
        *          -- coincidences between phi chamber layers (> 0-2-3) considering hits from trigger chambers with only-eta measurement    
        **  @param ctx: tells in which event we are
        **  @param truthParticle: reference to truth muon
        **  @param ids: reference to the empty chamberIdMap object that is going to be populated with truth muons'hits
        **  @param myDecors: reference to the summary write-decorator object
        */
        StatusCode addHitCounts(const EventContext& ctx,
                                const xAOD::TruthParticle& truthParticle,
                                ChamberIdMap& ids,
                                summaryDecors& myDecors) const;

        /** @brief This function collapses the information given by the ChamberIdMap, which contains all hits from a truth muon classified
        *          by chamber, into 6 vectors carrying these hits categorized by detector technology (MDT, TGC, RPC, sTGC, CSC, MM).
        **  @param truthParticle: reference to truth muon
        **  @param ids: reference to the previously populated chamberIdMap object
        **  @param myDecors: reference to the summary write-decorator object
        */
        StatusCode addHitIDVectors(const xAOD::TruthParticle& truthParticle, 
                            const ChamberIdMap& ids,
                            summaryDecors& myDecors) const;
        
        /** @brief  Key of the container of truth muons */
        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_muonTruth{this, "muonTruth", "MuonTruthParticles"};

        /** @brief  Keys for the containers of truth hits, grouped by detector technology. Each element of the container has a 
        *           hit ("sensor" or "channel") ID, representing the detector channel where the hit has been recorded, and a 
        *           link to the MC particle that generated the hit 
        */
        SG::ReadHandleKeyArray<PRD_MultiTruthCollection> m_PRD_TruthNames{
            this, "PRD_TruthMaps", {"CSC_TruthMap", "RPC_TruthMap", "TGC_TruthMap", "MDT_TruthMap"}, "remove NSW by default for now, can always be changed in the configuration"};

        /** @brief  Key for each write-decorator. List of decorators: nprecLayers counts coincidences between chamber layers (> 2-3)  
        *           considering hits from precision chambers; nphiLayers counts coincidences between phi chamber layers (> 0-2-3) considering hits 
        *           with phi-measurement; ntrigEtaLayers counts coincidences between phi chamber layers (> 0-2-3) considering hits from trigger chambers
        *           with only-eta measurement; innerSmallHits, innerLargeHits, middleSmallHits, middleLargeHits, outerSmallHits, outerLargeHits,
        *           extendedSmallHits, extendedLargeHits count hits from precision chambers classified by inner/middle/outer/extended stations and 
        *           small/large sectors; phiLayerjHits j=1,2,3,4 count hits with phi-measurment classified by 1/2/3/4 phi chamber layers; etaLayerjHits j=1,2,3,4 
        *           count hits from trigger chambers with only-eta measurement classified by 1/2/3/4 phi chamber layers; truthMdtHits, truthTgcHits, truthRpcHits,
        *           truthCscHits, truthStgcHits, truthMMHits are vectors carrying hit IDs categorized by detector technology (MDT, TGC, RPC, sTGC, CSC, MM).
        */
        using WriteDecorKey_t = SG::WriteDecorHandleKey<xAOD::TruthParticleContainer>;
        WriteDecorKey_t m_nprecLayersKey{this, "nprecLayersKey", m_muonTruth, "nprecLayers", ""};
        WriteDecorKey_t m_nphiLayersKey{this, "nphiLayersKey", m_muonTruth, "nphiLayers", ""};
        WriteDecorKey_t m_ntrigEtaLayersKey{this, "ntrigEtaLayersKey", m_muonTruth, "ntrigEtaLayers", ""};
        WriteDecorKey_t m_innerSmallHitsKey{this, "innerSmallHitsKey", m_muonTruth, "innerSmallHits", ""};
        WriteDecorKey_t m_innerLargeHitsKey{this, "innerLargeHitsKey", m_muonTruth, "innerLargeHits", ""};
        WriteDecorKey_t m_middleSmallHitsKey{this, "middleSmallHitsKey", m_muonTruth, "middleSmallHits", ""};
        WriteDecorKey_t m_middleLargeHitsKey{this, "middleLargeHitsKey", m_muonTruth, "middleLargeHits", ""};
        WriteDecorKey_t m_outerSmallHitsKey{this, "outerSmallHitsKey", m_muonTruth, "outerSmallHits", ""};
        WriteDecorKey_t m_outerLargeHitsKey{this, "outerLargeHitsKey", m_muonTruth, "outerLargeHits", ""};
        WriteDecorKey_t m_extendedSmallHitsKey{this, "extendedSmallHitsKey", m_muonTruth, "extendedSmallHits", ""};
        WriteDecorKey_t m_extendedLargeHitsKey{this, "extendedLargeHitsKey", m_muonTruth, "extendedLargeHits", ""};
        WriteDecorKey_t m_phiLayer1HitsKey{this, "phiLayer1HitsKey", m_muonTruth, "phiLayer1Hits", ""};
        WriteDecorKey_t m_phiLayer2HitsKey{this, "phiLayer2HitsKey", m_muonTruth, "phiLayer2Hits", ""};
        WriteDecorKey_t m_phiLayer3HitsKey{this, "phiLayer3HitsKey", m_muonTruth, "phiLayer3Hits", ""};
        WriteDecorKey_t m_phiLayer4HitsKey{this, "phiLayer4HitsKey", m_muonTruth, "phiLayer4Hits", ""};
        WriteDecorKey_t m_etaLayer1HitsKey{this, "etaLayer1HitsKey", m_muonTruth, "etaLayer1Hits", ""};
        WriteDecorKey_t m_etaLayer2HitsKey{this, "etaLayer2HitsKey", m_muonTruth, "etaLayer2Hits", ""};
        WriteDecorKey_t m_etaLayer3HitsKey{this, "etaLayer3HitsKey", m_muonTruth, "etaLayer3Hits", ""};
        WriteDecorKey_t m_etaLayer4HitsKey{this, "etaLayer4HitsKey", m_muonTruth, "etaLayer4Hits", ""};

        WriteDecorKey_t m_truthMdtHitsKey{this, "truthMdtHitsKey", m_muonTruth, "truthMdtHits", ""};
        WriteDecorKey_t m_truthTgcHitsKey{this, "truthTgcHitsKey", m_muonTruth, "truthTgcHits", ""};
        WriteDecorKey_t m_truthRpcHitsKey{this, "truthRpcHitsKey", m_muonTruth, "truthRpcHits", ""};
        WriteDecorKey_t m_truthCscHitsKey{this, "truthCscHitskey", m_muonTruth, "truthCscHits", ""};
        WriteDecorKey_t m_truthStgcHitsKey{this, "truthStgcHitsKey", m_muonTruth, "truthStgcHits", ""};
        WriteDecorKey_t m_truthMMHitsKey{this, "truthMMHitsKey", m_muonTruth, "truthMMHits", ""};

        /** @brief Handle for muonIdHelper service */
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    };

}  // namespace Muon

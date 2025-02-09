/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  ** @brief This alg receives the container of truth muons, selects good muons according to 
  *         their origin, creates segments, and store them into a new container. Segments are 
  *         decorated with number and type of hits available, link to the 
  *         corresponding truth muon, position and direction of the truth segment, and
  *         sector, chamber layer, station eta, and technology of the chamber where the 
  *         segment has been recorded.
*/

#pragma once

#include <map>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "Identifier/Identifier.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"

class MuonSimDataCollection;
class CscSimDataCollection;

namespace MuonGM {
    class MuonDetectorManager;
}

namespace Muon {

    class MuonTruthSegmentCreationAlg : public AthReentrantAlgorithm {
    public:

        // Constructor with parameters:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;

        // Basic algorithm methods:
        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;

        /** @brief This map contains all the hits corresponding to truth muons classified by chamber layer that
        *          recorded them. ChIndex is the chamber layer index (e.g. BMS, BOL,...), while Identifier is the hit ID,
        *          which represents the channel where the hit has been recorded.     
        */
        using ChamberIdMap = std::map<Muon::MuonStationIndex::ChIndex, std::vector<Identifier>>;

    private:

        /** @brief This function uses the 6 vectors, contained in @param hitsCollection , carrying truth muon hit IDs categorized 
        *          by detector technology (MDT, TGC, RPC, sTGC, CSC, MM) to (re-)fill the @param ids ChamberIdMap, which contains all
        *          truth muon hit IDs classified by chamber layer. This function retrieves the chamber layer from the hit ID by means of
        *          the muonIdHelper tool.
        **  @param hitsCollection: hit collection
        **  @param ids: empty chamberIdMap
        */
        StatusCode fillChamberIdMap(const EventContext& ctx, const xAOD::TruthParticle& truthParticle, ChamberIdMap& ids) const;

        /** @brief This function performs, for each truth muon, the actual segment creation and stores segments into a new container  
        *          @param segmentContainer. Firstly, we retrieve simulation data from m_SDO_TruthNames and copy it into a new vector, 
        *          keeping only the data corresponding to the correct detector technologies. Then, we loop over chamber layers and over 
        *          the corresponding hit IDs, both provided by @param ids , and we store: 
        *          *- gas-gap for hits with a phi measurement -> phiLayers
        *          *- gas-gap for hits recorded by MM or CSC -> precLayers
        *          *- 10*multi-layer + tube_layer for hits recorded by MDT -> precLayers
        *          *- gas-gap for hits recorded by trigger chambers without phi measurement -> etaLayers
        *          Then, in the same loop over hit IDs, we retrieve simulation data for that hit, specifically the global position, 
        *          needed to retrieve for each chamber layer the pair of hits with the largest distance. If a pair of hits is found 
        *          and at least three precision measurements are available, we proceed with the truth segment creation. We create a segment 
        *          obj and we fill it with number and type (see above) of hits available, the @param truthLink link to the corresponding
        *          truth muon, and the position and direction of the truth segment, calculated with hit pair global positions. Finally, 
        *          we append the sector, chamber layer, station eta, and technology of the chamber where the segment was recorded.
        ** @param  ctx: tells us in which event we are
        ** @param  truthLink: link to the corresponding truth muons
        ** @param  ids: chamberIdMap storing truth hit IDs classified by chamber layer
        ** @param  segmentContainer: segment container
        */
        StatusCode createSegments(const EventContext& ctx, 
                                  const ElementLink<xAOD::TruthParticleContainer>& truthLink,
                                  const ChamberIdMap& ids,
                                  xAOD::MuonSegmentContainer& segmentContainer) const;

        /** @brief  Key for the truth muon container and muon origin decoration */                          
        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_muonTruth{this, "muonTruth", "MuonTruthParticles"};
        SG::ReadDecorHandleKey<xAOD::TruthParticleContainer> m_truthOriginKey{this, "truthOriginKey", m_muonTruth, "truthOrigin"};

        /** @brief  Keys of the truth muon decorations that we need to read to (re-)fill the chamberIdMap. Each key corresponds to
        *           the decorator containing hit IDs for each muon for a specific detector technology. */
        SG::ReadDecorHandleKeyArray<xAOD::TruthParticleContainer, std::vector<unsigned long long>> m_truthHitsKeyArray{this, "truthHitsKeyArray", { }};

        /** @brief  Key for segment container that will be populated with segments */   
        SG::WriteHandleKey<xAOD::MuonSegmentContainer> m_muonTruthSegmentContainerName{this, "MuonTruthSegmentName", "MuonTruthSegments"};

        /** @brief  Keys for all çontainers of muon hit simulation data, classified by detector technology */   
        SG::ReadHandleKeyArray<MuonSimDataCollection> m_SDO_TruthNames{
            this, "SDOs", {"RPC_SDO", "TGC_SDO", "MDT_SDO" }, "remove NSW by default for now, can always be changed in the configuration"};
        SG::ReadHandleKey<CscSimDataCollection> m_CSC_SDO_TruthNames{this, "CSCSDOs", "CSC_SDO"};

        /** @brief  Handle for the muonIdHelper service */   
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        /** @brief  MuonDetectorManager from the conditions store */ 
        SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detMgrKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                      "Key of input MuonDetectorManager condition data"};

    };

}  // namespace Muon

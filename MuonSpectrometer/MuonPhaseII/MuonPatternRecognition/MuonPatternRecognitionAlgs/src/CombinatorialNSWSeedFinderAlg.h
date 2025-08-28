/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONPATTERNRECOGNITIONALGS_COMBINATORIALNSWSEEDFINDERALG_H
#define MUONR4_MUONPATTERNRECOGNITIONALGS_COMBINATORIALNSWSEEDFINDERALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <MuonSpacePoint/SpacePointContainer.h>
#include <MuonPatternEvent/MuonPatternContainer.h>

#include "MuonIdHelpers/MmIdHelper.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonPatternEvent/MuonHoughDefs.h"
#include "MuonRecToolInterfacesR4/IPatternVisualizationTool.h"
#include <MuonSpacePoint/SpacePointPerLayerSplitter.h>

#include <span>
#include <vector>
 

namespace MuonR4{


class CombinatorialNSWSeedFinderAlg : public AthReentrantAlgorithm {
  
    public:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;
        virtual ~CombinatorialNSWSeedFinderAlg() = default;
        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;    

    private:
        
        /** @brief Enumeration to classify the orientation of a NSW strip  */
        enum class StripOrient{
          U, /// Stereo strips with positive angle
          V, /// Stereo strips with negative angle
          X,  /// Ordinary eta strips
          Unknown
        };
        /** @brief Determines the orientation of the strip space point */
        StripOrient classifyStrip(const SpacePoint& spacePoint) const;

        using HitVec = SpacePointPerLayerSplitter::HitVec;

        using HitLayVec = SpacePointPerLayerSplitter::HitLayVec;
        /** @brief Abbrivation of the space comprising multiple hit vectors without copy */
        using HitLaySpan_t = std::vector<std::reference_wrapper<const HitVec>>;
        /** @brief Abbrivation of the container book keeping whether a hit is used or not */
        using UsedHitMarker_t = std::vector<std::vector<char>>;
        /** @brief Abbrivation of the container to pass a subset of markers wtihout copy */
        using UsedHitSpan_t = std::vector<std::reference_wrapper<std::vector<char>>>;
        /** @brief Abbrivation of the  */
        using InitialSeed_t = std::array<const SpacePoint*, 4>;
        /** @brief Vector of initial seeds */
        using InitialSeedVec_t = std::vector<InitialSeed_t>;
        /** @brief Constructs an empty HitMarker from the split space points
         *  @param sortedSp: List of space points sorted by layer */
        UsedHitMarker_t emptyBookKeeper(const HitLayVec& sortedSp) const;
        /** @brief To fastly check whether a hit is roughly compatible with a muon trajectory a narrow
         *         corridor is opened from the estimated beamspot to the first tested hit in the seed 
         *         finding. Hits in subsequent layers need to be within this corridor in order to be
         *         considered for seed construction. The HitWindow is the output classification of such
         *         a corridor test. */
        enum class HitWindow{tooLow, /// The hit is below the predefined corridor
                             inside, /// The hit is inside the defined window and hence an initial candidate
                             tooHigh}; /// The hit is above the predefined corridor
        /** @brief Tests whether a hit is inside the corridor defined by line connecting the centre of the
         *         first candidate hit in the seed and the beam spot. The theta angle is varied by m_windowTheta
         *         to define the lower & upper direction etimate. The function tests whether the strip then
         *         crosses the corridor.
         *  @param testHit: Reference to the hit to test
         *  @param beamSpotPos: Position of the beam spot serving as starting point
         *  @param dirEstUp: Direction vector defining the upper limit of the corridor
         *  @param dirEstDn: Direction vector defining the lower limit of the corridor */
        HitWindow hitFromIPCorridor(const SpacePoint& testHit, 
                                    const Amg::Vector3D& beamSpotPos, 
                                    const Amg::Vector3D& dirEstUp,
                                    const Amg::Vector3D& dirEstDn) const;
        /** @brief Construct a set of prelimnary seeds from the selected combinatoric layers. Quadruplets
         *         of hits, one from each layer, are formed if they are all within the the corridor as described
         *         above
         *  @param beamSpot: Position of the beam spot in the sector's frame
         *  @param combinatoricLayers: Quadruplet of four hit vectors from which the hits are retrieved
         *  @param usedHits: Mask marking hits that were already successfully added to a seed
         *  @param outVec: Reference to the output vector where the initial seeds are stored. The
         *                 vector is cleared at the beginning and capacity is allocated accordingly */
        void constructPrelimnarySeeds(const Amg::Vector3D& beamSpot,
                                      const HitLaySpan_t& combinatoricLayers,
                                      const UsedHitSpan_t& usedHits,
                                      InitialSeedVec_t& outVec) const;
        /** @brief Build the final seed from the initial seed hits and then attempt to append hits
         *         from the complementary layers onto the seed.
         *  @param initialSeed: Reference to the hit quadruplet that may form a seed
         *  @param bMatrix: Kernel matrix calculated from the layer configuration to construct the seed
         *  @param max: Refrence to the eta maximum from which the segment seed is constructed
         *  @param extensionLayers: Reference to the hits on the remaining layers of the detector
         *  @param usedExtensionHits: Refrence to the book keeper of which of the hits on the extension was already used */
        std::unique_ptr<SegmentSeed> buildSegmentSeed(const InitialSeed_t& initialSeed,
                                                      const AmgSymMatrix(2)& bMatrix, 
                                                      const HoughMaximum& max, 
                                                      const HitLaySpan_t& extensionLayers,
                                                      const UsedHitSpan_t& usedHits) const;
        void markHitsAsUsed(const SegmentSeed& seed,
                            const HitLayVec& allSortHits,
                            UsedHitMarker_t& usedHitMarker) const;
        //extend the seed with compatilbe hits using extrapolation to the layers
        HitVec extendHits(const Amg::Vector3D& startPos, 
                          const Amg::Vector3D& direction, 
                          const HitLaySpan_t& extensionLayers,
                          const UsedHitSpan_t& usedHits) const;
  
        // read handle key for the input maxima (from a previous eta-transform)
        SG::ReadHandleKey<EtaHoughMaxContainer> m_etaKey{this, "CombinatorialReadKey", "MuonHoughNswMaxima"};

         // write handle key for the otuput 
        SG::WriteHandleKey<SegmentSeedContainer> m_writeKey{this, "CombinatorialPhiWriteKey", "MuonHoughNswSegmentSeeds"};

        // access to the ACTS geometry context 
        SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

        // access to the Muon Id Helper
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        //build and return seeds from the same eta maximum
        std::vector<std::unique_ptr<SegmentSeed>> 
              findSeedsFromMaximum(const HoughMaximum& max, 
                                   const ActsGeometryContext& gctx) const;
  
        //the window in theta to search for hits in the seed extension
        DoubleProperty m_windowTheta {this, "thetaWindow", 0.5 * Gaudi::Units::deg};
        
        //apply a cut threshold in the pulls during the hit extension
        DoubleProperty m_minPullThreshold{this, "maxPull", 5.};
        
        /// Pattern visualization tool
        ToolHandle<MuonValR4::IPatternVisualizationTool> m_visionTool{this, "VisualizationTool", ""};





};

}

#endif

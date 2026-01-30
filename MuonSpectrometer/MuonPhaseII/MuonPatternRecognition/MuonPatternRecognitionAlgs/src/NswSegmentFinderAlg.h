/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONPATTERNRECOGNITIONALGS_NSWSEGMENTFINDERALG_H
#define MUONR4_MUONPATTERNRECOGNITIONALGS_NSWSEGMENTFINDERALG_H

#include <AthenaBaseComps/AthReentrantAlgorithm.h>
#include <StoreGate/WriteHandleKey.h>
#include <StoreGate/ReadCondHandleKey.h>

#include <MuonSpacePoint/SpacePointContainer.h>
#include <MuonPatternEvent/MuonPatternContainer.h>
#include <MuonPatternHelpers/SegmentLineFitter.h>

#include <MuonIdHelpers/MmIdHelper.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <MuonPatternEvent/MuonHoughDefs.h>
#include <MuonRecToolInterfacesR4/IPatternVisualizationTool.h>

#include "MuonRecToolInterfacesR4/ISpacePointCalibrator.h"


#include <MuonSpacePoint/SpacePointPerLayerSplitter.h>

#include <span>
#include <vector>
 

namespace MuonR4{


class NswSegmentFinderAlg : public AthReentrantAlgorithm {
  
    public:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;
        virtual ~NswSegmentFinderAlg() = default;
        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override;    
        virtual StatusCode finalize() override;

    private:
        
        /** @brief Enumeration to classify the orientation of a NSW strip  */
        enum class StripOrient{
          U, /// Stereo strips with positive angle
          V, /// Stereo strips with negative angle
          X, /// Ordinary eta strips
          P, /// Single phi measurements
          C, /// Combined 2D space point (sTGC wire + strip / sTgc pad)
          Unknown
        };

        /** @brief Seed statistics per sector to be printed in the end */
        class SeedStatistics{

        public:
        
        using chIdx_t = Muon::MuonStationIndex::ChIndex;
        
        SeedStatistics() = default;

         //dump seed statistics to the map
        void addToStat(const MuonGMR4::SpectrometerSector* msSector,
                       unsigned int nSeeds, 
                       unsigned int nExtSeeds,
                       unsigned int nSegments);

        // print the seed counting stats in the end of the algorithm */
        void printTableSeedStats(MsgStream& msg) const;
        
        private:

        struct SeedField{
          /** @brief number of total seeds constructed */
         unsigned int nSeeds{0};
          /** @brief number of successfully extended seeds */
          unsigned int nExtSeeds{0};
          /** @brief number of segments constucted*/
          unsigned int nSegments{0};
        };

        /** @brief sector's field to dump the seed statistics */
          struct SectorField{  
            chIdx_t chIdx{};      
            int8_t phi{0};
            int8_t eta{0};
            bool operator<(SectorField const& o) const noexcept {
              if(chIdx != o.chIdx) {
                return chIdx < o.chIdx;
              }
              if(eta != o.eta) {
                return eta < o.eta;              
              }
              return phi < o.phi;
            }
          };
          using SeedStatistic_T = std::map<SectorField, SeedField>;
          SeedStatistic_T m_seedStat{};

          std::mutex m_mutex{};

        };

        /** @brief Determines the orientation of the strip space point */
        StripOrient classifyStrip(const SpacePoint& spacePoint) const;

        using HitVec = SpacePointPerLayerSplitter::HitVec;

        using HitLayVec = SpacePointPerLayerSplitter::HitLayVec;
        /** @brief Abbrivation of the space comprising multiple hit vectors without copy */
        using HitLaySpan_t = std::vector<std::reference_wrapper<const HitVec>>;
        /** @brief Abbrivation of the container book keeping whether a hit is used or not */
        using UsedHitMarker_t = std::vector<std::vector<unsigned int>>;
        /** @brief Abbrivation of the container to pass a subset of markers wtihout copy */
        using UsedHitSpan_t = std::vector<std::reference_wrapper<std::vector<unsigned int>>>;
        /** @brief Abbrivation of the initial seed */
        using InitialSeed_t = std::array<const SpacePoint*, 4>;
        /** @brief Vector of initial seeds */
        using InitialSeedVec_t = std::vector<InitialSeed_t>;
        /** @brief Constructs an empty HitMarker from the split space points
         *  @param sortedSp: List of space points sorted by layer */
        UsedHitMarker_t emptyBookKeeper(const HitLayVec& sortedSp) const;
        /** @brief Abbrivation of the seed vector*/
        using SegmentSeedVec_t = std::vector<std::unique_ptr<SegmentSeed>>;
        /** @brief Abbrivation of the final segment vector*/
        using SegmentVec_t = std::vector<std::unique_ptr<Segment>>;
        
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
        void constructPreliminarySeeds(const Amg::Vector3D& beamSpot,
                                      const HitLaySpan_t& combinatoricLayers,
                                      const UsedHitSpan_t& usedHits,
                                      InitialSeedVec_t& outVec) const;
        /** @brief Construct a combinatorial seed from the initial 4-layer seed hits 
         *  @param initialSeed: Reference to the hit quadruplet that may form a seed
         *  @param bMatrix: Kernel matrix calculated from the layer configuration to construct the seed
         *  @param max: Refrence to the eta maximum from which the segment seed is constructed
         *  @param extensionLayers: Reference to the hits on the remaining layers of the detector
         *  @param usedHits: Refrence to the book keeper of which of the hits on the extension was already used */
        std::unique_ptr<SegmentSeed> constructCombinatorialSeed(const InitialSeed_t& initialSeed,
                                                                const AmgSymMatrix(2)& bMatrix,
                                                                const HoughMaximum& max,
                                                                const HitLaySpan_t& extensionLayers,
                                                                const UsedHitSpan_t& usedHits) const;
        /** @brief Build the final segment seed from strip like measurements using the combinatorial seeding for MicroMegas (or strip measurements) logic
         *  @param hitLayers: Reference to the hits of the strip layers
         *  @param gctx: The geometry context
         *  @param ctx: The event context
         *  @param max: Refrence to the eta maximum from which the segment seed is constructed
         *  @param beamSporPos: The beaspot position in the sector's frame to be used to constrain the hits selection
         *  @param usedHits: Refrence to the book keeper of which of the hits on the extension was already used
         *  @param useOnlyMM : Boolean to use only MM hits for the initial 4layer seed from the combinatorics */
        std::pair<SegmentSeedVec_t, SegmentVec_t> buildSegmentsFromMM(const EventContext& ctx,
                                                                      const ActsTrk::GeometryContext &gctx,                                                                     
                                                                      const HitLayVec& hitLayers,                                                                     
                                                                      const HoughMaximum& max,
                                                                      const Amg::Vector3D& beamSpotPos,
                                                                      UsedHitMarker_t& usedHits,
                                                                      bool useOnlyMM) const;
        /** @brief Build the segment for a seed from STGC 2D measurement layers directly and then attempt to append hits from the other layers  
         *  @param hitLayers : Reference to the hits of the layers
         *  @param gctx: The reference to the geometry context
         *  @param ctx: The reference to the event context
         *  @param max: Refrence to the eta maximum from which the segment seed is constructed
         *  @param beamSpotPos: The beamspot position in the sector's frame to be used to constrain the hits selection
         *  @param usedHits: Refrence to the book keeper of which of the hits on the extension was already used */
        std::pair<SegmentSeedVec_t, SegmentVec_t> buildSegmentsFromSTGC(const EventContext& ctx,
                                                                        const ActsTrk::GeometryContext &gctx,
                                                                        const HitLayVec& hitLayers,                                                                     
                                                                        const HoughMaximum& max,
                                                                        const Amg::Vector3D& beamSpotPos,
                                                                        UsedHitMarker_t& usedHits) const;
        /** @brief Fit the segment seeds
         * @param ctx The reference to the event context
         * @param gctx The reference to the Geometry Context
         * @param patternSeed The pointer to the seed of which we fit the calibrated space points */
        std::unique_ptr<Segment> fitSegmentSeed(const EventContext& ctx,
                                                const ActsTrk::GeometryContext& gctx, 
                                                const SegmentSeed *patternSeed) const;

        /** @brief Process the segment and mark the hits if it is successfully built or not by differently mark the hits as used
         *  @param segment The segment to process
         *  @param seedHits The seed hits which the segments is constructed from
         *  @param hitLayers The layers contributed to the seed
         *  @param usedHits The reference of the book keeper for the hits to mark as used 
         *  @param segments Reference to the segments otuput vector where the successfully built segments are stored*/
        void processSegment(std::unique_ptr<Segment> segment, 
                            const HitVec& seedHits, 
                            const HitLayVec& hitLayers, 
                            UsedHitMarker_t& usedHits,
                            SegmentVec_t& segments) const;

        /** @brief Hits that are used in a good seed/segment built should be flagged as used and not contribute to other seed 
         * @param spacePoints The space points to be marked as used
         * @param allSortHits All the available hits
         * @param usedHitMarker The book keeping of the hits
         * @param increase The hit counter increase
         * @param markNeighborHits Flag wether to mark hits on the layer in the vicinity
        */
        void markHitsAsUsed(const HitVec& spacePoints,
                            const HitLayVec& allSortHits,
                            UsedHitMarker_t& usedHitMarker,
                            unsigned int increase,
                            bool markNeighborHits) const;

       
        /** @brief Extend the seed with the hits from the other layers
         * @param startPos The seed position
         * @param direction The seed direction
         * @param extensionLayers The layers to which the seed is extended by extrapolation
         * @param usedHits The book keeping of the used hits to be skipped
        */
        HitVec extendHits(const Amg::Vector3D& startPos, 
                          const Amg::Vector3D& direction, 
                          const HitLaySpan_t& extensionLayers,
                          const UsedHitSpan_t& usedHits) const;

        /** @brief Find seed and segment from an eta hough maximum
         * @param max The maximum from the eta hough transform
         * @param gctx The geometry Context
         * @param ctx The event context
         */
        std::pair<SegmentSeedVec_t, SegmentVec_t>
              findSegmentsFromMaximum(const HoughMaximum& max, 
                                      const ActsTrk::GeometryContext& gctx,
                                      const EventContext& ctx) const;

        // read handle key for the input maxima (from a previous eta-transform)
        SG::ReadHandleKey<EtaHoughMaxContainer> m_etaKey{this, "CombinatorialReadKey", "MuonHoughNswMaxima"};

        //write handle key for the segment seeds container
        SG::WriteHandleKey<SegmentSeedContainer> m_writeSegmentSeedKey{this, "MuonNswSegmentSeedWriteKey", "MuonNswSegmentSeeds"};

        // write handle key for the segments container
        SG::WriteHandleKey<SegmentContainer> m_writeSegmentKey{this, "MuonNswSegmentWriteKey", "MuonNswSegments"};

        // access to the ACTS geometry context 
        SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

        // access to the Muon Id Helper
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        /// Pattern visualization tool
        ToolHandle<MuonValR4::IPatternVisualizationTool> m_visionTool{this, "VisualizationTool", ""};

        //Space point calibration tool 
        ToolHandle<ISpacePointCalibrator> m_calibTool{this, "Calibrator", "" };

        // Pointer to the line segment fitter 
        std::unique_ptr<SegmentFit::SegmentLineFitter> m_lineFitter{};
  
        //the window in theta to search for hits in the seed extension
        DoubleProperty m_windowTheta {this, "thetaWindow", 2.5 * Gaudi::Units::deg};
        
        //apply a cut threshold in the pulls during the hit extension
        DoubleProperty m_minPullThreshold{this, "maxPull", 5.};
        
        //minimum number of hits required to form a seed after extension
        UnsignedIntegerProperty m_minSeedHits{this, "minSeedHits", 4};

        //maximum number of MM Clusters that are invalid in the seed
        UnsignedIntegerProperty m_maxInvalidClusters{this, "maxInvalidClusters", 4};

        //reject also hits from the seed even if it does not lead to succesful segment
        BooleanProperty m_markHitsFromSeed{this, "markHitsFromSeed", true};

        //flag to use only MM layers for the estimation of the initial seed of the combinatorics
        BooleanProperty m_doOnlyMMCombinatorics{this, "doOnlyMMCombinatorics", false};

        //maximum number that hit is allowed to be used
        UnsignedIntegerProperty m_maxUsed{this, "maxHitIsUsed", 8};

        //minimum number of strips required for MMClusers not to be invalid
        UnsignedIntegerProperty m_minClusSize{this, "minClusterSize", 1};

        //maximum number of chi2 cut for the segment
        DoubleProperty m_maxChi2{this, "maxChi2", 5.};

        // maximum number of clusters in the layer for the seed finding
        UnsignedIntegerProperty m_maxClustersInLayer{this, "maxClustersInLayer", 8};

        //maximum number of dY window size for killing hits on the layer from the segments 
        DoubleProperty m_maxdYWindow{this, "maxdYWindow", 4.*Gaudi::Units::cm};  

        //dump statistics for the seeds per sector
        BooleanProperty m_dumpSeedStatistics{this, "dumpStatistics", true};

        std::unique_ptr<SeedStatistics> m_seedCounter ATLAS_THREAD_SAFE{};

        const MuonGMR4::MuonDetectorManager* m_detMgr{};
        
       
};

}

#endif

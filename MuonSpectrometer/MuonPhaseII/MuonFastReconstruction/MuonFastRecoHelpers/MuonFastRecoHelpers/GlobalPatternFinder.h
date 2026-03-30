/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_FASTRECONSTRUCTIONALGS_GLOBALHOUGHTRANSFORM__H
#define MUONR4_FASTRECONSTRUCTIONALGS_GLOBALHOUGHTRANSFORM__H

#include "MuonFastRecoEvent/GlobalPattern.h"
#include "MuonRecToolInterfacesR4/IFastRecoVisualizationTool.h"
#include "MuonSpacePoint/SpacePointContainer.h"
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "Acts/Utilities/KDTree.hpp"


namespace MuonR4::FastReco{
    
    /// @brief Standalone module to handle global pattern recognition. 
    /// 
    /// This tool performs global pattern recognition as the first step
    /// of the Phase-2 fast reconstruction stage. It builds global patterns
    /// of precision and non-precision hits using space-points created in 
    /// upstream algorithms. It first builds patterns in eta and then adds
    /// compatible phi-only hits to the patterns. It will optionally write
    /// the final GlobalPatterns into the event store for downstream use. 


    class GlobalPatternFinder : public AthMessaging {
        public:
            /** @brief Type alias for the station index */
            using StIndex = Muon::MuonStationIndex::StIndex;
            /** @brief Type alias for the station layer index */
            using LayerIndex = Muon::MuonStationIndex::LayerIndex;
            /** @brief Abrivation for a vector of space-point containers */
            using SpacePointContainerVec = std::vector<const SpacePointContainer*>;
            /** @brief Abrivation for a vector of global patterns */
            using PatternVec = std::vector<GlobalPattern>;
            /** @brief Abrivation for a collection of space-point buckets grouped by their corresponding input container  */
            using BucketPerContainer = std::unordered_map<const SpacePointContainer*, std::vector<const SpacePointBucket*>>;
            /** @brief Type alias for the visual information of a pattern */
            using PatternHitVisualInfo = MuonValR4::IFastRecoVisualizationTool::PatternHitVisualInfo;
            /** @brief Abrivation for a vector of visual information objects */
            using PatternHitVisualInfoVec = std::vector<PatternHitVisualInfo>;

            /** @brief Configuration object */           
            struct Config {
                /** @brief Size of theta window in radiants to search for comapatible hits with a pattern, tailored to the target pt cutoff */
                double thetaSearchWindow {0.033};
                /** @brief Maximum number of missed candidate hits in different measurement layers during pattern building */
                unsigned int maxMissedLayerHits {2};
                /** @brief Base radial compatibility window (in mm). This is the minimum allowed |R residual| between a test hit and the extrapolated line from the seed. */
                double baseRWindow {25};
                /** @brief Minimum difference in global Z between the seed and the pattern hit to be used to compute the pattern line. Use the beamspot otherwise. */
                double minZDiff4Line {10};
                /** @brief Minimum difference in global R between the seed and the pattern hit to be used to compute the pattern line. Use the beamspot otherwise. */
                double minRDiff4Line {40};
                /** @brief Maximum phi difference in radiants allowed between two hits */
                double phiTolerance {0.1};
                /** @brief Minimum number of trigger hits in the bending direction required to accept a pattern */
                unsigned int minBendingTriggerHits {3};
                /** @brief Minimum number of precision hits in the bending direction required to accept a pattern */
                unsigned int minBendingPrecisionHits {0};
                /** @brief Toggle the utilization of MDT hits to build patterns */
                bool useMdtHits {true};
                /** @brief Toggle the seeding from MDT hits */
                bool seedFromMdt {false};
                /** @brief Maximum number of attempts to build a pattern from hits already used in existing patterns */
                unsigned int maxSeedAttempts {2};
                /** @brief Vector configuring the seeding layers. By default we seed from Middle and Outer layers, and if toggled from Inner as well. */
                std::vector<LayerIndex> layerSeedings{LayerIndex::Middle, LayerIndex::Outer};
                /** @brief Pointer to the visualization tool */
                const MuonValR4::IFastRecoVisualizationTool* visionTool{nullptr};
                /** @brief Pointer to the idHelperSvc */
                const Muon::IMuonIdHelperSvc* idHelperSvc{nullptr};
            };

            /** @brief Standard constructor
             *  @param name: Name to be printed in the messaging
             *  @param config: Configuration parameters */
            GlobalPatternFinder(const std::string& name,
                                Config&& config);

            /** @brief Main methods steering the pattern finding. Given the space-point containers, it creates the search tree,  
             *         builds patterns in eta, attach compatible only-phi measurements, and convert PatternStates into GlobalPatterns
             *  @param gctx: Geometry context
             *  @param spacepoints: Vector of space point containers
             *  @param outBuckets: output collection of space-point buckets grouped by their corresponding input container to be written into StoreGate.
             *  @return: Vector of found patterns */
            PatternVec findPatterns(const ActsTrk::GeometryContext& gctx,
                                    const SpacePointContainerVec& spacepoints,
                                    BucketPerContainer& outBuckets) const;

        private:
            /** @brief Hit information stored during pattern building */
            struct HitPayload{
                /** @brief Pointer to the underlying hit */
                const SpacePoint* hit{nullptr};
                /** @brief Pointer to the parent bucket */
                const SpacePointBucket* bucket{nullptr};
                /** @brief Pointer to the parent container */
                const SpacePointContainer* container{nullptr};
                /** @brief Station index */
                StIndex station{};
                /** @brief Logical layer number in the sector frame */
                unsigned layerNum{};
                /** @brief Global R */
                double R{};
                /** @brief Global Z */
                double Z{};
                /** @brief Global Phi */
                double phi{};
                /** @brief Equal operator: it compares the underlying hit */
                bool operator==(const HitPayload& other) const;
            };
            /** @brief Definition of the search tree class */
            using SearchTree_t = Acts::KDTree<2, HitPayload, double, std::array, 5>;
            /** @brief Type alias for a tree node, formed by a hit payload and its indexing coordinates */
            using TreeNode = std::pair<SearchTree_t::coordinate_t, HitPayload>;
            /** @brief Abrivation of the seed coordinates */
            enum class SeedCoords : std::uint8_t{
                /** **Expanded** sector coordinate of the associated spectrometer sector */
                eSector,
                /** Global Theta */
                eTheta
            };
            /** @brief Pattern state object storing pattern information during construction */
            struct PatternState {
                /** @brief Constructor taking the seed information 
                 *  @param seed: seed hit
                 *  @param sectorCoord: **expanded** sector coordinate
                 *  @param seedTheta: global theta of the seed */
                PatternState(const HitPayload& seed,
                             const int sectorCoord,
                             const double seedTheta);
                PatternState() = delete;
                /** @brief Add a hit to the pattern and update the internal state
                 *  @param hit: hit to be added
                 *  @param residual: residual of the hit
                 *  @param accepWindow: acceptance window */
                void addHit(const HitPayload& hit,
                            const double residual,
                            const double acceptWindow);
                /** @brief Overwrite a hit in the pattern and update the internal state
                 *  @param oldHit: hit to be replaced
                 *  @param newHit: new hit to replace with
                 *  @param newResidual: residual of the new hit 
                 *  @param accepWindow: acceptance window of the new hit */
                void overWriteHit(const HitPayload& oldHit,
                                  const HitPayload& newHit,
                                  const double newResidual,
                                  const double newAcceptWindow);
                /** @brief Get the nth last inserted hit
                 *  @param n: the index of the hit to retrieve
                 *  @return: reference to the n-th last inserted hit */
                const HitPayload& getNthLastHit(const std::size_t n) const;
                /** @brief Check wheter a hit is present in the pattern
                 *  @param hit: hit to be checked
                 *  @return: boolean indicating if the hit is in the pattern */
                bool isInPattern(const HitPayload& hit) const;
                /** @brief Finalize the pattern and update its state */
                void finalizePattern();
                /** @brief Equal operator, it checks the hit-per-station map. It'svery expensive and in principle should be avoided */
                bool operator==(const PatternState& other) const;
                /** @brief Map collection of hits per station. A pattern is determined by the hits belonging to it. */
                std::unordered_map<StIndex, std::vector<HitPayload>> hitsPerStation{};
                /** @brief Pattern hit stations to save the filling order. Stations can be repeated when we invert 
                 *         the search direction (e.g. from seed ourward -> BM BO, then from seed inward BM BI) */
                std::vector<StIndex> stations{};
                /** @brief Map of spacepoint buckets per spacepoint container associated to the pattern */
                BucketPerContainer bucketsPerContainer{};
                /** @brief **expanded** sector coordinate & average theta & average phi of the pattern */
                int sectorCoord{-1};
                double theta{0.};
                double phi{0.};
                /** Counts of precision measurements / non-precision in bending direction / phi measurements  */
                unsigned nPrecisionHits{0};
                unsigned nBendingTriggerHits{0};
                unsigned nPhiHits{0};
                /** Total residual and last contribution to the residual (needed when replacing a hit) */
                double totalResidual{0.};
                double lastResidual{0.};
                /** Sum of residual divided by acceptance window and last contribution to the acceptance window (needed when replacing a hit) */
                double totalRes2AcceptWindow{0.};
                double lastAccepWindow{0.};
                /** Flag to indicate if the pattern is overlapping with another one, used during overlap removal */
                bool isOverlap{false};
                /** @brief Number of inserted hits during one of the two search stages (from seed outward and from seed inward) */
                unsigned nInsertedHits{0};
                /** @brief Number of missed candidate hits in different measurement layers during pattern building */
                unsigned nMissedLayerHits{0};        
                /** @brief Pointer to Visual Information for pattern visualization */
                Acts::CloneablePtr<PatternHitVisualInfo> visualInfo{nullptr};
                /** @brief Print the pattern candidate and stream operator */
                void print(std::ostream& ostr) const;
                friend std::ostream& operator<<(std::ostream& ostr, const PatternState& candidate) {
                    candidate.print(ostr);
                    return ostr;
                }
            };
            using PatternStateVec = std::vector<PatternState>;

            
            /** @brief Method to construct the search tree by filling it up with spacepoints from the given containers
             *  @param gctx: Geometry context
             *  @param spacepoints: Vector of space point containers
             *  @return: Constructed search tree */
            SearchTree_t constructTree(const ActsTrk::GeometryContext& gctx,
                                       const SpacePointContainerVec& spacepoints) const;
            /** @brief Method steering the building of patterns in eta
             *  @param orderedSpacepoints: Search tree with spacepoints ordered by their corresponding coordinates
             *  @param visualInfo: Pointer to visual information for pattern visualization (nullptr if the VisualizationTool is disabled)
             *  @return: resulting vector of PatternStates successfully built */
            PatternStateVec findPatternsInEta(const SearchTree_t& orderedSpacepoints,
                                              PatternHitVisualInfoVec* visualInfo = nullptr) const;
            /** @brief Function testing pattern compatibility of a set of active patterns (patterns produced from the same seed hit) against one 
             *         test hit. At the end, activePatterns contains the surviving patterns.
             *  @param activePatterns: Vector of active patterns to be extended
             *  @param test: Hit to be tested 
             *  @param seed: Seed hit information
             *  @param prevCandidate: Previous candidate hit information
             *  @param visualInfo: Pointer to visual information for pattern visualization (nullptr if the VisualizationTool is disabled) */
            void extendPatterns(PatternStateVec& activePatterns,
                                const HitPayload& test,
                                const HitPayload& seed,
                                const HitPayload& prevCandidate,
                                PatternHitVisualInfoVec* visualInfo = nullptr) const;
            /** @brief: Enum for the possible outcomes of the line compatibility test of one pattern against one test hit */        
            enum class CompatibilityResult : std::int8_t{
                /** @brief Test successfull, add the hit to the pattern */
                eAddHit = 0,
                /** @brief Test successfull with multiple pattern hits in the same logical measurement layer, branch the pattern */
                eBranchPattern = 1,
                /** @brief Test failed, discard the hit */
                eRejectHit = -1,
            };
            /** @brief : Small struct to encapsulate the checkLineCompatibility result */
            struct LineCompatibilityResult {
                CompatibilityResult result;
                double residual;
                double acceptanceWindow;
            };
            /** @brief Method to check the line compatibility of a test hit with a given pattern.
             *  @param seed: seed hit information
             *  @param test: test hit information
             *  @param pattern: pattern to be extended
             *  @return: a pair of the result of the test and the computed line residual for the test hit */
            LineCompatibilityResult checkLineCompatibility(const HitPayload& seed,
                                                           const HitPayload& test,
                                                           const PatternState& pattern) const;                     
            /** @brief Method to check the phi compatibility of a test hit with a given pattern
             *  @param seed: seed hit information
             *  @param test: test hit information 
             *  @param pattern: pattern to be extended
             *  @return: true if the test hit is phi compatible with the pattern, false otherwise */
            bool isPhiCompatible(const HitPayload& test,
                                 const HitPayload& seed, 
                                 const PatternState& pattern) const;
            /** @brief Helper method to compute the line slope between the seed and the last hit in a given pattern in the R-Z plane
             *  @param lastPatHit: reference to the last hit in the pattern
             *  @param seed: seed hit information
             *  @param useSeed2Beamspot: whether we should use the beamspot instead of the last pattern hit to compute the pattern line with the seed
             *  @param beamSpot: beamspot global position
             *  @return: the pattern line slope in the R-Z plane */
            double computeLineSlope(const HitPayload& lastPatHit,
                                    const HitPayload& seed,
                                    const bool useSeed2Beamspot,
                                    const Amg::Vector3D& beamSpot) const;
            /** @brief Method to compute the residual in globalR given the pattern line and test hit
             *  @param testHit: test hit information
             *  @param seed: seed hit information
             *  @param lineSlope: pattern line slope in the R-Z plane
             *  @return: the residual in global R of the test hit with respect to the pattern line */
            double computeResidual(const HitPayload& testHit,
                                   const HitPayload& seed,
                                   const double lineSlope) const;
            /** @brief Method to compute the acceptance window in global R for a given pattern line and test hit
             *  @param testHit: test hit information
             *  @param seed: seed hit information
             *  @param lastPatHit: last hit in the pattern, used to compute the pattern line
             *  @param lineSlope: pattern line slope in the R-Z plane
             *  @param useSeed2Beamspot: whether we used the beamspot instead of the last pattern hit to compute the pattern line with the seed
             *  @param beamspot: beamspot global position
             *  @return: the acceptance window in global R of the test hit given the pattern line */
            double computeAcceptanceWindow(const HitPayload& testHit,
                                           const HitPayload& seed,
                                           const HitPayload& lastPatHit,
                                           const double lineSlope,
                                           const bool useSeed2Beamspot,
                                           const Amg::Vector3D& beamSpot) const;         
            /** @brief Method to remove overlapping patterns
             *  @param toResolve: pattern to be resolved
             *  @param visualInfo: Pointer to visual information for pattern visualization (nullptr if the VisualizationTool is disabled) 
             *  @return: resolved patterns */
            PatternStateVec resolveOverlaps(PatternStateVec&& toResolve,
                                            PatternHitVisualInfoVec* visualInfo = nullptr) const;
            /** @brief Method to convert a PatternState into a GlobalPattern object
             *  @param candidate: PatternState to be converted
             *  @return: Converted GlobalPattern */
            GlobalPattern convertToPattern(const PatternState& candidate) const;
            /** @brief Method to convert a vector of PatternStates into GlobalPattern objects
             *  @param candidates: PatternStates to be converted
             *  @return: Vector of converted GlobalPatterns */
            PatternVec convertToPattern(const PatternStateVec& candidates) const;
            /** @brief Enum to express the logical measurement layer ordering given two hits */
            enum class LayerOrdering : std::int8_t{
                eSameLayer,
                eLowerLayer,
                eHigherLayer
            };
            /** @brief Method to check the logical layer ordering of two hits.
             *  @param hit1: first hit
             *  @param hit2: second hit
             *  @return: the logical measurement layer ordering of the two hits */
            LayerOrdering checkLayerOrdering(const HitPayload& hit1,
                                             const HitPayload& hit2) const;
            /** @brief Helper function to add visual information of a given pattern (which is usually going to be destroyed) to the final container
             *  @param candidate: PatternState whome visual information is to be added
             *  @param status: Status of the pattern (e.g. successfull, failed or overlap)
             *  @param visualInfo: Final vector of visual information to store the visual information*/
            void addVisualInfo(const PatternState& candidate, 
                               PatternHitVisualInfo::PatternStatus status,
                               PatternHitVisualInfoVec* visualInfo) const;

            /** @brief Spacepoint sorter per logical measurement layer */
            SpacePointPerLayerSorter m_spSorter{};
            /** @brief Global Pattern Recognition configuration */
            Config m_cfg;
    };
}


#endif

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_FASTRECONSTRUCTIONALGS_GLOBALPATTERNFINDER__H
#define MUONR4_FASTRECONSTRUCTIONALGS_GLOBALPATTERNFINDER__H

#include "MuonFastRecoEvent/GlobalPattern.h"
#include "MuonRecToolInterfacesR4/IFastRecoVisualizationTool.h"
#include "MuonSpacePoint/SpacePointContainer.h"
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "MuonSpacePoint/SpacePointHelpers.h"
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
    /// compatible phi-only hits to the patterns. The resulting patterns are
    /// returned by the main method of the tool. 

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
                /** @brief Toggle the utilization of MDT hits to build patterns */
                bool useMdtHits {true};
                /** @brief Toggle the seeding from MDT hits */
                bool seedFromMdt {false};
                /** @brief Vector configuring the seeding layers. By default we seed from Middle and Outer layers, and if toggled from Inner as well. */
                std::vector<LayerIndex> layerSeedings{LayerIndex::Middle, LayerIndex::Outer};
                /********* Pattern bulding acceptance **********/ 
                /** @brief Size of theta window in radians to search for comapatible hits with a pattern, tailored to the target pt cutoff */
                double thetaSearchWindow {0.033};
                /** @brief Base radial compatibility window (in mm). This is the minimum allowed |R residual| between a test hit and the extrapolated line from the seed. */
                double baseRWindow {25};
                /** @brief Maximum phi difference in radians allowed between two hits */
                double phiTolerance {0.1};
                /********* Pile-up & Fake rate suppression *****/
                /** @brief Minimum number of trigger hits in the bending direction required to accept a pattern */
                unsigned int minBendingTriggerHits {3};
                /** @brief Minimum number of precision hits in the bending direction required to accept a pattern */
                unsigned int minBendingPrecisionHits {0};
                /** @brief Minimum number of phi measurements required to accept a pattern */
                unsigned int minPhiHits {1};
                /** @brief Quality cut on pattern'mean squared normalized residual */
                double meanNormRes2Cut {0.2};
                /********* Pattern recovery power **************/
                /** @brief Maximum number of attempts to build a pattern from hits already used in existing patterns */
                unsigned int maxSeedAttempts {2};
                /** @brief Maximum number of missed candidate hits in different measurement layers during pattern building */
                unsigned int maxMissedLayerHits {2};
                /********* Numerical stability *****************/
                /** @brief Minimum separation (in mm) between the measurement layers of two hits for being used to compute a reliable pattern line. Use the beamspot otherwise. */
                double minLayerSeparation {40};
                /********* Pattern quality evaluation **********/
                /** @brief Weight of precision hits in the score, w.r.t trigger hits */
                double precisionWeight {0.75};
                /** @brief Hit counts saturates at nStations * this value */
                double hitScoreSaturation {10.0};
                /** @brief How strongly to penalize residual — higher = stricter quality requirement */
                double residualPenalty {1.5};
                /** @brief Saturation for phi bonus — beyond this many phi hits the bonus is maxed */
                double phiBonusSaturation {4.0};
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
                /** @brief Full constructor for eta hits 
                 *  @param hit: Pointer to the underlying hit
                 *  @param bucket: Pointer to the parent bucket
                 *  @param container: Pointer to the parent container
                 *  @param station: Station index
                 *  @param layerNum: Logical layer number in the sector frame
                 *  @param R: Global R                    
                 *  @param Z: Global Z
                 *  @param phi: Global Phi */
                HitPayload(const SpacePoint* hit, 
                           const SpacePointBucket* bucket,
                           const SpacePointContainer* container,
                           StIndex station,
                           uint8_t layerNum,
                           double R, 
                           double Z,
                           double phi);
                /** @brief Compact constructor for phi-only hits 
                 *  @param hit: Pointer to the underlying hit
                 *  @param station: Station index
                 *  @param layerNum: Logical layer number in the sector frame
                 *  @param phi: Global Phi */
                HitPayload(const SpacePoint* hit, 
                           StIndex station,
                           uint8_t layerNum,
                           double phi);
                /** @brief Pointer to the underlying hit */
                const SpacePoint* hit{nullptr};
                /** @brief Pointer to the parent bucket */
                const SpacePointBucket* bucket{nullptr};
                /** @brief Pointer to the parent container */
                const SpacePointContainer* container{nullptr};
                /** @brief Global R */
                double R{0.};
                /** @brief Global Z */
                double Z{0.};
                /** @brief Global Phi */
                double phi{0.};
                /** @brief Station index */
                StIndex station{};
                /** @brief Logical layer number in the sector frame */
                uint8_t layerNum{0u};
                /** @brief Is precision hit */
                bool isPrecision{MuonR4::isPrecisionHit(*hit)};
                /** @brief Equal operator: it compares the underlying hit */
                bool operator==(const HitPayload& other) const;
                /** @brief Arrow operator: it allows to access the underlying hit */
                const SpacePoint* operator->() const { return hit; }
                /** @brief Dereference operator: it allows to access the underlying hit */
                const SpacePoint& operator*() const { return *hit; }
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
                             const uint8_t sectorCoord,
                             const double seedTheta);
                /** @brief Delete default destructor - ensure patterns are always constructed from a seed or another pattern */
                PatternState() = delete; 
                /** @brief Move constructor
                 *  @param other: other pattern state to move from */
                PatternState(PatternState&& other) noexcept = default;
                /** @brief Move assignment operator
                 *  @param other: other pattern state to move from */
                PatternState& operator=(PatternState&& other) noexcept = default;
                /** @brief Copy constructor
                 *  @param other: other pattern state to copy from */
                PatternState(const PatternState& other) = default;
                /** @brief Copy assignment operator
                 *  @param other: other pattern state to copy from */
                PatternState& operator=(const PatternState& other) = default;
                /** @brief Destructor */
                ~PatternState() =default;
                /** @brief Add a hit to the pattern and update the internal state
                 *  @param hit: hit to be added
                 *  @param residual: residual of the hit
                 *  @param acceptWindow: acceptance window */
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
                /** @brief Get the n-th last inserted hit
                 *  @param n: the index of the hit to retrieve
                 *  @return: reference to the n-th last inserted hit */
                const HitPayload& getNthLastHit(const uint8_t n) const;
                /** @brief Check wheter a hit is present in the pattern
                 *  @param hit: hit to be checked
                 *  @return: boolean indicating if the hit is in the pattern */
                bool isInPattern(const HitPayload& hit) const;
                /** @brief Finalize the pattern building in eta and update its state */
                void finalizePatternEta();
                /** @brief Finalize the pattern building in phi and update its state */
                void finalizePatternPhi();

                /** @brief Pointer to Visual Information for pattern visualization */
                Acts::CloneablePtr<PatternHitVisualInfo> visualInfo{nullptr};
                /** @brief Pointer to the last inserted hit. Needed to speed-up lookup */
                const SpacePoint* lastInsertedHit{nullptr};
                /** @brief Pointer to the last hit in the second-to-last layer */
                const SpacePoint* prevLayerHit{nullptr};
                /** @brief Average theta & average phi of the pattern */
                double theta{0.};
                double phi{0.};
                /** @brief Mean over eta hits of the square of their residual divided by acceptance window */
                double meanNormResidual2{0.};
                /** @brief Residual & acceptance window of the last inserted hit (needed when replacing a hit) */
                double lastResidual{0.};
                double lastAcceptWindow{0.};
                /** @brief Counts of precision measurements / non-precision in bending direction / phi measurements  */
                uint8_t nPrecisionHits{0u};
                uint8_t nBendingTriggerHits{0u};
                uint8_t nPhiHits{0u};
                /** @brief Number of missed candidate hits in different measurement layers during pattern building */
                uint8_t nMissedLayerHits{0u};
                /** @brief **expanded** sector coordinate & the two corresponding physical sectors */
                uint8_t sectorCoord{0};
                uint8_t sector1{0};
                uint8_t sector2{0};
                /** @brief Flag to indicate if the pattern has been finalized */
                bool isFinalized{false};
                /** @brief Flag to indicate if the pattern is overlapping with another one, used during overlap removal */
                bool isOverlap{false};

                /** @brief Map collection of hits per station. A pattern is determined by the hits belonging to it. */
                std::unordered_map<StIndex, std::vector<HitPayload>> hitsPerStation{};
                /** @brief Pattern hit stations to save the filling order. Stations can be repeated when we invert 
                 *         the search direction (e.g. from seed ourward -> BM BO, then from seed inward BM BI) */
                std::vector<StIndex> stations{};
                /** @brief Map of spacepoint buckets per spacepoint container associated to the pattern */
                BucketPerContainer bucketsPerContainer{};

                /** @brief Patterns are considered identical if they have the same hit content. However, map comparison is very expensive */
                bool operator==(const PatternState& other) const = delete;        
                /** @brief Print the pattern candidate and stream operator */
                void print(std::ostream& ostr) const;
                friend std::ostream& operator<<(std::ostream& ostr, const PatternState& candidate) {
                    candidate.print(ostr);
                    return ostr;
                }
            };
            using PatternStateVec = std::vector<PatternState>;

            
            /** @brief Method to construct the search tree by filling it up with spacepoints from the given containers. The tree does not contain only-phi hits.
             *  @param gctx: Geometry context
             *  @param spacepoints: Vector of space point containers
             *  @return: Constructed search tree */
            SearchTree_t constructTree(const ActsTrk::GeometryContext& gctx,
                                       const SpacePointContainerVec& spacepoints) const;
            /** @brief Method steering the building of patterns in eta
             *  @param orderedSpacepoints: Search tree with spacepoints ordered by their corresponding coordinates
             *  @param visualInfo: Pointer to visual information for pattern visualization (nullptr if the VisualizationTool is disabled).
             *                     Needed to add visual info about pattern candidates discarded during the building stage.
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
                CompatibilityResult result {CompatibilityResult::eRejectHit};
                double residual{0.};
                double accWindow{0.};
            };
            /** @brief Method to check the line compatibility of a test hit with a given pattern.
             *  @param seed: seed hit information
             *  @param test: test hit information
             *  @param pattern: pattern to be extended
             *  @return: result of the test, including the computed line residual and acceptance window */
            LineCompatibilityResult checkLineCompatibility(const HitPayload& seed,
                                                           const HitPayload& test,
                                                           const PatternState& pat) const;
            /** @brief Helper method to compute the residual of a test hit against a reference pattern hit 
             *         and check whether it is within the acceptance window. The residual is defined 
             *         against the line passing through seed and the reference pattern hit
             *  @param seed: seed hit information
             *  @param test: test hit information
             *  @param pattern: pattern to be extended
             *  @param patHitIdx: Index of the pattern hit to use for residual computation
             *  @param beamSpot: Needed to draw the pattern line when the provided pattern hit is too close to the seed
             *  @return: result of the test, including the computed line residual and acceptance window */
            LineCompatibilityResult computeResidual(const HitPayload& seed,
                                                    const HitPayload& test,
                                                    const PatternState& pat,
                                                    const uint8_t patHitIdx,
                                                    const Amg::Vector3D& beamSpot) const;
            /** @brief Operator to compare two patterns 
             *  @param a: First pattern to compare
             *  @param b: Second pattern to compare
             *  @return: true if pattern a is better than pattern b, false otherwise */
            bool isBetter(const PatternState& a, const PatternState& b) const;
            /** @brief Method to remove overlapping patterns
             *  @param toResolve: pattern to be resolved
             *  @param visualInfo: Pointer to visual information for pattern visualization (nullptr if the VisualizationTool is disabled) 
             *  @return: resolved patterns */
            PatternStateVec resolveOverlaps(PatternStateVec&& toResolve,
                                            PatternHitVisualInfoVec* visualInfo = nullptr) const;
            /** @brief Method to add phi-only measurements to existing PatternStates
             *  @param gctx: Geometry context
             *  @param patterns: Vector of pattern states to which to add phi-only hits
             *  @return: void */
            void addPhiOnlyHits(const ActsTrk::GeometryContext& gctx,
                                PatternStateVec& patterns) const;
            /** @brief Method to check the phi compatibility of a test hit with a given pattern
             *  @param testPhi: test global phi
             *  @param pattern: pattern to be extended
             *  @return: true if the test hit is phi compatible with the pattern, false otherwise */
            bool isPhiCompatible(const double testPhi,
                                 const PatternState& pattern) const;
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
            using enum GlobalPatternFinder::LayerOrdering;
            /** @brief Method to check the logical layer ordering of two hits.
             *  @param hit1: first hit
             *  @param hit2: second hit
             *  @return: the logical measurement layer ordering of the two hits */
            static LayerOrdering checkLayerOrdering(const HitPayload& hit1,
                                                    const HitPayload& hit2);
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

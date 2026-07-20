/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_FASTRECONSTRUCTIONALGS_GLOBALPATTERNFINDER__H
#define MUONR4_FASTRECONSTRUCTIONALGS_GLOBALPATTERNFINDER__H

#include "MuonFastRecoEvent/GlobalPattern.h"
#include "MuonRecToolInterfacesR4/IFastRecoVisualizationTool.h"
#include "MuonSpacePoint/SpacePointContainer.h"
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include <MuonSpacePoint/SpacePointHelpers.h>
#include "MuonTrackEvent/ExpandedSector.h"
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
                double thetaSearchWindow {0.05};
                /** @brief Effective isotropic position uncertainty [mm], including detector resolution and unmodelled effects */
                double baseResidualSigma {25};
                /** @brief Maximum phi difference in radians allowed between two hits */
                double phiTolerance {0.1};
                /********* Pile-up & Fake rate suppression *****/
                /** @brief Minimum number of trigger layers in the bending direction required to accept a pattern */
                unsigned int minTriggerLayers {3};
                /** @brief Minimum number of precision layers in the bending direction required to accept a pattern */
                unsigned int minPrecisionLayers {0};
                /** @brief Minimum number of phi layers required to accept a pattern */
                unsigned int minPhiLayers {1};
                /** @brief Minimum number of layers in a station to be considered a good station */
                unsigned int minStationLayers {4};
                /** @brief Quality cut on pattern'mean squared normalized residual */
                double meanNormRes2Cut {0.2};
                /********* Pattern recovery power **************/
                /** @brief Maximum number of attempts to build a pattern from hits already used in existing patterns */
                unsigned int maxSeedAttempts {2};
                /** @brief Maximum number of missed candidate hits in different measurement layers in a station */
                unsigned int maxMissLayersInStation {2};
                /********* Numerical stability *****************/
                /** @brief Minimum distance (in mm) between two hits for being used to compute a reliable pattern line. Use the beamspot otherwise. */
                double minHitDistance4Line {40};
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
            static const int s_nStations{Acts::toUnderlying(StIndex::StIndexMax)};
            /** @brief Hit information stored during pattern building */
            struct HitPayload{
                /** @brief Pointer to the underlying hit */
                const SpacePoint* hit{nullptr};
                /** @brief Pointer to the parent bucket */
                const SpacePointBucket* bucket{nullptr};
                /** @brief Pointer to the parent container */
                const SpacePointContainer* container{nullptr};
                /** @brief Global position */
                Amg::Vector3D position{Amg::Vector3D::Zero()};
                /** @brief Sensor direction in global frame */
                Amg::Vector3D sensorDir{Amg::Vector3D::Zero()};
                /** @brief Station index */
                StIndex station{};
                /** @brief Layer number in the sector frame */
                uint8_t locLayer{0u};
                /** @brief Sector */
                uint8_t sector{0u};
                /** @brief Is precision hit */
                bool isPrecision{false};
                /** @brief Is straw hit */
                bool isStraw{false};
                /** @brief Equal operator: it compares the underlying hit */
                bool operator==(const HitPayload& other) const;
                /** @brief Arrow operator: it allows to access the underlying hit */
                const SpacePoint* operator->() const { return hit; }
                /** @brief Dereference operator: it allows to access the underlying hit */
                const SpacePoint& operator*() const { return *hit; }
                /** @brief Get the pointer to the underlying hit */
                const SpacePoint* sp() const { return hit; }
            };
            /** @brief Definition of the search tree class */
            using SearchTree_t = Acts::KDTree<2, HitPayload, double, std::array, 5>;
            /** @brief Abrivation of the seed coordinates */
            enum class SeedCoords : std::uint8_t{
                /** **Expanded** sector coordinate of the associated spectrometer sector */
                eSector,
                /** Global Theta */
                eTheta
            };
            /** @brief Small wrapper for candidate hits used to build patterns. This is needed
             *         because the global layer number cannot be defined globally, but it can be
             *         computed given a set of hits. We store locally most frequently accessed 
             *         data to avoid frequent pointer indirection */
            struct CandidateHit {
                /** @brief Pointer to the underlying hit */
                const HitPayload* hit{nullptr};
                /** @brief Station index */
                StIndex station{};
                /** @brief Global measurement layer number */
                uint8_t globLayer{0u};
                /** @brief Sector */
                uint8_t sector{0u};
                /** @brief Is straw hit */
                bool isStraw{false};

                // Forward commonly used accessors for convenience
                const HitPayload* operator->() const { return hit; }
                const HitPayload& operator*() const { return *hit; }
                const SpacePoint* sp() const { return hit->sp(); }
                bool operator==(const CandidateHit& other) const { return *hit == *other.hit; }
                bool operator==(const HitPayload& other) const { return *hit == other; }
                // Print and stream operator
                friend std::ostream& operator<<(std::ostream& ostr, const CandidateHit& c) {
                    c.print(ostr);
                    return ostr;
                }
                void print(std::ostream& ostr) const;
            };
            /** @brief: Enum for possible outcomes of pattern line compatibility test */        
            enum class LineTestDecision : std::int8_t{
                /** @brief Test successfull, add hit to pattern */
                eAddHit,
                /** @brief Test successfull with multiple pattern hits on same layer, branch the pattern */
                eBranchPattern,
                /** @brief Test failed, discard the hit */
                eRejectHit,
                /** @brief Test hit is a consecutive MDT hit */
                eConsecutiveMdt,
                /** @brief Test successful, overwrite the hit. Needed e.g. for sTGCs */
                eOverwriteLastHit
            };
            /** @brief : Small struct to encapsulate the result of the line compatibility test */
            struct LineTestRes  {
                LineTestDecision result {LineTestDecision::eRejectHit};
                double residual{0.};
                double accWindow{0.};
            };
            /** @brief Pattern state object storing pattern information during construction */
            struct PatternState {
                /** @brief Constructor taking the seed information 
                 *  @param seed: seed hit
                 *  @param expSector: **expanded** sector coordinate
                 *  @param cfg: pointer to configuration object
                 *  @param logger: pointer to messaging object */
                PatternState(const CandidateHit& seed,
                             const std::int8_t expSector,
                             const Config* cfg,
                             const AthMessaging* logger);
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
                void addHit(const CandidateHit& hit,
                            const double residual,
                            const double acceptWindow);
                /** @brief Overwrite the hits on the last layer with the new one
                 *  @param newHit: new hit to replace with
                 *  @param newResidual: residual of the new hit 
                 *  @param newAcceptWindow: acceptance window of the new hit
                 *  @param beamSpot: needed to update line parameters */
                void overWriteHit(const CandidateHit& newHit,
                                  const double newResidual,
                                  const double newAcceptWindow);
                /** @brief Method checking line compatibility of a test hit against the pattern
                 *  @param testHit: test hit information
                 *  @param beamSpot: Beam spot position, needed to update the pattern line
                 *  @return: result of the test, including the computed line residual and acceptance window */
                LineTestRes checkLineComp(const CandidateHit& testHit,
                                          const Amg::Vector3D& beamSpot);
                /** @brief Method to compute the residual of a test hit against the pattern line
                 *  @param testHit: test hit information
                 *  @return: Test result holding the residual and acceptance window. The decision is set later. */
                LineTestRes computeLineResidual(const CandidateHit& testHit) const;
                /** @brief Project a certain hit position onto the bending plane where the pattern is defined. 
                 *         The hit is moved along the sensor direction if it does not measure phi, 
                 *         or is rotated around the Z axis if it does.
                 *  @param hit: hit whose position is to be projected
                 *  @return: projected position */
                Amg::Vector3D projToPhiPlane(const HitPayload& hit) const;
                /** @brief Method to check the phi compatibility of a test hit with a given pattern
                 *  @param testPhi: test global phi
                 *  @return: true if the test hit is phi compatible with the pattern, false otherwise */
                bool isPhiCompatible(const double testPhi) const;
                /** @brief Check wheter a hit is present in the pattern
                 *  @param hit: hit to be checked
                 *  @return: boolean indicating if the hit is in the pattern */
                bool isInPattern(const HitPayload& hit) const;
                /** @brief Finalize the pattern building in phi and update its state */
                void finalizePatternPhi();
                /** @brief Move the line anchor hit given a reference hit. The anchor is defined
                           as the closest hit in the closest station to the referece hit, */
                void moveLineAnchorHit(const CandidateHit& refHit);
                /** @brief Update the line parameters based on the current hits
                 *  @param beamSpot: position of the beam spot, needed when there are not enough hits */
                void updateLineParameters(const Amg::Vector3D& beamSpot);
                /** @brief Helper method to update the pattern phi and bending plane normal */
                void updatePatternPhi(const double newPhi);
                /** @brief Return the mean normalized residual squared */
                double getMeanResidual2() const;
                /** @brief Return the number of hits in bending coordinate */
                uint8_t nBendingHits() const;
                /** @brief Return the number of layers in bending coordinate */
                uint8_t nBendingLayers() const;
                /** @brief Method returning the number of stations
                 *  @param onlyGoodStations: flag to indicate if only good stations should be counted,
                 *         i.e. having a minimum number of hits */
                uint8_t nStations(const bool onlyGoodStations) const;
                /** @brief Get the buckets associated with the pattern */
                std::vector<const SpacePointBucket*> getParentBuckets() const;
                /** @brief Check whether a given hit is in the last layer */
                bool isInLastLayer(const CandidateHit& hit) const;

                /** @brief Pointer to cfg option */
                const Config* cfg{nullptr};
                /** @brief Logger */
                const AthMessaging* logger{nullptr};
                /** @brief Pointer to Visual Information for pattern visualization */
                Acts::CloneablePtr<PatternHitVisualInfo> visualInfo{nullptr};
                /** @brief Last inserted hit. Needed to speed-up lookup */
                CandidateHit lastInsertedHit{};
                /** @brief Last hit in the second-to-last layer */
                CandidateHit prevLayerHit{};
                /** @brief Line anchor hit */
                CandidateHit lineAnchorHit{};
                /** @brief Seed hit */
                CandidateHit seedHit{};
                /** @brief Normal vector to the bending plane where the pattern lies */
                Amg::Vector3D bendPlaneNorm{Amg::Vector3D::Zero()};
                /** @brief Position and direction of the pattern line. Both are constructed to be
                 *         within the bending plane of the pattern */
                Amg::Vector3D linePos{Amg::Vector3D::Zero()};
                Amg::Vector3D lineDir{Amg::Vector3D::Zero()};
                /** @brief Distance between the two points defining the pattern line */
                double leverArm{0.};
                /** @brief Mean over eta hits of the square of their residual divided by acceptance window */
                double meanNormResidual2{0.};
                /** @brief Residual & acceptance window of the last inserted hit (needed when replacing a hit) */
                double lastResidual{0.};
                double lastAcceptWindow{0.};
                /** @brief Pattern phi, which is the phi of the bending plane where the pattern lies */
                double patPhi{0.};
                /** @brief **expanded** MS sector */
                ExpandedSector expSect{static_cast<int8_t>(0)};
                /** @brief Counts of precision / non-precision / phi layers  */
                uint8_t nPrecisionLayers{0u};
                uint8_t nTriggerLayers{0u};
                uint8_t nPhiLayers{0u};
                /** @brief Flag to indicate if the pattern has been finalized */
                bool isFinalized{false};
                /** @brief Flag to indicate if the pattern is overlapping with another one, used during overlap removal */
                bool isOverlap{false};
                /** @brief Whether we used the beamspot to compute the line parameters */
                bool useBeamspot{false};
                /** @brief Whether we need to update the pattern line the next time we find a hit in a new layer */
                bool needLineUpdate{false};
                
                /** @brief Counts of measurement layers per station */
                std::array<uint8_t, s_nStations> nMeasurementLayers{};
                /** @brief Map collection of hits per station. A pattern is determined by the hits belonging to it. */
                std::array<std::vector<CandidateHit>, s_nStations> hitsPerStation{};
                /** @brief Array holding phi-only hits */
                std::vector<HitPayload> phiOnlyHits{};

                /** @brief Patterns are considered identical if they have the same hit content. However, map comparison is very expensive */
                bool operator==(const PatternState& other) const = delete;
                /** @brief Print the pattern candidate */
                void print(std::ostream& ostr, bool detailed) const;
            };
            using PatternStateVec = std::vector<PatternState>;
            /** @brief Method to construct the search tree by filling it up with spacepoints from the given containers. The tree does not contain only-phi hits.
             *  @param gctx: Geometry context
             *  @param spacepoints: Vector of space point containers
             *  @return: Constructed search tree */
            SearchTree_t constructTree(const ActsTrk::GeometryContext& gctx,
                                       const SpacePointContainerVec& spacepoints) const;
            /** @brief Method steering the global pattern building in the bending plane.
             *  @param orderedSpacepoints: Search tree with spacepoints ordered by their corresponding coordinates
             *  @param visualInfo: Pointer to visual information for pattern visualization (nullptr if the VisualizationTool is disabled).
             *                     Needed to add visual info about pattern candidates discarded during the building stage.
             *  @return: resulting vector of PatternStates successfully built */
            PatternStateVec findPatternsInEta(const SearchTree_t& orderedSpacepoints,
                                              PatternHitVisualInfoVec* visualInfo = nullptr) const;
            /** @brief Main function controlling the development of patterns, including pattern branching when necessary. 
             *         It tests pattern compatibility of a set of active patterns (patterns produced from the same seed hit) against one 
             *         test hit. At the end, activePatterns contains the surviving patterns.
             *  @param startPatterns: Vector of active patterns to be extended
             *  @param endPatterns: Vector to store the surviving patterns after testing against the test hit.
             *  @param testHit: Hit to be tested against the patterns
             *  @param beamSpot: Beam spot position, needed when the pattern line cannot be reliably defined from the pattern hits
             *  @param visualInfo: Pointer to visual information for pattern visualization (nullptr if the VisualizationTool is disabled) */
            void extendPatterns(PatternStateVec& startPatterns,
                                PatternStateVec& endPatterns,
                                const CandidateHit& testHit,
                                const Amg::Vector3D& beamSpot,
                                PatternHitVisualInfoVec* visualInfo = nullptr) const;
            /** @brief Method to check if a pattern passes the quality cuts
             *  @param pattern: Pattern to be checked
             *  @return: true if the pattern passes the cuts, false otherwise */
            bool passPatternCuts(const PatternState& pat) const;
            /** @brief Method to compare two patterns and define which one is better.
             *  @param a: first pattern
             *  @param b: second pattern
             *  @return: true if pattern a is better than pattern b, false otherwise */
            static bool isBetter(const PatternState& a, 
                                 const PatternState& b);
            /** @brief Method to remove overlapping patterns
             *  @param toResolve: pattern to be resolved
             *  @param visualInfo: Pointer to visual information for pattern visualization (nullptr if the VisualizationTool is disabled) 
             *  @return: resolved patterns */
            PatternStateVec resolveOverlaps(PatternStateVec& toResolve,
                                            PatternHitVisualInfoVec* visualInfo = nullptr) const;
            /** @brief Method to add phi-only measurements to existing PatternStates
             *  @param gctx: Geometry context
             *  @param patterns: Vector of pattern states to which to add phi-only hits
             *  @return: void */
            void addPhiOnlyHits(const ActsTrk::GeometryContext& gctx,
                                PatternStateVec& patterns) const;
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
            /** @brief Helper function to check whether two hits are consecutive MDT measurements */
            static bool areConsecutiveMdt(const CandidateHit& hit1, 
                                          const CandidateHit& hit2);
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

            struct PatternPrintView {
                const PatternState& pat;
                bool detailed = false;
            };
            /** @brief Print the pattern candidate and stream operator */
            static PatternPrintView brief(const PatternState& p);
            static PatternPrintView detailed(const PatternState& p);
            friend std::ostream& operator<<(std::ostream& os, const PatternPrintView& v);
    };
}


#endif

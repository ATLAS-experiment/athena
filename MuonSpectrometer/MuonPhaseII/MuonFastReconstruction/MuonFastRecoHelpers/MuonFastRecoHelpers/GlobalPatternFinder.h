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

    /// @note The definition and implementation of structs and classes used
    ///       in the pattern finding are contained in the GlobalPatternFinderDefs.h
    class GlobalPatternFinder : public AthMessaging {
        public:  
            /** @brief Type alias for the station index */
            using StIndex = Muon::MuonStationIndex::StIndex;
            /** @brief Type alias for the station layer index */
            using LayerIndex = Muon::MuonStationIndex::LayerIndex;

            /** @brief Configuration object for the patter finder */     
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
                /** @brief Number of standard deviations to consider for residual acceptance */
                double nResidualSigma {3.0};
                /** @brief Residual uncertainty to consider the hit as low confidence */
                double lowConfidenceResSigma {50};
                /** @brief Number of standard deviations to consider for phi acceptance */
                double nPhiSigma {3.0};
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
                /********** Beamspot settings ************/
                /** @brief Beamspot radius */
                double beamSpotRadius{30.*Gaudi::Units::cm};
                /** @brief Beamspot length */
                double beamSpotLength{2.*Gaudi::Units::m};
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
             *  @return: Vector of found patterns */
            std::vector<GlobalPattern> findPatterns(const ActsTrk::GeometryContext& gctx,
                                                    std::span<const SpacePointContainer*> spacepoints) const;

        private:
            /** @brief Number of stations */
            static const int s_nStations{Acts::toUnderlying(StIndex::StIndexMax)};
            /** @brief Base class for hit struct containing hit information. */
            struct HitPayload;
            /** @brief Small wrapper for candidate hits used to build patterns. This is needed
             *         because the global layer number cannot be defined globally, but it can be
             *         computed given a set of hits. */
            struct CandidateHit;
            /** @brief Pattern state object storing pattern information during construction */
            struct PatternState;
            /** @brief Type alias for a vector of pattern states */
            using PatternStateVec = std::vector<PatternState>;
            /** @brief Type alias for the visual information of a pattern */
            using PatHitVisual = MuonValR4::IFastRecoVisualizationTool::PatternHitVisualInfo;
            /** @brief Definition of the search tree class */
            using SearchTree_t = Acts::KDTree<2, const HitPayload*, double, std::array, 15>;
            /** @brief Abrivation of the seed coordinates */
            enum class SeedCoords : std::uint8_t{
                /** **Expanded** sector coordinate of the associated spectrometer sector */
                eSector,
                /** Global Theta */
                eTheta
            };
            /** @brief Structure to hold the search tree data */
            struct SearchTreeData;

            /** @brief Construct the search tree from the given spacepoint containers. 
             *         The hit payloads are stored in a vector, and the tree will contain
             *         the index of the hits in the vector. Hits duplicated across overlapping 
             *         sectors share the same payload. The tree does not contain only-phi hits.
             *  @param gctx: Geometry context
             *  @param spacepoints: Vector of space point containers
             *  @param hitPayloads: Vector of hit payloads to be filled with the spacepoints. 
             *  @return: Constructed search tree */
            SearchTreeData constructTree(const ActsTrk::GeometryContext& gctx,
                                         std::span<const SpacePointContainer*> spacepoints) const;
            /** @brief Method steering the global pattern building in the bending plane.
             *  @param orderedSpacepoints: Search tree with spacepoints ordered by their corresponding coordinates
             *  @param visualInfo: Pointer to visual information for pattern visualization (nullptr if the VisualizationTool is disabled).
             *                     Needed to add visual info about pattern candidates discarded during the building stage.
             *  @return: resulting vector of PatternStates successfully built */
            PatternStateVec findPatternsInEta(const SearchTree_t& orderedSpacepoints,
                                              std::vector<PatHitVisual>* visualInfo = nullptr) const;
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
                                std::vector<PatHitVisual>* visualInfo = nullptr) const;
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
                                            std::vector<PatHitVisual>* visualInfo = nullptr) const;
            /** @brief Method to add phi-only measurements to existing PatternStates
             *  @param gctx: Geometry context
             *  @param patterns: Vector of pattern states to which to add phi-only hits
             *  @return: Vector of added phi-only hits */
            void addPhiOnlyHits(const ActsTrk::GeometryContext& gctx,
                                PatternStateVec& patterns) const;
            /** @brief Method to convert a PatternState into a GlobalPattern object
             *  @param candidate: PatternState to be converted
             *  @return: Converted GlobalPattern */
            GlobalPattern convertToPattern(const PatternState& candidate) const;
            /** @brief Method to convert a vector of PatternStates into GlobalPattern objects
             *  @param candidates: PatternStates to be converted
             *  @return: Vector of converted GlobalPatterns */
            std::vector<GlobalPattern> convertToPattern(const PatternStateVec& candidates) const;
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
            static LayerOrdering checkLayerOrdering(const HitPayload& hit1,
                                                    const HitPayload& hit2);
            /** @brief Helper function to add visual information of a given pattern (which is usually going to be destroyed) to the final container
             *  @param candidate: PatternState whome visual information is to be added
             *  @param status: Status of the pattern (e.g. successfull, failed or overlap)
             *  @param visualInfo: Final vector of visual information to store the visual information*/
            void addVisualInfo(const PatternState& candidate, 
                               PatHitVisual::PatternStatus status,
                               std::vector<PatHitVisual>* visualInfo) const;

            /** @brief A view of the pattern state for printing purposes */
            struct PatternPrintView;
            /** @brief Print the pattern state with brief information */
            static PatternPrintView brief(const PatternState& p);
            /** @brief Print the pattern state with detailed information */
            static PatternPrintView detailed(const PatternState& p);

            /** @brief Spacepoint sorter per logical measurement layer */
            SpacePointPerLayerSorter m_spSorter{};
            /** @brief Global Pattern Recognition configuration */
            Config m_cfg;
    };
}

#endif

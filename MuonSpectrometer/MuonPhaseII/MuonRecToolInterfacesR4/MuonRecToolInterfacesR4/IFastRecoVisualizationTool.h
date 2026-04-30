/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONRECTOOLINTERFACESR4_IPATTERNVISUALIZATIONTOOL_H
#define MUONRECTOOLINTERFACESR4_IPATTERNVISUALIZATIONTOOL_H

#include <GaudiKernel/IAlgTool.h>

#include <MuonFastRecoEvent/GlobalPattern.h>
#include "MuonSpacePoint/SpacePointContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include <xAODMuon/MuonSegment.h>
#include <TObject.h>

#include <memory>

#include "Acts/Utilities/CloneablePtr.hpp"
class EventContext;
class TObject;

namespace MuonR4 {
    class SpacePointBucket;
    class GlobalPattern;
    class SpacePoint;
}

namespace MuonValR4{
    /** @brief Helper tool to visualize a pattern recogntion incident or a certain stage of the segment fit. */

    class IFastRecoVisualizationTool : virtual public IAlgTool {
        public:
            DeclareInterfaceID(IFastRecoVisualizationTool, 1, 0);
            
            using PrimitivePtr = std::unique_ptr<TObject>;
            using PrimitiveVec = std::vector<PrimitivePtr>;
            using GlobalPatternPtr = Acts::CloneablePtr<MuonR4::GlobalPattern>;

            virtual ~IFastRecoVisualizationTool() = default;
            /** @brief Structure to hold visual information about a pattern */
            struct PatternHitVisualInfo {
                enum class PatternStatus : std::uint32_t {
                    eSuccessful,
                    eOverlap,
                    eFailed
                };
                enum class HitStatus : std::uint32_t {
                    eKept,
                    eReplaced,
                    eDiscarded
                };
                /** @brief c-tor */
                explicit PatternHitVisualInfo(const MuonR4::SpacePoint* seed, double thetaMin, double thetaMax) : 
                    seed{seed}, thetaSearchMin{thetaMin}, thetaSearchMax{thetaMax} {}                
                /** @brief Seed */
                const MuonR4::SpacePoint* seed{nullptr};
                /** @brief Parent buckets */
                std::vector<const MuonR4::SpacePointBucket*> parentBuckets{};
                /** @brief For each hit save slope and window */
                std::unordered_map<const MuonR4::SpacePoint*, std::pair<double, double>> hitLineInfo{};
                /** @brief vector of hits that have been replaced by other hits */
                std::vector<const MuonR4::SpacePoint*> replacedHits{};
                /** @brief vector of hits that have been discarded */
                std::vector<const MuonR4::SpacePoint*> discardedHits{};
                /** @brief Lines to display the search window */
                double thetaSearchMin{};
                double thetaSearchMax{};
                /** @brief Copy of the pattern */
                GlobalPatternPtr patternCopy{nullptr};
                /** @brief Status of the pattern to decide how to visualize it */
                PatternStatus status{PatternStatus::eOverlap};
            };
            using PatternHitVisualInfoVec = std::vector<PatternHitVisualInfo>;
            /** @brief Draws the content of the bucket on a TCanvas and adds the patterns found within the bucket
             *  @param ctx: EventContext to fetch calibration & alignment
             *  @param extraLabel: Extra label for the legend
                @param patternVisual: Vector of pattern visual information objects */
            virtual void plotPatternBuckets(const EventContext& ctx,
                                            const std::string& extraLabel,
                                            PatternHitVisualInfoVec&& patternVisual) const = 0;
            /** @brief Draws the content of the bucket on a TCanvas and adds the patterns found within the bucket
             *  @param ctx: EventContext to fetch calibration & alignment
             *  @param extraLabel: Extra label for the legend
             *  @param patternVisual: Vector of pattern visual information objects
             *  @param extraPaints: Other objects that shall be painted onto the canvas */
            virtual void plotPatternBuckets(const EventContext& ctx,
                                           const std::string& extraLabel,
                                           PatternHitVisualInfoVec&& patternVisual,
                                           PrimitiveVec&& extraPaints) const = 0;
            /** @brief Draws the content of the bucket on a TCanvas and adds the patterns found within the bucket
             *  @param ctx: EventContext to fetch calibration & alignment
             *  @param extraLabel: Extra label for the legend
             *  @param patternVisual: Pattern visual information object */
            virtual void plotPatternBuckets(const EventContext& ctx,
                                            const std::string& extraLabel,
                                            PatternHitVisualInfo&& patternVisual) const = 0;
            /** @brief Draws the content of the bucket on a TCanvas and adds the patterns found within the bucket
             *  @param ctx: EventContext to fetch calibration & alignment
             *  @param extraLabel: Extra label for the legend
             *  @param patternVisual: Pattern visual information object
             *  @param extraPaints: Other objects that shall be painted onto the canvas */
            virtual void plotPatternBuckets(const EventContext& ctx,
                                            const std::string& extraLabel,
                                            PatternHitVisualInfo&& patternVisual,
                                            PrimitiveVec&& extraPaints) const = 0;

            /** @brief Returns whether the hit has been used on the labeled segments we refer to (e.g. truth or data Zµµ)
             *  @param hit: Reference to the hit to check */
            virtual bool isLabeled(const MuonR4::SpacePoint& hit) const = 0;
            virtual bool isLabeled(const xAOD::UncalibratedMeasurement& hit) const = 0;

            using LabeledSegmentSet = std::unordered_set<const xAOD::MuonSegment*>;
            /** @brief Fetches all labeled (e.g. by truth or Zµµ reco) segments containing at least one measurement in the list passed as arg 
             *  @param hits: Vector of hits to search */
            virtual LabeledSegmentSet getLabeledSegments(const std::vector<const MuonR4::SpacePoint*>& hits) const = 0;
            virtual LabeledSegmentSet getLabeledSegments(const std::vector<const xAOD::UncalibratedMeasurement*>& hits) const = 0;

    };
}
#endif

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONRECTOOLINTERFACESR4_IPATTERNVISUALIZATIONTOOL_H
#define MUONRECTOOLINTERFACESR4_IPATTERNVISUALIZATIONTOOL_H


#include <GaudiKernel/IAlgTool.h>
#include <GeoPrimitives/GeoPrimitives.h>
#include <MuonPatternEvent/MuonHoughDefs.h>
#include <xAODMuon/MuonSegment.h>
#include <MuonPatternEvent/Segment.h>
#include <Acts/Visualization/ObjVisualization3D.hpp>

#include <memory>

class EventContext;
class TObject;

namespace MuonR4 {
    class SpacePointBucket;
    class SegmentSeed;
    class Segment;
}

namespace MuonValR4{
    /** @brief Helper tool to visualize a pattern recogntion incident or a certain stage of the segment fit. */

    class IPatternVisualizationTool : virtual public IAlgTool {
        public:
            DeclareInterfaceID(IPatternVisualizationTool, 1, 0);
            
            using PrimitivePtr = std::unique_ptr<TObject>;
            using PrimitiveVec = std::vector<PrimitivePtr>;
            using MaximumVec = std::vector<MuonR4::ActsPeakFinderForMuon::Maximum>;

            virtual ~IPatternVisualizationTool() = default;
            
            /** @brief Draws the content of the bucket on a TCanvas
             *  @param ctx: EventContext to fetch calibration & alignment
             *  @param bucket: Spacepoint bucket to draw
             *  @param extraLabel: Extra label for the legend */
            virtual void visualizeBucket(const EventContext& ctx,
                                         const MuonR4::SpacePointBucket& bucket,
                                         const std::string& extraLabel) const = 0;
            /** @brief Draws the content of the bucket on a TCanvas
             *  @param ctx: EventContext to fetch calibration & alignment
             *  @param bucket: Spacepoint bucket to draw
             *  @param extraLabel: Extra label for the legend
             *  @param extraPaints: Other objects that shall be painted onto the canvas */
            virtual void visualizeBucket(const EventContext& ctx,
                                         const MuonR4::SpacePointBucket& bucket,
                                         const std::string& extraLabel,
                                         PrimitiveVec&& extraPaints) const = 0;
            /** @brief Visualize the content of the Hough accumulator in a TH2 histogram
             *  @param ctx: EventContext to fetch calibration & alignment
             *  @param accumulator: Hough accumulator to visualize
             *  @param axisRanges: Current axis ranges to interpret the accumulator bins
             *  @param maxima: Extracted maxima from the accumulator
             *  @param extraLabel: Extra label for the legend */            
            virtual void visualizeAccumulator(const EventContext& ctx,
                                              const MuonR4::HoughPlane& accumulator,
                                              const Acts::HoughTransformUtils::HoughAxisRanges& axisRanges,
                                              const MaximumVec& maxima,
                                              const std::string& extraLabel) const = 0;
            /** @brief Visualize the content of the Hough accumulator in a TH2 histogram
             *  @param ctx: EventContext to fetch calibration & alignment
             *  @param accumulator: Hough accumulator to visualize
             *  @param axisRanges: Current axis ranges to interpret the accumulator bins
             *  @param maxima: Extracted maxima from the accumulator
             *  @param extraLabel: Extra label for the legend
             *  @param extraPaints: Other objects that shall be painted onto the canvas  */            
            virtual void visualizeAccumulator(const EventContext& ctx,
                                              const MuonR4::HoughPlane& accumulator,
                                              const Acts::HoughTransformUtils::HoughAxisRanges& axisRanges,
                                              const MaximumVec& maxima,
                                              const std::string& extraLabel,
                                              PrimitiveVec&& extraPaints) const = 0;
            
            /** @brief  Visualize the used space points of the seed together with the seed parameters
             *  @param ctx: EventContext to fetch calibration & alignment
             *  @param seed: Reference to the seed to visualize 
             *  @param extraLabel: Extra label for the legend */
            virtual void visualizeSeed(const EventContext& ctx,
                                       const MuonR4::SegmentSeed& seed,
                                       const std::string& extraLabel) const = 0;
            /** @brief  Visualize the used space points of the seed together with the seed parameters
             *  @param ctx: EventContext to fetch calibration & alignment
             *  @param seed: Reference to the seed to visualize 
             *  @param extraLabel: Extra label for the legend 
             *  @param extraPaints: Other objects that shall be painted onto the canvas  */            
            virtual void visualizeSeed(const EventContext& ctx,
                                       const MuonR4::SegmentSeed& seed,
                                       const std::string& extraLabel,
                                       PrimitiveVec&& extraPaints) const = 0;
            /** @brief  Visualize the used space points of the segment together with the seed parameters
             *  @param ctx: EventContext to fetch calibration & alignment
             *  @param segment: Reference to the segment to visualize 
             *  @param extraLabel: Extra label for the legend */
            virtual void visualizeSegment(const EventContext& ctx,
                                          const MuonR4::Segment& segment,
                                          const std::string& extraLabel) const = 0;
            /** @brief  Visualize the used space points of the segment together with the seed parameters
             *  @param ctx: EventContext to fetch calibration & alignment
             *  @param segment: Reference to the segment to visualize 
             *  @param extraLabel: Extra label for the legend 
             *  @param extraPaints: Other objects that shall be painted onto the canvas  */            
            virtual void visualizeSegment(const EventContext& ctx,
                                          const MuonR4::Segment& segment,
                                          const std::string& extraLabel,
                                          PrimitiveVec&& extraPaints) const = 0;

            /** @brief Visualize the measurement objects associated with a reconstructed segment fromt he segment.             
             *  @param ctx: EventContext used to access geometry, calibration, and alignment conditions.
             *  @param segment: Reference to the reconstructed segment whose measurements shall be visualized.
             *  @param extraLabel: The extra label for the obj file otuput name.*/    
            virtual void visualizeSegmentsMeasurementsObj(const EventContext& ctx, 
                                                          const MuonR4::Segment& segment,
                                                          const std::string& extraLabel) const = 0;

            /** @brief Visualize the underlying measurement objects from the calibrated space points.             
             *  @param ctx: EventContext used to access geometry, calibration, and alignment conditions.
             *  @param measVec: Reference to the calibrated spacepoints we want to visualize.
             *  @param extraLabel: The extra label for the obj file output name.*/   
            virtual void visualizeSegmentsMeasurementsObj(const EventContext& ctx, 
                                                          const MuonR4::Segment::MeasVec& measVec,
                                                          const std::string& extraLabel) const = 0;

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

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONPATTERNRECOGNITIONTEST_PATTERNVISUALIZATIONTOOL_H
#define MUONR4_MUONPATTERNRECOGNITIONTEST_PATTERNVISUALIZATIONTOOL_H

#include <MuonRecToolInterfacesR4/IPatternVisualizationTool.h>
#include <MuonRecToolInterfacesR4/IRootVisualizationService.h>

#include <AthenaBaseComps/AthAlgTool.h>

#include <StoreGate/ReadDecorHandleKeyArray.h>
#include <StoreGate/ReadHandleKeyArray.h>

#include <xAODMeasurementBase/UncalibratedMeasurementContainer.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <ActsGeometryInterfaces/GeometryContext.h>
#include <xAODMuon/MuonSegmentContainer.h>

#include <MuonPatternEvent/HoughEventData.h>


namespace MuonR4 {
    class SpacePoint;
}

namespace MuonValR4 {
    class PatternVisualizationTool : public extends<AthAlgTool, IPatternVisualizationTool> {
        public:
            
            using base_class::base_class;

            virtual StatusCode initialize() override final;

            virtual void visualizeBucket(const EventContext& ctx,
                                         const MuonR4::SpacePointBucket& bucket,
                                         const std::string& extraLabel) const override final;
            virtual void visualizeBucket(const EventContext& ctx,
                                         const MuonR4::SpacePointBucket& bucket,
                                         const std::string& extraLabel,
                                         PrimitiveVec&& extraPaints) const override final;
            
            virtual void visualizeSeed(const EventContext& ctx,
                                       const MuonR4::SegmentSeed& seed,
                                       const std::string& extraLabel) const override final;
            
            
            virtual void visualizeSeed(const EventContext& ctx,
                                       const MuonR4::SegmentSeed& seed,
                                       const std::string& extraLabel,
                                       PrimitiveVec&& extraPaints) const override final;


            virtual void visualizeAccumulator(const EventContext& ctx,
                                              const MuonR4::HoughPlane& accumulator,
                                              const Acts::HoughTransformUtils::HoughAxisRanges& axisRanges,
                                              const MaximumVec& maxima,
                                              const std::string& extraLabel) const override final;
            
            virtual void visualizeAccumulator(const EventContext& ctx,
                                              const MuonR4::HoughPlane& accumulator,
                                              const Acts::HoughTransformUtils::HoughAxisRanges& axisRanges,
                                              const MaximumVec& maxima,
                                              const std::string& extraLabel,
                                              PrimitiveVec&& extraPaints) const override final;
            
            virtual void visualizeSegment(const EventContext& ctx,
                                          const MuonR4::Segment& segment,
                                          const std::string& extraLabel) const override final;
            
            virtual void visualizeSegment(const EventContext& ctx,
                                          const MuonR4::Segment& segment,
                                          const std::string& extraLabel,
                                          PrimitiveVec&& extraPaints) const override final;


            using LabeledSegmentSet = std::unordered_set<const xAOD::MuonSegment*>;

            /** @brief Returns whether the hit has been used on the labeled segments we refer to (e.g. truth or data Zµµ)
             *  @param hits: Vector of hits to search */
            virtual LabeledSegmentSet getLabeledSegments(const std::vector<const MuonR4::SpacePoint*>& hits) const override final;
            virtual LabeledSegmentSet getLabeledSegments(const std::vector<const xAOD::UncalibratedMeasurement*>& hits) const override final;
            
            /** @brief Fetches all labeled (e.g. by truth or Zµµ reco) segments containing at least one measurement in the list passed as arg 
             *  @param hit: Reference to the hit to check */
            virtual bool isLabeled(const MuonR4::SpacePoint& hit) const override final;
            virtual bool isLabeled(const xAOD::UncalibratedMeasurement& hit) const override final;
        private:
            using Canvas_t = IRootVisualizationService::ICanvasObject;
            using Edges = Canvas_t::AxisRanges;
            /** @brief Translates the Spacepoint information into TObjects that are dawn on the canvas
              *         & evaluates the size of the box surrounding the event. Returns true 
              *         if at least 2 objects were created.
              *  @param bucket: SpacePoint bucket from which the hits are taken. If the <DisplayBucket>
              *                  is set true, the hits that were not in the subset are also painted with hollow filling
              *  @param hitsToDraw: Primary hit collection to draw which are drawn with fullFillStyle
              *  @param canvas: Reference to the Canvas object to which the drawn hits
              *                 are appended. The drawn hits also expand the drawn range
              *  @param view: Either the eta or the phi view? */
            template<class SpacePointType>
                bool drawHits(const MuonR4::SpacePointBucket& bucket,
                              const std::vector<SpacePointType>& hitsToDraw,
                              Canvas_t& canvasDim,
                              unsigned int view) const;

            /** @brief Converts a Hit into a particular TBox/ TEllipse for drawing. If the hit
             *         provides information that's requested for that view. The underlying space pointer
             *         is returned and the dimensions of the canvas are updated, if the drawing was succesful
             *  @param hit: Reference to the hit to draw
             *  @param canvas: Reference to the Canvas object to which the drawn hits
             *                 are appended. The drawn hits also expand the drawn range
             *  @param view: Draw the hit either in the eta or phi view,
             *  @param fillStyle: Standard fill style for the box e.g. full. */
            template<class SpacePointType>
                const MuonR4::SpacePoint* drawHit(const SpacePointType& hit,
                                                  Canvas_t& canvas,
                                                  const unsigned int view, 
                                                  unsigned int fillStyle) const;
            /** @brief Writes the chi2 of the hits onto the Canvas.
             *  @param pars: Parameter for which the chi2 is evaluated,
             *  @param hits: Vector of hits to consider
             *  @param canvas: Reference to the Canvas object to which the drawn hits
             *                 are appended
             *  @param legX: x-position of the list
             *  @param statLegY: y-position of the first entry. The next one is 0.05 apart
             *  @param endLeg: y-position of the last shown entry. The routine aborts if there're more
             *                 hits in the event. */
            template<class SpacePointType>
                void writeChi2(const MuonR4::SegmentFit::Parameters& pars,
                              const std::vector<SpacePointType>& hits,
                              Canvas_t& canvas,
                              const double legX = 0.2, double startLegY = 0.8, 
                              const double endLegY  = 0.3) const;
            
            /** @brief Paints the truth sim hits associated with the segment.
             *         Hits are drawn as orange arrows
             *  @param ctx: EventContext to fetch the alignment constants
             *  @param truthSeg: Segment made from truth sim hits
             *  @param canvas: Reference to the Canvas object to which the drawn hits
             *                 are appended
             *  @param view: Draw the hit either in the eta or phi view */
            void paintSimHits(const EventContext& ctx,
                              const xAOD::MuonSegment& truthSeg,
                              Canvas_t& canvas,
                              const int view) const;
            /** @brief Service handle of the visualization service */
            ServiceHandle<IRootVisualizationService> m_visualSvc{this, "VisualSvc", "MuonValR4::RootVisualizationService"};
            /** @brief Token to present to the visualization service such that the display froms this 
             *          tool are grouped together */
            IRootVisualizationService::ClientToken m_clientToken{};
            /** @brief Maximum canvases to draw */
            Gaudi::Property<unsigned int> m_canvasLimit{this, "CanvasLimits", 5000};
            /** @brief If set to true each canvas is saved into a dedicated pdf file */ 
            Gaudi::Property<bool> m_saveSinglePDFs{this, "saveSinglePDFs", false};
            /** @brief If set to true a summary Canvas is created. */
            Gaudi::Property<bool> m_saveSummaryPDF{this, "saveSummaryPDF",  false};
            /** @brief Prefix of the individual canvas file names <MANDATORY>  */
            Gaudi::Property<std::string> m_canvasPrefix{this, "CanvasPreFix", ""};
            /** @brief Define the subdirectory in which the plots shall be saved */
            Gaudi::Property<std::string> m_subDir{this, "outSubDir", ""};
            /** @brief Display the surrounding hits from the bucket not part of the seed*/            
            Gaudi::Property<bool> m_displayBucket{this, "displayBucket", true};
            /** @brief Swtich toggling whether the accumulator view are in the eta or phi plane.
             *         Just affects the axis labels */
            Gaudi::Property<bool> m_accumlIsEta{this, "AccumulatorsInEtaPlane", true}; 
            /** @brief Switch to visualize the eta view of the bucket event */
            Gaudi::Property<bool> m_doEtaBucketViews{this,"doEtaBucketViews", true};
            /** @brief Switch to visualize the phi view of the bucket event */
            Gaudi::Property<bool> m_doPhiBucketViews{this,"doPhiBucketViews", false};
            /** @brief Switch to visualize the truth hits  */
            Gaudi::Property<bool> m_paintTruthHits{this, "paintTruthHits", false};
            /** @brief Declare dependency on the prep data containers */
            SG::ReadHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_prepContainerKeys{this, "PrdContainer", {}};
            /** @brief List of truth segment links to fetch */
            Gaudi::Property<std::set<std::string>> m_truthSegLinks{this, "TruthSegDecors", {}};
            /** @brief Declaration of the dependency on the decorations. (Overwritten in initialize) */
            SG::ReadDecorHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_truthLinkDecorKeys{this, "LinkDecorKeys", {}};
            using SegLink_t = ElementLink<xAOD::MuonSegmentContainer>;
            using SegLinkVec_t = std::vector<SegLink_t>;
            using SegLinkDecor_t = SG::AuxElement::ConstAccessor<SegLinkVec_t>;
            std::vector<SegLinkDecor_t> m_truthLinkDecors{};

            Gaudi::Property<bool> m_displayOnlyTruth{this, "displayTruthOnly", false}; 

            /** @brief pointer to the Detector manager */
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
            /** @brief Geometry context key to retrieve the alignment */ 
            SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** @brief Service Handle to the IMuonIdHelperSvc */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Flag toggling whether all Canvases have been exhausted */
            mutable std::atomic<bool> m_plotsDone{false};

    };
    
}

#endif

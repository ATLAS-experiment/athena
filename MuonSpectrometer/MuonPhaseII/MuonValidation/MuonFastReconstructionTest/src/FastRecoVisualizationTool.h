/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONPATTERNRECOGNITIONTEST_PATTERNVISUALIZATIONTOOL_H
#define MUONR4_MUONPATTERNRECOGNITIONTEST_PATTERNVISUALIZATIONTOOL_H

#include <MuonRecToolInterfacesR4/IFastRecoVisualizationTool.h>
#include <MuonRecToolInterfacesR4/IRootVisualizationService.h>
#include "MuonVisualizationHelpersR4/VisualizationHelpers.h"
#include <MuonFastRecoEvent/GlobalPattern.h>

#include <AthenaBaseComps/AthAlgTool.h>
#include <StoreGate/ReadDecorHandleKeyArray.h>
#include <StoreGate/ReadHandleKeyArray.h>

#include <xAODMeasurementBase/UncalibratedMeasurementContainer.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <ActsGeometryInterfaces/GeometryContext.h>
#include <MuonPatternEvent/Segment.h>
#include <xAODMuon/MuonSegmentContainer.h>


namespace MuonR4 {
    class SpacePoint;
}

namespace MuonValR4 {
    class FastRecoVisualizationTool : public extends<AthAlgTool, IFastRecoVisualizationTool> {
        public:
            using base_class::base_class;

            virtual StatusCode initialize() override final;

            virtual void plotPatternBuckets(const EventContext& ctx,
                                            const std::string& extraLabel,
                                            PatternHitVisualInfoVec&& patternVisualVec) const override final;
            virtual void plotPatternBuckets(const EventContext& ctx,
                                            const std::string& extraLabel,
                                            PatternHitVisualInfoVec&& patternVisualVec,
                                            PrimitiveVec&& extraPaints) const override final;
            virtual void plotPatternBuckets(const EventContext& ctx,
                                            const std::string& extraLabel,
                                            PatternHitVisualInfo&& patternVisual) const override final;
            virtual void plotPatternBuckets(const EventContext& ctx,
                                            const std::string& extraLabel,
                                            PatternHitVisualInfo&& patternVisual,
                                            PrimitiveVec&& extraPaints) const override final;

            /** @brief Returns whether the hit has been used on the labeled segments we refer to (e.g. truth or data Zµµ)
             *  @param hits: Vector of hits to search */
            virtual LabeledSegmentSet getLabeledSegments(const std::vector<const MuonR4::SpacePoint*>& hits) const override final;
            virtual LabeledSegmentSet getLabeledSegments(const std::vector<const xAOD::UncalibratedMeasurement*>& hits) const override final;
            
            /** @brief Fetches all labeled (e.g. by truth or Zµµ reco) segments containing at least one measurement in the list passed as arg 
             *  @param hit: Reference to the hit to check */
            virtual bool isLabeled(const MuonR4::SpacePoint& hit) const override final;
            virtual bool isLabeled(const xAOD::UncalibratedMeasurement& hit) const override final;
        private:
            /** @brief Enum for the different views */
            enum class View {
                objViewEta = static_cast<int>(MuonValR4::objViewEta),
                objViewPhi = static_cast<int>(MuonValR4::objViewPhi),
                objViewZR = 2,
                objViewRZ = 3
            };
            using Canvas_t = IRootVisualizationService::ICanvasObject;
            using Edges = Canvas_t::AxisRanges;
            /** @brief Plot a single pattern bucket
             *  @param ctx: Event Context
             *  @param extraLabel: Extra label for the plot
             *  @param bucket: Reference to the pattern bucket to plot
             *  @param patternVisual: Visual information for the pattern
             *  @param extraPaints: Additional painting instructions */
            void plotPatternBucket(const EventContext& ctx,
                                   const std::string& extraLabel,
                                   const MuonR4::SpacePointBucket& bucket,
                                   PatternHitVisualInfo&& patternVisual,
                                   PrimitiveVec&& extraPaints) const;
            /** @brief Draw the search window lines in the local frame expressed by the Transform3D for eta views and in
              *        the GlobalR-GlobalZ plane for RZ/ZR views.
              *  @param localToGlobalBucket: Bucket frame transformation, used for the conversion of the line parameters 
              *                              from global to local frame and viversa
              *  @param outputContainer: Vector to which the drawn lines are appended
              *  @param thetaMin: Minimum global theta of the search window
              *  @param thetaMax: Maximum global theta of the search window
              *  @param canvas: Reference to the Canvas object to which the drawn lines. Needed to set the line limits
              *  @param view: Draw the line in the eta or RZ/ZR view */
            void drawSearchWindow(const Amg::Transform3D& localToGlobalBucket,
                                  PrimitiveVec& outputContainer,
                                  const double thetaMin, 
                                  const double thetaMax,
                                  const Canvas_t& canvas,
                                  const View view) const;
            /** @brief Draw the pattern line and acceptance window for the testHit used during pattern building in the 
              *        local frame expressed by the Transform3D for eta views and in the GlobalR-GlobalZ plane for RZ/ZR views.
              *  @param gctx: Geometry context needed to retrieve the seed global position and its position in the bucket frame
              *  @param localToGlobalBucket: Bucket frame transformation, used for the conversion of the line parameters 
              *                              from global to local frame and viversa
              *  @param outputContainer: Vector to which the drawn lines are appended
              *  @param seed: Seed hit of the pattern
              *  @param testHit: Hit being tested during pattern building
              *  @param lineSlope: Slope of the pattern line in the GlobalR-GlobalZ plane computed during pattern building to test the testHit
              *  @param Rwindow: Size of the acceptance window in the GlobalR direction to accept the testHit in pattern building
              *  @param status: Status of the testHit during pattern building (e.g. accepted, rejected or replaced)
              *  @param canvas: Reference to the Canvas object to which the drawn lines. Needed to set the line limits
              *  @param view: Draw the line in the eta or RZ/ZR view */
            void drawLineResidual(const ActsTrk::GeometryContext& gctx,
                                  const Amg::Transform3D& localToGlobalBucket,
                                  PrimitiveVec& outputContainer,
                                  const MuonR4::SpacePoint* seed, 
                                  const MuonR4::SpacePoint* testHit,
                                  const double lineSlope,
                                  const double Rwindow,
                                  const PatternHitVisualInfo::HitStatus status,
                                  const Canvas_t& canvas,
                                  const View view) const;
            /** @brief Draw a segment on the canvas
                *  @param segment: Segment to draw
                *  @param localToGlobalBucket: Bucket frame transformation, used for the conversion of the line parameters 
                *                              from global to local frame and viversa
                *  @param outputContainer: Vector to which the drawn lines are appended
                *  @param drawnTrueLabel: Boolean set to true if the label with the true segment parameters has already been drawn on the canvas. If not, it is drawn with the segment and the boolean is set to true.
                *  @param canvas: Reference to the Canvas object to which the drawn lines. Needed to set the line limits
                *  @param view: Draw the line in the eta or RZ/ZR view */
            void drawSegment(const xAOD::MuonSegment& segment,
                             const Amg::Transform3D& localToGlobalBucket,
                             PrimitiveVec& outputContainer,
                             bool& drawnTrueLabel,
                             const Canvas_t& canvas,
                             const View view) const;
            /** @brief Translates the Spacepoint information into TObjects that are dawn on the canvas
              *         & evaluates the size of the box surrounding the event. Returns true 
              *         if at least 2 objects were created.
              *  @param bucket: SpacePoint bucket from which the hits are taken. If the <DisplayBucket>
              *                  is set true, the hits that were not in the subset are also painted with hollow filling
              *  @param localToGlobalBucket: Bucket frame transformation, used for the conversion of the line parameters 
              *                              from global to local frame and viversa
              *  @param hitsToDraw: Primary hit collection to draw which are drawn with fullFillStyle
              *  @param canvas: Reference to the Canvas object to which the drawn hits
              *                 are appended. The drawn hits also expand the drawn range
              *  @param view: Either the eta or the phi view? */
            template<class SpacePointType>
                bool drawHits(const MuonR4::SpacePointBucket& bucket,
                              const Amg::Transform3D& localToGlobalBucket,
                              const std::vector<SpacePointType>& hitsToDraw,
                              Canvas_t& canvasDim,
                              const View view) const;

            /** @brief Converts a Hit into a particular TBox/ TEllipse for drawing. If the hit
             *         provides information that's requested for that view. The underlying space pointer
             *         is returned and the dimensions of the canvas are updated, if the drawing was succesful
             *  @param hit: Reference to the hit to draw
             *  @param localToGlobalBucket: Bucket frame transformation, used for the conversion of the line parameters 
             *                              from global to local frame and viversa
             *  @param canvas: Reference to the Canvas object to which the drawn hits
             *                 are appended. The drawn hits also expand the drawn range
             *  @param view: Draw the hit either in the eta or phi view,
             *  @param fillStyle: Standard fill style for the box e.g. full. */
            template<class SpacePointType>
                const MuonR4::SpacePoint* drawHit(const SpacePointType& hit,
                                                  const Amg::Transform3D& localToGlobalBucket,
                                                  Canvas_t& canvas,
                                                  const View view, 
                                                  unsigned int fillStyle) const;

            /** @brief Paints the truth sim hits associated with the segment.
             *         Hits are drawn as orange arrows
             *  @param gctx: Geometry context to fetch the alignment constants
             *  @param truthSeg: Segment made from truth sim hits
             *  @param outputContainer: Vector to which the drawn lines are appended
             *  @param view: Draw the hit either in the eta or phi view */
            void paintSimHits(const ActsTrk::GeometryContext& gctx,
                              const Amg::Transform3D& localToGlobalBucket,
                              const xAOD::MuonSegment& truthSeg,
                              PrimitiveVec& outputContainer,
                              const View view) const;
            /** @brief Draws a line given the parameters of the line in the local frame for Eta and Phi views 
             *         and in the R-Z plane for RZ (and ZR) views.
            *  @param lineDirection: Line direction vector in the local frame for Eta and Phi views or in the z-y plane (convention) for RZ and ZR views
            *  @param linePoint: Point on the line in the local frame for Eta and Phi views  or in the z-y plane (convention) for RZ and ZR views
            *  @param lowEnd: Lower boundary in Canvas-y of the line (i.e. loc Z) for Eta/Phi views, or in global R for ZR/RZ views. If the 
            *                 line is perpendicular to the Canvas-y/globalR axis, it will be interpreted as Canvas-x/globalZ lower limit.
            *  @param highEnd: Upper boundary in Canvas-y of the line, (i.e. loc Z) for Eta/Phi views, or in global R for ZR/RZ views. If the 
            *                  line is perpendicular to the Canvas-y/globalR axis, it will be interpreted as Canvas-x/globalZ upper limit.
            *  @param lineStyle: Style of the drawn line (cf. TAttLine documentation)
            *  @param view: Is the line placed in the y-z or x-z plane or R-Z/Z-R planes */
            std::unique_ptr<TLine> drawLine(const Amg::Vector3D& lineDirection,
                                            const Amg::Vector3D& linePoint,
                                            const double lowEnd, 
                                            const double highEnd,
                                            const int color = kRed +1,
                                            const int lineStyle = kDashed,
                                            const View view = View::objViewEta) const;

            /** @brief Service handle of the visualization service */
            ServiceHandle<IRootVisualizationService> m_visualSvc{this, "VisualSvc", "MuonValR4::RootVisualizationService"};
            /** @brief Token to present to the visualization service such that the display froms this tool are grouped together */
            IRootVisualizationService::ClientToken m_clientToken{};
            /** @brief Maximum canvases to draw */
            UnsignedIntegerProperty m_canvasLimit{this, "CanvasLimits", 5000};
            /** @brief If set to true each canvas is saved into a dedicated pdf file */ 
            BooleanProperty m_saveSinglePDFs{this, "saveSinglePDFs", false};
            /** @brief If set to true a summary Canvas is created. */
            BooleanProperty m_saveSummaryPDF{this, "saveSummaryPDF",  false};
            /** @brief Prefix of the individual canvas file names <MANDATORY>  */
            StringProperty m_canvasPrefix{this, "CanvasPreFix", ""};
            /** @brief Define the subdirectory in which the plots shall be saved */
            StringProperty m_subDir{this, "outSubDir", ""};
            /** @brief Switch to visualize the eta view of the bucket event */
            BooleanProperty m_doEtaBucketViews{this,"doEtaBucketViews", true};
            /** @brief Switch to visualize the phi view of the bucket event */
            BooleanProperty m_doPhiBucketViews{this,"doPhiBucketViews", false};
            BooleanProperty m_doRZBucketViews{this,"doRZBucketViews", false};
            /** @brief Switch to visualize the truth hits  */
            BooleanProperty m_paintTruthHits{this, "paintTruthHits", false};
            /** @brief Switch to visualize the truth segment */
            BooleanProperty m_paintTruthSegment{this, "paintTruthSegment", false};
            /** @brief Switch to visualize successfull patterns */
            BooleanProperty m_paintSuccessfullPatterns {this, "PaintSuccessfullPatterns", true, "Toggle the display of successfully built patterns in the visualization"};
            /** @brief Switch to visualize failed patterns */
            BooleanProperty m_paintFailedPatterns {this, "PaintFailedPatterns", false, "Toggle the display of failed patterns in the visualization"};
            /** @brief Switch to visualize overlap patterns */
            BooleanProperty m_paintOverlapPatterns {this, "PaintOverlapPatterns", false, "Toggle the display of overlap patterns in the visualization"};
            /** @brief Declare dependency on the prep data containers */
            SG::ReadHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_prepContainerKeys{this, "PrdContainer", {}};
            /** @brief List of truth segment links to fetch */
            Gaudi::Property<std::set<std::string>> m_truthSegLinks{this, "TruthSegDecors", {}};
            /** @brief Declaration of the dependency on the decorations. (Overwritten in initialize) */
            SG::ReadDecorHandleKeyArray<xAOD::UncalibratedMeasurementContainer> m_truthLinkDecorKeys{this, "LinkDecorKeys", {}};
            using SegLink_t = ElementLink<xAOD::MuonSegmentContainer>;
            using SegLinkVec_t = std::vector<SegLink_t>;
            using SegLinkDecor_t = SG::ConstAccessor<SegLinkVec_t>;
            std::vector<SegLinkDecor_t> m_truthLinkDecors{};

            /** @brief Toggle to print pattern buckets only if they contain truth hits */
            BooleanProperty m_displayOnlyTruth{this, "displayTruthOnly", false}; 
            /** @brief Toggle to print pattern buckets only if they contain pattern hits */
            BooleanProperty m_displayOnlyWithPattern{this, "displayOnlyWithPattern", true}; 

            /** @brief pointer to the Detector manager */
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
            /** @brief Geometry context key to retrieve the alignment */ 
            ActsTrk::GeoContextReadKey_t m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** @brief Service Handle to the IMuonIdHelperSvc */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Flag toggling whether all Canvases have been exhausted */
            mutable std::atomic<bool> m_plotsDone ATLAS_THREAD_SAFE {false};
    };
    
}

#endif

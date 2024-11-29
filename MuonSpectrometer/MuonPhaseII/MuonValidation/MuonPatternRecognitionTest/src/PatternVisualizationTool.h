/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONPATTERNRECOGNITIONTEST_PATTERNVISUALIZATIONTOOL_H
#define MUONR4_MUONPATTERNRECOGNITIONTEST_PATTERNVISUALIZATIONTOOL_H

#include <MuonRecToolInterfacesR4/IPatternVisualizationTool.h>
#include <AthenaBaseComps/AthAlgTool.h>

#include <StoreGate/ReadDecorHandleKeyArray.h>
#include <StoreGate/ReadHandleKeyArray.h>

#include <xAODMeasurementBase/UncalibratedMeasurementContainer.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>
#include <ActsGeometryInterfaces/ActsGeometryContext.h>
#include <xAODMuon/MuonSegmentContainer.h>

#include <MuonPatternEvent/HoughEventData.h>

#include "TCanvas.h"
#include "TFile.h"
#include <mutex>

namespace MuonR4 {
    class SpacePoint;
}

namespace MuonValR4 {
    class PatternVisualizationTool : public extends<AthAlgTool, IPatternVisualizationTool> {
        public:
            
            PatternVisualizationTool(const std::string& type, const std::string& name, const IInterface* parent);

            virtual StatusCode initialize() override final;
            virtual StatusCode finalize() override final;

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
            /** @brief Closes the summary canvas & closes the associated ROOT file */ 
            void closeSummaryCanvas() const;
            /** @brief Create a new Canvas & draw the underlying frame
             *  @param ctx: EventContext to generate a unique canvas name
             *  @param canvasDim: Array encoding the canvas dimensions
             *                    [0] -> low x;  [1] -> high x, [2] -> low y, [3] -> high y
             *  @param view: Is the canvas created in the context of an eta or phi visualization view
             *               The x-axis of the frame is adapted accordingly*/
            std::unique_ptr<TCanvas> makeCanvas(const EventContext& ctx,
                                                const std::array<double, 4>& canvasDim,
                                                const int view) const;

            /** @brief Saves the Canvas as pdf & also into the ROOT file. If the canvase limit is reached,
             *         then the summary files are closed as well. 
             *  @param ctx: EventContext to fetch the event number
             *  @param chambId: Identifier of the chamber from which the hits are stemming from
             *  @param primtives: List of objects to draw onto the canvas
             *  @param extraLabel: Externally passed extra label */
            void saveCanvas(const EventContext& ctx,
                            const Identifier& chambId,
                            TCanvas& canvas,
                            const std::string& extraLabel) const;
            
            /** @brief Translates the Spacepoint information into TObjects that are dawn on the canvas
              *         & evaluates the size of the box surrounding the event. Returns true 
              *         if at least 2 objects were created.
              *  @param bucket: SpacePoint bucket from which the hits are taken. If the <DisplayBucket>
              *                  is set true, the hits that were not in the subset are also painted with hollow filling
              *  @param hitsToDraw: Primary hit collection to draw which are drawn with fullFillStyle
              *  @param primitivesToPush: Output vector into which the painting objects are pushed_back
              *  @param canvasDim: Array encoding the canvas dimensions
              *                    [0] -> low x;  [1] -> high x, [2] -> low y, [3] -> high y
              *  @param view: Either the eta or the phi view? */
            template<class SpacePointType>
                bool drawHits(const MuonR4::SpacePointBucket& bucket,
                              const std::vector<SpacePointType>& hitsToDraw,
                              PrimitiveVec& primitivesToPush,
                              std::array<double, 4>& canvasDim,
                              unsigned int view) const;

            /** @brief Converts a Hit into a particular TBox/ TEllipse for drawing. If the hit
             *         provides information that's requested for that view. The underlying space pointer
             *         is returned and the dimensions of the canvas are updated, if the drawing was succesful
             *  @param hit: Reference to the hit to draw
             *  @param primitives: Vector into which the generated object is pushed
             *  @param canvasDim: Array encoding the canvas dimensions
             *                    [0] -> low x;  [1] -> high x, [2] -> low y, [3] -> high y
             *  @param view: Draw the hit either in the eta or phi view,
             *  @param fillStyle: Standard fill style for the box e.g. full. */
            template<class SpacePointType>
                const MuonR4::SpacePoint* drawHit(const SpacePointType& hit,
                                                  PrimitiveVec& primitives,
                                                  std::array<double, 4>& canvasDim,
                                                  const unsigned int view, 
                                                  unsigned int fillStyle) const;
            /** @brief Writes the chi2 of the hits onto the Canvas.
             *  @param pars: Parameter for which the chi2 is evaluated,
             *  @param hits: Vector of hits to consider
             *  @param primitivesToPush: Output vector into which the TLabels are pushed for later drawing
             *  @param legX: x-position of the list
             *  @param statLegY: y-position of the first entry. The next one is 0.05 apart
             *  @param endLeg: y-position of the last shown entry. The routine aborts if there're more
             *                 hits in the event. */
            template<class SpacePointType>
                void writeChi2(const MuonR4::SegmentFit::Parameters& pars,
                              const std::vector<SpacePointType>& hits,
                              PrimitiveVec& primitivesToPush,
                              const double legX = 0.2, double startLegY = 0.8, 
                              const double endLegY  = 0.3) const;
            
            /** @brief Draw the various primitives onto the Canvas. If a TLine is amongst them it's automatically
             *         adjusted to fit the canvas ranges
             *  @param canvas: Reference to the canvas to draw the primitives on
             *  @param prmitives: Primitives to draw onto the Canvas */
            void drawPrimitives(const TCanvas& can,
                                PrimitiveVec& primitives) const;
            /** @brief Maximum canvases to draw */
            Gaudi::Property<unsigned int> m_canvasLimit{this, "CanvasLimits", 5000};
            /** @brief If set to true each canvas is saved into a dedicated pdf file */ 
            Gaudi::Property<bool> m_saveSinglePDFs{this, "saveSinglePDFs", false};
            /** @brief If set to true a summary Canvas is created. 
             *         ATTENTION: There can be only one summary PDF in an athena job */
            Gaudi::Property<bool> m_saveSummaryPDF{this, "saveSummaryPDF",  false};
            /** @brief Name of the summary canvas & the ROOT file to save the monitoring plots */
            Gaudi::Property<std::string> m_allCanName{this, "AllCanvasName", "AllPatternPlots"};
            /** @brief Prefix of the individual canvas file names <MANDATORY>  */
            Gaudi::Property<std::string> m_canvasPrefix{this, "CanvasPreFix", ""};
            /** @brief Display the surrounding hits from the bucket not part of the seed*/            
            Gaudi::Property<bool> m_displayBucket{this, "displayBucket", true};
            /** @brief Extra safety margin to zoom out from the Canvas */
            Gaudi::Property<double> m_canvasExtraScale{this, "CanvasExtraScale" , 1.5};
            /** @brief Swtich toggling whether the accumulator view are in the eta or phi plane.
             *         Just affects the axis labels */
            Gaudi::Property<bool> m_accumlIsEta{this, "AccumulatorsInEtaPlane", true}; 
            /** @brief Switch to visualize the eta view of the bucket event */
            Gaudi::Property<bool> m_doEtaBucketViews{this,"doEtaBucketViews", true};
            /** @brief Switch to visualize the phi view of the bucket event */
            Gaudi::Property<bool> m_doPhiBucketViews{this,"doPhiBucketViews", true};
            /** @brief ATLAS label (Internal / Prelimnary / Simulation) */
            Gaudi::Property<std::string> m_AtlasLabel{this, "AtlasLabel", "Internal"};
            /** @brief Centre of mass energy label */
            Gaudi::Property<std::string> m_sqrtSLabel{this, "SqrtSLabel", "14"};
            /** @brief Luminosity label */
            Gaudi::Property<std::string> m_lumiLabel{this, "LumiLabel", ""};
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
           
            Gaudi::Property<unsigned> m_canvasWidth{this, "CanvasWidth", 800};
            Gaudi::Property<unsigned> m_canvasHeight{this, "CanvasHeight", 600};

            Gaudi::Property<bool> m_displayOnlyTruth{this, "displayTruthOnly", false}; 


            /** @brief pointer to the Detector manager */
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
            /** @brief Geometry context key to retrieve the alignment */ 
            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** @brief Service Handle to the IMuonIdHelperSvc */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            static std::mutex s_mutex;
            mutable std::unique_ptr<TCanvas> m_allCan ATLAS_THREAD_SAFE{};
            mutable std::unique_ptr<TFile> m_outFile ATLAS_THREAD_SAFE{};
            /** @brief how many canvases have been visualized */
            mutable std::atomic<unsigned int> m_canvCounter ATLAS_THREAD_SAFE{0};
    };
    
}

#endif

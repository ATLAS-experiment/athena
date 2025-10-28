/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTEST_TRACKVISUALIZATIONTOOL_H
#define MUONTRACKFINDINGTEST_TRACKVISUALIZATIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonRecToolInterfacesR4/ITrackVisualizationTool.h"

#include "Acts/Utilities/RangeXD.hpp"

#include "TCanvas.h"
#include "TFile.h"
#include "TLegend.h"

namespace MuonR4{
    class MsTrackSeeder;
}

namespace MuonValR4{
    class TrackVisualizationTool : public extends<AthAlgTool, ITrackVisualizationTool> {
        public:
            /** @brief Use the constructor from the parent class */
            using base_class::base_class;
            /** @brief Destructor */
            virtual ~TrackVisualizationTool() = default;
            
            virtual StatusCode initialize() override final;
            virtual StatusCode finalize() override final;

            /** @brief Displays all segments on the representative cylinder in the R-Z & X-Y plane
             *         and draws the markers of the successfully built seeds & truth segments
             *  @param ctx: EventContext to access store gate & conditions
             *  @param seederObj: Configured instance of the track seeder which actually constructed 
             *                    the seeds.
             *  @param segments: Container of all MS segments in the event
             *  @param seeds: The constructed track seeds from the event
             *  @param extraLabel: Extra label to be put onto the top of the shown canvases */
            virtual void displaySeeds(const EventContext& ctx,
                                      const MuonR4::MsTrackSeeder& seederObj,
                                      const xAOD::MuonSegmentContainer& segments,
                                      const MuonR4::MsTrackSeedContainer& seeds,
                                      const std::string& extraLabel) const override final;
            /** @brief Displays all segments on the representative cylinder in the R-Z & X-Y plane
            *         and draws the markers of the successfully built seeds & truth segments
            *  @param ctx: EventContext to access store gate & conditions
            *  @param seederObj: Configured instance of the track seeder which actually constructed 
            *                    the seeds.
            *  @param segments: Container of all MS segments in the event
            *  @param extraLabel: Extra label to be put onto the top of the shown canvases
            *  @param extPrimitives: Extra TObjects that should be additionally painted onto the Canvases */
            virtual void displaySeeds(const EventContext& ctx,
                                      const MuonR4::MsTrackSeeder& seederObj,
                                      const xAOD::MuonSegmentContainer& segments,
                                      const MuonR4::MsTrackSeedContainer& seeds,
                                      const std::string& extraLabel,
                                      PrimitivesVec_t && extPrimitives) const override final;
        private:
            /** @brief Helper struct to administer the markers & colors that are added to the legend */
            struct PlotLegend {
                /** @brief Constructor defining the Legned placement
                 *  @param xLow: Low x-end of the Legend (relative coords.)
                 *  @param yLow: Low y-end of the Legend (relative coords.)
                 *  @param xHigh: High x-end of the Legend (relative coords.)
                 *  @param yHigh: High y-end of the Legend (relative coords.) */
                PlotLegend(const double xLow, const double yLow,
                           const double xHigh, const double yHigh):
                    legend{std::make_unique<TLegend>(xLow, yLow, xHigh, yHigh)}{}
                /** @brief Add new color legend entry
                 *  @param color: TColor code of the color to add
                 *  @param label: Text to be dispayed */
                void addColor(const int color, const std::string& label);
                /** @brief Add new Marker style to the legend entry
                 *  @param marker: TMarkerStyle code to add
                 *  @param label: Text to be dispayed */
                void addMarker(const int marker, const std::string& label);
                std::unique_ptr<TLegend> legend{};
                using LegendEntry_t = std::unordered_map<int, PrimitivePtr_t>;
                LegendEntry_t markers{};
                LegendEntry_t colors{};
            };

            /** @brief Enumeration to toggle whether the segment shall displayed on the barrel/endcap */
            using Location = MuonR4::MsTrackSeed::Location;
            /** @brief Helper struct to determine the canvas ranges to be shown */
            using BoundBox_t = Acts::RangeXD<2, double>;
            /** @brief Createes a new empty range box ready for shrinkin useage */
            static BoundBox_t emptyBounds();
            /** @brief tuple to declare the axis labels */
            using AxisLabels_t = std::tuple<std::string, std::string>;
            /** @brief Creates a new TCanvas for drawing
             *  @param ctx: EventContext to give the canvas a unique name
             *  @param bounds: Array determining the x & y axis intervals
             *  @param axisLabels: Labels to be shown on the x & y axis */
            std::unique_ptr<TCanvas> makeCanvas(const EventContext& ctx,
                                                const BoundBox_t& bounds,
                                                const AxisLabels_t& axisLabels) const;
            /** @brief Closes the summary canvas & closes the associated ROOT file */ 
            void closeSummaryCanvas() const;
            /** @brief Enumeration to toggle whether the seeds shall be painted in x-y or R-z view */
            enum class DisplayView{RZ, XY};
            /** @brief Actual implementation of the display seeds method with the augmentation to define the view
            *  @param ctx: EventContext to access store gate & conditions
            *  @param seederObj: Configured instance of the track seeder which actually constructed the seeds.
            *  @param view: Actual view to display
            *  @param segments: Container of all MS segments in the event
            *  @param seeds: The constructed track seeds from the event
            *  @param extraLabel: Extra label to be put onto the top of the shown canvases
            *  @param extPrimitives: Extra TObjects that should be additionally painted onto the Canvases */
            void displaySeeds(const EventContext& ctx,
                              const MuonR4::MsTrackSeeder& seederObj,
                              const DisplayView view,
                              const xAOD::MuonSegmentContainer& segments,
                              const MuonR4::MsTrackSeedContainer& seeds,
                              const std::string& extraLabel,
                              PrimitivesVec_t& extPrimitives) const;


            /** @brief Add the truth segments to the canvas
             *  @param ctx: EventContext to access store gate
                @param seederObj: Configured instance of the track seeder which actually constructed the seeds.
                @param view: Actual view to display
                @param legend: Reference to the legend object to add the marker / truth legends
                @param drawPrim: Reference to the primitives to drawn. The markers are added to this vector */
            void fillTruthSeedPoints(const EventContext& ctx,
                                     const MuonR4::MsTrackSeeder& seeder,
                                     const DisplayView view,
                                     PlotLegend& legend, 
                                     PrimitivesVec_t& drawPrim) const;
            /** @brief Transforms the projected vector into the actual view, if it's xy then 
             *         an external phi is needed to place the marker. 
             *  @param phi: Angle for the xy view - taken from the segment direction
             *  @param posOnCylinder: Projection of the segment's position onto the seed cylinder
             *  @param view: Actual view to display */
            static Amg::Vector2D viewVector(const double phi,
                                            const Amg::Vector2D& posOnCylinder,
                                            const DisplayView view);
            /** @brief Key to the truth segment selection to draw the segment parameters */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_truthSegKey{this, "TruthSegkey", "MuonTruthSegments"};
            /** @brief Dependency on the geometry alignment */
            SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** @brief Maximum canvases to draw */
            Gaudi::Property<unsigned> m_canvasLimit{this, "CanvasLimit", 5000};
            /** @brief If set to true each canvas is saved into a dedicated pdf file */ 
            Gaudi::Property<bool> m_saveSinglePDFs{this, "saveSinglePDFs", true};
            /** @brief If set to true a summary Canvas is created. 
             *         ATTENTION: There can be only one summary PDF in an athena job */
            Gaudi::Property<bool> m_saveSummaryPDF{this, "saveSummaryPDF",  false};
            /** @brief Name of the summary canvas & the ROOT file to save the monitoring plots */
            Gaudi::Property<std::string> m_allCanName{this, "AllCanvasName", "AllMsTrackPlots"};
            /** @brief Prefix of the individual canvas file names <MANDATORY>  */
            Gaudi::Property<std::string> m_canvasPrefix{this, "CanvasPreFix", "MsTrackValid"};
            /** @brief Canvas dimensions */
            Gaudi::Property<unsigned> m_canvasWidth{this, "CanvasWidth", 800};
            Gaudi::Property<unsigned> m_canvasHeight{this, "CanvasHeight", 600};
            /** @brief ATLAS label (Internal / Prelimnary / Simulation) */
            Gaudi::Property<std::string> m_AtlasLabel{this, "AtlasLabel", "Internal"};
            /** @brief Centre of mass energy label */
            Gaudi::Property<std::string> m_sqrtSLabel{this, "SqrtSLabel", "14"};
            /** @brief Luminosity label */
            Gaudi::Property<std::string> m_lumiLabel{this, "LumiLabel", ""};
            /** @brief Abrivation of the helper struct to determine the Canvas dimensions */

            static std::mutex s_mutex;
            mutable std::unique_ptr<TCanvas> m_summaryCan ATLAS_THREAD_SAFE{};
            mutable std::unique_ptr<TFile> m_outFile ATLAS_THREAD_SAFE{};
            mutable std::atomic<unsigned int> m_canvCounter ATLAS_THREAD_SAFE{0};

    };
}

#endif
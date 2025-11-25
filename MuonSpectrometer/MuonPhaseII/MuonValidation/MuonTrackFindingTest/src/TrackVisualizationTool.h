/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTEST_TRACKVISUALIZATIONTOOL_H
#define MUONTRACKFINDINGTEST_TRACKVISUALIZATIONTOOL_H

#include <AthenaBaseComps/AthAlgTool.h>
#include <MuonRecToolInterfacesR4/ITrackVisualizationTool.h>
#include <MuonRecToolInterfacesR4/IRootVisualizationService.h>
#include <ActsGeometryInterfaces/IExtrapolationTool.h>

#include <TLegend.h>

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
            /** @brief Displays all segments on the representative cylinder in the R-Z & X-Y plane
             *         and draws the markers of the successfully built seeds & truth segments
             *  @param ctx: EventContext to access store gate & conditions
             *  @param seederObj: Configured instance of the track seeder which actually constructed 
             *                    the seeds.
             *  @param segments: Container of all MS segments in the event
             *  @param seeds: The constructed track seeds from the event */
            virtual void displaySeeds(const EventContext& ctx,
                                      const MuonR4::MsTrackSeeder& seederObj,
                                      const xAOD::MuonSegmentContainer& segments,
                                      const MuonR4::MsTrackSeedContainer& seeds) const override final;
            /** @brief Displays all segments on the representative cylinder in the R-Z & X-Y plane
            *         and draws the markers of the successfully built seeds & truth segments
            *  @param ctx: EventContext to access store gate & conditions
            *  @param seederObj: Configured instance of the track seeder which actually constructed 
            *                    the seeds.
            *  @param segments: Container of all MS segments in the event
            *  @param extPrimitives: Extra TObjects that should be additionally painted onto the Canvases */
            virtual void displaySeeds(const EventContext& ctx,
                                      const MuonR4::MsTrackSeeder& seederObj,
                                      const xAOD::MuonSegmentContainer& segments,
                                      const MuonR4::MsTrackSeedContainer& seeds,
                                      PrimitivesVec_t && extPrimitives) const override final;

            /** @brief Visualizes the measurements of the segments on the track seed together
             *         with their predicted local line parameters as an obj file. If parameters
             *         to extrapolate are parsed, then they're extrapolated to the end of the world
             *         and the trajectory is added to the obj
             * @param ctx: EventContext to fetch the conditions data & the event information
             * @param seed: MsTrack to visualize
             * @param parsToExt: Parameters to extrapolate on top
             * @param objName: Extra token to be added to the file name */
            virtual void displayTrackSeedObj(const EventContext& ctx,
                                             const MuonR4::MsTrackSeed& seed,
                                             const OptBoundPars_t& parsToExt,
                                             const std::string& objName = "") const override final;
        private:
            using Canvas_t = IRootVisualizationService::ICanvasObject;
            using Edges = Canvas_t::AxisRanges;
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
                /** @brief Add the primitives of the legend to the Canvas */
                void fillPrimitives(Canvas_t& canvas);
                std::unique_ptr<TLegend> legend{};
                using LegendEntry_t = std::unordered_map<int, PrimitivePtr_t>;
                LegendEntry_t markers{};
                LegendEntry_t colors{};
            };

            /** @brief Enumeration to toggle whether the segment shall displayed on the barrel/endcap */
            using Location = MuonR4::MsTrackSeed::Location;
            /** @brief Enumeration to toggle whether the seeds shall be painted in x-y or R-z view */
            enum class DisplayView{RZ, XY};
            /** @brief Actual implementation of the display seeds method with the augmentation to define the view
            *  @param ctx: EventContext to access store gate & conditions
            *  @param seederObj: Configured instance of the track seeder which actually constructed the seeds.
            *  @param view: Actual view to display
            *  @param segments: Container of all MS segments in the event
            *  @param seeds: The constructed track seeds from the event
            *  @param extPrimitives: Extra TObjects that should be additionally painted onto the Canvases */
            void displaySeeds(const EventContext& ctx,
                              const MuonR4::MsTrackSeeder& seederObj,
                              const DisplayView view,
                              const xAOD::MuonSegmentContainer& segments,
                              const MuonR4::MsTrackSeedContainer& seeds,
                              PrimitivesVec_t&& extPrimitives) const;
            /** @brief Add the truth segments to the canvas
             *  @param ctx: EventContext to access store gate
             *  @param seederObj: Configured instance of the track seeder which actually constructed the seeds.
             *  @param view: Actual view to display
             *  @param legend: Reference to the legend object to add the marker / truth legends
             *  @param canvas: Reference to the canvas to which the drawn markers are appended to */
            void fillTruthSeedPoints(const EventContext& ctx,
                                     const MuonR4::MsTrackSeeder& seeder,
                                     const DisplayView view,
                                     PlotLegend& legend, 
                                     Canvas_t& canvas) const;
            /** @brief Transforms the projected vector into the actual view, if it's xy then 
             *         an external phi is needed to place the marker. 
             *  @param phi: Angle for the xy view - taken from the segment direction
             *  @param posOnCylinder: Projection of the segment's position onto the seed cylinder
             *  @param view: Actual view to display */
            static Amg::Vector2D viewVector(const double phi,
                                            const Amg::Vector2D& posOnCylinder,
                                            const DisplayView view);
            /** @brief Track extrapolation tool */
            ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool" ,"" };
            /** @brief Service handle of the visualization service */
            ServiceHandle<IRootVisualizationService> m_visualSvc{this, "VisualSvc", "MuonValR4::RootVisualizationService"};
            /** @brief Token to present to the visualization service such that the display froms this 
             *          tool are grouped together */
            IRootVisualizationService::ClientToken m_clientToken{};
            /** @brief Key to the truth segment selection to draw the segment parameters */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_truthSegKey{this, "TruthSegkey", "MuonTruthSegments"};
            /** @brief Dependency on the geometry alignment */
            SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** @brief Maximum canvases to draw */
            Gaudi::Property<unsigned> m_canvasLimit{this, "CanvasLimit", 5000};
            /** @brief If set to true each canvas is saved into a dedicated pdf file */ 
            Gaudi::Property<bool> m_saveSinglePDFs{this, "saveSinglePDFs", true};
            /** @brief If set to true a summary Canvas is created. */
            Gaudi::Property<bool> m_saveSummaryPDF{this, "saveSummaryPDF",  true};
            /** @brief Define the subdirectory in which the plots shall be saved */
            Gaudi::Property<std::string> m_subDir{this, "outSubDir", ""};
            /** @brief Prefix of the individual canvas file names <MANDATORY>  */
            Gaudi::Property<std::string> m_canvasPrefix{this, "CanvasPreFix", "MsTrackValid"};
            /** @brief Flag toggling whether all Canvases have been exhausted */
            mutable std::atomic<bool> m_plotsDone{false};
            /** @brief How many obj have been produced */
            mutable std::atomic<unsigned> m_objCounter{0};
    };
}

#endif
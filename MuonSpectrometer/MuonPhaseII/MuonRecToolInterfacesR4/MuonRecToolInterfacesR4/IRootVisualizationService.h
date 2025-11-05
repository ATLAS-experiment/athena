/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONRECTOOLINTERFACESR4_IROOTVISUALIZATIONSERVICE_H
#define MUONRECTOOLINTERFACESR4_IROOTVISUALIZATIONSERVICE_H

#include <GaudiKernel/IService.h>


class TObject;
class EventContext;

namespace MuonValR4{
    /** @brief Definition of the IRootVisualizationService interface. The creation of plots
     *         using TCanvas is inheritly thread-hostile. The objects to be drawn mix on the
     *         various TCanvas throughout the job. This service enables the creation in MT jobs.
     *         It provides a container, the ICanvasObject, where the clients can add the objects to
     *         be put into a single plot. Either at the finalization stage or when the maximum number
     *         of created plots is reached, the service turns the provided ICanvasObject containers 
     *         into plots. 
     * 
     *         To disentangle the multiple clients, the ClientToken needs to be registered in the
     *         initialization stage of the client and then presented everytime when a new empty
     *         canvas is queried. If the pool of plots is exhausted a nullptr is returned from then on. */
    class IRootVisualizationService: virtual public IService {
        public:
            /** @brief Abrivation for a TObject to be drawn on a canvas */
            using PrimitivePtr_t = std::unique_ptr<TObject>;
            /** @brief List of all primitives */
            using PrimitiveVec_t = std::vector<PrimitivePtr_t>;
 
            DeclareInterfaceID(MuonValR4::IRootVisualizationService,1, 0);
            /** @brief Default destructor */
            virtual ~IRootVisualizationService() = default;
            /** @brief Token class to identify a particular visualization client. */
            struct ClientToken{
                /** @brief In which formats are the Canvases saved (pdf, png, C, ROOT, etc.) */
                std::set<std::string> fileFormats{"pdf"};
                /** @brief Prefix name of the saved Canvas. Serves as identifier
                 *         to the service and must be unque aceoss all clients */
                std::string preFixName{};
                /** @brief Subdirectory in which the plots are written */
                std::string subDirectory{};
                /** @brief How many canvases are drawn at maximum in a job */
                std::size_t canvasLimit{5000};
                /** @brief Save single plots */
                bool saveSinglePlots{true};
                /** @brief Save a summary pdf */
                bool saveSummaryPlot{true};
                /** @brief Comparison operator */
                bool operator<(const ClientToken& other) const {
                    return preFixName < other.preFixName;
                }
            };
            /** @brief Registers a new client to the Service. It needs to be 
             *         called during the initialization stage
             *  @param token: Token to uniquely identify the client */
            virtual StatusCode registerClient(const ClientToken& token) = 0;
            /** @brief Interface to the container class to temporarily cache the ROOT objects
             *         to be drawn on a TCanvas and then saved to disk*/
            struct ICanvasObject{
                public:
                    /** @brief Default destructor */
                    ~ICanvasObject() = default;
                    /** @brief Enum to select the corner coordinates shown by the plot */
                    enum class AxisRanges: std::uint8_t{
                        xLow, xHigh, yLow, yHigh
                    };
                    /** @brief Retrieves a corner coordinate of the drawn canvas.
                     *  @param r: Coordinate to return */
                    virtual double corner(const AxisRanges r) const = 0;
                    /** @brief Expands the axes of the pad such that the coordinates are guaranteed
                     *         to appear at least at the Canvas edges
                     *  @param x: Expansion in the width of the canvas
                     *  @param y: Expansion in the height of the canvas */
                    virtual void expandPad(const double x, const double y) = 0;
                    /** @brief To ensure that the drawn objects are not cut by the axis limits,
                     *         a flat scale-factor on the drawn axis intervals can be applied
                     * @param s: scale-factor >1.
                     * @param quadCan: Switch toggling whether the drawn intervals have
                     *                 an equal size */
                    virtual void setRangeScale(const double s, bool quadCan = true) = 0;
                    /** @brief Add a TObject to the ICanvasObject for later drawing onto
                     *         a TCanvas
                     *  @param drawMe: Unique_ptr to the object to be drawn
                     *  @param drawOpt: Option to be parsed to the draw command later
                     *                  (e.g. HIST) */
                    virtual void add(PrimitivePtr_t&& drawMe,
                                    const std::string& drawOpt="") = 0;
                    /** @brief Add a vector of TObjects to the ICanvasObject for later drawing
                     *         onto a TCanvas
                     *  @param drawMe: List of unique TObject pointers */
                    virtual void add(PrimitiveVec_t&& drawMe) = 0;
                    /** @brief Define the titles of the Canvas axes
                     *  @param xTitle: Title pf the x-axis
                     *  @param yTitle: Title pf the y-axis
                     *  @param zTitle: Title pf the z-axis */
                    virtual void setAxisTitles(const std::string& xTitle="",
                                               const std::string& yTitle="",
                                               const std::string& zTitle="") = 0;
                    /** @brief If no object has been drawn mark the plot as junk */
                    virtual void trash() = 0;
            };
            using CanvasPtr_t = std::shared_ptr<ICanvasObject>; 
            /** @brief Prepares a new ICanvasObject to be filled with content by the client.
             *         If the number of canvases returned exceeds the limit a nullptr is returned.
             *  @param ctx: Reference to the current EventContext to read the event meta data from
             *  @param token: ID token to associate the Canvas to the client's output
             *  @param canvasName: Name that's appended to the plot's out file name */
            virtual CanvasPtr_t prepareCanvas(const EventContext& ctx, 
                                              const ClientToken& token,
                                              const std::string& canvasName) = 0;
    };
}
#endif

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONVISUALIZATIONHELPERSR4_ROOVISUALIZATIONSERVICE_H
#define MUONVISUALIZATIONHELPERSR4_ROOVISUALIZATIONSERVICE_H
#include <MuonRecToolInterfacesR4/IRootVisualizationService.h>
#include <GaudiKernel/EventContext.h>
#include <AthenaBaseComps/AthService.h>

#include "DrawCanvasObject.h"

#include <TFile.h>

#include <mutex>

namespace MuonValR4{
    /** @brief Implementation of the IRootVisualization service.  */
    class RootVisualizationService: public extends<AthService, IRootVisualizationService>{
        public:
            using base_class::base_class;


            virtual StatusCode registerClient(const ClientToken& token) override final;

            virtual std::shared_ptr<ICanvasObject> 
                    prepareCanvas(const EventContext& ctx, const ClientToken& token,
                                  const std::string& canvasName) override final;
            virtual StatusCode finalize() override final;
            virtual StatusCode initialize() override final;

            using PlotPtr_t = std::shared_ptr<detail::DrawCanvasObject>;
            using PlotVec_t = std::vector<PlotPtr_t>;

        private:
            void paintObjects(const ClientToken& token,
                              PlotVec_t&& toDraw);

            /** @brief Extra safety margin to zoom out from the Canvas */
            Gaudi::Property<double> m_canvasExtraScale{this, "CanvasExtraScale" , 1.5};
            /** @brief Ensure that the canvas has the same interval sizes in x & y */
            Gaudi::Property<bool> m_quadCanvas{this, "QuadraticCanas", true};
            /** @brief Width of all drawn Canvases */
            Gaudi::Property<unsigned> m_canvasWidth{this, "CanvasWidth", 800};
            /** @brief  Height of all drawn Canvases */
            Gaudi::Property<unsigned> m_canvasHeight{this, "CanvasHeight", 600};
            /** @brief Directory into which all plots are written to  */
            Gaudi::Property<std::string> m_outDir{this, "outputDir" , "./Displays/"};
            /** @brief Name of the ROOT file into which  the output Canvases are written 
             *         (needs root to be in the list of outputFormats of the client tokens) */
            Gaudi::Property<std::string> m_outRootFileName{this, "outputROOTFile", "AllDisplays.root"};
  
            /** @brief Helper struct to group all plots that are belonging to a client
             *         of the service */
            struct PlotsPerClient{
                /** @brief List of already registered Canvases */
                PlotVec_t toDraw{};
                /** @brief Flag to indicate whether the plots were 
                 *         already dumped on disk. */
                bool elementsDrawn{false};
            };

            using StorageMap_t = std::map<ClientToken, PlotsPerClient>;
            StorageMap_t m_storage{};
            /** @brief File into which all Canvases are saved if root is defined as extension */
            std::unique_ptr<TFile> m_outFile{};

            std::mutex m_storageMutex{};
            std::mutex m_canvasMutex{};


    };
}
#endif
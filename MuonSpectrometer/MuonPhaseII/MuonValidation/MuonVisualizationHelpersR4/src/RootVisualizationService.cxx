/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "RootVisualizationService.h"

#include "MuonVisualizationHelpersR4/VisualizationHelpers.h"
#include "MuonVisualizationHelpersR4/FileHelpers.h"

#include "GeoModelKernel/throwExcept.h"

#include <TCanvas.h>
#include <TH2F.h>
#include <TROOT.h>
#include <TStyle.h>

#include <format>
#include <thread>
#include <filesystem>


namespace{
    using PlotVec_t = MuonValR4::RootVisualizationService::PlotVec_t;
    std::size_t cleanTrashed(PlotVec_t& toDraw) {
        toDraw.erase(std::remove_if(toDraw.begin(), toDraw.end(),
                                    [](const auto& plot){
                                        return plot->trashed();
                                    }), toDraw.end());
        return toDraw.size();
    }
}

namespace MuonValR4{
    using ICanvasObject = RootVisualizationService::ICanvasObject;
    StatusCode RootVisualizationService::initialize() {
        gROOT->SetStyle("ATLAS");
        TStyle* plotStyle = gROOT->GetStyle("ATLAS");
        plotStyle->SetOptTitle(0);
        plotStyle->SetHistLineWidth(1.);
        plotStyle->SetPalette(kViridis);

        return StatusCode::SUCCESS;
    }
    StatusCode RootVisualizationService::registerClient(const ClientToken& token) {
        if (token.preFixName.empty()){
            ATH_MSG_FATAL("Prefix name must not be empty");
            return StatusCode::FAILURE;
        }
        std::unique_lock guard{m_storageMutex};
        auto insert_itr = m_storage.insert(std::make_pair(token, PlotsPerClient{}));
        if (!insert_itr.second) {
            ATH_MSG_FATAL("The token "<<token.preFixName<<" is already registered");
            return StatusCode::FAILURE;
        }
        ATH_MSG_INFO("Registered new client "<<token.preFixName
                    <<", maximum number of plots "<<token.canvasLimit
                    <<", formats: "<<token.fileFormats<<", save single: "
                    <<(token.saveSinglePlots ? "yay" : "nay")
                   <<", save summary: "<<(token.saveSummaryPlot ? "yay" : "nay"));
        return StatusCode::SUCCESS;
    }

    std::shared_ptr<ICanvasObject> 
    RootVisualizationService::prepareCanvas(const EventContext& ctx, const ClientToken& token,
                                            const std::string& canvasName){
        if (canvasName.empty()) {
            THROW_EXCEPTION("The canvas name must not be empty");
        }
        std::unique_lock lock_guard{m_storageMutex};
        StorageMap_t::iterator store_itr = m_storage.find(token);
        if (store_itr == m_storage.end()) {
            THROW_EXCEPTION("The token "<<token.preFixName<<" is unknown.");
        }
        PlotsPerClient& dataHolder = store_itr->second;
        if (!dataHolder.elementsDrawn &&
            cleanTrashed(dataHolder.toDraw) < store_itr->first.canvasLimit){
            const std::size_t evt = ctx.eventID().event_number();
            ATH_MSG_VERBOSE("Provide new canvas "<<canvasName<<" for stream "<<token.preFixName
                            <<" in "<<ctx.eventID());
            auto newCanvas = dataHolder.toDraw.emplace_back(std::make_shared<detail::DrawCanvasObject>(canvasName, evt));
            newCanvas->setRangeScale(m_canvasExtraScale, m_quadCanvas);
            return newCanvas;
        } else if (!dataHolder.elementsDrawn) {
            paintObjects(store_itr->first, std::move(dataHolder.toDraw));
            dataHolder.elementsDrawn = true;
        }
        ATH_MSG_VERBOSE("Maximum elements for "<<token.preFixName
                        <<" reached. Don't provide any new canvas");
        return nullptr;
    }
    void RootVisualizationService::paintObjects(const ClientToken& token,
                                                PlotVec_t&& toDraw) {
        if (toDraw.empty()){
            return;
        }
        /// First sort the plots by the event number
        std::ranges::stable_sort(toDraw, [](const PlotPtr_t& a, const PlotPtr_t& b){
            return a->event() < b->event();
        });
        std::unique_lock guard{m_canvasMutex};
        std::unique_ptr<TCanvas> summaryCan{};
        const std::string summaryPdfName =  std::format("{:}/All{}.pdf", 
                                                        m_outDir.value(), token.preFixName);
        if (token.saveSummaryPlot) {
            ATH_MSG_DEBUG("Open "<<summaryPdfName<<" to dump all canvases in a common file");
            ensureDirectory(summaryPdfName);
            summaryCan = std::make_unique<TCanvas>("allCan","allCan", m_canvasWidth, m_canvasHeight);
            summaryCan->SaveAs(std::format("{:}[", summaryPdfName).c_str());
        }
        if (token.fileFormats.count("root") && !m_outFile) {
            std::string outFile = std::format("{:}/{:}", m_outDir.value(), 
                                              m_outRootFileName.value());
            ensureDirectory(m_outDir);
            m_outFile.reset(TFile::Open(outFile.c_str(), "RECREATE"));
            if (!m_outFile || m_outFile->IsZombie()) {
                THROW_EXCEPTION("Failed to create "<<outFile<<".");
            }
            ATH_MSG_DEBUG("Open "<<outFile<<" to save the plots in root format");
        }
        std::size_t currEvt{toDraw.back()->event()}, plotCount{0};
        for (const PlotPtr_t& drawMe : toDraw) {
            /// The client still holds the pointer to the plot
            while (!drawMe.unique()) {
                using namespace std::chrono_literals;
                ATH_MSG_DEBUG("Wait until "<<drawMe->name()<<" is finished.");
                std::this_thread::sleep_for(10ms);
            }
            /// Number the plots per event
            if (currEvt != drawMe->event()) {
                currEvt = drawMe->event();
                plotCount = 0;
            }
            std::string plotName = std::format("{:}/{:}/{:}_{:}_{:}_{:}", m_outDir.value(),
                                                token.subDirectory, token.preFixName,
                                                drawMe->event(), ++plotCount,
                                                removeNonAlphaNum(drawMe->name()));
                      
            auto singleCan = std::make_unique<TCanvas>("can", "can" , m_canvasWidth, m_canvasHeight);
            singleCan->cd();
            /// Setup the frame
            using enum ICanvasObject::AxisRanges;
            ATH_MSG_VERBOSE("Crate new canvas: "<<plotName<<" ["<<drawMe->corner(xLow)<<";"<<drawMe->corner(xHigh)
                        <<"], ["<<drawMe->corner(yLow)<<";"<<drawMe->corner(yHigh)<<"].");
            auto frameH = std::make_unique<TH2F>("frameH", 
                    std::format("frame;{:};{:};{:}", drawMe->xTitle(), drawMe->yTitle(), drawMe->zTitle()).c_str(),
                    1, drawMe->corner(xLow), drawMe->corner(xHigh), 
                    1, drawMe->corner(yLow), drawMe->corner(yHigh));
            frameH->Draw("AXIS");
            drawMe->add(drawAtlasLabel(0.65, 0.26, m_AtlasLabel));
            drawMe->add(drawLumiSqrtS(0.65,0.21, m_sqrtSLabel, m_lumiLabel));
            /// Draw the primitives
            for (auto& [primitive, opt] : drawMe->primitives()) {
                primitive->Draw(opt.c_str());
            }
            /// Save the single plots
            if (token.saveSinglePlots){
                for (const std::string& fileExt : token.fileFormats) {
                    if (fileExt != "root") {
                        ensureDirectory(plotName);
                        singleCan->SaveAs(std::format("{:}.{:}", plotName, fileExt).c_str());
                    } 
                }
            }
            if (token.fileFormats.count("root")) {
                TDirectory* writeTo = m_outFile.get();
                if (!token.subDirectory.empty()) {
                    writeTo = m_outFile->GetDirectory(token.subDirectory.c_str());
                    if (!writeTo) {
                        writeTo = m_outFile->mkdir(token.subDirectory.c_str());
                    }
                }
                writeTo->WriteObject(singleCan.get(),
                        std::format("{:}_{:}_{:}_{:}",token.preFixName,
                                    drawMe->event(), plotCount,
                                    removeNonAlphaNum(drawMe->name())).c_str());
            }
            if (summaryCan) {
                singleCan->SaveAs(summaryPdfName.c_str());
            }
        }
        if (summaryCan) {
            summaryCan->SaveAs(std::format("{:}]", summaryPdfName).c_str());
        }
    }
    StatusCode RootVisualizationService::finalize() {
        ATH_MSG_DEBUG("Finalize the visualization service. Dump all canvases that not have yet been drawn");
        for (auto& [token, dataHolder] : m_storage) {
            if (!dataHolder.elementsDrawn) {
                paintObjects(token, std::move(dataHolder.toDraw));
            }
            dataHolder.toDraw.clear();
        }
        m_outFile.reset();
        return StatusCode::SUCCESS;
    }
}
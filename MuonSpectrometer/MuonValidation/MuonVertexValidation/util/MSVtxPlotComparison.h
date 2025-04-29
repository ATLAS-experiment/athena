/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MSVTXPLOTCOMPARISON_H
#define MSVTXPLOTCOMPARISON_H

#include <memory>
#include <vector>
#include <array>
#include <string>
#include <tuple>

#include "TTree.h"
#include "TFile.h"
#include "TSystem.h"

#include "TClass.h"
#include "TKey.h"

#include "TROOT.h"
#include "TStyle.h"

#include "TCanvas.h"
#include "TH1.h"
#include "THStack.h"
#include "TEfficiency.h"
#include "TGraphAsymmErrors.h"
#include "TMultiGraph.h"
#include "TAxis.h"

#include "TString.h"
#include "TLegend.h"
#include "TColor.h"


class MSVtxPlotComparison {
    public:
        MSVtxPlotComparison(const std::vector<std::string> &datapaths, const std::vector<std::string> &labels, const std::string &pltdir);
        virtual ~MSVtxPlotComparison();
        void makeComparison();
    private:
        // struct to contain the plot and additional information about it
        template <typename T>
        struct PlotInfo {
            std::unique_ptr<T> plot{nullptr};
            TLegend* legend{nullptr};
            double maxy{0.};
            TString xlabel{};
            const TString ylabel{};

            PlotInfo(std::unique_ptr<T> plot, TLegend* legend, double maxy, TString xlabel, const TString ylabel) 
                    : plot(std::move(plot)), legend(legend), maxy(maxy), xlabel(std::move(xlabel)), ylabel(std::move(ylabel)) {};
        };

        void setup();
        void setPlotStyle();
        // comparing TH1s
        void makeTH1Comparison(TKey *key);
        std::unique_ptr<MSVtxPlotComparison::PlotInfo<THStack>> getTHStackPlotInfo(const TH1* h);
        void drawTHStack(std::unique_ptr<PlotInfo<THStack>> &hstackInfo);
        void drawTHStackPlot(std::unique_ptr<PlotInfo<THStack>> &hstackInfo);
        void drawTHStackRatioPlot(std::unique_ptr<PlotInfo<THStack>> &hstackInfo);
        const TString getTHStackplotpath(const TString &name);
        // comparing TEfficiencies
        void makeTEfficiencyComparison(TKey *key);
        std::unique_ptr<MSVtxPlotComparison::PlotInfo<TMultiGraph>> getTMultigraphPlotInfo(TEfficiency *h);
        void drawTMultigraph(std::unique_ptr<PlotInfo<TMultiGraph>> &mgInfo);
        void drawTMultigraphPlot(std::unique_ptr<PlotInfo<TMultiGraph>> &mgInfo);
        void drawTMultigraphRatioPlot(std::unique_ptr<PlotInfo<TMultiGraph>> &mgInfo);
        const TString getTMultigraphplotpath(const TString &name);
        // ratio computation and drawing functions
        TGraphAsymmErrors* getRatio(const TH1* num, const TH1* denom);
        TGraphAsymmErrors* getRatio(const TGraphAsymmErrors* num, const TGraphAsymmErrors* denom);
        void drawRatio(TGraphAsymmErrors* ratio, TGraphAsymmErrors* denomErrNorm, const TString &xlabel, const TAxis* plotXaxis, const TAxis* plotYaxis, double axisRescaling);
        // helper functions
        Bool_t ignorePlot(TKey *key);
        Bool_t isTH1(const TClass *objectClass);
        Bool_t isTEfficiency(const TClass *objectClass);
        void makeSinglePad();
        Int_t getPointIdx(const TGraphAsymmErrors* graph, double x);
        TGraphAsymmErrors* getNormalisedGraph(const TGraphAsymmErrors* graph);
        
        // input/output variables
        std::vector<std::string> m_datapaths{};
        std::vector<std::string> m_labels{};
        bool m_makeRatioPlots{}; // flag showing if ratio plots are made
        TString m_plotdir{};
        TString m_plotdir_truthVtx{};
        TString m_plotdir_recoVtx{};
        TString m_plotdir_recoVtxHits{};
        TString m_plotdir_vtxResiduals{};
        TString m_plotdir_inputObjects{};
        TString m_plotdir_vtxEfficiency{};
        TString m_plotdir_vtxFakeRate{};
        std::unique_ptr<TFile> m_output_file{nullptr};
        std::unique_ptr<TCanvas> m_c{nullptr};

        // custom color and marker cycle
        // Accessible Color Schemes not available in ROOT v6.32. 
        // Once it is, can remove this and make m_colors static constexpr and removing leading "m_" from elements
        const Int_t m_kP10Blue = TColor::GetColor("#3f90da");
        const Int_t m_kP10Yellow = TColor::GetColor("#ffa90e");
        const Int_t m_kP10Red = TColor::GetColor("#bd1f01");
        const Int_t m_kP10Grey = TColor::GetColor("#832db6");
        const Int_t m_kP10Violet = TColor::GetColor("#a96b59"); 
        const Int_t m_kP10Brown = TColor::GetColor("#94a4a2");
        const Int_t m_kP10Orange = TColor::GetColor("#e76300");
        const Int_t m_kP10Green = TColor::GetColor("#b9ac70");
        const Int_t m_kP10Ash = TColor::GetColor("#717581");
        const Int_t m_kP10Cyan = TColor::GetColor("#92dadd");

        const std::array<Int_t, 10> m_colors{m_kP10Blue, m_kP10Yellow, m_kP10Red, m_kP10Grey, m_kP10Violet, m_kP10Brown, m_kP10Orange, m_kP10Green, m_kP10Ash, m_kP10Cyan};
        static constexpr std::array m_markers{kOpenCircle, kOpenSquare, kOpenTriangleUp, kOpenDiamond, kOpenStar, kOpenTriangleDown, kOpenDoubleDiamond, kOpenCrossX};
};

#endif // MSVTXPLOTCOMPARISON_H
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MSVtxPlotComparison.h"
#include "PlotAnnotations.h"

using namespace MuonVertexValidationMacroPlotAnnotations;

MSVtxPlotComparison::MSVtxPlotComparison(const std::vector<std::string> &datapaths, const std::vector<std::string> &labels, const std::string &pltdir) :
                                         m_datapaths(datapaths), m_labels(labels), m_makeRatioPlots(m_datapaths.size()==2),
                                         m_plotdir(pltdir), 
                                         m_plotdir_truthVtx(m_plotdir+"truthVtx/"), 
                                         m_plotdir_recoVtx(m_plotdir+"recoVtx/"), 
                                         m_plotdir_recoVtxHits(m_plotdir_recoVtx+"hits/"), 
                                         m_plotdir_vtxResiduals(m_plotdir+"vtxResiduals/"), 
                                         m_plotdir_inputObjects(m_plotdir+"inputObjects/"),
                                         m_plotdir_vtxEfficiency(m_plotdir+"vtxEfficiency/"),
                                         m_plotdir_vtxFakeRate(m_plotdir+"vtxFakeRate/")
                                         {}


MSVtxPlotComparison::~MSVtxPlotComparison() = default;


void MSVtxPlotComparison::makeComparison(){
    // Generates a series of comparison plots, stored in the output directory `m_plotdir` as a root file and a pdf figure.
    // When only two data paths are passed, a ratio plot is also drawn.

    setup();
    setPlotStyle();

    // get list of histogram keys from one of the input files
    std::unique_ptr<TFile> file_0(TFile::Open(TString(m_datapaths[0]+"Histograms.root"), "read"));
    TIter keyIter(file_0->GetListOfKeys());

    // loop over the keys and call the appropriate comparison maker
    TKey* key{nullptr};
    while ((key = (TKey*)keyIter())) {
        TClass *objectClass = gROOT->GetClass(key->GetClassName());
        if (ignorePlot(key)) continue;
        if (isTH1(objectClass)) makeTH1Comparison(key);
        if (isTEfficiency(objectClass)) makeTEfficiencyComparison(key);
    }

    m_output_file->Write();
    m_output_file->Close();

    return;
}


void MSVtxPlotComparison::setup(){
    // set up the output directories, the output root file, and canvas.
    gSystem->mkdir(m_plotdir, kTRUE);
    gSystem->mkdir(m_plotdir_truthVtx, kTRUE);
    gSystem->mkdir(m_plotdir_recoVtx, kTRUE);
    gSystem->mkdir(m_plotdir_recoVtxHits, kTRUE);
    gSystem->mkdir(m_plotdir_vtxResiduals, kTRUE);
    gSystem->mkdir(m_plotdir_inputObjects, kTRUE);
    gSystem->mkdir(m_plotdir_vtxEfficiency, kTRUE);
    gSystem->mkdir(m_plotdir_vtxFakeRate, kTRUE);
    m_c = std::make_unique<TCanvas>();
    m_output_file = std::make_unique<TFile>(m_plotdir+"Histograms.root", "recreate");

    return;
}


void MSVtxPlotComparison::setPlotStyle(){
    gROOT->SetStyle("ATLAS");
    TStyle* plotStyle = gROOT->GetStyle("ATLAS");
    plotStyle->SetOptTitle(0);
    plotStyle->SetHistLineWidth(1.);
    plotStyle->cd();
    gROOT->ForceStyle();

    return;
}


// Comparison functions for TH1 // 

void MSVtxPlotComparison::makeTH1Comparison(TKey *key){
    // Finds the name and axis labels for the passed key which is assumed to belong to a TH1 object
    // Then calls the TH1 comparison maker to produce the plots

    TH1 *h = static_cast<TH1*>(key->ReadObj());

    std::unique_ptr<PlotInfo<THStack>> hstackInfo = getTHStackPlotInfo(h);
    drawTHStack(hstackInfo);

    return; 
}


std::unique_ptr<MSVtxPlotComparison::PlotInfo<THStack>> MSVtxPlotComparison::getTHStackPlotInfo(const TH1* h){
    // Extracts the histogram with `name` form the Histograms.root files for each passed data path and adds them to a THStack object.

    const TString name(h->GetName());
    TString xlabel(h->GetXaxis()->GetTitle());
    TString ylabel(h->GetYaxis()->GetTitle());

    // create a named THStack and the legend
    auto hstack = std::make_unique<THStack>(name, name); 
    TLegend* legend = makeLegend();
    legend->SetNColumns(2);

    // loop on TH1 histograms and add them to the THStack and legend
    double maxy{0.};
    for (unsigned int i=0; i<m_datapaths.size(); ++i){
        // retrieve histogram and set drawing style
        std::unique_ptr<TFile> file(TFile::Open(TString(m_datapaths[i]+"Histograms.root"), "read"));
        TH1* pH = static_cast<TH1*>(file->Get(name));
        pH->SetDirectory(0); // histogram doesn't belong to any directory now to avoid clashing memory management with the smart pointer file 
        pH->SetLineColor(m_colors[i]); 
        pH->SetLineStyle(1); 
        // add to THStack and legend
        hstack->Add(pH);
        legend->AddEntry(h, m_labels[i].c_str(), "L");
        // update maximal y value
        maxy = pH->GetMaximum() > maxy ? pH->GetMaximum() : maxy;
    }

    return std::make_unique<PlotInfo<THStack>>(std::move(hstack), legend, maxy, xlabel, ylabel);
}


void MSVtxPlotComparison::drawTHStack(std::unique_ptr<PlotInfo<THStack>> &hstackInfo){
    // The results are saved to the output directory in the `m_output_file` and as a pdf figure.
    m_output_file->cd(); // needed here as previously histograms are loaded from input files
    if (m_makeRatioPlots) drawTHStackRatioPlot(hstackInfo);
    else drawTHStackPlot(hstackInfo);

    m_c->SaveAs(getTHStackplotpath(TString(hstackInfo->plot->GetName()))+".pdf");
    m_c->Clear();

    return;
}


void MSVtxPlotComparison::drawTHStackPlot(std::unique_ptr<PlotInfo<THStack>> &hstackInfo){
    // Draws the THStack plot to the currently active pad and writes it to the m_output_file root file.
    // adjust maximum for sufficient space for annotations and draw annotations

    // required to set a new pad for the full canvas in order for style settings to apply
    if (!m_makeRatioPlots) makeSinglePad();

    hstackInfo->plot->Draw("nostack");
    // adjust maximum for sufficient space for annotations and draw annotations
    const TString name(hstackInfo->plot->GetName());
    double maxy_factor{1.4};
// TODO: fix this
    // if (name.Contains("log")){
    //     gPad->SetLogy();
    //     maxy_factor = 4;
    // } 
    hstackInfo->plot->SetMaximum(maxy_factor*hstackInfo->maxy);
    // axis labels
    hstackInfo->plot->GetXaxis()->SetTitle(hstackInfo->xlabel);
    hstackInfo->plot->GetYaxis()->SetTitle(hstackInfo->ylabel);
    // annotations
    hstackInfo->legend->Draw();
    drawDetectorRegionLabel(name.Data());
    drawATLASlabel("Simulation Internal");

    hstackInfo->plot->Write();
// TODO: adjust this when the above is fixed
    // gPad->SetLogy(0); // reset log scale for next plots

    return; 
}


void MSVtxPlotComparison::drawTHStackRatioPlot(std::unique_ptr<PlotInfo<THStack>> &hstackInfo){
    // Creates two pads to draw the THStack plot and its ratio to. 
    // The ratio is formed as the second/first histograms in the THStack.

    // create pads
    double x1{0.}, x2{1.};
    double y1{0.3}, y2{1.};
    double gap{0.05};
    TPad* padPlot = new TPad("padPlot", "padPlot", x1, y1, x2, y2);
    TPad* padRatio = new TPad("padRatio", "padRatio", x1, 0., x2, y1-gap);
    double axisRescaling = (y2-y1)/(y1-gap);

    // draw THStack in upper pad
    padPlot->SetBottomMargin(0);
    padPlot->Draw();
    padPlot->cd();
    TString xlabel_original = hstackInfo->xlabel;
    hstackInfo->xlabel = TString("");
    drawTHStackPlot(hstackInfo);
    // remove first y axis label for the upper plot as it gets cut off
    hstackInfo->plot->GetYaxis()->ChangeLabel(1, -1, -1, -1, -1, -1, " ");
    // compute and draw the ratio plot
    m_c->cd();
    padRatio->SetTopMargin(0);
    padRatio->SetBottomMargin(0.45);
    padRatio->Draw();
    padRatio->cd();

    std::vector<TH1*> hists{};
    for(TObject *obj : *(hstackInfo->plot->GetHists())) hists.push_back((TH1*)obj);
    TGraphAsymmErrors* ratio = getRatio(hists[1], hists[0]);
    TGraphAsymmErrors* denomErrNorm = getNormalisedGraph(new TGraphAsymmErrors(hists[0]));
    drawRatio(ratio, denomErrNorm, xlabel_original, hstackInfo->plot->GetXaxis(), hstackInfo->plot->GetYaxis(), axisRescaling);

    return;
}


const TString MSVtxPlotComparison::getTHStackplotpath(const TString &name){
    // choose plot path based on the name
    TString plotpath{};
    if (name.Contains("truth")) plotpath = m_plotdir_truthVtx+name;
    // else if (name.Contains("reco")) plotpath = m_plotdir_recoVtx+name;
    else if (name.Contains("MDT") || name.Contains("RPC") || name.Contains("TGC")) plotpath = m_plotdir_recoVtxHits+name;
    else if (name.Contains("delta")) plotpath = m_plotdir_vtxResiduals+name;
    else if (name.Contains("obj")) plotpath = m_plotdir_inputObjects+name;
    else plotpath = m_plotdir_recoVtx+name;

    return plotpath;
}


// Comparison functions for TEfficiency // 

void MSVtxPlotComparison::makeTEfficiencyComparison(TKey *key){
    // Finds the name and axis labels for the passed key which is assumed to belong to a TEfficiency object
    // Then calls the TEfficiency comparison maker to produce the plots

    TEfficiency *h = static_cast<TEfficiency*>(key->ReadObj());

    std::unique_ptr<PlotInfo<TMultiGraph>> mgInfo = getTMultigraphPlotInfo(h);
    drawTMultigraph(mgInfo);

    return;
}


std::unique_ptr<MSVtxPlotComparison::PlotInfo<TMultiGraph>> MSVtxPlotComparison::getTMultigraphPlotInfo(TEfficiency *h){
    // Extracts the histogram with `name` form the Histograms.root files for each passed data path to draw and adds them to a TMultiGraph object.

    const TString name(h->GetName());
    h->Draw();
    gPad->Update();
    const TString xlabel(h->GetPaintedGraph()->GetXaxis()->GetTitle());
    const TString ylabel(h->GetPaintedGraph()->GetYaxis()->GetTitle());
    m_c->Clear(); // needed to clear the canvas after drawing the TEfficiency object

    // create a named multigraph and the legend
    auto mg = std::make_unique<TMultiGraph>(name, name);  
    TLegend* legend = makeLegend();
    legend->SetNColumns(2);

    // loop on histograms and add them to the multigraph and legend
    double maxy{0.};
    for (unsigned int i=0; i<m_datapaths.size(); ++i){
        // retrieve histogram and set drawing style
        std::unique_ptr<TFile> file(TFile::Open(TString(m_datapaths[i]+"Histograms.root"), "read"));
        TEfficiency* pH = static_cast<TEfficiency*>(file->Get(name));
        pH->SetLineColor(m_colors[i]); 
        pH->SetLineStyle(1); 
        pH->SetMarkerColor(m_colors[i]);
        pH->SetMarkerStyle(m_markers[i]);
        pH->SetMarkerSize(0.5);
        // add to multigraph and legend
        pH->Draw(); 
        gPad->Update();
        TGraphAsymmErrors* g = pH->GetPaintedGraph();
        mg->Add(g, "PEZ"); // draw markers and no vertical lines on the error bars
        legend->AddEntry(pH, m_labels[i].c_str(), "PEL"); // draw marker, vertical error bar and line in legend
        maxy = getMaxy(g, maxy);
        m_c->Clear(); // needed to clear the canvas after drawing the TEfficiency object
    }

    return std::make_unique<PlotInfo<TMultiGraph>>(std::move(mg), legend, maxy, xlabel, ylabel);
}


void MSVtxPlotComparison::drawTMultigraph(std::unique_ptr<PlotInfo<TMultiGraph>> &mgInfo){
    // The results are saved to the output directory in the `m_output_file` and as a pdf figure.
    m_output_file->cd();
    if (m_makeRatioPlots)  drawTMultigraphRatioPlot(mgInfo);
    else drawTMultigraphPlot(mgInfo);

    m_c->SaveAs(getTMultigraphplotpath(TString(mgInfo->plot->GetName()))+".pdf");
    m_c->Clear();
    return;
}


void MSVtxPlotComparison::drawTMultigraphPlot(std::unique_ptr<PlotInfo<TMultiGraph>> &mgInfo){
    // Draws the TMultiGraph plot to the currently active pad and writes it to the `m_output_file` root file.

    // required to set a new pad for the full canvas in order for style settings to apply
    if (!m_makeRatioPlots) makeSinglePad();

    mgInfo->plot->Draw("AP");
    // adjust maximum for sufficient space for annotations and draw annotations
    mgInfo->plot->GetYaxis()->SetRangeUser(0, 1.4*mgInfo->maxy);
    // axis labels
    mgInfo->plot->GetXaxis()->SetTitle(mgInfo->xlabel);
    mgInfo->plot->GetYaxis()->SetTitle(mgInfo->ylabel);
    // annotations
    mgInfo->legend->Draw();
    const TString name(mgInfo->plot->GetName());
    if (name.Contains("_Lxy_")) drawDetectorBoundaryLines("Lxy", 1.1*mgInfo->maxy);
    if (name.Contains("_z_")) drawDetectorBoundaryLines("z", 1.1*mgInfo->maxy);
    drawDetectorRegionLabel(name.Data());
    drawATLASlabel("Simulation Internal");

    mgInfo->plot->Write();

    return;
}


void MSVtxPlotComparison::drawTMultigraphRatioPlot(std::unique_ptr<PlotInfo<TMultiGraph>> &mgInfo){
    // Creates two pads to draw the TMultiGraph plot and its ratio to. 
    // The ratio is formed between the first and second graph in the TMultiGraph.

    // create pads
    double x1{0.}, x2{1.};
    double y1{0.3}, y2{1.};
    double gap{0.05};
    TPad* padPlot = new TPad("padPlot", "padPlot", x1, y1, x2, y2);
    TPad* padRatio = new TPad("padRatio", "padRatio", x1, 0., x2, y1-gap);
    double axisRescaling = (y2-y1)/(y1-gap);

    // draw TMultiGraph in upper pad
    padPlot->SetBottomMargin(0);
    padPlot->Draw();
    padPlot->cd();
    TString xlabel_original = mgInfo->xlabel;
    mgInfo->xlabel = TString("");
    drawTMultigraphPlot(mgInfo);
    // remove first y axis label for the upper plot as it gets cut off
    mgInfo->plot->GetYaxis()->ChangeLabel(1, -1, -1, -1, -1, -1, " ");

    // compute and draw the ratio plot
    m_c->cd();
    padRatio->SetTopMargin(0);
    padRatio->SetBottomMargin(0.45);
    padRatio->Draw();
    padRatio->cd();

    std::vector<TGraphAsymmErrors*> efficiencies;
    for(TObject *obj : *(mgInfo->plot->GetListOfGraphs())) efficiencies.push_back((TGraphAsymmErrors*)obj);
    TGraphAsymmErrors* ratio = getRatio(efficiencies[1], efficiencies[0]);
    TGraphAsymmErrors* denomErrNorm = getNormalisedGraph(efficiencies[0]);
    drawRatio(ratio, denomErrNorm, xlabel_original, mgInfo->plot->GetXaxis(), mgInfo->plot->GetYaxis(), axisRescaling);

    return;
}


const TString MSVtxPlotComparison::getTMultigraphplotpath(const TString &name){
    // choose plot path based on the name
    TString plotpath{};
    if (name.Contains("eff_")) plotpath = m_plotdir_vtxEfficiency+name;
    else if (name.Contains("frate_")) plotpath = m_plotdir_vtxFakeRate+name;
    else plotpath = m_plotdir_recoVtx+name;

    return plotpath;
}


// ratio computation and drawing functions //

TGraphAsymmErrors* MSVtxPlotComparison::getRatio(const TH1* num, const TH1* denom){
    // returns ratio of two TH1 histograms as a TGraphAsymmErrors object by treating the histograms as two Poisson means.
// TODO: suppress warnings from TGraphAsymmErrors::Divide
    TGraphAsymmErrors* ratio = new TGraphAsymmErrors(num, denom, "pois"); // prints warning when denom is zero, in which case no ratio is computed

    return ratio;
}


TGraphAsymmErrors* MSVtxPlotComparison::getRatio(const TGraphAsymmErrors* num, const TGraphAsymmErrors* denom){
    // returns ratio of two TEfficiency histograms as a TGraphAsymmErrors object with manual ratio and error computation.
    TGraphAsymmErrors* ratio = new TGraphAsymmErrors();

    // can only compute the ratio for points where the denominator is non-zero and denominator and numerator values exist
    for (Int_t i=0; i<denom->GetN(); ++i){
        if (denom->GetPointY(i) <= 0.0) continue;
        Int_t num_idx = getPointIdx(num, denom->GetPointX(i));
        if (num_idx < 0) continue;

        Double_t ratio_val = num->GetPointY(num_idx)/denom->GetPointY(i);
        Double_t error_low = num->GetErrorYlow(num_idx)/denom->GetPointY(i);
        Double_t error_up = num->GetErrorYhigh(num_idx)/denom->GetPointY(i);
 
        ratio->AddPoint(denom->GetPointX(i), ratio_val);
        ratio->SetPointError(ratio->GetN()-1, denom->GetErrorXlow(i), denom->GetErrorXhigh(i), error_low, error_up);
    }
    
    return ratio;
}


void MSVtxPlotComparison::drawRatio(TGraphAsymmErrors* ratio, TGraphAsymmErrors* denomErrNorm, const TString &xlabel, const TAxis* plotXaxis, const TAxis* plotYaxis, double axisRescaling){
    // Draw the ratio plot to the currently active pad.
    // The error on the ratio is assumed to be numeratorError/denominator while denominatorError/denominator is shows as a shaded band around a ratio of 1.

    TMultiGraph* mg = new TMultiGraph();

    // add ratio
    ratio->SetLineColor(m_colors[1]);
    ratio->SetMarkerColor(m_colors[1]);
    ratio->SetMarkerStyle(m_markers[1]);
    ratio->SetMarkerSize(0.5);
    mg->Add(ratio, "PEZ"); // draw markers and no vertical lines on the error bars
    // add error band for the denominator error 
    denomErrNorm->SetFillColorAlpha(m_colors[0], 0.3);
    denomErrNorm->SetFillStyle(1001);
    mg->Add(denomErrNorm, "2");

    mg->Draw("AP");

    // axis formatting: scale to pad size
    // import x axis attributes and limits from the original plot
    mg->GetXaxis()->ImportAttributes(plotXaxis);
    mg->GetXaxis()->SetLimits(plotXaxis->GetXmin(), plotXaxis->GetXmax());
    mg->GetXaxis()->SetTitle(xlabel);
    mg->GetXaxis()->SetTitleSize(axisRescaling*plotXaxis->GetTitleSize());
    mg->GetXaxis()->SetTickLength(axisRescaling*plotXaxis->GetTickLength());
    mg->GetXaxis()->SetLabelSize(axisRescaling*plotXaxis->GetLabelSize());

    mg->GetYaxis()->SetRangeUser(-0.2, 2.2);
    mg->GetYaxis()->SetTitle(TString::Format("#frac{%s}{%s}", m_labels[1].c_str(), m_labels[0].c_str()).Data()); 
    mg->GetYaxis()->SetTitleOffset(0.5);
    mg->GetYaxis()->SetTitleSize(axisRescaling*plotYaxis->GetTitleSize());
    mg->GetYaxis()->SetLabelSize(axisRescaling*plotYaxis->GetLabelSize());

    // draw horizontal lines
    const std::vector<double> vlines{1.};
    for (double height : vlines) {
        TLine* l = new TLine(mg->GetXaxis()->GetXmin(), height, mg->GetXaxis()->GetXmax(), height);
        l->SetLineColor(m_colors[8]);
        l->SetLineStyle(7);
        l->Draw();
    }
    
    return;
}


// helper functions //

Bool_t MSVtxPlotComparison::isTH1(const TClass *objectClass){
    // returns true if the object class is a TH1 object
    Bool_t isfromTH1 = objectClass->InheritsFrom("TH1");
    Bool_t isNotTH2 = !objectClass->InheritsFrom("TH2");
    Bool_t isNotTHStack = !objectClass->InheritsFrom("THStack");
    return isfromTH1 && isNotTH2 && isNotTHStack;
}


Bool_t MSVtxPlotComparison::isTEfficiency(const TClass *objectClass){
    // returns true if the object class is a TEfficiency object
    return objectClass->InheritsFrom("TEfficiency");
}


Bool_t MSVtxPlotComparison::ignorePlot(TKey *key){
    // Specifies conditions to ignore certain plots:
    // stacked plots for efficiency numerator, denominator
    TObject *h = key->ReadObj();
    const TString name(h->GetName());

    return name.Contains("Reco") || name.Contains("Truth") || name.Contains("stack");
}


void MSVtxPlotComparison::makeSinglePad(){
    // creates and moves to a single pad for the full canvas
    TPad *pad = new TPad("fullCanvas", "fullCanvas", 0, 0, 1, 1);
    pad->Draw();
    pad->cd();

    return;
}


Int_t MSVtxPlotComparison::getPointIdx(const TGraphAsymmErrors* graph, double x){
    // Returns the index of the point in graph at position x
    // if x is not contained in the graph, returns -1
    Double_t *xs = graph->GetX();
    for (Int_t i=0; i<graph->GetN(); ++i) if (std::abs(xs[i]-x) <= 1e-6) return i;
    return -1;
}


TGraphAsymmErrors* MSVtxPlotComparison::getNormalisedGraph(const TGraphAsymmErrors* graph){
    // returns a TGraphAsymmErrors object with unit y values and the y error normalized to the y value at the point.
    TGraphAsymmErrors* graphErrNorm = new TGraphAsymmErrors();
    for (Int_t i=0; i<graph->GetN(); ++i){
        if (graph->GetPointY(i) == 0) continue;
        graphErrNorm->AddPoint(graph->GetPointX(i), 1.);
        graphErrNorm->SetPointError(i, graph->GetErrorXlow(i), graph->GetErrorXhigh(i), graph->GetErrorYlow(i)/graph->GetPointY(i), graph->GetErrorYhigh(i)/graph->GetPointY(i));
        graphErrNorm->SetPoint(i, graph->GetPointX(i), 1.); // need to set point again to avoid ROOT overwriting the point value to (0,0)
    }

    return graphErrNorm;

}

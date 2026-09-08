/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/scripts/effCurvesVersionsEta.cxx
 *
 * @brief Time-association efficiency and misassignment vs |eta|, plus the
 * stacked prime-fraction breakdown.
 *
 * ROOT/cling macro for the HGTD validation chain -- INTERPRETED ONLY, never
 * compiled. It relies on cling's implicit ROOT headers and will not build as a
 * translation unit; do not add it to any CMake source glob. Migrated as-is from
 * atlas-hgtd/SimulationAndPerformance/hgtdanalysisathena (HGTD_Plots/).
 *
 * Run it as its own root process -- the two macros in this directory declare
 * identically named globals and cannot share one ROOT session:
 *
 *     root -l -b -q 'effCurvesVersionsEta.cxx("/path/to/my.cfg")'
 *
 * See README.md for the config keys.
 */

/// @cond -- ROOT macro, not part of the package API.

#include "effCurvesVersionsEta.h"

///////////////////////////////////////////////////////

void effCurvesVersionsEta(TString config_file_name = "") {
  if (config_file_name != "") {
    g_config_file_name = config_file_name;
  }
  TEnv env(g_config_file_name);

  g_input_file_path = env.GetValue("input_file_path_TTI", "default");
  g_output_plot_path = env.GetValue("output_plot_path", "default");
  g_dataset_name = env.GetValue("dataset_name", "default");
  g_dataset_description = env.GetValue("dataset_description", "My Sample");
  g_print_png = env.GetValue("print_png", false);
  g_do_zoom = env.GetValue("do_zoom", false);
  g_use_log_scale = env.GetValue("use_log_scale", false);
  g_work_status = env.GetValue("work_status", "default");
  g_time_acc_tool = env.GetValue("time_acc_tool", "HGTD_TrkTimePerformanceStudies.TrackTimeAccTool/");
  g_track_selection_tool = env.GetValue("track_selection_tool", "HGTD_TrkTimePerformanceStudies.AllTracksSelection/");

  g_file = TFile::Open(g_input_file_path, "READ");

  SetAtlasStyle();

//   preparePlottingDir(g_config_file_name);

  efficiencyAndMistagLinePrimeFractionGT50("|#eta|");
  efficiencyStackPlotAllCases("|#eta|");

  return;

  g_file->Close();

}

void efficiencyAndMistagLinePrimeFractionGT50(TString xlabel) {

  auto eff = (TEfficiency *)g_file->Get(g_track_selection_tool + g_time_acc_tool + "m_eff_vs_eta");
  if (eff == nullptr) {
    cout << "s_m_eff_vs_eta histogram not found" << endl;
    return;
  }
  TGraphAsymmErrors *graph_total = eff->CreateGraph();
  graph_total->SetMarkerStyle(21);
  graph_total->SetMarkerSize(1.);
  Color_t marker_color = kBlack;
  graph_total->SetMarkerColor(marker_color);
  graph_total->SetLineColor(marker_color);
  graph_total->SetLineWidth(4);

  auto eff2 = (TEfficiency *)g_file->Get(g_track_selection_tool + g_time_acc_tool + "m_eff_gt50pcprimes_vs_eta");
  if (eff2 == nullptr) {
    cout << "histogram m_eff_gt50pcprimes_vs_eta not found" << endl;
    return;
  }
  TGraphAsymmErrors *graph2 = eff2->CreateGraph();
  graph2->SetMarkerStyle(21);
  graph2->SetMarkerSize(1.);
  Color_t marker_color2 = kBlue;
  graph2->SetMarkerColor(marker_color2);
  graph2->SetLineColor(marker_color2);
  graph2->SetLineWidth(4);

  auto eff3 = (TEfficiency *)g_file->Get(g_track_selection_tool + g_time_acc_tool + "m_eff_gt50pcprimes_vs_eta_mistag");
  if (eff3 == nullptr) {
    cout << "s_m_eff_gt50pcprimes_vs__mistag histogram not found" << endl;
    return;
  }
  TGraphAsymmErrors *graph3 = eff3->CreateGraph();
  graph3->SetMarkerStyle(21);
  graph3->SetMarkerSize(1.);
  Color_t marker_color3 = kRed;
  graph3->SetMarkerColor(marker_color3);
  graph3->SetLineColor(marker_color3);
  graph3->SetLineWidth(4);

  TCanvas *canvas = new TCanvas();

  auto hist =
      new TH1F("axishist", Form(";%s; Time Association Rate", xlabel.Data()),
               10, 2.4, 4.0);
  hist->SetAxisRange(0.001, 1.4, "Y");
  if (g_config_file_name == "muon10_mu0.cfg" or
      g_config_file_name == "pion0p1to5p0_mu0.cfg") {
    hist->SetAxisRange(0.001, 1.75, "Y");
  }
  hist->Draw();

  Color_t text_color = kBlack;
  atlas::ATLAS_LABEL(0.19, 0.89, text_color);
  atlas::myText(0.31, 0.89, text_color, g_work_status.Data());

  graph2->Draw("same P");
  graph3->Draw("same P");

  float label_text_size = 0.05;

  // TString dataset_descr = "VBF H #rightarrow invisible, #LT#mu#GT=200 ";
  // if (g_config_file_name == "muon10_mu0.cfg") {
  //   dataset_descr = "#mu^{+}, #it{p}_{#it{T}} = 10GeV, #LT#mu#GT=0 ";
  // } else if (g_config_file_name == "pion0p1to5p0_mu0.cfg") {
  //   dataset_descr = "#pi^{+}, 0.1 < #it{p}_{#it{T}} < 5 GeV, #LT#mu#GT=0 ";
  // } else if (g_config_file_name == "ttbar_mu200.cfg") {
  //   dataset_descr = "t#bar{t}, #LT#mu#GT=200";
  // }
  atlas::myText(0.19, 0.84, text_color, g_dataset_description.Data(), label_text_size);

  atlas::myText(0.19, 0.77, text_color, "Timing scenario \"Initial\"",
                label_text_size);

  auto legend = new TLegend(0.62, 0.7, 0.9, 0.92);
  legend->SetFillStyle(0);
  legend->SetTextFont(42);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.04);
  legend->AddEntry(graph2, "#splitline{Correctly}{reconstructed}", "lp");
  legend->AddEntry(graph3, "Misassignment", "lp");

  legend->Draw("same");

  gPad->RedrawAxis();

  TString plot_name =
      Form("%s/efficiencyAndMistagLinePrimeFractionGT50Vseta_%s.pdf",
           g_output_plot_path.Data(), g_dataset_name.Data());
  canvas->Print(plot_name);
  plot_name.ReplaceAll(".pdf", ".png");
  if (g_print_png)
    canvas->Print(plot_name);

  plot_name.ReplaceAll(".png", ".C");
  canvas->Print(plot_name);
}

void efficiencyStackPlotAllCases(TString xlabel) {

  auto eff = (TEfficiency *)g_file->Get(g_track_selection_tool + g_time_acc_tool + "m_eff_vs_eta");
  if (eff == nullptr) {
    cout << "s_m_eff_vs_eta histogram not found" << endl;
    return;
  }
  TGraphAsymmErrors *graph_total = eff->CreateGraph();
  graph_total->SetMarkerStyle(21);
  graph_total->SetMarkerSize(0);
  Color_t marker_color = kBlack;
  graph_total->SetMarkerColor(marker_color);
  graph_total->SetLineColor(marker_color);
  graph_total->SetLineWidth(2);

  std::vector<TEfficiency *> effs;
  std::vector<TH1F *> hists;

  for (int i = 0; i < primes_fractions.size(); i++) {
    TString plotname = Form("m_eff_vs_eta_primesfrac%s",
                             primes_fractions.at(i).Data());
    auto eff = (TEfficiency *)g_file->Get(g_track_selection_tool + g_time_acc_tool + plotname);
    if (eff == nullptr) {
      std::cout << "efficiencyStackPlotAllCases ERROR\n";
      std::cout << "could not find " << plotname << '\n';
      return;
    }
    eff->SetFillColor(colors.at(i));
    effs.push_back(eff);
  }

  THStack *stack_hist =
      new THStack("stack_hist", ";|#eta|; Time Association Rate");

  for (int i = 0; i < effs.size(); i++) {
    auto graph = effs.at(i)->CreateGraph();
    TH1F *hist =
        turnGraphIntoHist(graph, graph->GetName() + TString::Format("h%i", i),
                          "", 32, 2.4, 4.0, effs.at(i)->GetFillColor());
    hist->SetFillColor(colors.at(i));
    hists.push_back(hist);
    stack_hist->Add(hist);
  }

  TCanvas *canvas = new TCanvas();
  canvas->SetTicks(1, 1);

  auto hist =
      new TH1F("axishist", Form(";%s; Time Association Rate", xlabel.Data()),
               10, 2.4, 4.0);
  hist->SetAxisRange(0.001, 1.6, "Y");
  if (g_config_file_name == "muon10_mu0.cfg" or
      g_config_file_name == "pion0p1to5p0_mu0.cfg") {
    hist->SetAxisRange(0.001, 1.75, "Y");
  }
  hist->Draw();

  stack_hist->Draw("same");
  graph_total->Draw("same p");

  Color_t text_color = kBlack;
  atlas::ATLAS_LABEL(0.19, 0.89, text_color);
  atlas::myText(0.31, 0.89, text_color, g_work_status.Data());

  // TString dataset_descr = "VBF H #rightarrow invisible, #LT#mu#GT=200 ";
  //
  // if (g_config_file_name == "muon10_mu0.cfg") {
  //   dataset_descr = "#mu^{+}, #it{p}_{#it{T}} = 10GeV, #LT#mu#GT=0 ";
  // } else if (g_config_file_name == "pion0p1to5p0_mu0.cfg") {
  //   dataset_descr = "#pi^{+}, 0.1 < #it{p}_{#it{T}} < 5 GeV, #LT#mu#GT=0 ";
  // } else if (g_config_file_name == "ttbar_mu200.cfg") {
  //   dataset_descr = "t#bar{t}, #LT#mu#GT=200";
  // }

  float label_text_size = 0.05;

  atlas::myText(0.19, 0.84, text_color, g_dataset_description.Data(), label_text_size);

  atlas::myText(0.19, 0.77, text_color, "Timing scenario \"Initial\"",
                label_text_size);

  auto legend = new TLegend(0.65, 0.62, 0.85, 0.94);

  legend->SetTextFont(42);
  legend->SetFillStyle(0);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.035);

  legend->AddEntry(graph_total, "Total", "lp");
  for (int i = 0; i < labels.size(); i++) {
    legend->AddEntry(hists.at(i), labels.at(i), "f");
  }

  legend->Draw("same");

  gPad->RedrawAxis();

  TString plot_name = Form("%s/efficiencyStackPlotAllCases_eta_%s.pdf",
                           g_output_plot_path.Data(),
                           g_dataset_name.Data());
  canvas->Print(plot_name);
  plot_name.ReplaceAll(".pdf", ".png");
  if (g_print_png)
    canvas->Print(plot_name);

  plot_name.ReplaceAll(".png", ".C");
  canvas->Print(plot_name);
}

////////////////////////////////////////////////////////

TH1F* turnGraphIntoHist(TGraph *graph, TString name, TString label, int bins,
                        double xmin, double xmax, int color) {
  TH1F *hist = new TH1F(name, label, bins, xmin, xmax);
  for (int point = 0; point < graph->GetN(); point++) {
    double x, y;
    graph->GetPoint(point, x, y);
    int bin_n = hist->FindFixBin(x);
    hist->SetBinContent(bin_n, y);
  }
  hist->SetFillColor(color);
  hist->SetLineWidth(0);
  hist->SetMarkerSize(0);
  hist->SetMarkerColor(color);
  return hist;
}

/// @endcond

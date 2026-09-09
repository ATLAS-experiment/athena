/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/scripts/timeResSplitCases.cxx
 *
 * @brief Time-resolution outlier stack, split by prime-fraction case.
 *
 * ROOT/cling macro for the HGTD validation chain -- INTERPRETED ONLY, never
 * compiled. It relies on cling's implicit ROOT headers and will not build as a
 * translation unit; do not add it to any CMake source glob. Migrated as-is from
 * atlas-hgtd/SimulationAndPerformance/hgtdanalysisathena (HGTD_Plots/).
 *
 * Run it as its own root process -- the two macros in this directory declare
 * identically named globals and cannot share one ROOT session:
 *
 *     root -l -b -q 'timeResSplitCases.cxx("/path/to/my.cfg")'
 *
 * See README.md for the config keys.
 */

/// @cond -- ROOT macro, not part of the package API.

#include "timeResSplitCases.h"

TFile *file = nullptr;

void timeResSplitCases(TString config_file_name = "") {
  if (config_file_name != "") {
    g_config_file_name = config_file_name;
  }

  TEnv env(g_config_file_name);

  g_input_file_path = env.GetValue("input_file_path_TTI", "default");
  g_output_plot_path = env.GetValue("output_plot_path", "default");
  dataset_name = env.GetValue("dataset_name", "default");
  g_dataset_description = env.GetValue("dataset_description", "My Sample");
  print_png = env.GetValue("print_png", false);
  do_zoom = env.GetValue("do_zoom", false);
  g_use_log_scale = env.GetValue("use_log_scale", false);
  work_status = env.GetValue("work_status", "default");
  g_time_acc_tool = env.GetValue(
      "time_acc_tool",
      "HGTD_TrkTimePerformanceStudies.ExpertTrackTimeFromClusters/");
  g_track_selection_tool = env.GetValue(
      "track_selection_tool",
      "HGTD_TrkTimePerformanceStudies.AllTracksSelection/");

  file = TFile::Open(g_input_file_path, "READ");

  SetAtlasStyle();

  plot();

}

void plot() {

  std::vector<Color_t> colors = {
      kTeal + 4, kTeal + 2, kTeal, kTeal - 9, kMagenta, kRed, kRed, kRed, kRed};

  std::vector<TH1F *> hists;
  cout << g_track_selection_tool + g_time_acc_tool << endl;

  auto hist1 =
      (TH1F *)file->Get(g_track_selection_tool + g_time_acc_tool + "m_hist_timeres_outlier_casesAllPrimes");
  hists.push_back(hist1);

  auto hist2 = (TH1F *)file->Get(g_track_selection_tool + g_time_acc_tool + "m_hist_timeres_outlier_casesMoreThanHalfPrimes");
  hists.push_back(hist2);
  auto hist3 = (TH1F *)file->Get(g_track_selection_tool + g_time_acc_tool + "m_hist_timeres_outlier_casesHalfPrimesHasPrimes");
  hists.push_back(hist3);
  auto hist4 = (TH1F *)file->Get(g_track_selection_tool + g_time_acc_tool + "m_hist_timeres_outlier_casesLessThanHalfPrimes");
  hists.push_back(hist4);
  auto hist5 = (TH1F *)file->Get(g_track_selection_tool + g_time_acc_tool + "m_hist_timeres_outlier_casesNoPrimesNoPossiblePrimes");
  hists.push_back(hist5);
  auto hist6 = (TH1F *)file->Get(g_track_selection_tool + g_time_acc_tool + "m_hist_timeres_outlier_casesNoPrimes1PossiblePrimes");
  hists.push_back(hist6);
  auto hist7 = (TH1F *)file->Get(g_track_selection_tool + g_time_acc_tool + "m_hist_timeres_outlier_casesNoPrimes2PossiblePrimes");
  hists.push_back(hist7);
  auto hist8 = (TH1F *)file->Get(g_track_selection_tool + g_time_acc_tool + "m_hist_timeres_outlier_casesNoPrimes3PossiblePrimes");
  hists.push_back(hist8);
  auto hist9 = (TH1F *)file->Get(g_track_selection_tool + g_time_acc_tool + "m_hist_timeres_outlier_casesNoPrimes4PossiblePrimes");
  hists.push_back(hist9);

  double total = 0;


  auto overall_hist =
      new TH1F("overall_hist", ";t_{reco} - t_{truth} [ns]; number of tracks",
               200, -.4, 0.4);
  for (const auto &h : hists) {
    overall_hist->Add(h);
  }

  THStack *stack_hist =
      new THStack("stack_hist", ";t_{reco} - t_{truth} [ns]; Number of Tracks");

  int ntotal = 0;
  for (size_t i = 0; i < hists.size(); i++) {
    hists.at(i)->SetFillColor(colors.at(i));
    hists.at(i)->SetLineWidth(0.);
    ntotal += hists.at(i)->Integral(0, hists.at(i)->GetNbinsX() + 1);
    stack_hist->Add(hists.at(i));
  }

  stack_hist->SetMaximum(stack_hist->GetMaximum() * 1.7);

  if (do_zoom) {
    stack_hist->SetMaximum(100);
  }

  TCanvas *c = new TCanvas();
  c->SetTicks(1, 1);
  if (g_use_log_scale) {
    c->SetLogy();
  }

  auto hist = new TH1F(
      "axishist", ";t_{reco} - t_{truth} [ns]; Number of Tracks", 10, -0.4, 0.4);

  if (g_use_log_scale) {
    hist->SetAxisRange(1, 1.e8, "Y");
  } else {
    hist->SetAxisRange(0, 0.08, "Y");
  }

  hist->Draw();

  stack_hist->Draw("hist same");

  overall_hist->SetMarkerColor(kBlack);
  overall_hist->SetLineWidth(0);
  overall_hist->SetMarkerStyle(20);
  overall_hist->SetMarkerSize(0);

  c->Update();

  gPad->RedrawAxis();


  Color_t text_color = kBlack;
  atlas::ATLAS_LABEL(0.19, 0.88, text_color);
  atlas::myText(0.31, 0.88, text_color, work_status);

  float label_text_size = 0.05;

  atlas::myText(0.19, 0.83, text_color, g_dataset_description.Data(), label_text_size);
  atlas::myText(0.19, 0.77, text_color, "Timing scenario \"Initial\"",
                label_text_size);


  auto legend = new TLegend(0.62, 0.65, 0.9, 0.92);

  legend->SetTextFont(42);
  legend->SetBorderSize(0);
  legend->SetTextSize(0.035);

  for (int i = 0; i < labels.size(); i++) {
    legend->AddEntry(hists.at(i), labels.at(i), "f");
  }

  legend->Draw("same");

  TString plot_name =
      Form("%s/%s_%s.pdf", g_output_plot_path.Data(),
           dataset_name.ReplaceAll(" ", "").ReplaceAll(",", "_").Data(),
           descr.Data());
  if (g_use_log_scale) {
    plot_name.ReplaceAll(".pdf", "LogScale.pdf");
  }
  c->Print(plot_name);
  if (print_png)
    c->Print(plot_name.ReplaceAll(".pdf", ".png"));

  plot_name.ReplaceAll(".png", ".C");
  c->Print(plot_name);

}

/// @endcond

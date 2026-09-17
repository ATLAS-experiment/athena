/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/util/effCurvesVersionsEta.h
 *
 * @brief Declarations and configuration globals for effCurvesVersionsEta.cxx.
 *
 * Plotting step of the HGTD validation chain, built as the standalone
 * executable HGTD_effCurvesVersionsEta:
 *
 *     HGTD_effCurvesVersionsEta /path/to/my.cfg
 *
 * Migrated from atlas-hgtd/SimulationAndPerformance/hgtdanalysisathena
 * (HGTD_Plots/), where it was a ROOT macro. See README.md for the config keys.
 */

/// @cond -- plotting executable, not part of the package API.

#ifndef HGTD_ANALYSIS_EFFCURVESVERSIONSETA_H
#define HGTD_ANALYSIS_EFFCURVESVERSIONSETA_H

#include "HGTD_PlottingHelpers.h"

#include "TEfficiency.h"
#include "TFile.h"
#include "TString.h"

#include <vector>


TString g_input_file_path = "default";
TString g_output_plot_path = "default";
int g_skip = 0;
TString g_dataset_name = "default";
TString g_dataset_description = "default";
bool g_print_png = false;
TString g_work_status = "default";
bool g_use_log_scale = false;
bool g_do_zoom = false;
TString g_descr = "default";
TString g_time_acc_tool = "default";
TString g_track_selection_tool = "default";

const std::vector<Color_t> colors = {
    kTeal + 4, kTeal + 2, kTeal, kTeal - 9, kMagenta, kRed, kRed, kRed, kRed};
    
TString g_config_file_name = "VBFinv_mu200.cfg";

const std::vector<TString> primes_fractions = {"AllPrimes",
                                        "MoreThanHalfPrimes",
                                        "HalfPrimesHasPrimes",
                                        "LessThanHalfPrimes",
                                        "NoPrimesNoPossiblePrimes",
                                        "NoPrimes1PossiblePrimes",
                                        "NoPrimes2PossiblePrimes",
                                        "NoPrimes3PossiblePrimes",
                                        "NoPrimes4PossiblePrimes"};

const std::vector<TString> labels = {"Prime Frac. = 1",   "0.5 < Prime Frac. < 1",
                                 "Prime Frac. = 0.5", "0 < Prime Frac. < 0.5",
                                 "Misassignment",     "Confusion"};

TH1F* turnGraphIntoHist(TGraph *graph, TString name, TString label, int bins,
                        double xmin, double xmax, int color);

void efficiencyAndMistagLinePrimeFractionGT50(TFile* file, TString xlabel);

void efficiencyStackPlotAllCases(TFile* file, TString xlabel);

#endif  // HGTD_ANALYSIS_EFFCURVESVERSIONSETA_H

/// @endcond

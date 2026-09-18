/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/util/timeResSplitCases.h
 *
 * @brief Declarations and configuration globals for timeResSplitCases.cxx.
 *
 * Plotting step of the HGTD validation chain, built as the standalone
 * executable HGTD_timeResSplitCases:
 *
 *     HGTD_timeResSplitCases /path/to/my.cfg
 *
 * Migrated from atlas-hgtd/SimulationAndPerformance/hgtdanalysisathena
 * (HGTD_Plots/), where it was a ROOT macro. See README.md for the config keys.
 */

/// @cond -- plotting executable, not part of the package API.

#ifndef HGTD_ANALYSIS_TIMERESSPLITCASES_H
#define HGTD_ANALYSIS_TIMERESSPLITCASES_H

#include "HGTD_PlottingHelpers.h"

#include "TFile.h"
#include "TString.h"

#include <vector>

TString g_input_file_path = "default";
TString g_output_plot_path = "default";
TString dataset_name = "VBFinv, mu=200";
TString g_dataset_description = "";
bool print_png = true;
TString work_status = "Internal";
bool g_use_log_scale = false;
bool do_zoom = false;
TString descr = "OutlierStackPlotAllZ0_PF";
TString g_config_file_name = "VBFinv_mu200.cfg";
TString g_time_acc_tool = "default";
TString g_track_selection_tool = "default";

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

void plot(TFile* file);

#endif  // HGTD_ANALYSIS_TIMERESSPLITCASES_H

/// @endcond

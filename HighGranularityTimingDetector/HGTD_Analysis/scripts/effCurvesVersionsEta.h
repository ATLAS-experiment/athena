/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/scripts/effCurvesVersionsEta.h
 *
 * @brief Declarations and configuration globals for effCurvesVersionsEta.cxx.
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

#include "HGTD_PlottingHelpers.h"
// #include "preparePaths.cxx"


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

TFile *g_file = nullptr;

std::vector<Color_t> colors = {
    kTeal + 4, kTeal + 2, kTeal, kTeal - 9, kMagenta, kRed, kRed, kRed, kRed};
    
TString g_config_file_name = "VBFinv_mu200.cfg";

std::vector<TString> primes_fractions = {"AllPrimes",
                                        "MoreThanHalfPrimes",
                                        "HalfPrimesHasPrimes",
                                        "LessThanHalfPrimes",
                                        "NoPrimesNoPossiblePrimes",
                                        "NoPrimes1PossiblePrimes",
                                        "NoPrimes2PossiblePrimes",
                                        "NoPrimes3PossiblePrimes",
                                        "NoPrimes4PossiblePrimes"};

std::vector<TString> labels = {"Prime Frac. = 1",   "0.5 < Prime Frac. < 1",
                                 "Prime Frac. = 0.5", "0 < Prime Frac. < 0.5",
                                 "Misassignment",     "Confusion"};

TH1F* turnGraphIntoHist(TGraph *graph, TString name, TString label, int bins,
                        double xmin, double xmax, int color);

void efficiencyAndMistagLinePrimeFractionGT50(TString xlabel);

void efficiencyAndMistagLinePrimeFractionGT50(TString xlabel);

void efficiencyStackPlotAllCases(TString config_file_name);                                                                                         

/// @endcond

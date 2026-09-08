/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/scripts/timeResSplitCases.h
 *
 * @brief Declarations and configuration globals for timeResSplitCases.cxx.
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

#include "HGTD_PlottingHelpers.h"

// #include "preparePaths.cxx"

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

std::vector<TString> labels = {"Prime Frac. = 1",   "0.5 < Prime Frac. < 1",
                                "Prime Frac. = 0.5", "0 < Prime Frac. < 0.5",
                                "Misassignment",     "Confusion"};

void plot();

/// @endcond

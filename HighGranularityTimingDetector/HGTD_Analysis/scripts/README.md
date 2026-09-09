# HGTD_Analysis plotting macros

ROOT macros that turn the histogram file written by `HGTD_TrkTimePerformanceStudies`
(`HGTD_PerformanceStudies_output.root`, see `share/JO_Performance_Studies.py`) into
the HGTD track-time validation plots.

These are **interpreted**, not compiled. They are installed as package data
(`atlas_install_data` in the package `CMakeLists.txt`), so after a build they are
co-located in `<build>/<platform>/data/HGTD_Analysis/`.

| File | Produces |
|---|---|
| `effCurvesVersionsEta.cxx` | time-association efficiency and misassignment vs \|eta\|, and the stacked prime-fraction breakdown |
| `timeResSplitCases.cxx` | time-resolution outlier stack, split by prime-fraction case |
| `HGTD_PlottingHelpers.h` | ATLAS style + label helpers, shared by both |

## Running them

Each macro takes the path to a ROOT `TEnv` config file:

```sh
cd <build>/<platform>/data/HGTD_Analysis
root -l -b -q 'effCurvesVersionsEta.cxx("/path/to/my.cfg")'
root -l -b -q 'timeResSplitCases.cxx("/path/to/my.cfg")'
```

Run them from that directory — the `.h` files `#include "HGTD_PlottingHelpers.h"`
by relative path.

**Run them as separate `root` processes.** The two macro headers declare the same
global names (`g_input_file_path`, `labels`, …), so loading both into one ROOT
session will not work.

## Config keys

```
input_file_path_TTI:  /abs/path/HGTD_PerformanceStudies_output.root
output_plot_path:     /abs/path/to/output/dir
dataset_name:         ttbar                     # goes into the output file names
dataset_description:  ttbar, no pile-up         # drawn on the canvas
work_status:          Simulation Internal
print_png:            TRUE
use_log_scale:        TRUE
do_ratioplot:         TRUE
x_eff_min:            0.80
time_acc_tool:        HGTD_TrkTimePerformanceStudies.TrackTimeAccTool3/
track_selection_tool: HGTD_TrkTimePerformanceStudies.AllTracksSelection/
```

`time_acc_tool` and `track_selection_tool` are histogram *directory* names and must
match the instance names configured in
`python/HGTD_AnalysisConfig.py::HGTD_TrkTimePerformanceStudiesCfg` — histograms are
booked under `<TrackSelection>/<TimeAccTool>/<histname>`. Note the trailing `/`.

Each plot is written as `.pdf`, `.png` (if `print_png`) and `.C` (the ROOT canvas
dump). The `.C` files are plain text and diff cleanly, which makes them a good
regression check between runs.

## Two traps worth knowing

- **Use `TrackTimeAccTool3`, not `TrackTimeAccTool`.** `TrackTimeAccTool`'s
  `UseLastHitCut` reads `track.parameterX/Y/Z()` at `xAOD::LastMeasurement`, but the
  AOD writer strips `parameterX/Y/Z` and `parameterPosition` from
  `InDetTrackParticlesAux`. The cut then rejects every track and the efficiency curve
  is flat zero. `TrackTimeAccTool3` applies the same cut from the persisted
  `HGTD_summaryinfo` bitfield. Use `TrackTimeAccToolDefault` for no cleaning.
- **The AOD must be produced with `flags.Tracking.writeExtendedHGTDInfo=True`**
  (it defaults to `False`). Otherwise `HGTD_cluster_time`,
  `HGTD_cluster_truth_class`, `HGTD_primary_expected` and `HGTD_summaryinfo` are
  stripped from the AOD, the analysis job still succeeds, and every histogram comes
  out empty.

## Follow-up

These macros are carried over as-is from
`atlas-hgtd/SimulationAndPerformance/hgtdanalysisathena` to keep the plots
unchanged during the migration. Replacing them with a flake8-clean python script
(installed via `atlas_install_scripts( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )`,
the dominant Athena idiom) is a planned follow-up.

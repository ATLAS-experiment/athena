# HGTD_Analysis plotting executables

They turn the histogram file written by `HGTD_TrkTimePerformanceStudies`
(`HGTD_PerformanceStudies_output.root`, see `share/JO_Performance_Studies.py`) into the
HGTD track-time validation plots. Each takes one argument, the path to a ROOT `TEnv`
config file, and writes every plot as `.pdf`, `.png` (if `print_png`) and `.C`:

```sh
HGTD_effCurvesVersionsEta my.cfg   # time-association efficiency and misassignment vs |eta|,
                                   # plus the stacked prime-fraction breakdown
HGTD_timeResSplitCases    my.cfg   # time-resolution outlier stack, split by prime-fraction case
```

## Config file

```
input_file_path_TTI:  /abs/path/HGTD_PerformanceStudies_output.root
output_plot_path:     /abs/path/to/output/dir
dataset_name:         ttbar                     # goes into the output file names
dataset_description:  ttbar, no pile-up         # drawn on the canvas
work_status:          Simulation Internal
print_png:            TRUE
use_log_scale:        TRUE
do_zoom:              FALSE
do_ratioplot:         TRUE
x_eff_min:            0.80
time_acc_tool:        HGTD_TrkTimePerformanceStudies.ExpertTrackTimeFromSummary/
track_selection_tool: HGTD_TrkTimePerformanceStudies.AllTracksSelection/
```

`time_acc_tool` and `track_selection_tool` are histogram *directory* names, trailing `/`
included: histograms are booked under `<TrackSelection>/<TimeAccTool>/<histname>`, so these
must match the instance names configured in
`python/HGTD_AnalysisConfig.py::HGTD_TrkTimePerformanceStudiesCfg`.

## Two traps

- Use `ExpertTrackTimeFromSummary`, not `ExpertTrackTimeFromClusters`: the latter's
  `UseLastHitCut` reads `track.parameterX/Y/Z()` at `xAOD::LastMeasurement`, which the AOD
  writer strips, so the cut rejects every track and the efficiency curve is flat zero. Use
  `ExpertTrackTimeFromClustersNoSelection` for no cleaning.
- The AOD must be produced with `flags.Tracking.writeExtendedHGTDInfo=True` (it defaults to
  `False`), or the HGTD track decorations are stripped and every histogram comes out empty.

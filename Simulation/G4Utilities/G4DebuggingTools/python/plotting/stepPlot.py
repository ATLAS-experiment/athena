#!/usr/bin/env python3
"""
compare_volumes.py
Compare two G4Debugger “volumeSummary” histograms and plot their ratio. These histograms are
Created by G4DebuggingTools.G4DebuggingToolsConfig.StepHistogramToolCfg.

Example
-------
python compare_volumes.py \
    --base path_where_hisgrom\
    --in1 root_file_name_1 \
    --in2 root_file_name_2 \
    --atlas-style path_to_AtlasStyle \
    --out-dir ./plots
"""
import argparse
import os
import ROOT
from ROOT import gROOT

from G4Debugger import G4Debugger
from G4DebuggerUtils import plotSummaryRatio


# ---------------------------------------------------------------------------
def parse_args() -> argparse.Namespace:
    """Define and parse CLI arguments."""
    parser = argparse.ArgumentParser(
        description="Compare volume summaries from two G4Debugger runs."
    )

    parser.add_argument(
        "--base",
        required=True,
        help="Base directory containing the StepHistograms_* sub-directories.",
    )
    parser.add_argument(
        "--in1", required=True, metavar="SUBDIR_1",
        help="First StepHistograms_* sub-directory."
    )
    parser.add_argument(
        "--in2", required=True, metavar="SUBDIR_2",
        help="Second StepHistograms_* sub-directory."
    )
    parser.add_argument(
        "--atlas-style", required=True,
        help="Directory holding AtlasStyle.C / AtlasLabels.C / AtlasUtils.C."
    )
    parser.add_argument(
        "--out-dir", default=".",
        help="Where to write the output plot [default: current directory]."
    )
    parser.add_argument("--v1-label", default="Non Opt", help="Legend label for first sample.")
    parser.add_argument("--v2-label", default="Opt",      help="Legend label for second sample.")
    
    return parser.parse_args()


def set_atlas_style(style_dir: str) -> None:
    """Load ATLAS style macros if they exist."""
    if not os.path.isdir(style_dir):
        raise FileNotFoundError(f"AtlasStyle directory not found: {style_dir}")

    print(f"Loading ATLAS style from {style_dir}")
    gROOT.LoadMacro(os.path.join(style_dir, "AtlasStyle.C"))
    gROOT.LoadMacro(os.path.join(style_dir, "AtlasLabels.C"))
    gROOT.LoadMacro(os.path.join(style_dir, "AtlasUtils.C"))
    ROOT.SetAtlasStyle()


# ---------------------------------------------------------------------------
def main() -> None:
    args = parse_args()
    gROOT.SetBatch(True)
    set_atlas_style(args.atlas_style)

    dbg1 = G4Debugger(args.base, args.in1)
    dbg2 = G4Debugger(args.base, args.in2)

    print(dbg1.volumeSummary)  # quick sanity check

    plotSummaryRatio(
        dbg1.volumeSummary,
        dbg2.volumeSummary,
        xaxis="Volumes",
        v1=args.v1_label,
        v2=args.v2_label,
        directory=args.out_dir,
        name="volumeSummary",
    )


if __name__ == "__main__":
    main()

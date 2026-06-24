#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# External tuning harness:
#   1. run the regular, non ML reconstruction baseline
#   2. run the edge-classifier reconstruction for a threshold grid
#   3. measure signal-muon reconstruction efficiency in MsTrackValidTest
#   4. report threshold points with relative efficiency loss below target

from __future__ import annotations

import argparse
import csv
import json
import subprocess
import sys
from pathlib import Path

REQUIRED_BRANCHES = (
    "TruthMuons_truthOrigin",
    "TruthMuons_truthType",
    "MsTrkSeed_truthLink",
    "ActsMuons_seedLink",
)

def _parse_float_list(raw: str) -> list[float]:
    values = [float(value) for value in raw.split(",") if value.strip()]
    if not values:
        raise argparse.ArgumentTypeError("threshold list must contain at least one value")
    return values


def _run(command: list[str], log_path: Path) -> int:
    log_path.parent.mkdir(parents=True, exist_ok=True)
    with log_path.open("w", encoding="utf-8") as log:
        process = subprocess.run(
            command,
            stdout=log,
            stderr=subprocess.STDOUT,
            text=True,
        )
    return process.returncode


def _signal_muon_efficiency(root_file: Path,
                            tree_name: str,
                            signal_origin: int,
                            signal_type: int) -> dict[str, float | int]:
    """Measure the same signal-muon efficiency used in the comparison notebook.
    A denominator muon is a truth muon satisfying:

        TruthMuons_truthOrigin == signal_origin
        TruthMuons_truthType   == signal_type

    It is reconstructed when an ActsMuons track points to a seed via
    ActsMuons_seedLink and that seed points back to the truth-muon index via
    MsTrkSeed_truthLink.  All links are local to one event.
    """

    import ROOT

    input_file = ROOT.TFile.Open(str(root_file), "READ")
    if not input_file or input_file.IsZombie():
        raise RuntimeError(f"Could not open ROOT file: {root_file}")
    tree = input_file.Get(tree_name)
    if not tree or not tree.InheritsFrom("TTree"):
        input_file.Close()
        raise RuntimeError(
            f"Could not find TTree '{tree_name}' in {root_file}. "
            "The reco chain must write the MsTrackValidTest tree."
        )

    missing_branches = [
        branch for branch in REQUIRED_BRANCHES if not tree.GetBranch(branch)
    ]
    if missing_branches:
        input_file.Close()
        raise RuntimeError(
            f"Missing required branch(es) in {root_file}: {', '.join(missing_branches)}"
        )

    signal_muons = 0
    matched_signal_muons = 0

    for entry_number in range(tree.GetEntries()):
        tree.GetEntry(entry_number)
        
        truth_origins = [
            int(value) for value in tree.TruthMuons_truthOrigin
        ]
        truth_types = [
            int(value) for value in tree.TruthMuons_truthType
        ]
        seed_truth_links = [
            int(value) for value in tree.MsTrkSeed_truthLink
        ]
        reco_seed_links = [
            int(value) for value in tree.ActsMuons_seedLink
        ]

        if len(truth_origins) != len(truth_types):
            input_file.Close()
            raise RuntimeError(
                f"Truth origin/type vector-size mismatch in entry {entry_number} "
                f"of {root_file}"
            )

        signal_truth_indices = (
            truth_index
            for truth_index, (origin, truth_type) in enumerate(
                zip(truth_origins, truth_types)
            )
            if origin == signal_origin and truth_type == signal_type
        )

        for truth_index in signal_truth_indices:
            signal_muons += 1
            matching_seed_indices = {
                seed_index
                for seed_index, linked_truth_index in enumerate(seed_truth_links)
                if linked_truth_index == truth_index
            }
            if matching_seed_indices and any(
                seed_index in matching_seed_indices
                for seed_index in reco_seed_links
            ):
                matched_signal_muons += 1

    input_file.Close()

    if signal_muons == 0:
        raise RuntimeError(
            f"No signal truth muons found in {root_file}. "
            f"Selection is truthOrigin={signal_origin}, truthType={signal_type}."
        )

    return {
        "signalMuonCount": signal_muons,
        "matchedSignalMuonCount": matched_signal_muons,
        "signalMuonEfficiency": matched_signal_muons / signal_muons,
    }

def _chain_command(args,
                   out_root: Path,
                   edge: bool,
                   edge_threshold: float | None = None,
                   overlap_threshold: float | None = None) -> list[str]:
    """Build a command equivalent to reco_chain_ec.sh or the no-ML baseline."""
    recochain_script = Path(__file__).with_name("muonEdgeRecoChain.py")

    command = [
        sys.executable,
        str(recochain_script),
        "--threads", str(args.threads),
        "--nEvents", str(args.nEvents),
        "--skipEvents", str(args.skipEvents),
        "--inputFile", args.inputFile,
        "--outRootFile", str(out_root),
        "--defaultGeoFile", args.defaultGeoFile,
        "--noPerfMon", 
    ]

    if edge:
        command += [
            "--edgeModel", args.edgeModel,
            "--edgeThreshold", str(edge_threshold),
            "--overlapThreshold", str(overlap_threshold),
            "--enableBucketFilter",
            "--enableEdgeClassifier",
            "--useMlSeeder",
        ]
        if args.bucketThreshold is not None:
            command += ["--bucketThreshold", str(args.bucketThreshold)]
        if args.bucketModel:
            command += ["--bucketModel", args.bucketModel]
    else:
        # Same upstream chain, but no edge inference and old seeder.
        command += [
            "--disableBucketFilter",
            "--disableEdgeClassifier",
            "--useOldSeeder",
            "--skip-onnx",
        ]
        
    # muonEdgeRecoChain.py defaults to CUDA when ONNX inference is enabled.
    if args.use_cpu:
        command.append("--use-cpu")
        
    return command

def _result_row(edge_threshold: float,
                overlap_threshold: float,
                baseline: dict[str, float | int],
                edge: dict[str, float | int],
                target_relative_efficiency_loss: float,
                root_file: Path,
                log_file: Path) -> dict[str, float | int | str | bool]:
    if edge["signalMuonCount"] != baseline["signalMuonCount"]:
        return {
            "edgeThreshold": edge_threshold,
            "overlapThreshold": overlap_threshold,
            "status": "truth_count_mismatch",
            "baselineSignalMuonCount": baseline["signalMuonCount"],
            "edgeSignalMuonCount": edge["signalMuonCount"],
            "rootFile": str(root_file),
            "log": str(log_file),
        }

    baseline_efficiency = float(baseline["signalMuonEfficiency"])
    edge_efficiency = float(edge["signalMuonEfficiency"])
    relative_efficiency_difference = (
        (edge_efficiency - baseline_efficiency) / baseline_efficiency
    )
    relative_efficiency_loss = max(0.0, -relative_efficiency_difference)

    return {
        "edgeThreshold": edge_threshold,
        "overlapThreshold": overlap_threshold,
        "status": "ok",
        "baselineSignalMuonCount": baseline["signalMuonCount"],
        "baselineMatchedSignalMuons": baseline["matchedSignalMuonCount"],
        "baselineSignalEfficiency": baseline_efficiency,
        "edgeSignalMuonCount": edge["signalMuonCount"],
        "edgeMatchedSignalMuons": edge["matchedSignalMuonCount"],
        "edgeSignalEfficiency": edge_efficiency,
        "absoluteEfficiencyDifference": edge_efficiency - baseline_efficiency,
        "relativeEfficiencyDifference": relative_efficiency_difference,
        "relativeEfficiencyLoss": relative_efficiency_loss,
        "passesTarget": relative_efficiency_loss <= target_relative_efficiency_loss,
        "rootFile": str(root_file),
        "log": str(log_file),
    }

def main() -> None:
    parser = argparse.ArgumentParser(
        description=("Tune SegmentEdge thresholds using signal-muon reconstruction "
                     "efficiency relative to the regular no-ML reconstruction."))
    parser.add_argument("--inputFile", required=True)
    parser.add_argument("--bucketModel", default=None, help=("Optional bucket-filter ONNX model."),)
    parser.add_argument("--bucketThreshold", "--score-threshold", dest="bucketThreshold", type=float,
                        default=None, help="Bucket-filter score threshold")
    parser.add_argument("--bucket-output-is-logit", dest="bucketOutputIsLogit", action="store_true", default=False,
                        help=("Interpret the scalar bucket-model output as a logit"))
    parser.add_argument("--edgeModel", required=True)
    parser.add_argument("--nEvents", type=int, default=100)
    parser.add_argument("--skipEvents", type=int, default=0)
    parser.add_argument("--threads", type=int, default=1)
    parser.add_argument("--defaultGeoFile", default="RUN4")    
    parser.add_argument("--workDir", default="edge_threshold_tuning")
    parser.add_argument("--edgeThresholds", default="0.08,0.10,0.119,0.14,0.16", help="Comma-separated recovery edge-probability thresholds to scan",)
    parser.add_argument("--overlapThresholds", default="0.20,0.30,0.50,0.80", help=("Comma-separated high-purity core edge-probability thresholds to scan."))
    parser.add_argument("--targetRelativeEfficiencyLoss", "--targetLoss", dest="targetRelativeEfficiencyLoss",
        type=float, default=0.05, help=("Maximum allowed relative loss in signal-muon efficiency"),)
    parser.add_argument("--treeName", default="MsTrackValidTest")
    parser.add_argument("--signalOrigin", type=int, default=13)
    parser.add_argument("--signalType", type=int, default=6)
    parser.add_argument("--use-cpu", dest="use_cpu", action="store_true", default=False,)    
    parser.add_argument("--skipExisting", action="store_true")
    args = parser.parse_args()

    edge_thresholds = _parse_float_list(args.edgeThresholds)
    overlap_thresholds = _parse_float_list(args.overlapThresholds)

    work_dir = Path(args.workDir).resolve()
    work_dir.mkdir(parents=True, exist_ok=True)

    baseline_root = work_dir / "baseline_noml.root"
    baseline_log = work_dir / "baseline_noml.log"
    baseline_command = _chain_command(args, baseline_root, edge=False)
    if not args.skipExisting or not baseline_root.exists():
        return_code = _run(baseline_command, baseline_log)
        if return_code != 0:
            raise SystemExit(
                f"No-ML baseline job failed with rc={return_code}. See {baseline_log}"
            )
    if not baseline_root.exists():
        raise SystemExit(
            f"No-ML baseline finished but ROOT output is missing: {baseline_root}. "
            f"See {baseline_log}"
        )

    baseline = _signal_muon_efficiency(baseline_root, args.treeName, args.signalOrigin, args.signalType,)

    rows: list[dict[str, float | int | str | bool]] = []

    best = None
    for edge_threshold in edge_thresholds:
        for overlap_threshold in overlap_thresholds:
            tag = (
                f"edge{edge_threshold:.6f}_overlap{overlap_threshold:.6f}"
                .replace(".", "p")
            )
            out_root = work_dir / f"{tag}.root"
            out_log = work_dir / f"{tag}.log"
            command = _chain_command(
                args,
                out_root,
                edge=True,
                edge_threshold=edge_threshold,
                overlap_threshold=overlap_threshold,
            )
            if not args.skipExisting or not out_root.exists():
                return_code = _run(command, out_log)
                if return_code != 0:
                    rows.append({
                        "edgeThreshold": edge_threshold,
                        "overlapThreshold": overlap_threshold,
                        "status": "failed",
                        "rootFile": str(out_root),
                        "log": str(out_log),
                    })
                    continue
            if not out_root.exists():
                rows.append({
                    "edgeThreshold": edge_threshold,
                    "overlapThreshold": overlap_threshold,
                    "status": "missing_output",
                    "rootFile": str(out_root),
                    "log": str(out_log),
                })
                continue

            edge = _signal_muon_efficiency(
                out_root,
                args.treeName,
                args.signalOrigin,
                args.signalType,
            )
            row = _result_row(
                edge_threshold,
                overlap_threshold,
                baseline,
                edge,
                args.targetRelativeEfficiencyLoss,
                out_root,
                out_log,
            )
            
            rows.append(row)

            if row["status"] == "ok" and row["passesTarget"]:
                key = (edge_threshold, overlap_threshold)
                if best is None or key > (
                    best["edgeThreshold"],
                    best["overlapThreshold"],
                ):
                    best = row

    csv_path = work_dir / "edge_threshold_scan.csv"
    fieldnames = [
        "edgeThreshold",
        "overlapThreshold",
        "status",
        "baselineSignalMuonCount",
        "baselineMatchedSignalMuons",
        "baselineSignalEfficiency",
        "edgeSignalMuonCount",
        "edgeMatchedSignalMuons",
        "edgeSignalEfficiency",
        "absoluteEfficiencyDifference",
        "relativeEfficiencyDifference",
        "relativeEfficiencyLoss",
        "passesTarget",
        "rootFile",
        "log",
    ]
    with csv_path.open("w", newline="", encoding="utf-8") as csv_file:
        writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)

    summary = {
        "treeName": args.treeName,
        "signalSelection": {
            "truthOrigin": args.signalOrigin,
            "truthType": args.signalType,
        },
        "matching": (
            "Truth muon <- MsTrkSeed_truthLink -> seed <- "
            "ActsMuons_seedLink -> reconstructed track"
        ),
        "baseline": baseline,
        "bucketFilter": {
            "model": args.bucketModel,
            "scoreThreshold": args.bucketThreshold,
            "scoreThresholdSource": (
                "CLI override" if args.bucketThreshold is not None
                else "GraphBucketFilterToolCfg default"
            ),
        },
        "segmentEdgeGraph": {
            "ReadSpacePoints": "FilteredMlBuckets",
            "OrderingSpacePoints": "MuonSpacePoints",
            "note": (
                "Applied by muonEdgeRecoChain.py when bucket filtering and "
                "edge inference are enabled."
            ),
        },
        "targetRelativeEfficiencyLoss": args.targetRelativeEfficiencyLoss,
        "selectionPolicy": (
            "largest (edgeThreshold, overlapThreshold) pair with relative "
            "signal-efficiency loss at or below the target"
        ),
        "best": best,
        "scanCsv": str(csv_path),
    }
    (work_dir / "summary.json").write_text(
        json.dumps(summary, indent=2),
        encoding="utf-8",
    )
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()

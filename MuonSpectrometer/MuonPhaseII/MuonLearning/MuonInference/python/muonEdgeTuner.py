#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# External tuning harness:
#   1. run baseline reco chain without edge inference / ML seeder
#   2. run edge reco chain for a threshold grid
#   3. compare a configurable track-count metric from the output ROOT files
#   4. report threshold points with relative track loss below target

from __future__ import annotations

import argparse
import csv
import json
import subprocess
import sys
from pathlib import Path


def _parse_float_list(raw: str) -> list[float]:
    return [float(x) for x in raw.split(",") if x.strip()]


def _run(cmd: list[str], log_path: Path) -> int:
    log_path.parent.mkdir(parents=True, exist_ok=True)
    with log_path.open("w", encoding="utf-8") as log:
        proc = subprocess.run(cmd, stdout=log, stderr=subprocess.STDOUT, text=True)
    return proc.returncode


def _iter_root_trees(root_file: Path):
    import ROOT

    f = ROOT.TFile.Open(str(root_file), "READ")
    if not f or f.IsZombie():
        raise RuntimeError(f"Could not open ROOT file: {root_file}")
    if not f.GetListOfKeys() or f.GetListOfKeys().GetEntries() == 0:
        f.Close()
        raise RuntimeError(
            f"ROOT file has no keys: {root_file}. "
            "The Athena job finished, but no monitoring/tester tree was written. "
            "Run muonEdgeRecoChain.py with --enableRecoChainTester, or pass an "
            "output configuration that writes a tree containing the track-count metric."
        )

    def walk(directory, prefix=""):
        for key in directory.GetListOfKeys():
            obj = key.ReadObj()
            name = key.GetName()
            full = f"{prefix}/{name}" if prefix else name
            if obj.InheritsFrom("TTree"):
                yield full, obj
            elif obj.InheritsFrom("TDirectory"):
                yield from walk(obj, full)

    yield from walk(f)
    f.Close()


def _pt_to_gev(pt: float, units: str) -> float:
    if units == "MeV":
        return pt / 1000.0
    if units == "GeV":
        return pt
    # auto: ATLAS track pT branches are usually MeV
    return pt / 1000.0 if abs(pt) > 200.0 else pt


def _matched_ms_track_metric(tree,
                             truth_link_branch: str,
                             pt_branch: str | None,
                             truth_link_threshold: int,
                             min_pt_gev: float,
                             pt_units: str) -> float:
    branches = {b.GetName() for b in tree.GetListOfBranches()}
    if truth_link_branch not in branches:
        raise RuntimeError(f"Missing truth-link branch '{truth_link_branch}'")
    if pt_branch and pt_branch not in branches:
        raise RuntimeError(f"Missing pT branch '{pt_branch}'")

    total = 0
    for entry in tree:
        links = list(getattr(entry, truth_link_branch))
        pts = list(getattr(entry, pt_branch)) if pt_branch else [None] * len(links)

        for i, link in enumerate(links):
            if int(link) < truth_link_threshold:
                continue
            if pt_branch:
                pt_gev = _pt_to_gev(float(pts[i]), pt_units)
                if pt_gev < min_pt_gev:
                    continue
            total += 1
    return float(total)


def _metric_from_root(root_file: Path,
                      preferred_tree: str | None,
                      preferred_branch: str | None,
                      args) -> float:
    """
    Generic ROOT metric reader.

    Preferred usage:
      --metricTree <tree> --metricBranch <branch>

    If not provided, the script tries common branch names. This is intentionally
    external to Athena so the tuning loop can run many complete jobs.
    """

    branch_candidates = []
    if preferred_branch:
        branch_candidates.append(preferred_branch)
    branch_candidates += [
        "nMsTracks",
        "nMSTracks",
        "nTracks",
        "nRecoTracks",
        "nMuonTracks",
    ]

    trees = list(_iter_root_trees(root_file))
    if preferred_tree:
        trees = [(name, tree) for name, tree in trees if name == preferred_tree or name.endswith("/" + preferred_tree)]
    if not trees:
        raise RuntimeError(f"No matching TTree found in {root_file}")

    if args.metricMode == "matchedTruthTracks":
        for _, tree in trees:
            return _matched_ms_track_metric(tree,
                                            args.truthLinkBranch,
                                            args.trackPtBranch,
                                            args.truthLinkThreshold,
                                            args.minPtGeV,
                                            args.ptUnits)

    for tree_name, tree in trees:
        branches = {b.GetName() for b in tree.GetListOfBranches()}
        for branch in branch_candidates:
            if branch not in branches:
                continue
            total = 0.0
            for entry in tree:
                val = getattr(entry, branch)
                try:
                    total += float(val)
                except TypeError:
                    total += float(len(val))
            return total

    available = {}
    for tree_name, tree in trees:
        available[tree_name] = [b.GetName() for b in tree.GetListOfBranches()]
    raise RuntimeError(
        "Could not find a metric branch. Pass --metricTree/--metricBranch. "
        f"Available branches: {json.dumps(available, indent=2)}"
    )


def _chain_cmd(args, out_root: Path, edge: bool,
               edge_threshold: float | None = None,
               overlap_threshold: float | None = None) -> list[str]:
    recochain_script = Path(__file__).with_name("muonEdgeRecoChain.py")
    cmd = [
        sys.executable,
        str(recochain_script),
        "--inputFile", args.inputFile,
        "--nEvents", str(args.nEvents),
        "--outRootFile", str(out_root),
        "--bucketModel", args.bucketModel,
        "--bucketThreshold", str(args.bucketThreshold),
    ]
    if args.extraRecoArgs:
        cmd += args.extraRecoArgs.split()

    # GPU passthrough to muonEdgeRecoChain
    if getattr(args, 'use_gpu', None) is True:
        cmd.append("--use-gpu")
    elif getattr(args, 'use_gpu', None) is False:
        cmd.append("--use-cpu")

    if edge:
        cmd += [
            "--edgeModel", args.edgeModel,
            "--enableEdgeClassifier",
            "--useMlSeeder",
            "--edgeThreshold", str(edge_threshold),
            "--overlapThreshold", str(overlap_threshold),
        ]
    else:
        # Same upstream chain, but no edge inference and old seeder.
        cmd += [
            "--disableEdgeClassifier",
            "--useOldSeeder",
        ]
    return cmd


def main():
    parser = argparse.ArgumentParser(
        description="Tune SegmentEdge thresholds by comparing edge-chain track loss to baseline."
    )
    parser.add_argument("--inputFile", required=True)
    parser.add_argument("--bucketModel", required=True)
    parser.add_argument("--bucketThreshold", "--score-threshold", dest="bucketThreshold", type=float, default=0.0,
                        help="Threshold on bucket filter score")
    parser.add_argument("--edgeModel", required=True)
    parser.add_argument("--nEvents", type=int, default=100)
    parser.add_argument("--workDir", default="edge_threshold_tuning")
    parser.add_argument("--edgeThresholds", default="0.10,0.15,0.20,0.25,0.30,0.35,0.40,0.50")
    parser.add_argument("--overlapThresholds", default="0.60,0.70,0.80,0.90")
    parser.add_argument("--targetLoss", type=float, default=0.001,
                        help="Maximum allowed relative loss, default 0.001 = 0.1 percent")
    parser.add_argument("--metricTree", default=None)
    parser.add_argument("--metricBranch", default=None)
    parser.add_argument("--metricMode", default="matchedTruthTracks",
                        choices=["matchedTruthTracks", "rawTrackCount"],
                        help="matchedTruthTracks counts MS tracks with truthLink >= threshold and pT cut")
    parser.add_argument("--truthLinkBranch", default="MSTrksR4_truthLink")
    parser.add_argument("--trackPtBranch", default="MSTrksR4_pt")
    parser.add_argument("--truthLinkThreshold", type=int, default=1)
    parser.add_argument("--minPtGeV", type=float, default=2.0)
    parser.add_argument("--ptUnits", default="auto",
                        choices=["auto", "MeV", "GeV"])
    parser.add_argument("--extraRecoArgs", default="",
                        help="Extra args forwarded to muonEdgeRecoChain.py")
    parser.add_argument("--use-gpu", action="store_true", dest="use_gpu", default=None,
                        help="Use GPU for ONNX inference in the reco chain (default: auto-detect)")
    parser.add_argument("--use-cpu", dest="use_gpu", action="store_false",
                        help="Force CPU for ONNX inference in the reco chain")
    parser.add_argument("--skipExisting", action="store_true")
    args = parser.parse_args()

    work = Path(args.workDir).resolve()
    work.mkdir(parents=True, exist_ok=True)

    baseline_root = work / "baseline.root"
    baseline_log = work / "baseline.log"
    baseline_cmd = _chain_cmd(args, baseline_root, edge=False)
    if not args.skipExisting or not baseline_root.exists():
        rc = _run(baseline_cmd, baseline_log)
        if rc != 0:
            raise SystemExit(f"Baseline job failed with rc={rc}. See {baseline_log}")
    if not baseline_root.exists():
        raise SystemExit(
            f"Baseline job finished but output ROOT file is missing: {baseline_root}. "
            f"Ensure reco args produce this output. "
            f"See {baseline_log}"
        )

    baseline_metric = _metric_from_root(baseline_root, args.metricTree, args.metricBranch, args)
    if baseline_metric <= 0:
        raise SystemExit(f"Baseline metric is non-positive: {baseline_metric}")

    rows = []
    best = None
    for edge_thr in _parse_float_list(args.edgeThresholds):
        for overlap_thr in _parse_float_list(args.overlapThresholds):
            tag = f"edge{edge_thr:.3f}_overlap{overlap_thr:.3f}".replace(".", "p")
            out_root = work / f"{tag}.root"
            out_log = work / f"{tag}.log"
            cmd = _chain_cmd(args, out_root, edge=True,
                             edge_threshold=edge_thr,
                             overlap_threshold=overlap_thr)
            if not args.skipExisting or not out_root.exists():
                rc = _run(cmd, out_log)
                if rc != 0:
                    rows.append({
                        "edgeThreshold": edge_thr,
                        "overlapThreshold": overlap_thr,
                        "status": "failed",
                        "log": str(out_log),
                    })
                    continue
            if not out_root.exists():
                rows.append({
                    "edgeThreshold": edge_thr,
                    "overlapThreshold": overlap_thr,
                    "status": "missing_output",
                    "log": str(out_log),
                    "rootFile": str(out_root),
                })
                continue

            metric = _metric_from_root(out_root, args.metricTree, args.metricBranch, args)
            loss = max(0.0, (baseline_metric - metric) / baseline_metric)
            row = {
                "edgeThreshold": edge_thr,
                "overlapThreshold": overlap_thr,
                "status": "ok",
                "baselineMetric": baseline_metric,
                "edgeMetric": metric,
                "relativeLoss": loss,
                "passesTarget": loss < args.targetLoss,
                "rootFile": str(out_root),
                "log": str(out_log),
            }
            rows.append(row)

            if row["passesTarget"]:
                # Prefer the largest EdgeThreshold that still passes, then largest OverlapThreshold.
                key = (edge_thr, overlap_thr)
                if best is None or key > (best["edgeThreshold"], best["overlapThreshold"]):
                    best = row

    csv_path = work / "edge_threshold_scan.csv"
    with csv_path.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=sorted({k for r in rows for k in r}))
        writer.writeheader()
        writer.writerows(rows)

    summary = {
        "baselineMetric": baseline_metric,
        "targetLoss": args.targetLoss,
        "best": best,
        "scanCsv": str(csv_path),
    }
    (work / "summary.json").write_text(json.dumps(summary, indent=2), encoding="utf-8")

    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()

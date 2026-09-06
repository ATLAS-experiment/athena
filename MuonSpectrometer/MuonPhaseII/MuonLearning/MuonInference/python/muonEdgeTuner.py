#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Tune ``muonEdgeRecoChain.py --edgeThreshold`` against a no-ML baseline.

For every scan stage, this script runs the standard no-ML reconstruction once:
``muonEdgeRecoChain.py --skip-onnx --useStandardSeeder``.  It then runs the
ML edge-classifier chain at a sequence of ``--edgeThreshold`` values and
compares each point with that stage's no-ML track efficiency.

The tested ML configuration matches the current ``muonEdgeRecoChain.py``:

* ``--enableBucketFilter`` (enabled by default, configurable);
* ``--enableEdgeClassifier --useMlSeeder``;
* ``--filterSegmentsWithoutMlConnections`` (enabled by default);
* a required ``--edgeModel`` and a scanned ``--edgeThreshold``.

The baseline mirrors ``reco_chain_noml.sh``.  The same executable truth-muon
selection used by ``muonBFTuner.py`` is evaluated from ``MsTrackValidTest``
through PyROOT, so no uproot/awkward/numpy dependency is required.

Stages:
  * coarse: 10 events; threshold 0.0..1.0 in 0.05 steps;
  * medium: 100 events; start at the last passing coarse point; 0.01 steps;
  * fine: 1000 events; start at the last passing medium point; 0.005 steps.

By default, a point passes when:
    edgeTrackEfficiency / noMlTrackEfficiency >= 0.995
"""

from __future__ import annotations

import argparse
import csv
import json
import subprocess
import sys
from dataclasses import dataclass
from decimal import Decimal, InvalidOperation
from pathlib import Path
from typing import Any, Iterable


TREE_DEFAULT = "MsTrackValidTest"
REQUIRED_BRANCHES = (
    "TruthMuons_eta",
    "TruthMuons_truthSegLinks",
    "TruthMuons_ActsMuonLink",
    "ActsMuons_pt",
)


@dataclass(frozen=True)
class ScanStage:
    """A reconstruction-statistics / score-resolution scan stage."""

    name: str
    n_events: int
    step: float


def _decimal(value: float | str, name: str) -> Decimal:
    """Return a finite Decimal, with an argparse-style diagnostic on failure."""

    try:
        result = Decimal(str(value))
    except (InvalidOperation, ValueError) as error:
        raise ValueError(f"{name} must be a finite decimal value") from error
    if not result.is_finite():
        raise ValueError(f"{name} must be a finite decimal value")
    return result


def _format_threshold(value: float) -> str:
    """Return a deterministic, filename-safe number, e.g. -0.025 -> m0p025."""

    text = format(_decimal(value, "threshold").normalize(), "f")
    if "." not in text:
        text += ".0"
    return text.replace("-", "m").replace(".", "p")


def _ascending_thresholds(
    start: float,
    stop: float,
    step: float,
    *,
    include_start: bool = True,
) -> Iterable[float]:
    """Yield a decimal threshold grid including ``stop`` when it lies on-grid."""

    current = _decimal(start, "threshold")
    end = _decimal(stop, "threshold")
    increment = _decimal(step, "step")
    if increment <= 0:
        raise ValueError("step must be positive")
    if not include_start:
        current += increment
    while current <= end:
        yield float(current)
        current += increment


def _descending_thresholds(
    start: float,
    stop: float,
    step: float,
    *,
    include_start: bool = False,
) -> Iterable[float]:
    """Yield a descending decimal threshold grid."""

    current = _decimal(start, "threshold")
    end = _decimal(stop, "threshold")
    decrement = _decimal(step, "step")
    if decrement <= 0:
        raise ValueError("step must be positive")
    if not include_start:
        current -= decrement
    while current >= end:
        yield float(current)
        current -= decrement


def _run(command: list[str], log_path: Path) -> int:
    """Run one reconstruction and send stdout/stderr to a per-job log."""

    log_path.parent.mkdir(parents=True, exist_ok=True)
    with log_path.open("w", encoding="utf-8") as log_file:
        completed = subprocess.run(
            command,
            stdout=log_file,
            stderr=subprocess.STDOUT,
            text=True,
            check=False,
        )
    return completed.returncode


def _vector_size(value: Any) -> int:
    """Obtain a PyROOT STL-vector size without assuming a Python list."""

    try:
        return len(value)
    except TypeError:
        try:
            return int(value.size())
        except AttributeError as error:
            raise RuntimeError(
                f"Object of type {type(value)!r} does not expose an STL-vector size"
            ) from error


def _read_notebook_efficiency(root_file: Path, tree_name: str) -> dict[str, int | float]:
    """Evaluate the notebook's actual selection from an Athena ROOT output.

    The function deliberately uses the local links exactly as the supplied
    notebook does.  It does *not* use the older MsTrkSeed / ActsMuons seed-link
    matching method in muonEdgeTuner.py.
    """

    try:
        import ROOT
    except ImportError as error:
        raise RuntimeError(
            "PyROOT is unavailable. Run muonEdgeTuner.py from a configured Athena "
            "environment so that `import ROOT` works."
        ) from error

    input_file = ROOT.TFile.Open(str(root_file), "READ")
    if not input_file or input_file.IsZombie():
        raise RuntimeError(f"Could not open ROOT output: {root_file}")

    try:
        tree = input_file.Get(tree_name)
        if not tree or not tree.InheritsFrom("TTree"):
            raise RuntimeError(f"Could not find TTree '{tree_name}' in {root_file}.")

        missing = [name for name in REQUIRED_BRANCHES if not tree.GetBranch(name)]
        if missing:
            raise RuntimeError(
                f"Missing required branch(es) in {root_file}: {', '.join(missing)}"
            )

        truth_muons = 0
        matched_truth_muons = 0

        for entry_number in range(int(tree.GetEntries())):
            tree.GetEntry(entry_number)

            truth_eta = tree.TruthMuons_eta
            truth_segment_links = tree.TruthMuons_truthSegLinks
            truth_to_acts_link = tree.TruthMuons_ActsMuonLink
            acts_muon_pt = tree.ActsMuons_pt

            n_truth = _vector_size(truth_eta)
            n_segments = _vector_size(truth_segment_links)
            n_links = _vector_size(truth_to_acts_link)
            if n_truth != n_segments or n_truth != n_links:
                raise RuntimeError(
                    f"Truth-muon branch-size mismatch in entry {entry_number} of "
                    f"{root_file}: eta={n_truth}, truthSegLinks={n_segments}, "
                    f"ActsMuonLink={n_links}"
                )

            n_acts_muons = _vector_size(acts_muon_pt)
            for truth_index in range(n_truth):
                # Exact notebook denominator:
                # ak.num(TruthMuons_truthSegLinks[event], axis=-1) > 0
                # and abs(TruthMuons_eta[event]) < 2.5
                if (
                    _vector_size(truth_segment_links[truth_index]) <= 0
                    or abs(float(truth_eta[truth_index])) >= 2.5
                ):
                    continue

                truth_muons += 1
                acts_link = int(truth_to_acts_link[truth_index])
                if 0 <= acts_link < n_acts_muons:
                    matched_truth_muons += 1
    finally:
        input_file.Close()

    if truth_muons == 0:
        raise RuntimeError(
            f"No denominator truth muons found in {root_file}. Required selection: "
            "len(TruthMuons_truthSegLinks) > 0 and abs(TruthMuons_eta) < 2.5."
        )

    return {
        "truthMuonCount": truth_muons,
        "matchedTruthMuonCount": matched_truth_muons,
        "trackEfficiency": matched_truth_muons / truth_muons,
    }


def _reco_launcher(args: argparse.Namespace) -> list[str]:
    """Resolve muonEdgeRecoChain.py, preferring an explicit/local source."""

    if args.recoChain:
        chain = Path(args.recoChain).expanduser().resolve()
        if not chain.is_file():
            raise RuntimeError(f"--recoChain does not point to a file: {chain}")
        return [sys.executable, str(chain)]

    sibling_chain = Path(__file__).with_name("muonEdgeRecoChain.py")
    if sibling_chain.is_file():
        return [sys.executable, str(sibling_chain)]

    return [sys.executable, "-m", args.recoModule]


def _common_chain_command(
    args: argparse.Namespace,
    *,
    n_events: int,
    out_root: Path,
) -> list[str]:
    """Build arguments common to edge-classifier and no-ML jobs."""

    return [
        *_reco_launcher(args),
        "--threads", str(args.threads),
        "--nEvents", str(n_events),
        "--skipEvents", str(args.skipEvents),
        "--inputFile", args.inputFile,
        "--outRootFile", str(out_root),
        "--defaultGeoFile", args.defaultGeoFile,
        "--noPerfMon",
    ]


def _edge_chain_command(
    args: argparse.Namespace,
    *,
    threshold: float,
    n_events: int,
    out_root: Path,
) -> list[str]:
    """Build one ML edge-classifier reconstruction command."""

    command = _common_chain_command(args, n_events=n_events, out_root=out_root)
    command += [
        "--edgeModel", args.edgeModel,
        "--edgeThreshold", str(threshold),
        "--enableEdgeClassifier",
        "--useMlSeeder",
    ]
    command.append(
        "--enableBucketFilter" if args.enableBucketFilter else "--disableBucketFilter"
    )

    if args.filterSegmentsWithoutMlConnections:
        command.append("--filterSegmentsWithoutMlConnections")
    if args.bucketModelPath:
        command += ["--bucketModel", args.bucketModelPath]
    if args.bucketThreshold is not None:
        command += ["--bucketThreshold", str(args.bucketThreshold)]
    if args.outputName:
        command += ["--output-name", args.outputName]
    if args.singleOutputMode:
        command += ["--single-output-mode", args.singleOutputMode]
    if args.use_cpu:
        command.append("--use-cpu")
    if args.athenaDebug:
        command.append("--athenaDebug")

    optional_int_arguments = (
        ("maxEdgesPerSegment", "--maxEdgesPerSegment"),
        ("maxSegmentsPerBucket", "--maxSegmentsPerBucket"),
        ("maxEdgesBeforeInference", "--maxEdgesBeforeInference"),
        ("maxEdgesPerTargetChamber", "--maxEdgesPerTargetChamber"),
        ("seedAnchorsPerComponent", "--seedAnchorsPerComponent"),
        ("minSegmentsPerComponent", "--minSegmentsPerComponent"),
    )
    for attribute, option in optional_int_arguments:
        value = getattr(args, attribute)
        if value is not None:
            command += [option, str(value)]

    optional_flags = (
        ("keepSameChamberEdgesBeforeInference", "--keepSameChamberEdgesBeforeInference"),
        ("keepIsolatedNodesBeforeInference", "--keepIsolatedNodesBeforeInference"),
        ("useDegreeCappedMlComponents", "--useDegreeCappedMlComponents"),
        ("allowOneSidedMlEdges", "--allowOneSidedMlEdges"),
        ("disableOrphanRecovery", "--disableOrphanRecovery"),
        ("keepAllSegmentsPerChamber", "--keepAllSegmentsPerChamber"),
    )
    for attribute, option in optional_flags:
        if getattr(args, attribute):
            command.append(option)

    # Escape hatch for chain switches introduced after this tuner.  Each use
    # appends one argv token; use it repeatedly for an option and its value.
    command.extend(args.chainArg)
    return command


def _noml_chain_command(
    args: argparse.Namespace,
    *,
    n_events: int,
    out_root: Path,
) -> list[str]:
    """Build the no-ML baseline command from reco_chain_noml.sh."""

    command = _common_chain_command(args, n_events=n_events, out_root=out_root)
    command += ["--skip-onnx", "--useStandardSeeder"]
    command.extend(args.baselineChainArg)
    return command


def _write_csv(rows: list[dict[str, Any]], path: Path) -> None:
    fields = [
        "stage",
        "nEvents",
        "mode",
        "threshold",
        "status",
        "passesTarget",
        "truthCountMatchesNoMl",
        "truthMuonCount",
        "matchedTruthMuonCount",
        "trackEfficiency",
        "noMlTruthMuonCount",
        "noMlMatchedTruthMuonCount",
        "noMlTrackEfficiency",
        "relativeTrackEfficiency",
        "relativeEfficiencyLoss",
        "minRelativeEfficiency",
        "rootFile",
        "log",
        "returnCode",
        "command",
        "error",
    ]
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="") as output:
        writer = csv.DictWriter(output, fieldnames=fields, extrasaction="ignore")
        writer.writeheader()
        writer.writerows(rows)


def _write_json(payload: dict[str, Any], path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Tune muonEdgeRecoChain.py --edgeThreshold using relative track "
            "efficiency against a --skip-onnx --useStandardSeeder baseline "
            "at every scan stage."
        ),
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "--inputFile",
        required=True,
        help="Input HITS file/list forwarded to muonEdgeRecoChain.py.",
    )
    parser.add_argument(
        "--edgeModel",
        required=True,
        help="ONNX segment-edge classifier model forwarded to --edgeModel.",
    )
    parser.add_argument(
        "--recoChain",
        default=None,
        help="Explicit path to muonEdgeRecoChain.py. Defaults to a sibling file.",
    )
    parser.add_argument(
        "--recoModule",
        default="MuonInference.muonEdgeRecoChain",
        help="Module fallback when no local reco-chain source file is present.",
    )
    parser.add_argument(
        "--workDir",
        default="edge_threshold_tuning",
        help="Directory where roots/, logs/, CSV and JSON outputs are written.",
    )
    parser.add_argument("--treeName", default=TREE_DEFAULT)
    parser.add_argument("--threads", type=int, default=1)
    parser.add_argument("--skipEvents", type=int, default=0)
    parser.add_argument("--defaultGeoFile", default="RUN4")
    parser.add_argument("--use-cpu", dest="use_cpu", action="store_true", default=False)
    parser.add_argument(
        "--athenaDebug",
        action="store_true",
        default=False,
        help="Forward --athenaDebug to every ML edge-classifier reconstruction job.",
    )
    parser.add_argument(
        "--skipExisting",
        action="store_true",
        default=False,
        help="Reuse an existing ROOT output when its exact point is requested again.",
    )

    parser.set_defaults(
        enableBucketFilter=True,
        filterSegmentsWithoutMlConnections=True,
    )
    bucket_filter = parser.add_mutually_exclusive_group()
    bucket_filter.add_argument(
        "--enableBucketFilter",
        dest="enableBucketFilter",
        action="store_true",
        help="Run the bucket-filter preselection in every ML scan job.",
    )
    bucket_filter.add_argument(
        "--disableBucketFilter",
        dest="enableBucketFilter",
        action="store_false",
        help="Disable bucket filtering while tuning the edge threshold.",
    )
    segment_filter = parser.add_mutually_exclusive_group()
    segment_filter.add_argument(
        "--filterSegmentsWithoutMlConnections",
        dest="filterSegmentsWithoutMlConnections",
        action="store_true",
        help=(
            "Use the ML-connected segment view in track finding, matching "
            "reco_chain_ecfilter.sh."
        ),
    )
    segment_filter.add_argument(
        "--keepSegmentsWithoutMlConnections",
        dest="filterSegmentsWithoutMlConnections",
        action="store_false",
        help="Do not restrict the track-finder segment container to ML-connected segments.",
    )

    parser.add_argument(
        "--bucketModel",
        "--bucket-model-path",
        dest="bucketModelPath",
        default=None,
        help="Optional bucket-filter ONNX model; otherwise use the chain default.",
    )
    parser.add_argument(
        "--bucketThreshold",
        "--score-threshold",
        dest="bucketThreshold",
        type=float,
        default=None,
        help="Optional fixed bucket-filter score threshold for every ML scan job.",
    )
    parser.add_argument(
        "--output-name",
        dest="outputName",
        default=None,
        help="Optional bucket-filter ONNX output tensor name.",
    )
    parser.add_argument(
        "--single-output-mode",
        choices=("logit", "prob"),
        dest="singleOutputMode",
        default=None,
        help="Optional bucket-filter scalar ONNX-output interpretation.",
    )

    parser.add_argument(
        "--minRelativeEfficiency",
        "--targetRelativeEfficiency",
        "--targetEfficiency",
        dest="minRelativeEfficiency",
        type=float,
        default=0.995,
        help=(
            "An edge-threshold point passes when edgeTrackEfficiency / "
            "noMlTrackEfficiency is at least this value."
        ),
    )
    parser.add_argument(
        "--minThreshold",
        type=float,
        default=0.0,
        help="Lowest edge probability threshold considered.",
    )
    parser.add_argument(
        "--maxThreshold",
        type=float,
        default=1.0,
        help="Highest edge probability threshold considered.",
    )
    parser.add_argument("--coarseEvents", type=int, default=10)
    parser.add_argument("--coarseStep", type=float, default=0.05)
    parser.add_argument("--mediumEvents", type=int, default=100)
    parser.add_argument("--mediumStep", type=float, default=0.01)
    parser.add_argument("--fineEvents", type=int, default=1000)
    parser.add_argument("--fineStep", type=float, default=0.005)

    edge_graph = parser.add_argument_group(
        "Fixed SegmentEdgeInferenceAlg settings forwarded to every ML scan job"
    )
    edge_graph.add_argument("--maxEdgesPerSegment", type=int, default=None)
    edge_graph.add_argument("--maxSegmentsPerBucket", type=int, default=None)
    edge_graph.add_argument("--maxEdgesBeforeInference", type=int, default=None)
    edge_graph.add_argument("--maxEdgesPerTargetChamber", type=int, default=None)
    edge_graph.add_argument("--seedAnchorsPerComponent", type=int, default=None)
    edge_graph.add_argument("--minSegmentsPerComponent", type=int, default=None)
    edge_graph.add_argument("--keepSameChamberEdgesBeforeInference", action="store_true")
    edge_graph.add_argument("--keepIsolatedNodesBeforeInference", action="store_true")
    edge_graph.add_argument("--useDegreeCappedMlComponents", action="store_true")
    edge_graph.add_argument("--allowOneSidedMlEdges", action="store_true")
    edge_graph.add_argument("--disableOrphanRecovery", action="store_true")
    edge_graph.add_argument("--keepAllSegmentsPerChamber", action="store_true")
    parser.add_argument(
        "--chainArg",
        action="append",
        default=[],
        metavar="ARG",
        help="Append one extra argv token to each ML edge-classifier command.",
    )
    parser.add_argument(
        "--baselineChainArg",
        action="append",
        default=[],
        metavar="ARG",
        help="Append one extra argv token to each no-ML baseline command.",
    )

    return parser.parse_args()


def _validate_args(args: argparse.Namespace) -> None:
    if args.threads <= 0:
        raise SystemExit("--threads must be positive")
    if args.skipEvents < 0:
        raise SystemExit("--skipEvents must not be negative")
    if not 0.0 <= args.minRelativeEfficiency <= 1.0:
        raise SystemExit("--minRelativeEfficiency must be between 0 and 1")
    if not 0.0 <= args.minThreshold < args.maxThreshold <= 1.0:
        raise SystemExit(
            "edge thresholds must satisfy 0.0 <= --minThreshold < "
            "--maxThreshold <= 1.0"
        )
    if args.bucketThreshold is not None and not 0.0 <= args.bucketThreshold <= 1.0:
        raise SystemExit("--bucketThreshold must be between 0 and 1")
    for name in ("coarseEvents", "mediumEvents", "fineEvents"):
        if getattr(args, name) <= 0:
            raise SystemExit(f"--{name} must be positive")
    for name in ("coarseStep", "mediumStep", "fineStep"):
        if getattr(args, name) <= 0.0:
            raise SystemExit(f"--{name} must be positive")
    for name in (
        "maxEdgesPerSegment",
        "maxSegmentsPerBucket",
        "maxEdgesBeforeInference",
        "maxEdgesPerTargetChamber",
        "seedAnchorsPerComponent",
        "minSegmentsPerComponent",
    ):
        value = getattr(args, name)
        if value is not None and value < 0:
            raise SystemExit(f"--{name} must not be negative")


def main() -> None:
    args = _parse_args()
    _validate_args(args)

    work_dir = Path(args.workDir).expanduser().resolve()
    roots_dir = work_dir / "roots"
    logs_dir = work_dir / "logs"
    csv_path = work_dir / "edge_threshold_scan.csv"
    summary_path = work_dir / "summary.json"

    # Athena/THistSvc will not create parent directories itself.
    roots_dir.mkdir(parents=True, exist_ok=True)
    logs_dir.mkdir(parents=True, exist_ok=True)

    stages = (
        ScanStage("coarse", args.coarseEvents, args.coarseStep),
        ScanStage("medium", args.mediumEvents, args.mediumStep),
        ScanStage("fine", args.fineEvents, args.fineStep),
    )
    rows: list[dict[str, Any]] = []
    baselines: dict[str, dict[str, Any]] = {}
    best_by_stage: dict[str, dict[str, Any] | None] = {}
    stage_notes: dict[str, str] = {}

    def summary(status: str) -> dict[str, Any]:
        recommended = (
            best_by_stage.get("fine")
            or best_by_stage.get("medium")
            or best_by_stage.get("coarse")
        )
        return {
            "status": status,
            "inputFile": args.inputFile,
            "treeName": args.treeName,
            "workDir": str(work_dir),
            "recoLauncher": _reco_launcher(args),
            "edgeModel": args.edgeModel,
            "mlConfiguration": {
                "bucketFilterEnabled": args.enableBucketFilter,
                "filterSegmentsWithoutMlConnections": (
                    args.filterSegmentsWithoutMlConnections
                ),
                "bucketModel": args.bucketModelPath,
                "bucketThreshold": args.bucketThreshold,
            },
            "target": {
                "minimumRelativeEfficiency": args.minRelativeEfficiency,
                "acceptedWhen": (
                    "edgeTrackEfficiency / noMlTrackEfficiency >= "
                    "minimumRelativeEfficiency"
                ),
            },
            "selection": {
                "denominator": (
                    "len(TruthMuons_truthSegLinks[i]) > 0 and "
                    "abs(TruthMuons_eta[i]) < 2.5"
                ),
                "numerator": (
                    "denominator muon with 0 <= TruthMuons_ActsMuonLink[i] < "
                    "len(ActsMuons_pt)"
                ),
                "implementation": "PyROOT; equivalent to the notebook's executable code",
            },
            "stages": [
                {"name": stage.name, "nEvents": stage.n_events, "step": stage.step}
                for stage in stages
            ],
            "baselines": baselines,
            "bestByStage": best_by_stage,
            "stageNotes": stage_notes,
            "recommendedThreshold": (
                None if recommended is None else recommended["threshold"]
            ),
            "recommendedResult": recommended,
            "scanCsv": str(csv_path),
        }

    def persist(status: str) -> None:
        _write_csv(rows, csv_path)
        _write_json(summary(status), summary_path)

    def evaluate_baseline(stage: ScanStage) -> dict[str, Any]:
        """Run/reuse one no-ML reference output for this stage."""

        if stage.name in baselines:
            return baselines[stage.name]

        tag = f"noml_{stage.name}_events{stage.n_events:04d}"
        out_root = roots_dir / f"{tag}.root"
        out_log = logs_dir / f"{tag}.log"
        command = _noml_chain_command(args, n_events=stage.n_events, out_root=out_root)
        row: dict[str, Any] = {
            "stage": stage.name,
            "nEvents": stage.n_events,
            "mode": "noml",
            "threshold": "",
            "rootFile": str(out_root),
            "log": str(out_log),
            "command": " ".join(command),
        }

        if not args.skipExisting or not out_root.is_file():
            return_code = _run(command, out_log)
            row["returnCode"] = return_code
            if return_code != 0:
                row.update({
                    "status": "failed",
                    "error": f"no-ML reconstruction returned {return_code}",
                })
                rows.append(row)
                persist("failed")
                raise RuntimeError(
                    f"No-ML baseline failed for stage={stage.name} with rc={return_code}. "
                    f"See {out_log}"
                )
        else:
            row["returnCode"] = "reused"

        if not out_root.is_file():
            row.update({
                "status": "missing_output",
                "error": "no-ML ROOT output is missing after reconstruction",
            })
            rows.append(row)
            persist("failed")
            raise RuntimeError(
                f"No-ML ROOT output is missing for stage={stage.name}: {out_root}"
            )

        try:
            metrics = _read_notebook_efficiency(out_root, args.treeName)
        except Exception as error:
            row.update({"status": "evaluation_failed", "error": str(error)})
            rows.append(row)
            persist("failed")
            raise

        row.update(metrics)
        row.update({
            "noMlTruthMuonCount": metrics["truthMuonCount"],
            "noMlMatchedTruthMuonCount": metrics["matchedTruthMuonCount"],
            "noMlTrackEfficiency": metrics["trackEfficiency"],
            "status": "ok",
        })
        rows.append(row)
        baselines[stage.name] = row
        persist("running")
        print(
            f"[{stage.name:6s} | {stage.n_events:4d} events] "
            f"no-ML efficiency={metrics['trackEfficiency']:.6f} "
            f"({metrics['matchedTruthMuonCount']}/{metrics['truthMuonCount']})",
            flush=True,
        )
        return row

    def evaluate_edge(stage: ScanStage, threshold: float) -> dict[str, Any]:
        """Run/reuse and compare one edge-classifier threshold point."""

        baseline = evaluate_baseline(stage)

        tag = (
            f"{stage.name}_events{stage.n_events:04d}_"
            f"threshold{_format_threshold(threshold)}"
        )
        out_root = roots_dir / f"{tag}.root"
        out_log = logs_dir / f"{tag}.log"
        command = _edge_chain_command(
            args,
            threshold=threshold,
            n_events=stage.n_events,
            out_root=out_root,
        )
        row: dict[str, Any] = {
            "stage": stage.name,
            "nEvents": stage.n_events,
            "mode": "edge",
            "threshold": threshold,
            "minRelativeEfficiency": args.minRelativeEfficiency,
            "rootFile": str(out_root),
            "log": str(out_log),
            "command": " ".join(command),
        }

        if not args.skipExisting or not out_root.is_file():
            return_code = _run(command, out_log)
            row["returnCode"] = return_code
            if return_code != 0:
                row.update({
                    "status": "failed",
                    "error": f"edge-classifier reconstruction returned {return_code}",
                })
                rows.append(row)
                persist("failed")
                raise RuntimeError(
                    f"Reconstruction failed at stage={stage.name}, threshold={threshold} "
                    f"with rc={return_code}. See {out_log}"
                )
        else:
            row["returnCode"] = "reused"

        if not out_root.is_file():
            row.update({
                "status": "missing_output",
                "error": "edge-classifier ROOT output is missing after reconstruction",
            })
            rows.append(row)
            persist("failed")
            raise RuntimeError(
                f"ROOT output is missing for stage={stage.name}, threshold={threshold}: "
                f"{out_root}"
            )

        try:
            metrics = _read_notebook_efficiency(out_root, args.treeName)
        except Exception as error:
            row.update({"status": "evaluation_failed", "error": str(error)})
            rows.append(row)
            persist("failed")
            raise

        noml_efficiency = float(baseline["trackEfficiency"])
        if noml_efficiency <= 0.0:
            row.update({
                "status": "evaluation_failed",
                "error": "no-ML baseline has zero track efficiency",
            })
            rows.append(row)
            persist("failed")
            raise RuntimeError(
                f"No-ML efficiency is zero in stage={stage.name}; cannot calculate "
                "relative efficiency."
            )

        relative_efficiency = float(metrics["trackEfficiency"]) / noml_efficiency
        row.update(metrics)
        row.update({
            "noMlTruthMuonCount": baseline["truthMuonCount"],
            "noMlMatchedTruthMuonCount": baseline["matchedTruthMuonCount"],
            "noMlTrackEfficiency": noml_efficiency,
            "truthCountMatchesNoMl": (
                int(metrics["truthMuonCount"]) == int(baseline["truthMuonCount"])
            ),
            "relativeTrackEfficiency": relative_efficiency,
            "relativeEfficiencyLoss": max(0.0, 1.0 - relative_efficiency),
            "passesTarget": relative_efficiency >= args.minRelativeEfficiency,
            "status": "ok",
        })
        rows.append(row)
        persist("running")
        print(
            f"[{stage.name:6s} | {stage.n_events:4d} events] "
            f"threshold={threshold: .6f}  "
            f"edge={metrics['trackEfficiency']:.6f} "
            f"({metrics['matchedTruthMuonCount']}/{metrics['truthMuonCount']})  "
            f"relative={relative_efficiency:.6f}  "
            f"{'PASS' if row['passesTarget'] else 'FAIL'}",
            flush=True,
        )
        return row

    def adaptive_scan(
        stage: ScanStage,
        start_threshold: float,
    ) -> tuple[dict[str, Any] | None, str]:
        """Find the highest passing point, with first-point recovery downward."""

        first = evaluate_edge(stage, start_threshold)
        if bool(first["passesTarget"]):
            best = first
            for threshold in _ascending_thresholds(
                start_threshold,
                args.maxThreshold,
                stage.step,
                include_start=False,
            ):
                candidate = evaluate_edge(stage, threshold)
                if not bool(candidate["passesTarget"]):
                    return best, "stopped_at_first_below_target"
                best = candidate
            return best, "reached_upper_threshold_bound"

        # At the larger sample, the previous stage's value may already be too
        # high. Walk down until the nearest passing value is recovered.
        for threshold in _descending_thresholds(
            start_threshold,
            args.minThreshold,
            stage.step,
            include_start=False,
        ):
            candidate = evaluate_edge(stage, threshold)
            if bool(candidate["passesTarget"]):
                return candidate, "recovered_by_lowering_threshold"

        return None, "no_passing_threshold_in_range"

    try:
        coarse, medium, fine = stages

        # The score cut is assumed to become no more permissive as it rises.
        # Therefore the coarse stage stops as soon as the first failing point is
        # seen; the last passing point is the seed for the 100-event scan.
        coarse_best: dict[str, Any] | None = None
        coarse_note = "reached_upper_threshold_bound"
        for threshold in _ascending_thresholds(
            args.minThreshold,
            args.maxThreshold,
            coarse.step,
        ):
            candidate = evaluate_edge(coarse, threshold)
            if not bool(candidate["passesTarget"]):
                coarse_note = "stopped_at_first_below_target"
                break
            coarse_best = candidate

        if coarse_best is None:
            best_by_stage[coarse.name] = None
            stage_notes[coarse.name] = "no_passing_threshold_in_range"
            persist("no_passing_threshold")
            raise RuntimeError(
                "No coarse edge threshold met the relative-efficiency target. "
                "Lower --minRelativeEfficiency or extend --minThreshold."
            )

        best_by_stage[coarse.name] = coarse_best
        stage_notes[coarse.name] = coarse_note
        persist("running")

        medium_best, medium_note = adaptive_scan(
            medium,
            float(coarse_best["threshold"]),
        )
        best_by_stage[medium.name] = medium_best
        stage_notes[medium.name] = medium_note
        if medium_best is None:
            persist("no_passing_threshold")
            raise RuntimeError(
                "No medium-stage edge threshold met the relative-efficiency target."
            )
        persist("running")

        fine_best, fine_note = adaptive_scan(
            fine,
            float(medium_best["threshold"]),
        )
        best_by_stage[fine.name] = fine_best
        stage_notes[fine.name] = fine_note
        if fine_best is None:
            persist("no_passing_threshold")
            raise RuntimeError(
                "No fine-stage edge threshold met the relative-efficiency target."
            )

    except KeyboardInterrupt:
        persist("interrupted")
        raise SystemExit(
            f"Interrupted. Partial results were saved under {work_dir}."
        )
    except Exception:
        persist("failed")
        raise

    persist("completed")
    final = summary("completed")
    print(f"\nRecommended --edgeThreshold: {final['recommendedThreshold']:.6f}")
    print(f"Scan CSV: {csv_path}")
    print(f"Summary:  {summary_path}")


if __name__ == "__main__":
    main()

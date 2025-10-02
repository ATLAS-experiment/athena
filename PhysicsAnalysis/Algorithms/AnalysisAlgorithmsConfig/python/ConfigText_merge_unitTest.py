#!/usr/bin/env python3
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nikita Pond
"""
Merge tests for multi-include semantics (no pytest), using neutral keys foo/bar.

Usage:
  python ConfigText_merge_unitTest.py --dir AnalysisAlgorithmsConfig
  python ConfigText_merge_unitTest.py --case all --dir path/to/yamls
"""

import argparse
import os
import pathlib
import sys
import traceback
import warnings
from AnalysisAlgorithmsConfig.ConfigText import combineConfigFiles, TextConfigWarning
import yaml 

def _load_yaml(p: pathlib.Path):
    with p.open("r") as f:
        return yaml.safe_load(f)

def _assert(cond, msg):
    if not cond:
        raise AssertionError(msg)

def _run_merge(top_cfg: dict, base_dir: pathlib.Path):
    # Round-trip copy for parity with production behavior
    top = yaml.safe_load(yaml.safe_dump(top_cfg))
    changed = combineConfigFiles(top, base_dir, fragment_key="include")
    _assert(changed is True, "combineConfigFiles reported no changes")
    _assert("include" not in top, "include key should be removed after merging")
    return top

def case_earlier_wins(base_dir: pathlib.Path):
    """
    Nested include: earlier wins inside test_merge_mid, local overrides afterward.
    """
    top_cfg = _load_yaml(base_dir / "test_merge_top.yaml")
    merged = _run_merge(top_cfg, base_dir)

    # From override.json (only there), should be present
    _assert(merged["a"] == 2, "Expected 'a' == 2 from override.json")

    # From override.json (new key) and local override
    _assert(merged["nested"]["y"] == 20, "Expected nested.y == 20 from override.json")
    _assert(merged["nested"]["x"] == 999, "Local override of nested.x should win")

    # Carry-through from frag_a (since mid includes frag_a first and no frag_b)
    _assert(abs(merged["bar"]["rate"] - 0.1) < 1e-12, "Expected bar.rate from frag_a (0.1)")
    _assert(merged["foo"]["alpha"] == 1 and merged["foo"]["beta"] == "A",
            "Expected foo.{alpha,beta} from frag_a")

    # Local override over fragments
    _assert(merged["bar"]["size"] == 32, "Local bar.size should override frag_a (32 vs 64)")

    # Presence of local-only key
    _assert(merged["local_only"] is True, "Local-only key should remain")

def case_earlier_wins_between_fragments(base_dir: pathlib.Path):
    """
    Direct include [frag_a, frag_b]: earlier (a) wins on scalar conflicts; lists concat.
    """
    top_cfg = {
        "include": ["test_merge_frag_a.yaml", "test_merge_frag_b.yaml"]
    }
    merged = _run_merge(top_cfg, base_dir)

    # Earlier wins on conflicts
    _assert(merged["foo"]["alpha"] == 1, "Earlier frag_a should win foo.alpha")
    _assert(merged["foo"]["beta"] == "A", "Earlier frag_a should win foo.beta")
    _assert(abs(merged["bar"]["rate"] - 0.1) < 1e-12, "Earlier frag_a should win bar.rate")

    # Values only in earlier remain
    _assert(merged["bar"]["size"] == 64, "bar.size should come from frag_a")

    # Lists concatenate
    _assert(merged["nums"] == [1, 2, 3], "Expected nums concatenation [1,2,3]")

    # Keys unique to later survive
    _assert(merged["only_in_b"] is True, "Key unique to later fragment should be present")

def case_list_concat(base_dir: pathlib.Path):
    """Lists concatenate without deduplication."""
    top_cfg = _load_yaml(base_dir / "test_merge_list_top.yaml")
    merged = _run_merge(top_cfg, base_dir)
    _assert(merged["items"] == [1, 2, 3], "items should concatenate")
    _assert(merged["dups"] == [1, 1, 1], "dups should concatenate without dedup")

def case_single_string_include(base_dir: pathlib.Path):
    """Single-string include path is supported, but should warn for deprecation."""
    top_cfg = _load_yaml(base_dir / "test_merge_single_top.yaml")

    # Capture warnings of the specific type
    with warnings.catch_warnings(record=True) as w:
        warnings.simplefilter("always", TextConfigWarning)  # ensure it’s recorded

        merged = _run_merge(top_cfg, base_dir)

        # Pull only the TextConfigWarning messages
        msgs = [
            str(rec.message) for rec in w
            if issubclass(rec.category, TextConfigWarning)
        ]

    # Assert that the expected deprecation message appeared
    _assert(
        any("should be followed with a list of files" in m for m in msgs),
        "Expected TextConfigWarning about include needing a list",
    )

    # Keep your existing assertions
    _assert(merged["foo"]["alpha"] == 1, "Single include should resolve frag_a (foo.alpha)")
    _assert(merged["only_in_a"] is True, "Value from frag_a should appear")

def run_case(name: str, base_dir: pathlib.Path):
    mapping = {
        "earlier_wins": case_earlier_wins,
        "earlier_wins_between_fragments": case_earlier_wins_between_fragments,
        "list_concat": case_list_concat,
        "single_string_include": case_single_string_include,
    }
    if name == "all":
        for k in mapping:
            print(f"→ {k} ... ", end="", flush=True)
            try:
                mapping[k](base_dir)
                print("OK")
            except Exception:
                print("FAIL")
                traceback.print_exc()
                sys.exit(1)
    else:
        if name not in mapping:
            sys.exit(f"Unknown case '{name}'. Choose from: all, " + ", ".join(mapping))
        print(f"→ {name} ... ", end="", flush=True)
        mapping[name](base_dir)
        print("OK")

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dir", dest="yamldir", required=True,
                    help="Directory where the test_merge_*.yaml/json files live")
    ap.add_argument("--case", default="all",
                    choices=[
                        "all", "earlier_wins", "earlier_wins_between_fragments", 
                        "list_concat", "single_string_include"],
                    help="Which case to run (default: all)")
    args = ap.parse_args()

    # Avoid DATAPATH interference (your _find_fragment should handle empty)
    os.environ["DATAPATH"] = ""
    from PathResolver import PathResolver
    base_dir = PathResolver.FindCalibDirectory(args.yamldir)
    base_dir = pathlib.Path(base_dir)
    if not base_dir.is_dir():
        sys.exit(f"--dir {base_dir} is not a directory")

    run_case(args.case, base_dir)

if __name__ == "__main__":
    main()
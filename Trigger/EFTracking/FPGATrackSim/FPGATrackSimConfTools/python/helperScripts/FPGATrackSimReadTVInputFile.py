#!/usr/bin/env python3
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""
FPGATrackSimReadTVInputFile.py

Validate that all EDM stages/trees and branches are accessible in the input ROOT file for all regions in the phi slice.
Prints basic info for each TTree and branch.

This script essentially follows the logic used in the bytestreammaker script, but without the bytestreammaker dependencies.

More in https://gitlab.cern.ch/atlas-tdaq-ph2upgrades/atlas-tdaq-eftracking/data-format-tools/bytestreammaker
and https://gitlab.cern.ch/atlas-tdaq-ph2upgrades/atlas-tdaq-eftracking/data-format-tools/eftrackingtestdatautility
"""
import sys
import ROOT

regions = [34,98,162,226,290,354,418,482,546,610,674,738,802,866,930,994,1058,1122,1186,1250]

def print_tree_info(tree, name, check_content=False, max_events=2):
    if not tree:
        print(f"  [!] Tree '{name}' not found.")
        return
    n_entries = tree.GetEntries()
    print(f"  Tree '{name}': {n_entries} entries")
    for branch in tree.GetListOfBranches():
        print(f"    Branch: {branch.GetName()} ({branch.GetClassName()})")
    if check_content and n_entries > 0:
        print(f"    [Checking up to {max_events} events for content...]")
        for i in range(min(n_entries, max_events)):
            tree.GetEntry(i)
            for branch in tree.GetListOfBranches():
                bname = branch.GetName()
                try:
                    obj = getattr(tree, bname, None)
                    if obj is not None:
                        if hasattr(obj, 'size'):
                            print(f"      Event {i}: {bname} size = {obj.size()}")
                        elif hasattr(obj, '__len__'):
                            print(f"      Event {i}: {bname} len = {len(obj)}")
                        else:
                            print(f"      Event {i}: {bname} type = {type(obj)}")
                except Exception as e:
                    print(f"      Event {i}: {bname} [error accessing: {e}]")

def check_event_header(header, label):
    try:
        # Try to access typical sub-objects/attributes
        if hasattr(header, 'getNHits'):
            print(f"      {label}: nHits = {header.getNHits()}")
        if hasattr(header, 'getNTracks'):
            print(f"      {label}: nTracks = {header.getNTracks()}")
        if hasattr(header, 'getNClusters'):
            print(f"      {label}: nClusters = {header.getNClusters()}")
        if hasattr(header, 'getNSpacepoints'):
            print(f"      {label}: nSpacepoints = {header.getNSpacepoints()}")
        # Try to access collections if present
        if hasattr(header, 'tracks'):
            print(f"      {label}: tracks len = {len(header.tracks)}")
        if hasattr(header, 'hits'):
            print(f"      {label}: hits len = {len(header.hits)}")
        if hasattr(header, 'clusters'):
            print(f"      {label}: clusters len = {len(header.clusters)}")
        if hasattr(header, 'spacepoints'):
            print(f"      {label}: spacepoints len = {len(header.spacepoints)}")
    except Exception as e:
        print(f"      {label}: [error accessing sub-objects: {e}]")

def check_tree_events(tree, branch_map, max_events=2):
    n_entries = tree.GetEntries()
    for i in range(min(n_entries, max_events)):
        tree.GetEntry(i)
        for bname, label in branch_map.items():
            try:
                header = getattr(tree, bname, None)
                if header is not None:
                    check_event_header(header, label)
            except Exception as e:
                print(f"      {label}: [error accessing: {e}]")

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <input_root_file>")
        sys.exit(1)
    input_file = sys.argv[1]
    f = ROOT.TFile.Open(input_file)
    if not f or f.IsZombie():
        print(f"[ERROR] Could not open file: {input_file}")
        sys.exit(2)
    print(f"Opened file: {input_file}\n")
    # Check main input tree
    tree = f.Get("FPGATrackSimDataPrepTree")
    print_tree_info(tree, "FPGATrackSimDataPrepTree", check_content=True)
    if tree:
        branch_map = {
            "LogicalEventInputHeader_PreCluster": "PreCluster",
            "LogicalEventInputHeader_PostCluster": "PostCluster"
        }
        check_tree_events(tree, branch_map)
    for region in regions:
        print(f"\n=== Region {region} ===")
        tree1 = f.Get(f"FPGATrackSimLogicalEventTree_reg{region}")
        print_tree_info(tree1, f"FPGATrackSimLogicalEventTree_reg{region}", check_content=True)
        if tree1:
            branch_map = {
                "LogicalEventStripHeader": "Strip",
                "LogicalEventSpacepointHeader": "Spacepoint",
                "LogicalEventFirstPixelHeader": "FirstPixel",
                "LogicalEventSecondPixelHeader": "SecondPixel",
                "LogicalEventOutputHeader": "Output"
            }
            check_tree_events(tree1, branch_map)
        tree2 = f.Get(f"FPGATrackSimSecondStageTree_reg{region}")
        print_tree_info(tree2, f"FPGATrackSimSecondStageTree_reg{region}", check_content=True)
        if tree2:
            branch_map = {
                "LogicalEventOutputHeader": "Output2nd",
                "LogicalEventSlicedHeader": "Sliced2nd"
            }
            check_tree_events(tree2, branch_map)
    f.Close()
    print("\nValidation complete.")

if __name__ == "__main__":
    main()

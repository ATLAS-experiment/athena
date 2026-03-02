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

VERBOSE = False # it should remain False as default for the CI


def get_regions():
    """Return the list of regions in the phi slice."""
    return [34, 98, 162, 226, 290, 354, 418, 482, 546, 610, 674, 738, 802, 866, 930, 994, 1058, 1122, 1186, 1250]


def print_tree_info(tree, name, check_content=False, max_events=None):
    if not tree:
        print(f"  [!] Tree '{name}' not found.")
        return
    n_entries = tree.GetEntries()
    print(f"  Tree '{name}': {n_entries} entries")
    for branch in tree.GetListOfBranches():
        print(f"    Branch: {branch.GetName()} ({branch.GetClassName()})")
    if check_content and n_entries > 0:
        limit = n_entries if max_events is None else min(n_entries, max_events)
        if VERBOSE:
            print(f"    [Checking {limit} events for content...]")
        for i in range(limit):
            tree.GetEntry(i)
            for branch in tree.GetListOfBranches():
                bname = branch.GetName()
                try:
                    getattr(tree, bname, None)
                except Exception as e:
                    raise RuntimeError(f"Event entry {i}: {bname} [error accessing: {e}]") from e


def get_event_number(tree):
    """Try to extract an event number from the tree's current entry."""
    for name in ['LogicalEventSlicedHeader',
                 'LogicalEventStripHeader',
                 'LogicalEventSpacepointHeader',
                 'LogicalEventFirstPixelHeader',
                 'LogicalEventSecondPixelHeader']:
        header = getattr(tree, name, None)
        if header is None:
            continue
        if hasattr(header, 'event'):
            try:
                evt = header.event()
                if hasattr(evt, 'eventNumber'):
                    return evt.eventNumber()
            except Exception:
                pass
        if hasattr(header, 'getEventNumber'):
            try:
                return header.getEventNumber()
            except Exception:
                pass
    return None


def iter_road_hits(road):
    """Yield every non-None hit from a single FPGATrackSimRoad."""
    for layerHits in road.getAllHits():
        for hit in layerHits:
            if hit is not None:
                yield hit



def summarize_roads_tracks(header, label, eventLabel, stage, only_nonzero=True):
    """Print summary of roads/tracks for a given stage ('1st' or '2nd')."""
    suffix = '2nd' if stage == '2nd' else '1st'
    roads = getattr(header, f'getFPGATrackSimRoads_{suffix}')()
    tracks = getattr(header, f'getFPGATrackSimTracks_{suffix}')()

    roadCount = len(roads)
    roadHitTotal, roadRealTotal = 0, 0
    for road_idx, road in enumerate(roads):
        for hit in iter_road_hits(road):
            roadHitTotal += 1
            try:
                if hit.isReal():
                    roadRealTotal += 1
            except Exception:
                pass
            if VERBOSE:
                try:
                    is_real = hit.isReal()
                except Exception:
                    is_real = False
                print(f"        Road {road_idx} hit: x={hit.getX():.4f}, y={hit.getY():.4f}, z={hit.getZ():.4f}, isReal={is_real}")
    if (not only_nonzero) or roadCount > 0:
        print(f"      Event {eventLabel}: {label}: roads={roadCount}, "
              f"road_hits={roadHitTotal}, road_real_hits={roadRealTotal}")

    trackCount = len(tracks)
    trackHitTotal, trackRealTotal = 0, 0
    for track_idx, track in enumerate(tracks):
        for hit in track.getFPGATrackSimHits():
            trackHitTotal += 1
            try:
                if hit.isReal():
                    trackRealTotal += 1
            except Exception:
                pass
            if VERBOSE:
                try:
                    is_real = hit.isReal()
                except Exception:
                    is_real = False
                print(f"        Track {track_idx} hit: x={hit.getX():.4f}, y={hit.getY():.4f}, z={hit.getZ():.4f}, isReal={is_real}")
    if (not only_nonzero) or trackCount > 0:
        print(f"      Event {eventLabel}: {label}: tracks={trackCount}, "
              f"track_hits={trackHitTotal}, track_real_hits={trackRealTotal}")


def check_tree_events(tree, branch_map, max_events=None):
    n_entries = tree.GetEntries()
    limit = n_entries if max_events is None else min(n_entries, max_events)
    for i in range(limit):
        tree.GetEntry(i)
        evtNum = get_event_number(tree)
        eventLabel = evtNum if evtNum is not None else f"entry {i}"
        for bname, info in branch_map.items():
            if isinstance(info, tuple):
                label, stage = info
            else:
                label, stage = info, None
            try:
                header = getattr(tree, bname, None)
                if header is not None and stage is not None:
                    summarize_roads_tracks(header, label, eventLabel, stage)
            except Exception as e:
                raise RuntimeError(f"Event {eventLabel}: {label} [error accessing: {e}]") from e


def find_events_with_data(tree, stage):
    """Return {event_number: first_hit} for events that have roads+tracks with accessible hits."""
    if not tree:
        return {}
    suffix = '2nd' if stage == '2nd' else '1st'
    events = {}
    for i in range(tree.GetEntries()):
        tree.GetEntry(i)
        evtNum = get_event_number(tree)
        if evtNum is None:
            continue
        header = getattr(tree, 'LogicalEventOutputHeader', None)
        if header is None:
            continue
        roads = getattr(header, f'getFPGATrackSimRoads_{suffix}')()
        tracks = getattr(header, f'getFPGATrackSimTracks_{suffix}')()
        if len(roads) == 0 or len(tracks) == 0:
            continue
        firstHit = next(
            (hit for road in roads for hit in iter_road_hits(road)),
            None
        )
        if firstHit is not None:
            events[evtNum] = firstHit
    return events


def main():
    if len(sys.argv) < 2:
        raise SystemExit(f"Usage: {sys.argv[0]} <input_root_file>")
    input_file = sys.argv[1]
    f = ROOT.TFile.Open(input_file)
    if not f or f.IsZombie():
        raise RuntimeError(f"Could not open file: {input_file}")
    print(f"Opened file: {input_file}\n")

    # Check DataPrep tree
    tree = f.Get("FPGATrackSimDataPrepTree")
    print_tree_info(tree, "FPGATrackSimDataPrepTree", check_content=True)
    if tree:
        for i in range(tree.GetEntries()):
            tree.GetEntry(i)
            for bname in ["LogicalEventInputHeader_PreCluster", "LogicalEventInputHeader_PostCluster"]:
                try:
                    header = getattr(tree, bname, None)
                    if header is not None:
                        header.event()
                        header.nTowers()
                except Exception as e:
                    raise RuntimeError(f"Event entry {i}: {bname} [error accessing: {e}]") from e

    test_passed = False

    # Check LogicalEvent trees
    for region in get_regions():
        print(f"\n=== Region {region} ===")
        tree1 = f.Get(f"FPGATrackSimLogicalEventTree_reg{region}")
        print_tree_info(tree1, f"FPGATrackSimLogicalEventTree_reg{region}", check_content=True)
        if tree1:
            check_tree_events(tree1, {
                "LogicalEventStripHeader": "Strip",
                "LogicalEventSpacepointHeader": "Spacepoint",
                "LogicalEventFirstPixelHeader": "FirstPixel",
                "LogicalEventSecondPixelHeader": "SecondPixel",
                "LogicalEventOutputHeader": ("Output", "1st")
            })
        tree2 = f.Get(f"FPGATrackSimSecondStageTree_reg{region}")
        print_tree_info(tree2, f"FPGATrackSimSecondStageTree_reg{region}", check_content=True)
        if tree2:
            check_tree_events(tree2, {
                "LogicalEventOutputHeader": ("Output2nd", "2nd"),
                "LogicalEventSlicedHeader": "Sliced2nd"
            })

        if not test_passed and tree1 and tree2:
            events_1st = find_events_with_data(tree1, "1st")
            events_2nd = find_events_with_data(tree2, "2nd")
            common_events = sorted(set(events_1st) & set(events_2nd))
            if common_events:
                evtNum = common_events[0]
                hit = events_2nd[evtNum]
                try:
                    is_real = hit.isReal()
                except Exception:
                    is_real = False
                print(
                    f"  [PASS] Region {region} event {evtNum}: "
                    f"first hit x={hit.getX()}, y={hit.getY()}, z={hit.getZ()}, isReal={is_real}"
                )
                test_passed = True

    f.Close()
    if not test_passed:
        raise RuntimeError(
            "No event found with both 1st and 2nd stage roads/tracks and accessible hits "
            "in the OutputHeader branch."
        )
    print("\nValidation complete.")

if __name__ == "__main__":
    main()

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
VERBOSE = False # it should remain False as default for the CI

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

def check_event_header(header, label):
    try:
        # Touch common members to validate access and avoid noisy printing...
        if hasattr(header, 'getNHits'):
            header.getNHits()
        if hasattr(header, 'getNTracks'):
            header.getNTracks()
        if hasattr(header, 'getNClusters'):
            header.getNClusters()
        if hasattr(header, 'getNSpacepoints'):
            header.getNSpacepoints()
        if hasattr(header, 'tracks'):
            len(header.tracks)
        if hasattr(header, 'hits'):
            len(header.hits)
        if hasattr(header, 'clusters'):
            len(header.clusters)
        if hasattr(header, 'spacepoints'):
            len(header.spacepoints)
    except Exception as e:
        raise RuntimeError(f"{label}: error accessing sub-objects: {e}") from e

def _try_get_event_number(header):
    if header is None:
        return None
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

def _get_event_id(tree):
    for name in [
        'LogicalEventSlicedHeader',
        'LogicalEventStripHeader',
        'LogicalEventSpacepointHeader',
        'LogicalEventFirstPixelHeader',
        'LogicalEventSecondPixelHeader'
    ]:
        header = getattr(tree, name, None)
        event_number = _try_get_event_number(header)
        if event_number is not None:
            return event_number
    return None

def _collection_len(obj):
    if obj is None:
        return 0
    if hasattr(obj, 'size'):
        return obj.size()
    try:
        return len(obj)
    except Exception:
        return 0

def _try_get_collection(obj, names):
    for name in names:
        try:
            attr = getattr(obj, name, None)
            if attr is None:
                continue
            if callable(attr):
                return attr()
            return attr
        except Exception:
            continue
    return None

def _iter_track_hits(track):
    hits = _try_get_collection(track, [
        'getFPGATrackSimHits',
        'hits'
    ])
    if hits is None:
        return []
    return hits

def _get_stage_collections(header, stage):
    if stage == "2nd":
        road_getters = [
            'getFPGATrackSimRoads_2nd',
            'getFPGATrackSimRoads_1st',
            'roads'
        ]
        track_getters = [
            'getFPGATrackSimTracks_2nd',
            'getFPGATrackSimTracks_1st',
            'tracks'
        ]
    else:
        road_getters = [
            'getFPGATrackSimRoads_1st',
            'getFPGATrackSimRoads_2nd',
            'roads'
        ]
        track_getters = [
            'getFPGATrackSimTracks_1st',
            'getFPGATrackSimTracks_2nd',
            'tracks'
        ]
    return _try_get_collection(header, road_getters), _try_get_collection(header, track_getters)

def _first_hit_from_roads(roads):
    for road in roads:
        if road is None:
            continue
        road_use = road
        try:
            road_use = ROOT.FPGATrackSimRoad(road)
        except Exception:
            road_use = road
        all_hits = None
        if hasattr(road_use, 'getAllHits'):
            try:
                all_hits = road_use.getAllHits()
            except Exception:
                all_hits = None
        if all_hits is not None:
            for layer_hits in all_hits:
                for hit in layer_hits:
                    if hit is not None:
                        return hit
        else:
            n_layers = 0
            if hasattr(road_use, 'getNLayers'):
                try:
                    n_layers = road_use.getNLayers()
                except Exception:
                    n_layers = 0
            for layer in range(n_layers):
                hits = None
                if hasattr(road_use, 'getHits'):
                    try:
                        hits = road_use.getHits(layer)
                    except Exception:
                        hits = None
                if hits is None:
                    continue
                for hit in hits:
                    if hit is not None:
                        return hit
    return None

def _summarize_roads_tracks(header, label, event_label, stage=None, only_nonzero=True):
    roads, tracks = _get_stage_collections(header, stage)
    if roads is not None:
        road_count = _collection_len(roads)
        road_hit_total = 0
        road_real_total = 0
        for road in roads:
            if road is None:
                continue
            road_use = road
            try:
                road_use = ROOT.FPGATrackSimRoad(road)
            except Exception:
                road_use = road
            # No need to repopulate hits anymore - persistent hits are directly accessible
            all_hits = None
            if hasattr(road_use, 'getAllHits'):
                try:
                    all_hits = road_use.getAllHits()
                except Exception:
                    all_hits = None
            if all_hits is not None:
                for layer_hits in all_hits:
                    for hit in layer_hits:
                        if hit is None:
                            continue
                        road_hit_total += 1
                        try:
                            if hit.isReal():
                                road_real_total += 1
                        except Exception:
                            continue
            else:
                n_layers = 0
                if hasattr(road_use, 'getNLayers'):
                    try:
                        n_layers = road_use.getNLayers()
                    except Exception:
                        n_layers = 0
                if n_layers <= 0:
                    continue
                for layer in range(n_layers):
                    hits = None
                    if hasattr(road_use, 'getHits'):
                        try:
                            hits = road_use.getHits(layer)
                        except Exception:
                            hits = None
                    if hits is None:
                        continue
                    for hit in hits:
                        if hit is None:
                            continue
                        road_hit_total += 1
                        try:
                            if hit.isReal():
                                road_real_total += 1
                        except Exception:
                            continue
        if (not only_nonzero) or road_count > 0:
            print(f"      Event {event_label}: {label}: roads={road_count}, road_hits={road_hit_total}, road_real_hits={road_real_total}")

    if tracks is not None:
        track_count = _collection_len(tracks)
        track_hit_total = 0
        track_real_total = 0
        for track in tracks:
            if track is None:
                continue
            hits = _iter_track_hits(track)
            for hit in hits:
                if hit is None:
                    continue
                track_hit_total += 1
                try:
                    if hit.isReal():
                        track_real_total += 1
                except Exception:
                    continue
        if (not only_nonzero) or track_count > 0:
            print(f"      Event {event_label}: {label}: tracks={track_count}, track_hits={track_hit_total}, track_real_hits={track_real_total}")

def check_tree_events(tree, branch_map, max_events=None):
    n_entries = tree.GetEntries()
    limit = n_entries if max_events is None else min(n_entries, max_events)
    for i in range(limit):
        tree.GetEntry(i)
        event_number = _get_event_id(tree)
        event_label = event_number if event_number is not None else f"entry {i}"
        for bname, info in branch_map.items():
            if isinstance(info, tuple):
                label, read_hits = info
            else:
                label, read_hits = info, False
            try:
                header = getattr(tree, bname, None)
                if header is not None:
                    check_event_header(header, label)
                    if read_hits:
                        stage = "2nd" if "2nd" in label else "1st"
                        _summarize_roads_tracks(header, label, event_label, stage=stage)
            except Exception as e:
                raise RuntimeError(f"Event {event_label}: {label} [error accessing: {e}]") from e

def _find_events_with_data(tree, stage):
    if not tree:
        return {}
    events = {}
    n_entries = tree.GetEntries()
    for i in range(n_entries):
        tree.GetEntry(i)
        event_number = _get_event_id(tree)
        if event_number is None:
            continue
        header = getattr(tree, 'LogicalEventOutputHeader', None)
        if header is None:
            continue
        roads, tracks = _get_stage_collections(header, stage)
        if roads is None or tracks is None:
            continue
        if _collection_len(roads) == 0 or _collection_len(tracks) == 0:
            continue
        first_hit = _first_hit_from_roads(roads)
        if first_hit is not None:
            events[event_number] = first_hit
    return events

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
    test_passed = False
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
                "LogicalEventOutputHeader": ("Output", True)
            }
            check_tree_events(tree1, branch_map)
        tree2 = f.Get(f"FPGATrackSimSecondStageTree_reg{region}")
        print_tree_info(tree2, f"FPGATrackSimSecondStageTree_reg{region}", check_content=True)
        if tree2:
            branch_map = {
                "LogicalEventOutputHeader": ("Output2nd", True),
                "LogicalEventSlicedHeader": "Sliced2nd"
            }
            check_tree_events(tree2, branch_map)

        if not test_passed and tree1 and tree2:
            events_1st = _find_events_with_data(tree1, "1st")
            events_2nd = _find_events_with_data(tree2, "2nd")
            common_events = sorted(set(events_1st.keys()) & set(events_2nd.keys()))
            if common_events:
                event_number = common_events[0]
                hit = events_2nd[event_number]
                try:
                    is_real = hit.isReal()
                except Exception:
                    is_real = False
                print(
                    f"  [PASS] Region {region} event {event_number}: "
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

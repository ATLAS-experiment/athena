#!/usr/bin/env python3
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import ROOT
import sys
import argparse
import numpy as np


def get_cluster_branches(use_itk_names):
    if use_itk_names:
        return [
            'ITkPixelClusters',
            'ITkStripClusters'
        ]
    return [
        'xAODPixelClustersFromFPGACluster',
        'xAODStripClustersFromFPGACluster'
    ]


def get_track_branch(use_itk_names):
    if use_itk_names:
        return 'InDetTrackParticles'
    return 'FPGATrackParticles'


def validate_expected_branches(tree, expected_branches):
    available_branches = {branch.GetName() for branch in tree.GetListOfBranches()}
    missing_branches = [branch for branch in expected_branches if branch not in available_branches]
    if missing_branches:
        raise KeyError(
            f"missing expected branches in CollectionTree: {', '.join(missing_branches)}"
        )


def main():
    parser = argparse.ArgumentParser(
        description="Validate AOD Output from FPGATrackSim."
    )
    parser.add_argument(
        "input_root_file",
        help="Path to the input ROOT file"
    )
    parser.add_argument(
        "--useITkNames",
        dest="use_itk_names",
        action="store_true",
        help="Validate ITk cluster and track container names instead of the default FPGA names"
    )
    args = parser.parse_args()

    input_root_file = args.input_root_file
    rootFile = ROOT.TFile.Open(input_root_file)
    if not rootFile or rootFile.IsZombie():
        print(f"Error: Could not open file {input_root_file}")
        sys.exit(1)

    tree = rootFile.Get("CollectionTree")
    if not tree:
        raise ValueError("could not find CollectionTree in input file")

    clustersToCheck = get_cluster_branches(args.use_itk_names)
    trackBranchToCheck = get_track_branch(args.use_itk_names)
    validate_expected_branches(tree, clustersToCheck + [trackBranchToCheck])

    # Work around for ATEAM-1000
    import cppyy.ll
    cppyy.ll.cast["xAOD::PixelClusterContainer_v1"](0)

    for branch in clustersToCheck:
        averageClustersPerEvent = np.mean([getattr(evt, branch).size() for evt in tree])
        if np.isclose(averageClustersPerEvent, 0):
            raise ValueError(f"no recorded clusters in {branch}")
        else:
            print(f"there are on average {averageClustersPerEvent} clusters per event in {branch}")

    averageNumberOfTracksPerEvent = np.mean([getattr(evt, trackBranchToCheck).size() for evt in tree])
    if np.isclose(averageNumberOfTracksPerEvent, 0):
        raise ValueError(f"no recorded tracks in {trackBranchToCheck}")
    else:
        print(f"there are on average {averageNumberOfTracksPerEvent} tracks per event in {trackBranchToCheck}")

if __name__ == "__main__":
    main()
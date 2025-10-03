#!/usr/bin/env python3
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import ROOT
import sys
import argparse
import numpy as np

def main():
    parser = argparse.ArgumentParser(
        description="Validate AOD Output from FPGATrackSim."
    )
    parser.add_argument(
        "input_root_file",
        help="Path to the input ROOT file"
    )
    args = parser.parse_args()

    input_root_file = args.input_root_file
    rootFile = ROOT.TFile.Open(input_root_file)
    if not rootFile or rootFile.IsZombie():
        print(f"Error: Could not open file {input_root_file}")
        sys.exit(1)

    clustersToCheck = [
        'xAODPixelClustersFromFPGACluster',
        'xAODStripClustersFromFPGACluster'
    ]
    tree = rootFile.Get("CollectionTree")

    # Work around for ATEAM-1000
    import cppyy.ll
    cppyy.ll.cast["xAOD::PixelClusterContainer_v1"](0)

    for branch in clustersToCheck:
        averageClustersPerEvent = np.mean([getattr(evt, branch).size() for evt in tree])
        if np.isclose(averageClustersPerEvent, 0):
            raise ValueError(f"no recorded clusters in {branch}")
        else:
            print(f"there are on average {averageClustersPerEvent} clusters per event in {branch}")

    averageNumberOfTracksPerEvent = np.mean([evt.FPGATrackParticles.size() for evt in tree])
    if np.isclose(averageNumberOfTracksPerEvent, 0):
        raise ValueError("no recorded tracks in FPGATrackparticles")
    else:
        print(f"there are on average {averageNumberOfTracksPerEvent} tracks per event in FPGATrackparticles")

if __name__ == "__main__":
    main()
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#!/usr/bin/env python3

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.TestDefaults import defaultTestFiles
from PyUtils import PoolFile
import argparse
import sys

def checkSorted(fileName, containerName, column):
    # RDataFrame reads both TTree and RNTuple with the same interface
    import ROOT
    import numpy as np
    values = ROOT.RDataFrame(containerName, fileName).AsNumpy([column])[column]
    assert len(values) > 0, f"{containerName} in {fileName} has no entries"
    assert np.all(values[:-1] <= values[1:]), f"{column} is not sorted: {values}"
    print(f"OK: {len(values)} entries of {column} are sorted")

def main():

    # Parse command-line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("--container-type", default="ROOTTREE",
                        choices=["ROOTTREE", "ROOTRNTUPLE"],
                        help="Value of flags.Output.DefaultContainerType")
    args = parser.parse_args()

    # Initialize configuration flags
    flags=initConfigFlags()
    flags.Input.Files=defaultTestFiles.AOD_RUN3_MC
    flags.Output.DefaultContainerType = args.container_type
    flags.lock()

    # Create the main services
    from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
    acc=MainEvgenServicesCfg(flags)

    # Set up PoolReadCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    # Set up PoolWriteCfg
    from AthenaPoolCnvSvc.PoolWriteConfig import PoolWriteCfg
    acc.merge(PoolWriteCfg(flags))
	
    # Set up SortedEventTagWriter
    outputFile = f"sorted-aod-{args.container_type}.pool.root"
    sortAttribute = "EventNumber"
    writer = CompFactory.SortedEventTagWriter(SortAttribute=sortAttribute,
                                              OutputFile=outputFile)
    acc.addEventAlgo(writer)

    # Run the event loop
    sc = acc.run()
    if sc.isFailure():
        sys.exit(1)

    # Verify that the output file contains the sorted event-tag rows
    containerName = PoolFile.PoolOpts.RNTupleNames.EventTag if "RNTUPLE" in args.container_type else PoolFile.PoolOpts.TTreeNames.EventTag
    checkSorted(outputFile, containerName, sortAttribute)

if __name__ == "__main__":
    main()

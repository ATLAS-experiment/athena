#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles
# from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from AthenaCommon.Constants import DEBUG, INFO

from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg

flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.RAW_RUN3 # this is completely dummy input to get event loop going
flags.Exec.MaxEvents = 10
flags.Exec.OutputLevel=INFO
flags.Concurrency.NumThreads=3
flags.Concurrency.NumOffloadThreads=3
flags.lock()
acc = MainServicesCfg(flags)

acc.merge(ByteStreamReadCfg(flags))


packInDet = CompFactory.BSPackagingTool("PackInDet", detectors=["PIXEL", "SCT"])
packCalo = CompFactory.BSPackagingTool("PackCalo", detectors=["LAR", "TILE"], )
packEI = CompFactory.EventInfoPackagingTool("PackEI")

# acc.addEventAlgo(CompFactory.AsyncgRPCComputeAlg("CompAlg1",
#     PackagingTool=packInDet,
#     OutputLevel=DEBUG ))

acc.addEventAlgo(CompFactory.AsyncgRPCComputeAlg("CompAlg2",
    PackagingTools=[packEI, packCalo],
    OutputLevel=DEBUG ))

# ------------------------------------------------------------
# 5. Run
# ------------------------------------------------------------
if __name__ == "__main__":
    sc = acc.run()

    # exit code handling
    import sys
    sys.exit(not sc.isSuccess())
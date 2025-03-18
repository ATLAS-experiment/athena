# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG

def FPGADataFormatToolCfg(flags, name='FPGADataFormatTool', **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault('name', name)
    acc.setPrivateTools(CompFactory.FPGADataFormatTool(**kwargs))
    return acc

def StripClusteringCfg(flags, name='FPGAStripClustering', **kwargs):
    acc = ComponentAccumulator()

    tool = acc.popToolsAndMerge(FPGADataFormatToolCfg(flags))

    kwargs.setdefault('name', name)
    kwargs.setdefault('xclbin', 'StripClustering.xclbin')  # Path to the strip clustering xclbin
    kwargs.setdefault('KernelName', 'strip_clustering_tool')  # Kernel name
    kwargs.setdefault('InputTV', '')  # Input TestVector file (set as needed)
    kwargs.setdefault('RefTV', '')  # Reference TestVector file (set as needed)
    kwargs.setdefault('FPGADataFormatTool', tool)

    acc.addEventAlgo(CompFactory.FPGAStripClustering(**kwargs))

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from InDetConfig.ITkTrackRecoConfig import ITkTrackRecoCfg

    # Initialize flags
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    flags.Input.Files = [
        "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"
    ]

    # Disable calo for this test
    flags.Detector.EnableCalo = False

    # Ensure xAOD space point and cluster containers are available
    flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint = True
    flags.Tracking.ITkMainPass.doAthenaToActsCluster = True

    # Acts configuration
    flags.Acts.doRotCorrection = False

    # Enable debug-level logging
    flags.Debug.DumpEvtStore = True
    flags.lock()

    # Main services configuration
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    cfg.getService("MessageSvc").debugLimit = 99999999

    # Pool read configuration
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    # Add truth information for MC data
    if flags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        cfg.merge(GEN_AOD2xAODCfg(flags))

    # Standard reco
    cfg.merge(ITkTrackRecoCfg(flags))


    # Add strip clustering configuration
    kwargs = {}
    kwargs["OutputLevel"] = DEBUG
    acc = StripClusteringCfg(flags, **kwargs)
    cfg.merge(acc)

    # Run the configuration for 1 event
    cfg.run(1)
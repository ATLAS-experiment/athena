# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG

def FPGADataFormatToolCfg(flags, name = 'FPGADataFormatTool', **kwarg):

    acc = ComponentAccumulator()

    kwarg.setdefault('name', name)
    acc.setPrivateTools(CompFactory.FPGADataFormatTool(**kwarg))

    return acc

def PixelClusteringCfg(flags, name = 'PixelClustering', **kwarg):
    acc=ComponentAccumulator()

    tool = acc.popToolsAndMerge(FPGADataFormatToolCfg(flags))

    kwarg.setdefault('name', name)
    kwarg.setdefault('xclbin', 'F110.hw.xclbin')
    kwarg.setdefault('KernelName', 'pixel_clustering_tool')
    kwarg.setdefault('InputTV', '')
    kwarg.setdefault('RefTV', '')
    kwarg.setdefault('FPGADataFormatTool', tool)

    acc.addEventAlgo(CompFactory.PixelClustering(**kwarg))

    return acc

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from InDetConfig.ITkTrackRecoConfig import ITkTrackRecoCfg

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    #flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33675668._000016.pool.root.1"]
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"]

    # Disable calo for this test
    flags.Detector.EnableCalo = False

    # ensure that the xAOD SP and cluster containers are available
    flags.Tracking.ITkMainPass.doAthenaToActsSpacePoint=True
    flags.Tracking.ITkMainPass.doAthenaToActsCluster=True

    flags.Acts.doRotCorrection = False

    flags.Debug.DumpEvtStore = True
    flags.lock()
    flags = flags.cloneAndReplace("Tracking.ActiveConfig","Tracking.MainPass")

    # Main services
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(flags)
    cfg.getService("MessageSvc").debugLimit = 99999999

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    from EFTrackingFPGAIntegration.FPGAOutputValidationConfig import FPGAOutputValidationCfg
    cfg.merge(FPGAOutputValidationCfg(flags, **{
        "pixelKeys": ["fpgaPixelClusters", "ITkPixelClusters"],
    }))

    #Truth
    if flags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        cfg.merge(GEN_AOD2xAODCfg(flags))

    # Standard reco
    cfg.merge(ITkTrackRecoCfg(flags))
    
    kwarg = {}
    kwarg["OutputLevel"] = DEBUG

    acc=PixelClusteringCfg(flags, **kwarg)
    cfg.merge(acc)

    cfg.run(1)

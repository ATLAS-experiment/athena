# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Constants import DEBUG

def FPGADataFormatToolCfg(flags, name = 'FPGADataFormatTool', **kwarg):
    
    acc = ComponentAccumulator()
    
    kwarg.setdefault('name', name)
    acc.setPrivateTools(CompFactory.FPGADataFormatTool(**kwarg))

    return acc

def FPGATestVectorToolCfg(flags, name = 'FPGATestVectorTool', **kwarg):
    
    acc = ComponentAccumulator()
    
    kwarg.setdefault('name', name)
    acc.setPrivateTools(CompFactory.TestVectorTool(**kwarg))

    return acc

def FPGAOutputConversionToolCfg(flags, name = 'FPGAOutputConversionTool', **kwarg):
    
    acc = ComponentAccumulator()
    
    kwarg.setdefault('name', name)
    acc.setPrivateTools(CompFactory.OutputConversionTool(**kwarg))

    return acc

def xAODContainerMakerCfg(flags, name = 'xAODContainerMaker', **kwarg):
    
    acc = ComponentAccumulator()
    
    kwarg.setdefault('name', name)
    kwarg.setdefault('OutputStripName', 'FPGAStripClusters')
    kwarg.setdefault('OutputPixelName', 'FPGAPixelClusters')    
    # Spacepoints below will be further refined when the full pass-through kernel is ready
    kwarg.setdefault('OutputStripSpacePointName', 'PlaceHolderStripSpacePoints') 
    kwarg.setdefault('OutputPixelSpacePointName', 'PlaceHolderPixelSpacePoints')
    
    acc.setPrivateTools(CompFactory.xAODContainerMaker(**kwarg))
    return acc

def FPGAFormatterPrepCfg(flags, name = "FPGAFormatterPrep", **kwarg):

    acc = ComponentAccumulator()
    
    tool = acc.popToolsAndMerge(FPGADataFormatToolCfg(flags))
    tvTool = acc.popToolsAndMerge(FPGATestVectorToolCfg(flags))
    outputTool = acc.popToolsAndMerge(FPGAOutputConversionToolCfg(flags))
    containerMakerTool = acc.popToolsAndMerge(xAODContainerMakerCfg(flags))
    
    kwarg.setdefault('name', name)
    kwarg.setdefault('FPGADataFormatTool', tool)
    kwarg.setdefault('TestVectorTool', tvTool)
    kwarg.setdefault('OutputConversionTool', outputTool)
    kwarg.setdefault('xAODContainerMaker', containerMakerTool)
    
    kwarg.setdefault('PixelEDMRefTV', '')
    kwarg.setdefault('StripEDMRefTV', '')
    kwarg.setdefault('SpacePointRefTV', '')

    acc.addEventAlgo(CompFactory.FPGADataFormatAlg(**kwarg))
    return acc

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from InDetConfig.ITkTrackRecoConfig import ITkTrackRecoCfg

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    # Use a dummy input file for the EventInfo
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33675668._000016.pool.root.1"]

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
    top_acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(flags))

    #Truth
    if flags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        top_acc.merge(GEN_AOD2xAODCfg(flags))

    # Standard reco
    top_acc.merge(ITkTrackRecoCfg(flags))
    
    kwarg = {}
    kwarg["OutputLevel"] = DEBUG

    acc = FPGAFormatterPrepCfg(flags, **kwarg)
    top_acc.merge(acc)

    top_acc.run(1)

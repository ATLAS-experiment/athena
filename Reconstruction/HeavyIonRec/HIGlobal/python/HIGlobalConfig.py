# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from OutputStreamAthenaPool.OutputStreamConfig import addToAOD, addToESD

def HITowerWeightToolCfg(flags, name="WeightTool", **kwargs):
    """Configures HITowerWeightTool"""
    acc = ComponentAccumulator()

    if "InputFile" not in kwargs:
        from HIJetRec.HIJetRecUtilsCA import getHIClusterGeoWeightFile
        kwargs.setdefault("InputFile", getHIClusterGeoWeightFile(flags))
    kwargs.setdefault("ApplyCorrection", flags.HeavyIon.Jet.ApplyTowerEtaPhiCorrection)
    kwargs.setdefault("ConfigDir", 'HIJetCorrection/')

    acc.setPrivateTools(CompFactory.HITowerWeightTool(name, **kwargs))
    return acc

def HIEventShapeMapToolCfg(flags, name="HIEventShapeMapTool", **kwargs):
    """Configures HIEventShapeMapTool"""
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.HIEventShapeMapTool(name, **kwargs))
    return acc

def HIEventShapeFillerToolCfg(flags, name="HIEventShapeFillerTool", **kwargs):
    """Configures HIEventShapeFillerTool"""
    acc = ComponentAccumulator()
    if "EventShapeMapTool" not in kwargs:
        eventShapeMapTool = acc.popToolsAndMerge(HIEventShapeMapToolCfg(flags, name="HIEventShapeMapTool"))
        kwargs.setdefault("EventShapeMapTool", eventShapeMapTool)
    kwargs.setdefault("UseClusters", False)
    kwargs.setdefault("CaloCellContainerKey", "AllCalo")
    if kwargs["UseClusters"]:
        #Add weight tool to filler tool
        TWTool=acc.popToolsAndMerge(HITowerWeightToolCfg(flags, name="WeightTool"))
        kwargs.setdefault("TowerWeightTool", TWTool)

    acc.setPrivateTools(CompFactory.HIEventShapeFillerTool(name, **kwargs))
    return acc

def HIEventShapeMakerCfg(flags, name="HIEventShapeMaker", doWeighted=False, **kwargs):
    """Configures HIEventShapeMaker, either with weights (for HIJets) or without weight (for HIGlobal)"""
    acc = ComponentAccumulator()
    
    # merge dependencies
    from CaloRec.CaloRecoConfig import CaloRecoCfg  
    acc.merge(CaloRecoCfg(flags))
    from CaloRec.CaloTowerMakerConfig import CaloTowerMakerCfg
    towerMaker = acc.getPrimaryAndMerge(CaloTowerMakerCfg(flags))

    kwargs.setdefault("NaviTowerKey", towerMaker.TowerContainerName)
    kwargs.setdefault("InputTowerKey", "")
    kwargs.setdefault("OutputContainerKey", "HIEventShape")
    if "HIEventShapeFillerTool" not in kwargs:
        name_esft="HIEventShapeFillerTool_Weighted" if doWeighted else "HIEventShapeFillerTool"
        eventShapeTool = acc.popToolsAndMerge(HIEventShapeFillerToolCfg(flags, 
                                                                        name=name_esft,
                                                                        UseClusters=doWeighted))
        kwargs.setdefault("HIEventShapeFillerTool",eventShapeTool)

    acc.addEventAlgo(CompFactory.HIEventShapeMaker(name, **kwargs))
    return acc

def HIEventShapeSummaryToolCfg(flags, name="HIEventShapeSummaryTool", **kwargs):
    """Configures HIEventShapeSummaryTool"""
    acc = ComponentAccumulator()

    # TODO configure MBTS &FWD conversion once available
    kwargs.setdefault("SubCalos", ['FCal','EMCal','HCal','ALL'])
    kwargs.setdefault("Samplings", ['FCAL0','FCAL1','FCAL2'])
    kwargs.setdefault("DoPositiveNegativeSides", False)

    acc.setPrivateTools(CompFactory.HIEventShapeSummaryTool(name, **kwargs))
    return acc

def HIGlobalRecCfg(flags):
    """Configures Heavy Ion Global quantities """
    acc = ComponentAccumulator()

    shapeKey=flags.HeavyIon.Global.EventShape
    output = [ f"xAOD::HIEventShapeContainer#{shapeKey}", f"xAOD::HIEventShapeAuxContainer#{shapeKey}Aux."]

    kwargs_hies=dict()
    if flags.HeavyIon.Global.doEventShapeSummary:
        summaryKey = "CaloSums"
        summaryTool = acc.popToolsAndMerge(HIEventShapeSummaryToolCfg(flags))
        kwargs_hies["SummaryTool"] = summaryTool
        kwargs_hies["SummaryContainerKey"] = summaryKey
        output.extend([ f"xAOD::HIEventShapeContainer#{summaryKey}", f"xAOD::HIEventShapeAuxContainer#{summaryKey}Aux."])

    acc.merge(HIEventShapeMakerCfg(flags, **kwargs_hies))  

    acc.merge(addToESD(flags, output))
    acc.merge(addToAOD(flags, output))

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags
    flags = initConfigFlags()
    flags.Input.Files = [defaultTestFiles.d + "/RecJobTransformTests/data18_hi.00367384.physics_HardProbes.daq.RAW._lb0145._SFO-8._0001.data"]
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Exec.MaxEvents=5
    flags.Concurrency.NumThreads=1

    flags.fillFromArgs() # enable unit tests to switch only parts of reco: python -m HIRecConfig.HIRecConfig HeavyIon.doGlobal = 0 and so on
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
    acc.merge(ByteStreamReadCfg(flags))

    acc.merge(HIGlobalRecCfg(flags))
    from AthenaCommon.Constants import DEBUG
    acc.getEventAlgo("HIEventShapeMaker").OutputLevel=DEBUG
    
    acc.printConfig(withDetails=True, summariseProps=True)
    flags.dump()

    import sys
    sys.exit(acc.run().isFailure())

"""ComponentAccumulator configuration utilities for LArRecUtils

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging
from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
from AthenaConfiguration.Enums import LHCPeriod

def LArMCSymCondAlgCfg(flags, name="LArMCSymCondAlg", **kwargs):
    """Return ComponentAccumulator with configured LArMCSymCondAlg"""
    acc = ComponentAccumulator()
    kwargs.setdefault("ReadKey", "LArOnOffIdMap")
    acc.addCondAlgo(CompFactory.LArMCSymCondAlg(name, **kwargs))
    return acc


def LArAutoCorrNoiseCondAlgCfg(flags, name="LArAutoCorrNoiseCondAlg", **kwargs):
    """Return ComponentAccumulator with configured LArAutoCorrNoiseCondAlg"""

    LArAutoCorrNoiseCondAlg=CompFactory.LArAutoCorrNoiseCondAlg
    # The LArAutoCorrNoiseCondAlgCfg needs the cabling, the sym-object and the AutoCorr
    acc = LArOnOffIdMappingCfg(flags)
    acc.merge(LArMCSymCondAlgCfg(flags))
    acc.merge(LArElecCalibDBCfg(flags,["AutoCorr",]))
    kwargs.setdefault("nSampl", flags.LAr.ROD.nSamples)
    acc.addCondAlgo(LArAutoCorrNoiseCondAlg(name, **kwargs))
    return acc


def LArOFCCondAlgCfg (flags, name = 'LArOFCCondAlg', **kwargs):

    mlog = logging.getLogger ('LArOFCCondAlgCfg')
    mlog.info(" entering LArOFCCondAlgCfg")

    kwargs.setdefault ('isMC', True)
    kwargs.setdefault ('firstSample', flags.LAr.ROD.FirstSample)
    kwargs.setdefault ('useHighestGainAutoCorr', flags.LAr.ROD.UseHighestGainAutoCorr)
    kwargs.setdefault('Nminbias',flags.LAr.ROD.NumberOfCollisions)
    mlog.info("Setup LArOFCCOndAlg Nminbias %f ", flags.LAr.ROD.NumberOfCollisions)
    #The LArPileUpTool needs: Calbling, Shape, Noise, Pedestal and the (total) AutoCorr
    acc = LArOnOffIdMappingCfg(flags)
    requiredConditons=["Shape","Noise","Pedestal"]
    acc.merge(LArElecCalibDBCfg(flags,requiredConditons))
    acc.merge(LArAutoCorrTotalCondAlgCfg(flags))
    LArOFCCondAlg=CompFactory.LArOFCCondAlg
    acc.addCondAlgo (LArOFCCondAlg (name, **kwargs))
    return acc


def LArOFCSCCondAlgCfg (flags, name = 'LArOFCSCCondAlg', **kwargs):

    mlog = logging.getLogger ('LArOFCSCCondAlgCfg')
    mlog.info(" entering LArOFCSCCondAlgCfg")

    kwargs.setdefault ('isMC', True)
    kwargs.setdefault ('isSuperCell', True)
    kwargs.setdefault ('firstSample', flags.LAr.ROD.FirstSample)
    kwargs.setdefault ('useHighestGainAutoCorr', flags.LAr.ROD.UseHighestGainAutoCorr)
    kwargs.setdefault ('NGains',1) #SuperCells have only one gain
    from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
    acc = LArOnOffIdMappingSCCfg(flags)
    kwargs.setdefault("LArOnOffIdMappingObjKey", 'LArOnOffIdMapSC') # Provided by LArOnOffMappingAlgSC
    requiredConditions=["ShapeSC","PedestalSC","NoiseSC"]
    from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBMCSCCfg
    acc.merge(LArElecCalibDBMCSCCfg(flags,requiredConditions))
    kwargs.setdefault("LArShapeObjKey", 'LArShapeSC') # Provided by LArFlatConditionsAlg<LArShapeSC>
    kwargs.setdefault("LArNoiseObjKey", 'LArNoiseSC') # Provided by LArFlatConditionsAlg<LArNoiseSC>
    kwargs.setdefault("LArPedestalObjKey", 'LArPedestalSC') # Provided by LArFlatConditionsAlg<LArPedestalSC>

    acc.merge(LArAutoCorrTotalSCCondAlgCfg(flags))
    kwargs.setdefault("LArAutoCorrTotalObjKey", 'LArAutoCorrTotalSC') # Provided by LArAutoCorrTotalSCCondAlg
    kwargs.setdefault("LArOFCObjKey", 'LArOFCSC') # Output

    kwargs.setdefault('Nminbias',flags.LAr.ROD.NumberOfCollisions)
    mlog.info("Setup LArOFCSCCOndAlg Nminbias %f ", flags.LAr.ROD.NumberOfCollisions)

    acc.addCondAlgo (CompFactory.LArOFCCondAlg (name, **kwargs))
    return acc


def LArAutoCorrTotalCondAlgCfg (flags, name = 'LArAutoCorrTotalCondAlg', **kwargs):
    mlog = logging.getLogger ('LArAutoCorrTotalCondAlgCfg')
    mlog.info(" entering LArAutoCorrTotalCondAlgCfg")
    from AthenaCommon.SystemOfUnits import ns


    kwargs.setdefault('NGains', 2 if flags.GeoModel.Run==LHCPeriod.Run4 and flags.IOVDb.UseCREST else 3)
    kwargs.setdefault('Nsamples', flags.LAr.ROD.nSamples)
    kwargs.setdefault('firstSample',flags.LAr.ROD.FirstSample)
    mlog.info("Nsamples %d",flags.LAr.ROD.nSamples)
    mlog.info("firstSample %d",flags.LAr.ROD.FirstSample)
    deltaBunch = int(flags.Beam.BunchSpacing/( 25.*ns)+0.5)
    mlog.info("DeltaBunch %d " , deltaBunch)
    kwargs.setdefault('deltaBunch',deltaBunch)

    if flags.LAr.ROD.NumberOfCollisions <= 0:
        kwargs.setdefault("NoPileUp", True)
        mlog.info(" no pileup noise in LArAutoCorrTotal ")
    else:
        kwargs.setdefault("NoPileUp", False)

    #The LArAutoCorrTotalAlg needs cabling and
    #Shape, AutoCorr, Noise, Pedestal, fSampl and MinBias
    acc = LArOnOffIdMappingCfg(flags)
    requiredConditons=["Shape","AutoCorr","Noise","Pedestal","fSampl","MinBias"]
    acc.merge(LArElecCalibDBCfg(flags,requiredConditons))
    LArAutoCorrTotalCondAlg=CompFactory.LArAutoCorrTotalCondAlg
    acc.addCondAlgo (LArAutoCorrTotalCondAlg (name, **kwargs))
    return acc


def LArAutoCorrTotalSCCondAlgCfg (flags, name = 'LArAutoCorrTotalSCCondAlg', **kwargs):
    mlog = logging.getLogger ('LArAutoCorrTotalSCCondAlgCfg')
    mlog.info(" entering LArAutoCorrTotalSCCondAlgCfg")
    from AthenaCommon.SystemOfUnits import ns

    acc = LArOnOffIdMappingCfg(flags)
    kwargs.setdefault("LArOnOffIdMappingObjKey", 'LArOnOffIdMapSC') # Provided by LArOnOffMappingAlgSC

    from LArRecUtils.LArADC2MeVSCCondAlgConfig import LArADC2MeVSCCondAlgCfg
    acc.merge(LArADC2MeVSCCondAlgCfg(flags))
    kwargs.setdefault("LArADC2MeVObjKey", 'LArADC2MeVSC') # Provided by LArADC2MeVSCCondAlg

    requiredConditons=["ShapeSC","AutoCorrSC","NoiseSC","PedestalSC","fSamplSC","MinBiasSC"]
    from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBMCSCCfg
    acc.merge(LArElecCalibDBMCSCCfg(flags,requiredConditons))
    kwargs.setdefault("LArShapeObjKey", 'LArShapeSC') # Provided by LArFlatConditionsAlg<LArShapeSC>
    kwargs.setdefault("LArAutoCorrObjKey", 'LArAutoCorrSC')# Provided by LArFlatConditionsAlg<LArAutoCorrSC>
    kwargs.setdefault("LArNoiseObjKey", 'LArNoiseSC') # Provided by LArFlatConditionsAlg<LArNoiseSC>
    kwargs.setdefault("LArPedestalObjKey", 'LArPedestalSC') # Provided by LArFlatConditionsAlg<LArPedestalSC>
    kwargs.setdefault("LArfSamplObjKey", 'LArfSamplSC') # Provided by LArFlatConditionsAlg<LArfSamplSC>
    kwargs.setdefault("LArMinBiasObjKey", 'LArMinBiasSC') # Provided by LArFlatConditionsAlg<LArMinBiasSC>

    kwargs.setdefault("LArAutoCorrTotalObjKey", 'LArAutoCorrTotalSC') # Output

    kwargs.setdefault("isSuperCell", True)
    kwargs.setdefault('Nsamples', flags.LAr.ROD.nSamples)
    kwargs.setdefault('firstSample',flags.LAr.ROD.FirstSample)
    mlog.info("Nsamples %d",flags.LAr.ROD.nSamples)
    mlog.info("firstSample %d",flags.LAr.ROD.FirstSample)
    deltaBunch = int(flags.Beam.BunchSpacing/( 25.*ns)+0.5)
    mlog.info("DeltaBunch %d " , deltaBunch)
    kwargs.setdefault('deltaBunch',deltaBunch)


    if flags.LAr.ROD.NumberOfCollisions <= 0:
        kwargs.setdefault("NoPileUp", True)
        mlog.info(" no pileup noise in LArAutoCorrTotal ")
    else:
        kwargs.setdefault("NoPileUp", False)

    LArAutoCorrTotalCondAlg=CompFactory.LArAutoCorrTotalCondAlg
    acc.addCondAlgo (LArAutoCorrTotalCondAlg (name, **kwargs))
    return acc


def LArRoIMapCondAlgCfg (flags, name = 'LArRoIMapCondAlg', **kwargs):
    acc = ComponentAccumulator()

    from LArCabling.LArCablingConfig import LArFebRodMappingCfg, LArOnOffIdMappingCfg
    acc.merge (LArFebRodMappingCfg (flags))
    acc.merge (LArOnOffIdMappingCfg (flags))

    from CaloConditions.CaloConditionsConfig import LArTTCellMapCfg, CaloTTIdMapCfg
    acc.merge(LArTTCellMapCfg(flags))
    acc.merge(CaloTTIdMapCfg(flags))

    CaloTriggerTowerService = CompFactory.CaloTriggerTowerService # CaloTriggerTool
    kwargs.setdefault ('TriggerTowerSvc', CaloTriggerTowerService())
    
    LArRoIMapCondAlg = CompFactory.LArRoIMapCondAlg
    acc.addCondAlgo (LArRoIMapCondAlg (name, **kwargs))
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    print ('--- LArOFCCondAlg 1')
    flags1 = initConfigFlags()
    flags1.LAr.ROD.NumberOfCollisions=0
    flags1.Input.Files = defaultTestFiles.RDO_RUN2
    flags1.lock()
    acc1 = LArOFCCondAlgCfg (flags1)
    acc1.printCondAlgs(summariseProps=True)
    acc1.wasMerged()

    print ('--- LArAutoCorrTotalCondAlg')
    flags4 = initConfigFlags()
    flags4.LAr.ROD.NumberOfCollisions=0
    flags4.Input.Files = defaultTestFiles.RDO_RUN2
    flags4.LAr.ROD.nSamples = 32
    flags4.lock()
    acc4 = LArAutoCorrTotalCondAlgCfg (flags4)
    acc4.printCondAlgs(summariseProps=True)
    acc4.wasMerged()

    print ('--- LArRoIMapCondAlg')
    flags5 = initConfigFlags()
    flags5.LAr.ROD.NumberOfCollisions=0
    flags5.Input.Files = defaultTestFiles.RDO_RUN2
    flags5.lock()
    acc5 = LArRoIMapCondAlgCfg (flags5)
    acc5.printCondAlgs(summariseProps=True)
    acc5.wasMerged()

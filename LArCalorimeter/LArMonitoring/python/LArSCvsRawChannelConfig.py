# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
from LArByteStream.LArRawSCDataReadingConfig import LArRawSCDataReadingCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg, LArOnOffIdMappingSCCfg


def LArSCvsRawChannelMonAlgCfg(flags):
    acc=ComponentAccumulator()    
    from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg
    acc.merge(LArBadChannelCfg(flags))
    acc.merge(LArBadChannelCfg(flags,isSC=True))
    
    acc.merge(LArOnOffIdMappingCfg(flags))
    acc.merge(LArOnOffIdMappingSCCfg(flags))
    acc.merge(LArRawSCDataReadingCfg(flags))
    
    from CaloRec.CaloBCIDAvgAlgConfig import CaloBCIDAvgAlgCfg
    acc.merge(CaloBCIDAvgAlgCfg(flags))

    if flags.Input.isMC is False and not flags.Common.isOnline:
       from LumiBlockComps.LuminosityCondAlgConfig import  LuminosityCondAlgCfg
       acc.merge(LuminosityCondAlgCfg(flags))
       from LumiBlockComps.LBDurationCondAlgConfig import  LBDurationCondAlgCfg
       acc.merge(LBDurationCondAlgCfg(flags))

    from AthenaMonitoring.AthMonitorCfgHelper import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'LArSuperCellMonAlgCfg')
    acc.merge(LArSCvsRawChannelMonConfigCore(helper, flags))

    return acc


def LArSCvsRawChannelMonConfigCore(helper, flags, algname="LArSCvsRawChannelMon"):


    alg= helper.addAlgorithm(CompFactory.LArSCvsRawChannelMonAlg,algname)
    

    GroupName="LArSCvsRawGroup"
    alg.MonGroupName = GroupName
    alg.SCEnergyCut=90
    alg.ProblemsToMask=["deadReadout","deadPhys"]
    cellMonGroup = helper.addGroup(alg,GroupName,'/LAr/LArSuperCellMon_RC')

    for pName in alg.PartitionNames:
        cellMonGroup.defineHistogram(f"SCEne_{pName},eneSum_{pName};h_SCEne_vs_RawChannelEne_{pName}",
                                 title=f'Super Cell energy vs sum of RawChannel energies ({pName}) ;SC [MeV]; Sum [MeV]',
                                 type='TH2F', path="",
                                 xbins = 100, xmin=0,xmax=50000,
                                 ybins = 100, ymin=0,ymax=50000)
    
    return helper.result()
    
if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags=initConfigFlags()
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    #from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.LAr.doAlign=False
    flags.Input.Files = ["data24_13p6TeV.00481893.physics_Main.daq.RAW._lb1058._SFO-17._0002.data",]
    flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN3
    flags.Output.HISTFileName = 'LArSuperCellvsRC.root'
    flags.Exec.FPE=-1
    flags.fillFromArgs()
    flags.lock()

    acc = MainServicesCfg( flags )
    acc.merge(LArRawDataReadingCfg(flags))

    acc.merge(LArSCvsRawChannelMonAlgCfg(flags))
    acc.getService("AvalancheSchedulerSvc").ShowDataDependencies=True
    alg=acc.getEventAlgo("LArSCvsRawChannelMon")
    alg.EnableLumi=False
    alg.TrigDecisionTool=""
    alg.WarnOffenders=True
    acc.run()

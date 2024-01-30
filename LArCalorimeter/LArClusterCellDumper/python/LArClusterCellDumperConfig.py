# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
"""Define functions for LAr Cluster Cell Dumper with ComponentAccumulator"""

# utilities
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging
from AthenaConfiguration.Enums import LHCPeriod

def EventReaderAlgCfg(flags, name="EventReaderAlgCfg", **kwargs):
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    from IOVDbSvc.CondDB import conddb

    acc = ComponentAccumulator()

    isMC = flags.Input.isMC

    # -- Dump variables Selection -- 
    kwargs.setdefault("doAssocTopoCluster711Dump", False)       # dump the 7x11 window built from the hottest cell in an electron cluster.
    kwargs.setdefault("doClusterDump", False)       # Dump only a cluster container. (override the electron cluster)
    kwargs.setdefault("doPhotonDump", False)       # Perform a photon particle dump based on offline Photons Container.
    kwargs.setdefault("doTruthEventDump", isMC)        # Dump the Truth Event variables.
    kwargs.setdefault("doTruthPartDump",  isMC)        # Perform a truth particle dump.
    kwargs.setdefault("doLArEMBHitsDump", isMC) # Dump MC Hits for LAr EMB partition.

    kwargs.setdefault("skipEmptyEvents", True) # If true, do not fill the event that has no reco electrons. 
    kwargs.setdefault("noBadCells", False)  # If True, skip the cells tagged as badCells/channels.
    kwargs.setdefault("printCellsClus", False) # Debugging

    # Electrons and crosstalk studies
    kwargs.setdefault("doTagAndProbe", True)  # select by tag and probe method, electron pairs (start the chain of selection: track + T&P)
    kwargs.setdefault("doElecSelectByTrackOnly", True)  # select only single electrons which pass track criteria (only track)
    kwargs.setdefault("getAssociatedTopoCluster", True)  # Get the topo cluster associated to a super cluster, which was linked to an Electron
    kwargs.setdefault("getLArCalibConstants", True)  # Get the LAr calorimeter calibration constants, related to cells energy and time (online and offline).
    kwargs.setdefault("etMinProbe", 15)    # Min electron Pt value for Zee probe selection loose (GeV).
    kwargs.setdefault("etMinTag", 15)    # Min Et value for the electrons in Zee tag selection (GeV).
    kwargs.setdefault("minZeeMassTP", 66)    # Minimum value of Zee mass for checking the TP pairs (GeV).
    kwargs.setdefault("maxZeeMassTP", 116)   # Maximum value of Zee mass for checking the TP pairs (GeV).
    
    # Containers
    kwargs.setdefault("EventInfoContainerKey"       , "EventInfo")
    kwargs.setdefault("PrimaryVertexContainerKey"   , "PrimaryVertices")
    kwargs.setdefault("CaloClusterContainerKey"     , "CaloCalTopoClusters")
    kwargs.setdefault("TruthParticleContainerKey"   , "TruthParticles")
    kwargs.setdefault("ElectronContainerKey"        , "Electrons")
    kwargs.setdefault("TruthEventContainerKey"      , "TruthEvents")
    kwargs.setdefault("LArEMBHitContainerKey"       , "LArHitEMB")
    kwargs.setdefault("LArRawChannelContainerKey"   , "LArRawChannels")
    kwargs.setdefault("CaloCellContainerKey"        , "AllCalo")

    if (kwargs.get("doElecSelectByTrackOnly")):
        kwargs.setdefault("MyElectronSelectionKey"      , "MySelectedElectrons")
    else:
        kwargs.setdefault("MyElectronSelectionKey"      , "MyTagAndProbeElectrons")
        
    if isMC:
        kwargs.setdefault("LArDigitContainerKey","LArDigitContainer_MC")
    else:       
        kwargs.setdefault("LArDigitContainerKey","FREE")

    kwargs.setdefault("isMC", isMC)  # set to True in case of MC sample.

    #------------------------------------------------------------------------------------------------------
    #                                      Condition Database Access
    #------------------------------------------------------------------------------------------------------

    # ## LArIDMapping
    from LArCabling.LArCablingConfig import LArFebRodMappingCfg, LArOnOffIdMappingCfg
    acc.merge (LArFebRodMappingCfg (flags))
    acc.merge (LArOnOffIdMappingCfg (flags))

    ## Pedestal
    from CaloCellCorrection.CaloCellCorrectionConfig import CaloCellPedestalCorrCfg
    acc.popToolsAndMerge(CaloCellPedestalCorrCfg(flags))

    obj = "AthenaAttributeList"
    dbOnline    = 'LAR_ONL'

    ## Data
    if not isMC:

        if 'COMP200' in flags.IOVDb.DatabaseInstance:
            obj='LArDSPThresholdsComplete'
            dbString = 'COMP200'
            fldThr='/LAR/Configuration/DSPThreshold/Thresholds'
            obj='LArDSPThresholdsComplete'

            kwargs.setdefault("Run2DSPThresholdsKey", fldThr)
            conddb.addFolder (dbOnline, fldThr, className=obj)

        else: # Run2
            fldThr="/LAR/Configuration/DSPThresholdFlat/Thresholds"
            fldOflEneResc="/LAR/CellCorrOfl/EnergyCorr"
            fsampl="/LAR/ElecCalibMC/fSampl"
            dbString="CONDBR2"

            kwargs.setdefault("Run2DSPThresholdsKey", fldThr)#flags.EventReaderAlg.Run2DSPThresholdsKey)
            kwargs.setdefault("OflEneRescalerKey", fldOflEneResc)#flags.EventReaderAlg.OflEneRescalerKey)
            kwargs.setdefault("fSamplKey", fsampl)

        acc.merge(addFolders(flags,fldThr, "LAR_ONL", className=obj, db=dbString))
        acc.merge(addFolders(flags,[fldOflEneResc,], "LAR_OFL", className="AthenaAttributeList"))

        from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg ## Version 24+
        acc.merge(LArElecCalibDBCfg(flags, ["OFC", "Pedestal", "Shape"]))

    ## MC
    else:        
        from LArRecUtils.LArRecUtilsConfig import LArOFCCondAlgCfg
        acc.merge(LArOFCCondAlgCfg(flags))

        if flags.GeoModel.Run is LHCPeriod.Run1:  # back to flat threshold
            fldThr = ''
        else: # Run2
            dbString="OFLP200"
            fldThr=""
            fldOflEneResc=""

        kwargs.setdefault("Run2DSPThresholdsKey",fldThr)
        kwargs.setdefault("OflEneRescalerKey", fldOflEneResc)

    ## Further calibration constants and electronics conditions
    from LArRecUtils.LArADC2MeVCondAlgConfig import LArADC2MeVCondAlgCfg
    acc.merge(LArADC2MeVCondAlgCfg(flags))

    from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg, LArBadFebCfg
    acc.merge(LArBadFebCfg(flags))
    acc.merge(LArBadChannelCfg(flags))

    ## Main algorithm
    EventReaderAlg = CompFactory.EventReaderAlg(name, **kwargs)    
    acc.addEventAlgo(EventReaderAlg)

    return acc


def CaloCablingAndGeometryCfg(flags,name='CaloCablingAndGeometryCfg',**kwargs):
    mlog = logging.getLogger('CaloCablingAndGeometryCfg...')

    from LArCabling.LArCablingConfig import LArFebRodMappingCfg, LArOnOffIdMappingCfg
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg

    acc = ComponentAccumulator()

    acc.merge(LArFebRodMappingCfg (flags))
    acc.merge(LArOnOffIdMappingCfg (flags))
    acc.merge(GeoModelCfg(flags))

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))
    
    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    mlog.info("CaloCablingAndGeometryCfg added to CA.")
    
    return acc


def CaloNoiseCfg(flags,name='CaloNoiseCfg',**kwargs):
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    from CaloRec.CaloBCIDCoeffsCondAlgConfig import CaloBCIDCoeffsCondAlgCfg

    acc = ComponentAccumulator()

    acc.merge(CaloNoiseCondAlgCfg(flags,noisetype="totalNoise"))
    acc.merge(CaloNoiseCondAlgCfg(flags,noisetype="electronicNoise"))
    acc.merge(CaloNoiseCondAlgCfg(flags,noisetype="pileupNoise"))
    
    acc.merge (CaloBCIDCoeffsCondAlgCfg (flags))

    if flags.Input.isMC:
        from LumiBlockComps.BunchCrossingCondAlgConfig import BunchCrossingCondAlgCfg
        acc.merge (BunchCrossingCondAlgCfg(flags))
    else:
        from LumiBlockComps.LuminosityCondAlgConfig import LuminosityCondAlgCfg
        acc.merge (LuminosityCondAlgCfg (flags))
    
    return acc

def LArClusterCellDumperCfg(flags, name='LArClusterCellDumperCfg'):
    
    mlog = logging.getLogger('LArClusterCellDumperCfg')
    mlog.info("merging LArClusterCellDumperCfg config files...")

    CaloNoise           = CaloNoiseCfg(flags)
    CaloCablingAndGeo   = CaloCablingAndGeometryCfg(flags)
    EventReaderAlg      = EventReaderAlgCfg(flags)
    
    acc = ComponentAccumulator()

    acc.merge(CaloNoise)
    acc.merge(CaloCablingAndGeo)
    acc.merge(EventReaderAlg)
        
    mlog.info("All algorithms merged successfully!")

    return acc

if __name__ == "__main__":
    mlog = logging.getLogger('LArClusterCellDumperCfg')
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags ## > V24+
    from AthenaCommon.GlobalFlags import globalflags
    from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
    
    histSvc = CompFactory.THistSvc(Output = ["rec DATAFILE='dumper_outputMC.root', OPT='RECREATE'"])

    dumperFlags                             = initConfigFlags()
    dumperFlags.loadAllDynamicFlags()
    
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    dumperFlags.Input.Files                 = defaultTestFiles.ESD
    dumperFlags.Input.Files                 = ['/eos/user/m/mhufnage/scripts_lxplus/Reco/ALP_reco/MC_Zee_EVNTtoESD_standardBeamSpot/ESD_Zee_0.pool.root']
    dumperFlags.Exec.MaxEvents              = 50
    
    if not dumperFlags.Input.isMC:
        globalflags.DataSource  = 'data'
    else:
        globalflags.DataSource  = 'geant4'

    dumperFlags.fillFromArgs()
    dumperFlags.lock()

    athenaCommonFlags.FilesInput = dumperFlags.Input.Files
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(dumperFlags)

    acc.merge(PoolReadCfg(dumperFlags)) # athena service required for POOL file reading
    acc.addService(histSvc)
    acc.merge(LArClusterCellDumperCfg(dumperFlags))
    acc.getService("MessageSvc").defaultLimit=999999

    mlog.info("Executing LArClusterCellDumperCfg...")

    with open("configData.pkl", "wb") as f:
        acc.store(f)

    acc.run()
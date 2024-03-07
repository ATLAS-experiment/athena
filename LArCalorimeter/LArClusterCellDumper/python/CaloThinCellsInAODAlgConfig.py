"""Configuration for the LAr Calo Cells Digits and RawChannels By Cluster Sampling Thinner algorithm for AODs
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
from LArConfiguration.LArConfigFlags import RawChannelSource 
from OutputStreamAthenaPool.OutputStreamConfig import addToAOD

def CaloThinCellsInAODAlgCfg(flags, **kwargs):
    """Return ComponentAccumulator for CaloThinCellsInAODAlgCfg algorithm"""
    # based on LArDigitThinner+CaloThinCellsByClusterAlg+CaloThinCellsBySamplingAlg

    acc = ComponentAccumulator()
    
    kwargs.setdefault("ClusterPtCut", 1000) # MeV
    kwargs.setdefault("ClusterEtaCut", 1.4)
        
    # Input Containers
    if not flags.Input.isMC and not flags.Overlay.DataOverlay:
        kwargs.setdefault("InputDigitsContainerName","FREE")
        from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
        acc.merge(LArRawDataReadingCfg(flags, LArDigitKey="FREE"))

    acc.merge(LArOnOffIdMappingCfg(flags))
   
    if flags.LAr.RawChannelSource is RawChannelSource.Calculated:
        kwargs.setdefault("InputRawChannelContainerName","LArRawChannels_FromDigits")
        
    kwargs.setdefault("InputCaloCellContainerName"  ,"AllCalo")
    kwargs.setdefault("CaloClusterContainerKey"     ,"CaloCalTopoClusters")
    
    # Output Containers
    outputDigCntName      = "LArDigitContainer_ClusterThinned"
    outputRawChCntName    = "LArRawChannels_ClusterThinned"
    outputCaloCellCntName = "AllCalo_ClusterThinned"
    
    kwargs.setdefault("OutputDigitsContainerName"    ,outputDigCntName)
    kwargs.setdefault("OutputRawChannelContainerName",outputRawChCntName)
    kwargs.setdefault("OutputCaloCellContainerName"  ,outputCaloCellCntName)
    
    acc.addEventAlgo(CompFactory.CaloThinCellsInAODAlg(**kwargs))
    
    acc.merge(addToAOD(flags,[f"LArDigitContainer#{outputDigCntName}"]))
    acc.merge(addToAOD(flags,[f"LArRawChannelContainer#{outputRawChCntName}"]))
    acc.merge(addToAOD(flags,[f"CaloCellContainer#{outputCaloCellCntName}"]))
    
    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    
    mlog = logging.getLogger('CaloThinCellsInAODAlgCfg')

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.Output.doWriteAOD = True
    flags.Exec.MaxEvents = 10
    flags.lock()
    
    acc = MainServicesCfg(flags)

    acc.merge(CaloThinCellsInAODAlgCfg(flags))
    
    acc.getService("MessageSvc").defaultLimit=999999
    
    acc.run()
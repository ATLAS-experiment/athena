# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod, MetadataCategory

def TCAL2MbtsToVectorsToolCfg(flags, **kwargs):
    """ Configure the MbtsToVectorsTool augmentation tool """

    acc = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))

    kwargs.setdefault('name', 'TCAL2MbtsToVectorsTool')
    kwargs.setdefault('Prefix', 'TCAL2_mbts_')
    kwargs.setdefault('SaveEtaPhiInfo', True)
    kwargs.setdefault('CellContainer', 'MBTSContainer')

    MbtsToVectorsTool = CompFactory.DerivationFramework.MbtsToVectorsTool
    acc.addPublicTool(MbtsToVectorsTool(**kwargs), primary = True)

    return acc


def TCAL2E4prToVectorsToolCfg(flags, **kwargs):
    """ Configure the E4prToVectorsTool augmentation tool """

    acc = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))

    kwargs.setdefault('name', 'TCAL2E4prToVectorsTool')
    kwargs.setdefault('Prefix', 'TCAL2_e4pr')
    kwargs.setdefault('SaveEtaPhiInfo', False)
    kwargs.setdefault('CellContainer', 'E4prContainer')

    MbtsToVectorsTool = CompFactory.DerivationFramework.MbtsToVectorsTool
    acc.addPublicTool(MbtsToVectorsTool(**kwargs), primary = True)

    return acc


def TCAL2KernelCfg(flags, name='TCAL2Kernel', **kwargs):
    """Configure the TCAL2 derivation framework driving algorithm (kernel)"""

    acc = ComponentAccumulator()

    prefix = kwargs.pop('Prefix', 'TCAL2_')

    cellsToVectorstools = []
    if flags.GeoModel.Run in [LHCPeriod.Run1, LHCPeriod.Run2, LHCPeriod.Run3]:
        cellsToVectorstools.append( acc.getPrimaryAndMerge(TCAL2MbtsToVectorsToolCfg(flags, Prefix=f'{prefix}mbts_')) )
    if flags.GeoModel.Run is LHCPeriod.Run2:
        cellsToVectorstools += acc.getPrimaryAndMerge(TCAL2E4prToVectorsToolCfg(flags, Prefix=f'{prefix}e4pr_'))

    DerivationKernel = CompFactory.DerivationFramework.DerivationKernel
    acc.addEventAlgo(DerivationKernel(name, AugmentationTools = cellsToVectorstools))

    return acc

def TCAL2Cfg(flags):
    """Configure the TCAL1 derivation framework"""

    TCAL2Prefix = 'TCAL2_'

    acc = ComponentAccumulator()
    acc.merge(TCAL2KernelCfg(flags, name="TCAL2Kernel", StreamName = "StreamDAOD_TCAL2", Prefix=TCAL2Prefix))

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    TCAL2SlimmingHelper = SlimmingHelper("TCAL2SlimmingHelper", NamesAndTypes = flags.Input.TypedCollections, flags = flags)
    TCAL2SlimmingHelper.SmartCollections = ["EventInfo"]

    if flags.GeoModel.Run in [LHCPeriod.Run1, LHCPeriod.Run2, LHCPeriod.Run3]:
        mbtsItems = [f'std::vector<float>#TCAL2_mbts_{item}' for item in ['energy', 'time', 'eta', 'phi']]
        mbtsItems += [f'std::vector<int>#TCAL2_mbts_{item}' for item in ['quality', 'module', 'channel', 'type']]
        TCAL2SlimmingHelper.StaticContent = mbtsItems
    if flags.GeoModel.Run is LHCPeriod.Run2:
        e4prItems = [f'std::vector<float>#TCAL2_e4pr_{item}' for item in ['energy', 'time']]
        e4prItems += ['std::vector<int>#TCAL2_e4pr_{item}' for item in ['quality', 'module', 'channel', 'type']]
        TCAL2SlimmingHelper.StaticContent += e4prItems
    TCAL2ItemList = TCAL2SlimmingHelper.GetItemList()

    acc.merge(OutputStreamCfg(flags, "DAOD_TCAL2", ItemList=TCAL2ItemList, AcceptAlgs=["TCAL2Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_TCAL2", AcceptAlgs=["TCAL2Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))

    return acc

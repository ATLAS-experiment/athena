# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import ProductionStep

from AthenaConfiguration.ComponentFactory import CompFactory
from OutputStreamAthenaPool.OutputStreamConfig import addToAOD, addToESD

spacePointKeys = [ "xAOD::BaseContainer#SpacePoints", "xAOD::AuxContainerBase#SpacePointsAux.x.y.z.tot.csize.module1.module2" ]
mbtsBitsKeys = [ "xAOD::TrigT2MbtsBitsContainer#MBTSBits", "xAOD::TrigT2MbtsBitsAuxContainer#MBTSBitsAux." ]

def _inRecoOrLater(flags):
    return flags.Common.ProductionStep is ProductionStep.Reconstruction \
            or flags.Common.ProductionStep is ProductionStep.Derivation

def addSpacePoints(flags):
    """
    Adds Space Points data to the AOD/ESD output, if necessary schedule producer algorithm
    """
    acc = ComponentAccumulator()

    if not _inRecoOrLater(flags):
        return acc

    acc.merge(addToAOD(flags, spacePointKeys))
    acc.merge(addToESD(flags, spacePointKeys))

    if spacePointKeys[0].split("#")[1] not in flags.Input.Collections:
        copier = CompFactory.SpacePointCopier(maxTracks=20)
        acc.addEventAlgo(copier)
    return acc

def addMBTS(flags):
    """
    Adds MBTS data to the AOD/ESD output, if necessary schedule producer algorithm
    """
    acc = ComponentAccumulator()
    if not _inRecoOrLater(flags):
        return acc
    
    acc.merge(addToAOD(flags, mbtsBitsKeys))
    acc.merge(addToESD(flags, mbtsBitsKeys))

    if mbtsBitsKeys[0].split("#")[1] not in flags.Input.Collections:
        copier = CompFactory.MBTSInfoCopier()
        acc.addEventAlgo(copier)

    return acc

def addDetailedCaloClusterInfo(flags):
    """
    Add detail of Topo clusters for UPC analyses
    """
    acc = ComponentAccumulator()
    if not _inRecoOrLater(flags):
        return acc
    clusterDetails= ["xAOD::CaloClusterAuxContainer#CaloCalTopoClustersAux.SECOND_R.SECOND_LAMBDA.CENTER_MAG.CENTER_LAMBDA.FIRST_ENG_DENS.ENG_FRAC_MAX.ISOLATION.ENG_BAD_CELLS.N_BAD_CELLS.BADLARQ_FRAC.ENG_POS.AVG_LAR_Q.AVG_TILE_Q.EM_PROBABILITY.BadChannelList.CELL_SIGNIFICANCE.CELL_SIG_SAMPLING"]

    acc.merge(addToAOD(flags, clusterDetails))
    acc.merge(addToESD(flags, clusterDetails))
    return acc
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured private Tile hit vector to container tool"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from DigitizationConfig.PileUpMergeSvcConfig import PileUpMergeSvcCfg, PileUpXingFolderCfg
from DigitizationConfig.PileUpToolsConfig import PileUpToolsCfg

def getTileFirstXing():
    """Return the earliest bunch crossing time for which interactions will be sent to the TileHitVecToCntTool"""
    return -200


def getTileLastXing():
    """Return the latest bunch crossing time for which interactions will be sent to the TileHitVecToCntTool"""
    return 150


def TileRangeCfg(flags, name = 'TileRange', **kwargs):
    """Return a PileUpXingFolder tool for Tile"""
    item_list = ['TileHitVector#TileHitVec']
    if flags.Detector.EnableMBTS:
        item_list += ['TileHitVector#MBTSHits']
    kwargs.setdefault('FirstXing', getTileFirstXing() )
    kwargs.setdefault('LastXing',  getTileLastXing() )
    kwargs.setdefault('ItemList', item_list )
    return PileUpXingFolderCfg(flags, name, **kwargs)


def TileHitVecToCntToolCfg(flags, **kwargs):
    """Return component accumulator with configured private Tile hit vector to container tool

    Arguments:
        flags  -- Athena configuration flags
    """

    kwargs.setdefault('name', 'TileHitVecToCntTool')
    kwargs.setdefault('RndmEvtOverlay', flags.Common.isOverlay)
    if flags.Common.isOverlay:
        kwargs.setdefault('OnlyUseContainerName', False)
    else:
        kwargs.setdefault('OnlyUseContainerName', flags.Digitization.PileUp)

    acc = ComponentAccumulator()

    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge( TileInfoLoaderCfg(flags) )

    from TileConditions.TileSamplingFractionConfig import TileSamplingFractionCondAlgCfg
    acc.merge( TileSamplingFractionCondAlgCfg(flags) )

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge(TileCablingSvcCfg(flags))

    if flags.Detector.EnableMBTS:
        kwargs.setdefault('TileHitVectors', ['TileHitVec', 'MBTSHits'])
    else:
        kwargs.setdefault('TileHitVectors', ['TileHitVec'])
    kwargs.setdefault('TileHitContainer', 'TileHitCnt')

    if flags.Common.isOverlay and not flags.Sim.DoFullChain:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, [f'TileHitVector#{vec}' for vec in kwargs['TileHitVectors']]))

    kwargs.setdefault('DoHSTruthReconstruction', flags.Digitization.EnableCaloHSTruthRecoInputs)
    if kwargs['DoHSTruthReconstruction']:
        kwargs.setdefault('TileHitContainer_DigiHSTruth', 'TileHitCnt_DigiHSTruth')
    else:
        kwargs.setdefault('TileHitContainer_DigiHSTruth', '')

    if 'RndmSvc' not in kwargs:
        from RngComps.RandomServices import AthRNGSvcCfg
        kwargs['RndmSvc'] = acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name

    if kwargs['RndmEvtOverlay']:
        kwargs.setdefault('PileUp', False)
    else:
        kwargs.setdefault('PileUp', flags.Digitization.PileUp)

    if flags.Beam.Type is BeamType.Cosmics:
        CosmicTriggerTimeTool=CompFactory.CosmicTriggerTimeTool
        kwargs.setdefault('TriggerTimeTool', CosmicTriggerTimeTool())
        kwargs.setdefault('HitTimeFlag', 2)
        kwargs.setdefault('UseTriggerTime', True)

    if flags.Digitization.PileUp:
        intervals = []
        if flags.Digitization.DoXingByXingPileUp: # PileUpTool approach
            kwargs.setdefault("FirstXing", getTileFirstXing() )
            kwargs.setdefault("LastXing",  getTileLastXing() )
        else:
            intervals += [acc.popToolsAndMerge(TileRangeCfg(flags))]
        kwargs.setdefault("PileUpMergeSvc", acc.getPrimaryAndMerge(PileUpMergeSvcCfg(flags, Intervals=intervals)).name)
    else:
        kwargs.setdefault("PileUpMergeSvc", '')
    kwargs.setdefault("OnlyUseContainerName", flags.Digitization.PileUp)
    TileHitVecToCntTool=CompFactory.TileHitVecToCntTool
    acc.setPrivateTools(TileHitVecToCntTool(**kwargs))

    return acc


def TileHitVecToCntCfg(flags, **kwargs):
    """Return component accumulator with configured Tile hit vector to container algorithm

    Arguments:
        flags  -- Athena configuration flags
    """

    acc = ComponentAccumulator()

    if 'DigitizationTool' not in kwargs:
        tool = acc.popToolsAndMerge( TileHitVecToCntToolCfg(flags) )
        kwargs.setdefault('DigitizationTool', tool)

    # choose which alg to attach to, following PileUpToolsCfg
    if flags.Common.isOverlay:
        if flags.Concurrency.NumThreads > 0:
            kwargs.setdefault('Cardinality', flags.Concurrency.NumThreads)

        kwargs.setdefault('name', 'TileHitVecToCnt')
        Alg = CompFactory.TileHitVecToCnt
        acc.addEventAlgo(Alg(**kwargs))
    else:
        kwargs["PileUpTools"] = [kwargs.pop("DigitizationTool")]
        acc.merge(PileUpToolsCfg(flags, **kwargs))

    return acc


def TileHitOutputCfg(flags, **kwargs):
    """Return component accumulator with Output Stream configuration for Tile hits

    Arguments:
        flags  -- Athena configuration flags
    """

    if flags.Output.doWriteRDO:
        acc = OutputStreamCfg(flags, 'RDO', ['TileHitContainer#*'])
    else:
        acc = ComponentAccumulator()

    return acc


def TileHitVecToCntOutputCfg(flags, **kwargs):
    """Return component accumulator with configured Tile hit vector to container algorithm and Output Stream

    Arguments:
        flags  -- Athena configuration flags
    """
    
    acc = TileHitVecToCntCfg(flags, **kwargs)
    acc.merge(TileHitOutputCfg(flags))

    return acc



if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG

    # Test setup
    log.setLevel(DEBUG)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.HITS_RUN2
    flags.Output.RDOFileName = 'myRDO-TileHitVecToCnt.pool.root'
    flags.IOVDb.GlobalTag = 'OFLCOND-MC16-SDR-16'
    flags.Digitization.PileUp = False
    flags.Exec.MaxEvents = 3

    flags.fillFromArgs()
    flags.lock()

    # Construct our accumulator to run
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    if 'EventInfo' not in flags.Input.Collections:
        from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
        acc.merge(EventInfoCnvAlgCfg(flags,
                                     inputKey='McEventInfo',
                                     outputKey='EventInfo'))

    acc.merge(TileHitVecToCntOutputCfg(flags))

    acc.getService('StoreGateSvc').Dump = True
    acc.printConfig(withDetails = True, summariseProps = True)
    flags.dump()
    acc.store( open('TileHitVecToCnt.pkl','wb') )

    sc = acc.run()
    # Success should be 0
    import sys
    sys.exit(not sc.isSuccess())

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured Tile muon receiver decision algorithm"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from TileConfiguration.TileConfigFlags import TileRunType

def TileMuonReceiverDecisionCfg(flags, **kwargs):
    """Return component accumulator with configured Tile muon receiver decision algorithm

    Arguments:
        flags  -- Athena configuration flags
    Keyword arguments:
        Name -- name of TileMuonReceiverDecision algorithm. Defaults to TileMuonReceiverDecision.
    """

    name = kwargs.get('Name', 'TileMuonReceiverDecision')

    acc = ComponentAccumulator()

    TileMuonReceiverDecision=CompFactory.TileMuonReceiverDecision
    muRcvDecisionAlg = TileMuonReceiverDecision(name,
                                                MuonReceiverEneThreshCellD6Low = 500,
                                                MuonReceiverEneThreshCellD6andD5Low = 500,
                                                MuonReceiverEneThreshCellD6High = 600,
                                                MuonReceiverEneThreshCellD6andD5High = 600)


    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        muRcvDecisionAlg.TileMuonReceiverContainer = flags.Overlay.BkgPrefix + 'TileMuRcvCnt'

    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge( TileInfoLoaderCfg(flags) )

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge(TileCablingSvcCfg(flags))

    from TileConditions.TileEMScaleConfig import TileEMScaleCondAlgCfg
    acc.merge( TileEMScaleCondAlgCfg(flags) )

    acc.addEventAlgo(muRcvDecisionAlg, primary = True)

    return acc


def TileMuonReceiverDecisionOutputCfg(flags, **kwargs):
    """Return component accumulator with configured Tile muon receiver decision algorithm and Output stream

    Arguments:
        flags  -- Athena configuration flags
    """

    acc = TileMuonReceiverDecisionCfg(flags, **kwargs)
    muRcvDecisionAlg = acc.getPrimary()

    if hasattr(muRcvDecisionAlg, 'TileMuonReceiverContainer'):
        muRcvContainer = muRcvDecisionAlg.TileMuonReceiverContainer
    else:
        muRcvContainer = muRcvDecisionAlg.getDefaultProperty('TileMuonReceiverContainer')
    muRcvContainer = str(muRcvContainer).split('+').pop()
    outputItemList = ['TileMuonReceiverContainer#' + muRcvContainer]

    if flags.Output.doWriteRDO:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge( OutputStreamCfg(flags, streamName = 'RDO', ItemList = outputItemList) )

    return acc



if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG

    # Test setup
    log.setLevel(DEBUG)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.Tile.RunType = TileRunType.PHY
    flags.Output.RDOFileName = 'myRDO->TileMuonReceiverDecision.pool.root'
    flags.IOVDb.GlobalTag = 'OFLCOND-MC16-SDR-16'

    flags.fillFromArgs()
    flags.lock()
    flags.dump()

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

    acc.merge( TileMuonReceiverDecisionOutputCfg(flags, TileMuonReceiverContainer = 'TileMuRcvCntNew') )

    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('TileMuonReceiverDecision.pkl','wb') )

    sc = acc.run(maxEvents=3)
    # Success should be 0
    import sys
    sys.exit(not sc.isSuccess())

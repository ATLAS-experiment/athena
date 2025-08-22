"""Combined Tile Digitization functions

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import ProductionStep
from AthenaConfiguration.Enums import LHCPeriod
from AthenaConfiguration.Enums import BeamType
from TileConfiguration.TileConfigFlags import TileRunType

def TileTriggerDigitizationCfg(flags):
    """Return ComponentAccumulator with standard Tile Trigger Digitization configuration"""

    if flags.Beam.Type is BeamType.TestBeam:
        return ComponentAccumulator()

    from TileSimAlgs.TileHitToTTL1Config import TileHitToTTL1OutputCfg
    acc = TileHitToTTL1OutputCfg(flags)

    if flags.GeoModel.Run > LHCPeriod.Run1:
        from TileSimAlgs.TileMuonReceiverConfig import TilePulseForTileMuonReceiverOutputCfg
        acc.merge( TilePulseForTileMuonReceiverOutputCfg(flags) )

        from TileSimAlgs.TileMuonReceiverDecisionConfig import TileMuonReceiverDecisionOutputCfg
        acc.merge( TileMuonReceiverDecisionOutputCfg(flags) )

    from TileL2Algs.TileL2Config import TileRawChannelToL2OutputCfg
    acc.merge( TileRawChannelToL2OutputCfg(flags, streamName = 'RDO') )

    return acc


def TileOverlayTriggerDigitizationCfg(flags):
    """Return ComponentAccumulator with Overlay Tile Trigger Digitization configuration"""
    acc = ComponentAccumulator()

    if flags.GeoModel.Run > LHCPeriod.Run1:
        from TileSimAlgs.TileMuonReceiverConfig import TilePulseForTileMuonReceiverOutputCfg
        acc.merge( TilePulseForTileMuonReceiverOutputCfg(flags) )

        from TileSimAlgs.TileMuonReceiverDecisionConfig import TileMuonReceiverDecisionOutputCfg
        acc.merge( TileMuonReceiverDecisionOutputCfg(flags) )

    from TileL2Algs.TileL2Config import TileRawChannelToL2OutputCfg
    acc.merge( TileRawChannelToL2OutputCfg(flags, streamName = 'RDO') )

    return acc


def TileDigitizationCfg(flags):
    """Return ComponentAccumulator with standard Tile Digitization configuration"""

    from TileSimAlgs.TileDigitsMakerConfig import TileDigitsMakerOutputCfg
    acc = TileDigitsMakerOutputCfg(flags)

    if flags.Common.ProductionStep != ProductionStep.PileUpPresampling and flags.Output.doWriteRDO:
        from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerOutputCfg
        acc.merge( TileRawChannelMakerOutputCfg(flags, streamName = 'RDO') )
    else:
        from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
        acc.merge( TileRawChannelMakerCfg(flags) )

    if flags.Digitization.EnableCaloHSTruthRecoInputs:
        if flags.Output.doWriteRDO:
            from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerDigiHSTruthOutputCfg
            acc.merge( TileRawChannelMakerDigiHSTruthOutputCfg(flags, streamName = 'RDO') )
        else:
            from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerDigiHSTruthCfg
            acc.merge( TileRawChannelMakerDigiHSTruthCfg(flags) )

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
    flags.Tile.RunType = TileRunType.PHY
    flags.Output.RDOFileName = 'myRDO-TileDigitization.pool.root'
    flags.IOVDb.GlobalTag = 'OFLCOND-MC16-SDR-16'
    flags.Digitization.PileUp = False

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

    acc.merge( TileDigitizationCfg(flags) )
    acc.merge( TileTriggerDigitizationCfg(flags) )

    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('TileDigitization.pkl','wb') )

    sc = acc.run(maxEvents=3)
    # Success should be 0
    import sys
    sys.exit(not sc.isSuccess())

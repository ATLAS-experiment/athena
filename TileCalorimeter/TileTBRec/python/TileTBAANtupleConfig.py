#!/usr/bin/env python
#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
'''
@file TileTBAANtupleConfig.py
@brief Python configuration of TileTBAANtuple algorithm for the Run III
'''

from AthenaConfiguration.ComponentFactory import CompFactory
from TileConfiguration.TileConfigFlags import TileRunType

import sys


def getTileTBperiod(run, useFELIX):
    ''' Function to get Tile TB period.'''

    TBperiod = 2015 if (run // 100000 == 5) else 2016

    if useFELIX:
        TBperiod = 2017
    if run > 800000:
        TBperiod += 2
    if run >= 2200000:
        TBperiod = 2022
    elif run >= 2110000:
        TBperiod = 2021

    return TBperiod


def TileTBAANtupleCfg(flags, outputFile='', useFELIX=None, **kwargs):

    ''' Function to configure TileTBAANtuple algorithm.'''

    cisRun      = flags.Tile.RunType is TileRunType.CIS
    monoRun     = flags.Tile.RunType is TileRunType.MONOCIS
    pedestalRun = flags.Tile.RunType is TileRunType.PED

    # Units: TileRawChannelUnit::ADC (0) or TileRawChannelUnit::MegaElectronVolts (3)
    offlineUnits = 0 if cisRun or monoRun or pedestalRun else 3
    calibrateEnergy = (offlineUnits != 0)

    run = flags.Input.RunNumbers[0]

    if useFELIX is None:
        useFELIX = True if run > 2310438 else False

    TBperiod = getTileTBperiod(run, useFELIX) if not flags.Input.isMC else 2003

    kwargs.setdefault('TileDigitsContainer', 'TileDigitsCnt')
    kwargs.setdefault('TileBeamElemContainer', 'TileBeamElemCnt' if not flags.Input.isMC else "")
    kwargs.setdefault('TileRawChannelContainerFlat', "")
    kwargs.setdefault('TileRawChannelContainerFit', 'TileRawChannelFit' if flags.Tile.doFit else "")
    kwargs.setdefault('TileRawChannelContainerOpt', "TileRawChannelOpt2" if flags.Tile.doOpt2 else "")
    kwargs.setdefault('TileRawChannelContainerDsp', "")
    kwargs.setdefault('TileRawChannelContainerFitCool', 'TileRawChannelFitCool' if flags.Tile.doFitCOOL else "")

    kwargs.setdefault('TileDigitsContainerFlx', 'TileDigitsFlxCnt' if useFELIX else "")
    kwargs.setdefault('TileRawChannelContainerFitFlx', 'TileRawChannelFlxFit' if useFELIX and flags.Tile.doFit else "")
    kwargs.setdefault('TileRawChannelContainerOptFlx', 'TileRawChannelFlxOpt2' if useFELIX and flags.Tile.doOpt2 else "")

    kwargs.setdefault('TileHitContainer', 'TileHitCnt' if flags.Input.isMC else "")
    kwargs.setdefault('TileHitVector',  'TileHitVec' if flags.Input.isMC else "")
    kwargs.setdefault('CaloCellContainer', 'AllCalo')

    kwargs.setdefault('TileLaserObj', "")

    kwargs.setdefault('OfflineUnits', offlineUnits)
    kwargs.setdefault('CalibrateEnergy', calibrateEnergy)
    kwargs.setdefault('CalibMode', pedestalRun or cisRun)
    kwargs.setdefault('PMTOrder', not (cisRun or monoRun or pedestalRun))

    kwargs.setdefault('NSamples', -1)
    kwargs.setdefault('NSamplesFelix', 16 if useFELIX else 0)

    kwargs.setdefault('TBperiod', TBperiod)

    if flags.Input.isMC:
        kwargs.setdefault('beamFragList', [])
        kwargs.setdefault('CompleteNtuple', False)

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc = TileGMCfg(flags)

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    if 'TileCondToolEmscale' not in kwargs:
        from TileConditions.TileEMScaleConfig import TileCondToolEmscaleCfg
        emScaleTool = acc.popToolsAndMerge( TileCondToolEmscaleCfg(flags) )
        kwargs['TileCondToolEmscale'] = emScaleTool

    from TileRecUtils.TileDQstatusConfig import TileDQstatusAlgCfg
    acc.merge( TileDQstatusAlgCfg(flags) )

    if flags.Input.isMC:
        from TileConditions.TileSamplingFractionConfig import TileSamplingFractionCondAlgCfg
        acc.merge( TileSamplingFractionCondAlgCfg(flags) )
    else:
        kwargs['TileSamplingFraction'] = ""

    if not outputFile:
        outputFile = f'tiletb_{run}.aan.root'
    histsvc = CompFactory.THistSvc()
    histsvc.Output += ["%s DATAFILE='%s' OPT='RECREATE'" % ('AANT', outputFile)]
    acc.addService(histsvc)

    alg = CompFactory.TileTBAANtuple(**kwargs)
    from TileMonitoring.TileTBBeamChambersCalibration import updateBeamChambersCalibrations
    updateBeamChambersCalibrations(alg, run)
    acc.addEventAlgo(alg, primary=True)

    return acc


if __name__ == '__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()
    parser = flags.getArgumentParser()
    parser.add_argument('--postExec', help='Code to execute after setup')
    args, _ = parser.parse_known_args()

    flags.Exec.MaxEvents = 3
    flags.GeoModel.AtlasVersion = 'ATLAS-R2-2015-04-00-00'
    flags.Tile.doFit = False
    flags.Tile.doOpt2 = False
    flags.Tile.RunType = TileRunType.PHY
    flags.Input.Files = defaultTestFiles.RAW_RUN2

    flags.fillFromArgs(parser=parser)
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg.merge( TileRawDataReadingCfg(flags, readMuRcv=False,
                                     readDigits=True,
                                     readRawChannel=True,
                                     readDigitsFlx=True,
                                     readBeamElem=True,
                                     stateless=False) )

    cfg.merge( TileTBAANtupleCfg(flags, TileBeamElemContainer="") )

    cfg.getCondAlgo('TileHid2RESrcIDCondAlg').RODStatusProxy = None

    # =======>>> Any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    if args.config_only:
        cfg.store(open('TileTBAANtuple.pkl', 'wb'))
    else:
        sc = cfg.run()
        # Success should be 0
        sys.exit(not sc.isSuccess())

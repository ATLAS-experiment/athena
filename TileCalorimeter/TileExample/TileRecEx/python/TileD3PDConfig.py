#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import Format

'''
@file TileD3PDConfig.py
@brief Python configuration of TileD3PD for the Run III
'''


def _args(level, name, kwin, **kw):
    kw = kw.copy()
    kw['level'] = level
    for (k, v) in kwin.items():
        if k.startswith(name + '_'):
            kw[k[len(name)+1:]] = v
    return kw


def TileD3PDCfg(flags, outputFile=None, saveCells=True, saveMBTS=True,
                saveE4pr=False, saveClusters=False, saveMuId=False,
                saveMuonFit=False, savePosition=True, saveEventInfo=False,
                **kwargs):
    ''' Function to configure Tile D3PD.'''

    acc = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
    acc.merge( TileBadChannelsCondAlgCfg(flags) )

    if not outputFile:
        runNumber = flags.Input.RunNumbers[0]
        outputFile = f'tile_{runNumber}.aan.root'

    from D3PDMakerCoreComps.MakerAlgConfig import MakerAlgConfig
    alg = MakerAlgConfig(flags, acc, 'caloD3PD', outputFile, ExistDataHeader=False)

    if saveEventInfo:
        excludeEventInfoBlocks = ['eventMu'] if flags.Input.Format is Format.BS else []
        from EventCommonD3PDMaker.EventInfoD3PDObject import EventInfoD3PDObject
        alg += EventInfoD3PDObject(**_args(0, 'EventInfo', kwargs), exclude=excludeEventInfoBlocks)

    from CaloSysD3PDMaker.TileDetailsD3PDObject import TileDetailsD3PDObject
    from CaloSysD3PDMaker.CaloInfoD3PDObject import CaloInfoD3PDObject
    from CaloD3PDMaker.MBTSD3PDObject import MBTSD3PDObject

    isBiGainRun = flags.Tile.RunType.isBiGain()

    if isBiGainRun:
        if saveCells:
            alg += TileDetailsD3PDObject(**_args(1, 'TileDetails', kwargs, sgkey='AllCaloHG',
                                                 prefix='tile_', Kinematics_WriteEtaPhi=True,
                                                 TileDetails_SavePositionInfo=savePosition))

            alg += CaloInfoD3PDObject(**_args(0, 'CaloInfo', kwargs, sgkey='AllCaloHG', prefix='calo_'))

        if saveMBTS:
            alg += MBTSD3PDObject(**_args(1, 'MBTS', kwargs, prefix='mbts_', sgkey='MBTSContainerHG'))
            alg += MBTSD3PDObject(**_args(1, 'MBTS', kwargs, prefix='mbtsLG_', sgkey='MBTSContainerLG'))

        if saveE4pr:
            alg += MBTSD3PDObject(**_args(1, 'MBTS', kwargs, prefix='e4pr_',
                                          sgkey='E4prContainerHG', MBTS_SaveEtaPhiInfo=False))

            alg += MBTSD3PDObject(**_args(1, 'MBTS', kwargs, prefix='e4prLG_',
                                          sgkey='E4prContainerLG', MBTS_SaveEtaPhiInfo=False))
    else:
        if saveCells:
            alg += TileDetailsD3PDObject(**_args(1, 'TileDetails', kwargs, sgkey='AllCalo',
                                                 prefix='tile_', Kinematics_WriteEtaPhi=True,
                                                 TileDetails_SavePositionInfo=savePosition))

            alg += CaloInfoD3PDObject(**_args(0, 'CaloInfo', kwargs, sgkey='AllCalo', prefix='calo_'))

        if saveMBTS:
            alg += MBTSD3PDObject(**_args(1, 'MBTS', kwargs, prefix='mbts_', sgkey='MBTSContainer'))

        if saveE4pr:
            alg += MBTSD3PDObject(**_args(1, 'MBTS', kwargs, prefix='e4pr_',
                                          sgkey='E4prContainer', MBTS_SaveEtaPhiInfo=False))

    if saveClusters:
        from CaloD3PDMaker.xAODClusterD3PDObject import xAODClusterD3PDObject
        alg += xAODClusterD3PDObject(**_args(3, 'topo_cl', kwargs, sgkey='TileTopoCluster', prefix='topo_'))

    if saveMuId:
        from CaloSysD3PDMaker.TileMuD3PDObject import TileMuD3PDObject
        alg += TileMuD3PDObject(**_args(0, 'TileMus', kwargs, sgkey='TileMuObj', prefix='tilemuid_'))

    if saveMuonFit:
        from CaloSysD3PDMaker.TileCosmicMuonD3PDObject import TileCosmicMuonD3PDObject
        alg += TileCosmicMuonD3PDObject(**_args(2, 'TileCosMusHT', kwargs, sgkey='TileCosmicMuonHT', prefix='TileCosmicsHT_'))

    acc.addEventAlgo(alg.alg)
    return acc


if __name__ == '__main__':

    # Set the Athena configuration flags
    from D3PDMakerConfig.D3PDMakerFlags import configFlags as flags
    parser = flags.getArgumentParser()
    parser.add_argument('--cells', action='store_true', help='Save Tile cells in D3PD')
    parser.add_argument('--mbts', action='store_true', help='Save Tile MBTS in D3PD')
    parser.add_argument('--e4pr', action='store_true', help='Save Tile E4pr in D3PD')
    parser.add_argument('--muid', action='store_true', help='Save Tile MuID in D3PD')
    parser.add_argument('--muonfit', action='store_true', help='Save Tile Muon Fitter in D3PD')
    parser.add_argument('--clusters', action='store_true', help='Save Tile clusters in D3PD')
    parser.add_argument('--postExec', help='Code to execute after setup')
    args, _ = parser.parse_known_args()

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.ESD
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs(parser=parser)

    log.info('FINAL CONFIG FLAGS SETTINGS FOLLOW')
    flags.dump()

    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    cfg.merge( TileD3PDCfg(flags,
                           saveCells=args.cells,
                           saveMBTS=args.mbts,
                           saveE4pr=args.e4pr,
                           saveMuId=args.muid,
                           saveMuonFitter=args.muonfit,
                           saveClusters=args.clusters,
                           saveEventInfo=True) )

    cfg.setAppProperty('CreateSvc', [cfg.getService('TileCablingSvc').getFullJobOptName()])

    # Any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    cfg.printConfig(withDetails=True, summariseProps=True)

    cfg.store( open('TileD3PDConfig.pkl', 'wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(0 if sc.isSuccess() else 1)

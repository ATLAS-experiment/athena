#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

'''
@file TileSimD3PDConfig.py
@brief Python configuration of Tile D3PD for simulaiton for the Run III
'''


def TileSimD3PDCfg(flags, outputFile=None, saveHitsInfo=True, saveHits=True, saveDigits=None,
                   saveChannels=None, saveCellsInfo=None, saveCells=None, saveMBTS=None):
    ''' Function to configure Tile D3PD for simulaiton.'''

    acc = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    from xAODTruthCnv.xAODTruthCnvConfig import GEN_EVNT2xAODCfg
    acc.merge(GEN_EVNT2xAODCfg(flags, name='GEN_EVNT2xAOD', AODContainerName='TruthEvent'))

    if not outputFile:
        outputFile = f'tiletb_{flags.Input.RunNumbers[0]}.d3pd.root'

    from D3PDMakerCoreComps.MakerAlgConfig import MakerAlgConfig
    alg = MakerAlgConfig(flags, acc, 'truth', outputFile, ExistDataHeader=False)

    from TruthD3PDMaker.TruthParticleD3PDObject  import TruthParticleD3PDObject
    alg += TruthParticleD3PDObject(1)

    if saveHitsInfo:
        from CaloSysD3PDMaker.TileHitInfoD3PDObject import TileHitInfoD3PDObject
        alg += TileHitInfoD3PDObject(0, sgkey='TileHitVec', prefix='Tile_')

    if saveHits:
        from CaloSysD3PDMaker.TileHitD3PDObject import TileHitD3PDObject
        alg += TileHitD3PDObject(0, prefix='TileHit_')

    if saveDigits:
        from CaloSysD3PDMaker.TileDigitD3PDObject import TileDigitD3PDObject
        alg += TileDigitD3PDObject(1, prefix='tiledigit_', sgkey='TileDigitsCnt')

    if saveChannels:
        from CaloSysD3PDMaker.TileRawChannelD3PDObject import TileRawChannelD3PDObject
        alg += TileRawChannelD3PDObject(2, prefix='tileraw_', sgkey='TileRawChannelOpt2')

    if saveCellsInfo:
        from CaloSysD3PDMaker.CaloInfoD3PDObject import CaloInfoD3PDObject
        alg += CaloInfoD3PDObject(0, sgkey='AllCalo', prefix='calo_')

    if saveCells:
        from CaloSysD3PDMaker.TileDetailsD3PDObject import TileDetailsD3PDObject
        alg += TileDetailsD3PDObject(1, sgkey='AllCalo', prefix='tile_', Kinematics_WriteEtaPhi=True)

    if saveMBTS:
        from CaloD3PDMaker.MBTSD3PDObject import MBTSD3PDObject
        alg += MBTSD3PDObject(1, prefix='mbts_', sgkey='MBTSContainer')

    acc.addEventAlgo(alg.alg)

    acc.setAppProperty('HistogramPersistency', 'ROOT')

    return acc


if __name__ == '__main__':

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.HITS_RUN3
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs()

    log.info('FINAL CONFIG FLAGS SETTINGS FOLLOW')
    flags.dump()

    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    cfg.merge(TileSimD3PDCfg(flags))

    cfg.printConfig(withDetails=True, summariseProps=True)

    cfg.store( open('TileSimOutputConfig.pkl', 'wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(0 if sc.isSuccess() else 1)

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def _args (level, name, kwin, **kw):
    kw = kw.copy()
    kw['level'] = level
    for (k, v) in kwin.items():
        if k.startswith (name + '_'):
            kw[k[len(name)+1:]] = v
    return kw

def caloD3PDCfg (flags, stream = 'Calo', file = 'egamma.root',
                   level = 10,
                   clevel = 6,
                   autoflush = -1,
                   **kw):
    acc = ComponentAccumulator()

    from D3PDMakerCoreComps.MakerAlgConfig import MakerAlgConfig
    alg = MakerAlgConfig (flags, acc, stream, file,
                          clevel = clevel,
                          autoflush = autoflush)

    from LumiBlockComps.LumiBlockMuWriterConfig import LumiBlockMuWriterCfg
    acc.merge(LumiBlockMuWriterCfg(flags))

    from EventCommonD3PDMaker.EventInfoD3PDObject import EventInfoD3PDObject
    alg += EventInfoD3PDObject        (**_args (0, 'EventInfo', kw))

    from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
    acc.merge( TileBadChannelsCondAlgCfg(configFlags))

    from CaloD3PDMaker.CaloCellD3PDObject import AllCaloCellD3PDObject
    alg += AllCaloCellD3PDObject      (**_args (level, 'Detail2', kw))

    acc.addEventAlgo (alg.alg)

    return acc

if __name__=='__main__':

    from D3PDMakerConfig.D3PDMakerFlags import configFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    configFlags.fillFromArgs()

    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    configFlags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3

    configFlags.lock()
    
    cfg = MainServicesCfg (configFlags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge (PoolReadCfg (configFlags))

    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    cfg.merge(CaloNoiseCondAlgCfg(configFlags,noisetype="totalNoise"))

    from LumiBlockComps.LuminosityCondAlgConfig import  LuminosityCondAlgCfg
    cfg.merge(LuminosityCondAlgCfg(configFlags))

    cfg.merge (caloD3PDCfg(configFlags, file = configFlags.D3PD.OutputFile)) 

    sc = cfg.run (configFlags.Exec.MaxEvents)

    import sys
    sys.exit (sc.isFailure())

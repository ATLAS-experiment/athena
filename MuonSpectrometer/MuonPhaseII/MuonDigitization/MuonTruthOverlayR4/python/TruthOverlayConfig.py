# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def TruthOverlayCfg(flags, name="TruthOverlay", **kwargs) :
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    result = ComponentAccumulator()

    kwargs.setdefault("WriteKey",  "")
    kwargs.setdefault("SignalInputKey", "{prefix}{sdoKey}".format(prefix=flags.Overlay.SigPrefix, 
                                                                  sdoKey= kwargs["WriteKey"]))
    kwargs.setdefault("BkgInputKey", "{prefix}{sdoKey}".format(prefix=flags.Overlay.BkgPrefix,
                                                               sdoKey=kwargs["WriteKey"]))
    
    if kwargs["BkgInputKey"]:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        result.merge(SGInputLoaderCfg(flags, ["xAOD::MuonSimHitContainer#{sdoKey}".format(sdoKey= kwargs["BkgInputKey"]),
                                              "xAOD::MuonSimHitAuxContainer#{sdoKey}Aux.".format(sdoKey= kwargs["BkgInputKey"])]))

    
    # Setup output
    if flags.Output.doWriteRDO:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        result.merge(OutputStreamCfg(flags, "RDO", ItemList=["xAOD::MuonSimHitContainer#{sdoKey}".format(sdoKey= kwargs["WriteKey"]),
                                                             "xAOD::MuonSimHitAuxContainer#{sdoKey}Aux.".format(sdoKey= kwargs["WriteKey"])]))

    
    the_alg = CompFactory.MuonR4.TruthOverlay(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

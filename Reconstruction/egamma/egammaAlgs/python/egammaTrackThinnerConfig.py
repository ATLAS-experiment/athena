# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def egammaTrackThinnerCfg(
        flags,
        name='egammaTrackThinner',
        **kwargs):

    mlog = logging.getLogger(name)
    mlog.info('Starting configuration')

    acc = ComponentAccumulator()

    kwargs.setdefault("StreamName", "StreamAOD")
    kwargs.setdefault("InputElectronContainerName",
                      flags.Egamma.Keys.Output.Electrons)
    kwargs.setdefault("InputPhotonContainerName",
                      flags.Egamma.Keys.Output.Photons)
    if flags.Detector.GeometryITk:
        kwargs.setdefault("InputForwardElectronContainerName",
                        flags.Egamma.Keys.Output.ForwardElectrons)
    kwargs.setdefault("TrackParticleContainerName",
                      flags.Egamma.Keys.Output.GSFTrackParticles)
    kwargs.setdefault("VertexContainerName",
                      flags.Egamma.Keys.Output.ConversionVertices)

    acc.addEventAlgo(CompFactory.egammaTrackThinner(name, **kwargs))

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.ESD

    acc = MainServicesCfg(flags)
    mlog = logging.getLogger("egammaTrackThinnerConfigTest")
    mlog.info("Configuring  egammaTrackThinner: ")
    acc.merge(egammaTrackThinnerCfg(flags))
    printProperties(mlog,
                    acc.getEventAlgo("egammaTrackThinner"),
                    nestLevel=1,
                    printDefaults=True)
    with open("egammatrackthinner.pkl", "wb") as f:
        acc.store(f)

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from RecExRecoTest.RecExReco_setupData22 import RecExReco_setupData22
    RecExReco_setupData22(flags)
    from egammaConfig.egammaOnlyFromRawFlags import egammaOnlyFromRaw
    egammaOnlyFromRaw(flags)
    flags.lock()

    from RecJobTransforms.RecoSteering import RecoSteering
    acc = RecoSteering(flags)

    with open("config.pkl", "wb") as file:
        acc.store(file)

    acc.run(100)

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    # Required for MT using --threads argument
    flags.fillFromArgs(parser=flags.getArgumentParser())

    from RecExRecoTest.RecExReco_setupData22 import RecExReco_setupData22
    RecExReco_setupData22(flags)
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from CaloRec.CaloRecoConfig import CaloRecoCfg
    acc.merge(CaloRecoCfg(flags))

    acc.run(100)

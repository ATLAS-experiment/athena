# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from BoostedJetTaggerConfig import WZTagAlgCfg


if __name__=='__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO 
    log.setLevel(INFO)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    fileName = "/eos/atlas/atlascerngroupdisk/perf-jets/TreeStorage/TAGGING/test-files/DAOD/mc20_13TeV.701125.Sh_2214_WlvWqq.deriv.DAOD_PHYS.e8547_s3797_r13145_p7018/DAOD_PHYS.46768158._000231.pool.root.1"

    flags = initConfigFlags()
    flags.Input.Files = [fileName]

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    # config file
    config_file = "/eos/atlas/atlascerngroupdisk/perf-jets/LocalStorage/TAGGING/PreliminaryConfigs/WTagger/"

    # algorithm for one WP
    testacc = WZTagAlgCfg(flags, tagger='WZ', generation='ParT', WP='50',
                          cfg_file=config_file)
    cfg.merge(testacc)

    # and for a second WP
    testacc_2WP = WZTagAlgCfg(flags, tagger='WZ', generation='ParT', WP='80',
                              cfg_file=config_file)
    cfg.merge(testacc_2WP)

    # and for another generation
    testacc_2gen = WZTagAlgCfg(flags, tagger='WZ', generation='ParT_MassDec', WP='50',
                               cfg_file=config_file)
    cfg.merge(testacc_2gen)

    cfg.run(15)

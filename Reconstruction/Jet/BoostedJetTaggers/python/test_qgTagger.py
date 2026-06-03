# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from BoostedJetTaggerConfig import qgTagAlgCfg


if __name__=='__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG 
    log.setLevel(DEBUG)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    fileName = "/eos/atlas/atlascerngroupdisk/perf-jets/TreeStorage/TAGGING/test-files/DAOD/mc20_13TeV.701125.Sh_2214_WlvWqq.deriv.DAOD_PHYS.e8547_s3797_r13145_p7018/DAOD_PHYS.46768158._000231.pool.root.1"

    flags = initConfigFlags()
    flags.Input.Files = [fileName]

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))


    testacc = qgTagAlgCfg(flags,
                          WP='50',
                          addSFs=True,
                          run='run2')

    cfg.merge(testacc)
    cfg.run(15)

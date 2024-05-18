# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
#
# File: CaloClusterCorrection/python/WriteCorrectionsConfig.py
# Author: scott snyder <snyder@bnl.gov>
# Date: May 2024, from old config version
# Purpose: Helpers for configuring jobs to write correction constants.
#


from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def WriteCorrectionsFlags (coolfile):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from Campaigns.Utils import Campaign

    flags = initConfigFlags()
    flags.Exec.MaxEvents = 1
    flags.Input.isMC = True
    flags.IOVDb.DBConnection = f'sqlite://;schema=${coolfile};dbname=OFLP200'
    flags.IOVDb.GlobalTag = 'None'
    flags.IOVDb.DatabaseInstance = ''
    flags.Input.MCCampaign = Campaign.Unknown
    flags.fillFromArgs()
    flags.lock()
    return flags


def WriteCorrectionsCfg (flags, poolfile,
                         corr_output_list,
                         tag_list):
    cfg = ComponentAccumulator()

    from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
    cfg.merge (OutputConditionsAlgCfg (flags,
                                       outputFile = poolfile,
                                       ObjectList = corr_output_list,
                                       WriteIOV = True,
                                       IOVTagList = tag_list))

    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
    cfg.merge (IOVDbSvcCfg (flags))

    cfg.addService (CompFactory.IOVRegistrationSvc (writeKeyInfo = False))
    return cfg


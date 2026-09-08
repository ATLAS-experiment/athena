#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Test of Trigger config flags autoconfiguration
# Exercised on BS data and MC POOL file formats

import os
import sys

from AthenaCommon.Logging import logging
log = logging.getLogger('test_TriggerFlags_autoconf')

from AthenaCommon.Constants import DEBUG
from TriggerJobOpts.TriggerConfigFlags import log as tcf_log
tcf_log.setLevel(DEBUG)

from AthenaConfiguration.TestDefaults import defaultTestFiles
from TrigValTools.TrigValSteering import Input
inputfiles = {
    # name     : (EDMVersion, filename)
    "Run1_Data": (1, Input.get_input('data_run1').paths[0]),
    "Run2_Data": (2, Input.get_input('data_run2_EB').paths[0]),
    "Run3_Data": (3, Input.get_input('data').paths[0]),
    "Run2_MC":   (2, defaultTestFiles.AOD_RUN2_MC[0]),
    "Run3_MC":   (3, defaultTestFiles.AOD_RUN3_MC[0]),
}


def test_TriggerFlags(sample):

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = [inputfiles[sample][1]]

    def __isAnalysis():
        return "AthAnalysis_DIR" in os.environ

    def __trigger():
        from TriggerJobOpts.TriggerConfigFlags import createTriggerFlags
        return createTriggerFlags(not __isAnalysis())

    flags.addFlagsCategory("Trigger", __trigger)
    flags.lock()

    EDMDecode_ref = inputfiles[sample][0]
    diff = f"EDMVersion: expected {EDMDecode_ref}, configured {flags.Trigger.EDMVersion}"
    if flags.Trigger.EDMVersion != EDMDecode_ref:
        log.error(diff)
        return False
    else:
        log.info(diff)
        return True

    
if __name__=="__main__":

    ok = True
    for sample, (version, filename) in inputfiles.items():
        log.info("%s input file: %s", sample, filename)
        ok &= test_TriggerFlags(sample)

    sys.exit(0 if ok else 1)

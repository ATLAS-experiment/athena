# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createTrigCTPConfigFlags():
    flags = AthConfigFlags()

    flags.addFlag('Trigger.CTP.UseEDMxAOD', False, help='Use xAOD EDM (xAOD::CTPResult) instead of Run 2 EDM (ROIB::CTP_RDO)')

    return flags


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2

    flags.lock()
    flags.dump("CTP|Trigger")

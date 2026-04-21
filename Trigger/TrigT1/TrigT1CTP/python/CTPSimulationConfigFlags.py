# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import LHCPeriod

# For the case of the ZDC CI tests, we do not want to use xAOD::CTPResult and/or the CTP ROB
def zdcOnly(flags):
    return (
        flags.Detector.GeometryZDC and
        not flags.Detector.GeometryAFP and
        not flags.Detector.GeometryALFA and
        not flags.Detector.GeometryLucid and
        not flags.Detector.GeometryMDT and
        not flags.Detector.GeometryMM and
        not flags.Detector.GeometryMuon
    )

def createTrigCTPConfigFlags():
    flags = AthConfigFlags()

    flags.addFlag('Trigger.CTP.UseEDMxAOD',lambda flags: True if (flags.GeoModel.Run >= LHCPeriod.Run3 and not flags.Trigger.doHLT and not zdcOnly(flags)) else False,
        help='Use xAOD EDM (xAOD::CTPResult) instead of Run 2 EDM (ROIB::CTP_RDO)'
    )

    flags.addFlag('Trigger.CTP.UseRoibROB',lambda flags: False if (flags.GeoModel.Run >= LHCPeriod.Run3 and not flags.Trigger.doHLT and not zdcOnly(flags)) else True,
        help='Use ROIB ROB which includes only L1A bunch information, or CTP ROB with information about all bunches in the readout window (default window of +/-1 bunch around L1A bunch for Run 3).'
    )

    return flags


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2

    flags.lock()
    flags.dump("CTP|Trigger")

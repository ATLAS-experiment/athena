# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createFPGAMgmtFlags():
    """
    flag generator for AthXRTServices FPGA management
    """

    flags = AthConfigFlags()
    flags.addFlag('FPGAMgmt.HLSDir', '')
    return flags



if __name__ == "__main__":

    flags = createFPGAMgmtFlags()
    flags.dump()

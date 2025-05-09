# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def createITkAlignFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    icf = AthConfigFlags()

    icf.addFlag("accumulate", False)
    icf.addFlag("baseDir", "./")
    icf.addFlag("alignITk", False)
    icf.addFlag("alignITkPixel", False)
    icf.addFlag("alignITkStrip", False)
    icf.addFlag("useLocalDatabase", False)
    icf.addFlag("writeSilicon", False)
    icf.addFlag("writeAlignNtuple", False)
    icf.addFlag("doMonitoring", False)
    icf.addFlag("inputTFiles", "AlignmentTFile.root")
    icf.addFlag("solveLocal", True)
    icf.addFlag("writeConstantsToPool", True)
    icf.addFlag("writeDynamicDB", True)
    icf.addFlag("tagSi", "ITkAlign_test")
    icf.addFlag("outputConditionFile", "alignment_output.pool.root")

    return icf
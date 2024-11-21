# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from enum import Enum, auto


class FlagEnum(Enum):
    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            raise TypeError(f"Invalid comparison of {self.__class__} with {type(other)}")
        return self is other

    __hash__ = Enum.__hash__


class Project(FlagEnum):
    Athena = 'Athena'
    AthAnalysis = 'AthAnalysis'
    AthDerivation = 'AthDerivation'
    AthGeneration = 'AthGeneration'
    AthSimulation = 'AthSimulation'
    AnalysisBase = 'AnalysisBase'

    @classmethod
    def determine(cls):
        import os
        if "AthSimulation_DIR" in os.environ:
            return cls.AthSimulation
        if "AthGeneration_DIR" in os.environ:
            return cls.AthGeneration
        if "AthAnalysis_DIR" in os.environ:
            return cls.AthAnalysis
        if "AthDerivation_DIR" in os.environ:
            return cls.AthDerivation
        if "AnalysisBase_DIR" in os.environ:
            return cls.AnalysisBase
        return cls.Athena


class Format(FlagEnum):
    BS = 'BS'
    POOL = 'POOL'


class ProductionStep(FlagEnum):
    # steps should be added when needed
    Default = 'Default'
    Generation = 'Generation'
    Simulation = 'Simulation'
    PileUpPresampling = 'PileUpPresampling'
    Overlay = 'Overlay'
    MinbiasPreprocessing = 'MinbiasPreprocessing'
    FastChain = 'FastChain'
    Digitization = 'Digitization'
    PileUpPretracking = 'PileUpPretracking'
    Reconstruction = 'Reconstruction'
    Derivation = 'Derivation'


def validrunformat(lhcperiod):
    import re
    for item in lhcperiod.__members__.values():
        if not re.match("^RUN[0-9]$", item.value):
            raise ValueError("Value not in a format RUN+single digit %s", item.value)
    return lhcperiod


@validrunformat
class LHCPeriod(FlagEnum):
    def __lt__(self, other):      #operator specific to validrunformat
        if not isinstance(other, self.__class__):
            raise TypeError(f"Invalid comparison of {self.__class__} with {type(other)}")
        else:
            return self.value < other.value
    def __le__(self, other):
        if not isinstance(other, self.__class__):
            raise TypeError(f"Invalid comparison of {self.__class__} with {type(other)}")
        else:
            return self.value <= other.value

    Run1 = 'RUN1'
    Run2 = 'RUN2'
    Run3 = 'RUN3'
    Run4 = 'RUN4'


class BeamType(FlagEnum):
    Collisions = 'collisions'
    SingleBeam = 'singlebeam'
    Cosmics = 'cosmics'
    TestBeam = 'testbeam'
    

class BunchStructureSource(FlagEnum):
    FILLPARAMS = 0
    MC = 1
    TrigConf = 2
    Lumi = 3


class MetadataCategory(FlagEnum):
    FileMetaData = auto()
    EventStreamInfo = auto()
    EventFormat = auto()
    CutFlowMetaData = auto()
    ByteStreamMetaData = auto()
    LumiBlockMetaData = auto()
    TriggerMenuMetaData = auto()
    TruthMetaData = auto()


class HIMode(FlagEnum):
    pp  = "pp"
    HI  = "hi"
    HIP = "hip"
    UPC = "upc"

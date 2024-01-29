# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.Enums import FlagEnum


class BeamPipeSimMode(FlagEnum):
    Normal = 'Normal'
    FastSim = 'FastSim'
    EGammaRangeCuts = 'EGammaRangeCuts'
    EGammaPRangeCuts = 'EGammaPRangeCuts'


class CalibrationRun(FlagEnum):
    LAr = 'LAr'
    Tile = 'Tile'
    ZDC = 'ZDC'
    LArTile = 'LAr+Tile'
    LArTileZDC = 'LAr+Tile+ZDC'
    DeadLAr = 'DeadLAr'
    Off = 'Off'


class CavernBackground(FlagEnum):
    Off = 'Off'
    Write = 'Write'
    Read = 'Read'
    Signal = 'Signal'
    WriteWorld = 'WriteWorld'
    SignalWorld = 'SignalWorld'


class LArParameterization(FlagEnum):
    NoFrozenShowers = 0
    FrozenShowers = 1
    DeadMaterialFrozenShowers = 2
    FrozenShowersFCalOnly = 3
    FastCaloSim = 4

class PixelRadiationDamageSimulationType(FlagEnum):
    NoRadiationDamage = 0
    RamoPotential = 1
    TemplateCorrection = 2

class SimulationFlavour(FlagEnum):
    Unknown = 'Unknown'
    FullG4MT = 'FullG4MT'
    FullG4MT_QS = 'FullG4MT_QS'
    PassBackG4MT = 'PassBackG4MT'
    ATLFASTIIMT = 'ATLFASTIIMT'
    ATLFASTIIFMT = 'ATLFASTIIFMT'
    ATLFASTIIF_G4MS = 'ATLFASTIIF_G4MS'
    ATLFAST3MT = 'ATLFAST3MT'
    ATLFAST3MT_QS = 'ATLFAST3MT_QS'
    ATLFAST3F_G4MS = 'ATLFAST3F_G4MS'
    ATLFAST3F_ACTSMT = 'ATLFAST3F_ACTSMT'
    AtlasG4 = 'AtlasG4' # Outside ISF
    AtlasG4_QS = 'AtlasG4_QS' # Outside ISF
    CosmicsG4 = 'CosmicsG4'

    def isFullSim(self):
        return 'FullG4' in self.value or 'PassBackG4' in self.value or 'AtlasG4' in self.value or 'CosmicsG4' in self.value

    def usesFastCaloSim(self):
        return 'ATLFAST' in self.value

    def usesFatras(self):
        return 'ATLFASTIIF' in self.value or 'ATLFAST3F' in self.value # TODO Extend for Acts::Fatras in the future

    def isQuasiStable(self):
        return 'QS' in self.value

    def isMT(self):
        return 'MT' in self.value


class TruthStrategy(FlagEnum):
    MC12 = 'MC12'
    MC12LLP = 'MC12LLP'
    MC12Plus = 'MC12Plus'
    MC15 = 'MC15'
    MC15a = 'MC15a'
    MC15aPlus = 'MC15aPlus'
    MC15aPlusLLP = 'MC15aPlusLLP'
    MC16 = 'MC16'
    MC16LLP = 'MC16LLP'
    MC18 = 'MC18'
    MC18LLP = 'MC18LLP'
    PhysicsProcess = 'PhysicsProcess'
    Global = 'Global'
    Validation = 'Validation'
    Cosmic = 'Cosmic'


class VertexSource(FlagEnum):
    CondDB = 'CondDB'
    VertexOverrideFile = 'VertexOverrideFile'
    VertexOverrideEventFile = 'VertexOverrideEventFile'
    LongBeamspotVertexPositioner = 'LongBeamspotVertexPositioner'
    AsGenerated = 'AsGenerated' # I.e. no shift

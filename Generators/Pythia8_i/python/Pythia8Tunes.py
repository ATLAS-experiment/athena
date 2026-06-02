# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


# Pythia 4C tune
def _4C_cmds():
    return [
        
        "Tune:pp = 5",
        "SigmaProcess:alphaSvalue = 0.135",
        "SigmaTotal:zeroAXB = true",
        "SigmaDiffractive:dampen = true",
        "SigmaDiffractive:maxXB = 65.0",
        "SigmaDiffractive:maxAX = 65.0",
        "SigmaDiffractive:maxXX = 65.0",
        "Diffraction:largeMassSuppress = 2.0",
        "TimeShower:dampenBeamRecoil = true  ",
        "TimeShower:phiPolAsym  = true  ",
        "SpaceShower:alphaSvalue = 0.137 ",
        "SpaceShower:alphaSorder = 1     ",
        "SpaceShower:alphaSuseCMW = false ",
        "SpaceShower:samePTasMPI = false ",
        "SpaceShower:pT0Ref = 2.0   ",
        "SpaceShower:ecmRef = 1800.0",
        "SpaceShower:ecmPow = 0.0   ",
        "SpaceShower:pTmaxFudge = 1.0   ",
        "SpaceShower:pTdampFudge = 1.0   ",
        "SpaceShower:rapidityOrderMPI = true  ",
        "SpaceShower:phiPolAsym = true  ",
        "SpaceShower:phiIntAsym = true  ",
        "MultipartonInteractions:alphaSvalue = 0.135",
        "MultipartonInteractions:ecmRef = 1800. ",
        "MultipartonInteractions:expPow = 2.0   ",
        "BeamRemnants:primordialKTsoft = 0.5   ",
        "BeamRemnants:primordialKThard = 2.0   ",
        "BeamRemnants:halfScaleForKT = 1.0   ",
        "BeamRemnants:halfMassForKT = 1.0   ",
        "ColourReconnection:mode = 0  ",
    ]


# A2 tune
def _a2_specific_cmds():
    return [ 
        "MultipartonInteractions:bProfile = 4",
        "MultipartonInteractions:a1 = 0.03",
        "MultipartonInteractions:pT0Ref = 1.90",
        "MultipartonInteractions:ecmPow = 0.30",
        "SpaceShower:rapidityOrder = 0",
        "PDF:pSet = LHAPDF6:MSTW2008lo68cl",
        "ColourReconnection:range = 2.28",
    ]


# A2 tune with MSTW2008lo PDF set
def a2_mstw2008lo_tune_cmds():
    return _4C_cmds() + _a2_specific_cmds()


# Monash 2013 ee tune
def _monash2013_ee_cmds():
    return [
        "Tune:ee = 7",
        "StringFlav:probStoUD = 0.217",
        "StringFlav:probQQtoQ = 0.081",
        "StringFlav:probSQtoQQ = 0.915",
        "StringFlav:probQQ1toQQ0 = 0.0275",
        "StringFlav:mesonUDvector = 0.50",
        "StringFlav:mesonSvector = 0.55",
        "StringFlav:mesonCvector = 0.88",
        "StringFlav:mesonBvector = 2.20",
        "StringFlav:etaSup = 0.60",
        "StringFlav:etaPrimeSup = 0.12",
        "StringFlav:popcornSpair = 0.90",
        "StringFlav:popcornSmeson = 0.50",
        "StringFlav:suppressLeadingB = false",
        "StringZ:aLund = 0.68",
        "StringZ:bLund = 0.98",
        "StringZ:aExtraSquark = 0.00",
        "StringZ:aExtraDiquark = 0.97",
        "StringZ:rFactC = 1.32",
        "StringZ:rFactB = 0.855",
        "StringPT:sigma = 0.335",
        "StringPT:enhancedFraction = 0.01",
        "StringPT:enhancedWidth = 2.0",
        "TimeShower:alphaSorder = 1",
        "TimeShower:alphaSuseCMW = false",
        "TimeShower:pTmin = 0.5",
        "TimeShower:pTminChgQ = 0.5",
        "StringZ:useOldAExtra = on",
    ]


# Monash ee tune
def _monash2013_pp_cmds():
    return [
        "Tune:pp = 14",
        "SigmaTotal:zeroAXB = true",
        "SigmaDiffractive:dampen = true",
        "SigmaDiffractive:maxXB = 65.0",
        "SigmaDiffractive:maxAX = 65.0",
        "SigmaDiffractive:maxXX = 65.0",
        "Diffraction:largeMassSuppress = 4.0",
        "TimeShower:dampenBeamRecoil = true",
        "TimeShower:phiPolAsym = true",
        "SpaceShower:alphaSorder = 1",
        "SpaceShower:alphaSuseCMW = false",
        "SpaceShower:samePTasMPI = false",
        "SpaceShower:ecmRef = 7000.0",
        "SpaceShower:ecmPow = 0.0",
        "SpaceShower:rapidityOrderMPI = true",
        "SpaceShower:phiPolAsym = true",
        "SpaceShower:phiIntAsym = true",
        "MultipartonInteractions:ecmRef = 7000.",
        "MultipartonInteractions:ecmPow = 0.215",
        "MultipartonInteractions:bProfile = 3",
        "MultipartonInteractions:expPow = 1.85",
        "MultipartonInteractions:a1 = 0.15",
        "BeamRemnants:primordialKTsoft = 0.9",
        "BeamRemnants:halfScaleForKT = 1.5",
        "BeamRemnants:halfMassForKT = 1.0",
        "ColourReconnection:mode = 0",
    ]


# A14 tune
def _a14_specific_cmds():
    return [
        "SpaceShower:rapidityOrder = on",
        "SigmaProcess:alphaSvalue = 0.140",
        "SpaceShower:pT0Ref = 1.56",
        "SpaceShower:pTmaxFudge = 0.91",
        "SpaceShower:pTdampFudge = 1.05",
        "SpaceShower:alphaSvalue = 0.127",
        "TimeShower:alphaSvalue = 0.127",
        "BeamRemnants:primordialKThard = 1.88",
        "MultipartonInteractions:pT0Ref = 2.09",
        "MultipartonInteractions:alphaSvalue = 0.126",
        "PDF:pSet = LHAPDF6:NNPDF23_lo_as_0130_qed",
        "ColourReconnection:range = 1.71",
    ]


def a14_nnpdf23lo_tune_cmds():
    return (
        _monash2013_ee_cmds()
      + _monash2013_pp_cmds()
      + _a14_specific_cmds()
    )

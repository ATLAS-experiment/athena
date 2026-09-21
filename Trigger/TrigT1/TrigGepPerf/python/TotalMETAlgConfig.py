# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


# Block sizes the GEP JwoJ hard term supports. Only sizes that tile BOTH axes of the
# tower grid exactly are allowed: 1 and 2 both divide it (phi 64 = 32x2, eta 98 = 49x2),
# 3 divides neither (phi 64 = 21x3 + 1, eta 98 = 32x3 + 2). A ragged block at the phi
# seam would be judged against the same threshold as its full neighbors, pushing a phi
# slice systematically into the soft term -- i.e. giving MET a preferred direction.
# TotalMETAlg re-checks this against the configured grid and fails if it does not hold.
GEP_JWOJ_BLOCK_SIZES = (1, 2)


def GepTotalMETAlgCfg(
        flags,
        name,
        caloClustersKey,
        gepJetsKey,

        # ---- outputs: one EnergySumRoI per enabled flavor -------------------
        # An enabled flavor with an empty key is rejected at initialize() rather than
        # silently computed and dropped.
        outputTotalMETKey='',
        outputJetMETKey='',
        outputTowerMETKey='',
        outputJwoJMETKey='',
        outputJwoJHardMETKey='',
        outputJwoJSoftMETKey='',

        # ---- algorithm enables ---------------------------------------------
        # Only total MET by default. The jet and tower terms are computed either way,
        # since total MET is their weighted sum; these decide what gets recorded.
        doTotalMET=True,
        doJetMET=False,
        doTowerMET=False,
        doGEPJwoJMET=False,

        # ---- physics thresholds / flow -------------------------------------
        # The jet cone radius and the jet multiplicity are NOT settable here: both are
        # properties of the upstream WTACone algorithm that produced gepJetsKey -- the
        # radius is its WTAJet_dR and the multiplicity is how many jets it emits -- so
        # MET takes what it is given rather than declaring its own.
        jetEtThresholdGeV=0.0,
        towerEtThresholdGeV=0.0,
        doJetTowerOverlapRemoval=False,
        towerScaleFactor=1.0,
        jetScaleFactor=1.0,

        # ---- multiplicities -------------------------------------------------
        maxTowersConsidered=4096,

        # ---- digitization: field widths -------------------------------------
        etBitLength=13,
        signedEtBitLength=13,
        etaBitLength=7,
        phiBitLength=6,
        sinBitLength=13,

        # ---- digitization: the tower grid -----------------------------------
        # Sized by index COUNTS, never by the field widths above.
        etaRange=98,
        phiRange=64,
        etaMin=-4.85,
        etaGranularity=0.1,

        # ---- digitization: E_T ----------------------------------------------
        etMin=0.0,
        etMax=2048.0,           # with etBitLength 13 this is the 0.25 GeV LSB
        inputEtToGeV=1.0e-3,

        # ---- MET output azimuth ---------------------------------------------
        # Unlike the tower grid, the OUTPUT azimuth is sized by its field width: it comes
        # from arctan(Ey, Ex), so nothing physical caps its resolution and widening the
        # field resolves more finely. metPhiTanScaleBitLength must grow with it -- roughly
        # metPhiBitLength + 4 to stay within 0.501 bins of ideal atan2 binning.
        metPhiBitLength=6,
        metPhiTanScaleBitLength=10,

        # ---- MET magnitude LUT ----------------------------------------------
        sqrtMantissaBitLength=9,
        sqrtFracBitLength=13,
        sqrtCoeffBitLength=14,
        sqrtRadicandBitLength=26,

        # ---- GEP JwoJ --------------------------------------------------------
        # The hard/soft coefficients are SEPARATE from the jet/tower scale factors, unlike
        # the standalone emulation which reuses that pair. That is what lets total MET run
        # at unit weights while GEP JwoJ uses its own soft coefficient.
        jwojHardEtThresholdGeV=7.5,
        jwojBlockSize=1,
        jwojHardCoeff=1.0,
        jwojSoftCoeff=0.3,

        OutputLevel=None):
    """Configure TotalMETAlg, the bitwise GEP MET emulation.

    Every value written is in GeV, matching the standalone metEmulation.cc metTree and
    NOT the MeV convention of GepMETAlg. The two are separate containers, so they do not
    mix in the ntuple.

    The pileup-suppression variant is chosen purely by which containers caloClustersKey
    and gepJetsKey point at; there is no SK/EtaSK/NoSK switch here.
    """

    cfg = ComponentAccumulator()

    if not caloClustersKey:
        raise ValueError("caloClustersKey must be set (tower collection for the tower MET term)")
    if not gepJetsKey:
        raise ValueError("gepJetsKey must be set (jet collection for the jet MET term)")

    # Caught here as well as in C++ so a bad grid fails at configure time with the Python
    # traceback pointing at the caller, rather than in initialize().
    if jwojBlockSize not in GEP_JWOJ_BLOCK_SIZES:
        raise ValueError(f"jwojBlockSize must be one of {GEP_JWOJ_BLOCK_SIZES}; "
                         f"got {jwojBlockSize}. Only these tile the tower grid exactly.")
    if doGEPJwoJMET:
        if phiRange % jwojBlockSize or etaRange % jwojBlockSize:
            raise ValueError(f"jwojBlockSize {jwojBlockSize} does not tile the configured "
                             f"grid ({phiRange} phi x {etaRange} eta)")
    if metPhiBitLength < 3:
        raise ValueError("metPhiBitLength must be at least 3 so the azimuth divides into octants")
    if etMax <= etMin:
        raise ValueError("etMax must exceed etMin")

    # An enabled flavor needs somewhere to go.
    for enabled, key, flag, keyName in (
            (doTotalMET,   outputTotalMETKey,    'doTotalMET',   'outputTotalMETKey'),
            (doJetMET,     outputJetMETKey,      'doJetMET',     'outputJetMETKey'),
            (doTowerMET,   outputTowerMETKey,    'doTowerMET',   'outputTowerMETKey'),
            (doGEPJwoJMET, outputJwoJMETKey,     'doGEPJwoJMET', 'outputJwoJMETKey'),
            (doGEPJwoJMET, outputJwoJHardMETKey, 'doGEPJwoJMET', 'outputJwoJHardMETKey'),
            (doGEPJwoJMET, outputJwoJSoftMETKey, 'doGEPJwoJMET', 'outputJwoJSoftMETKey')):
        if enabled and not key:
            raise ValueError(f"{flag} is set but {keyName} is empty; nothing would be written")

    alg = CompFactory.TotalMETAlg(
        name,
        caloClustersKey=caloClustersKey,
        gepJetsKey=gepJetsKey,

        outputTotalMETKey=outputTotalMETKey,
        outputJetMETKey=outputJetMETKey,
        outputTowerMETKey=outputTowerMETKey,
        outputJwoJMETKey=outputJwoJMETKey,
        outputJwoJHardMETKey=outputJwoJHardMETKey,
        outputJwoJSoftMETKey=outputJwoJSoftMETKey,

        DoTotalMET=doTotalMET,
        DoJetMET=doJetMET,
        DoTowerMET=doTowerMET,
        DoGEPJwoJMET=doGEPJwoJMET,

        JetEtThresholdGeV=jetEtThresholdGeV,
        TowerEtThresholdGeV=towerEtThresholdGeV,
        DoJetTowerOverlapRemoval=doJetTowerOverlapRemoval,
        TowerScaleFactor=towerScaleFactor,
        JetScaleFactor=jetScaleFactor,

        MaxTowersConsidered=maxTowersConsidered,

        EtBitLength=etBitLength,
        SignedEtBitLength=signedEtBitLength,
        EtaBitLength=etaBitLength,
        PhiBitLength=phiBitLength,
        SinBitLength=sinBitLength,

        EtaRange=etaRange,
        PhiRange=phiRange,
        EtaMin=etaMin,
        EtaGranularity=etaGranularity,

        EtMin=etMin,
        EtMax=etMax,
        InputEtToGeV=inputEtToGeV,

        METPhiBitLength=metPhiBitLength,
        METPhiTanScaleBitLength=metPhiTanScaleBitLength,

        SqrtMantissaBitLength=sqrtMantissaBitLength,
        SqrtFracBitLength=sqrtFracBitLength,
        SqrtCoeffBitLength=sqrtCoeffBitLength,
        SqrtRadicandBitLength=sqrtRadicandBitLength,

        JwoJHardEtThresholdGeV=jwojHardEtThresholdGeV,
        JwoJBlockSize=jwojBlockSize,
        JwoJHardCoeff=jwojHardCoeff,
        JwoJSoftCoeff=jwojSoftCoeff)

    if OutputLevel is not None:
        alg.OutputLevel = OutputLevel

    cfg.addEventAlgo(alg)

    return cfg

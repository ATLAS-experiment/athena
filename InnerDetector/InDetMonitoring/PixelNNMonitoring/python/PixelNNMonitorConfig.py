# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoring import AthMonitorCfgHelper


def _bookPositionGroup(helper, alg, n):
    """Per-particle position-net plots for true multiplicity n."""
    g = helper.addGroup(alg, f'PixelNNPosN{n}',
                        f'/PixelNNMonitor/PositionNet/{n}Particle/')
    rng = 40.0 * n  # um, residual range widens with multiplicity
    g.defineHistogram('resX;h_resX', type='TH1F',
        title=f'{n}-particle residual X (NN - truth);#Deltax [#mum];Entries',
        xbins=100, xmin=-rng, xmax=rng)
    g.defineHistogram('resY;h_resY', type='TH1F',
        title=f'{n}-particle residual Y (NN - truth);#Deltay [#mum];Entries',
        xbins=100, xmin=-rng, xmax=rng)
    g.defineHistogram('pullX;h_pullX', type='TH1F',
        title=f'{n}-particle pull X (NN - truth)/#sigma_{{NN}};pull_{{X}};Entries',
        xbins=100, xmin=-5, xmax=5)
    g.defineHistogram('pullY;h_pullY', type='TH1F',
        title=f'{n}-particle pull Y (NN - truth)/#sigma_{{NN}};pull_{{Y}};Entries',
        xbins=100, xmin=-5, xmax=5)
    g.defineHistogram('errX;h_errX', type='TH1F',
        title=f'{n}-particle predicted #sigma_{{X}};#sigma_{{X}} [#mum];Entries',
        xbins=100, xmin=0, xmax=60)
    g.defineHistogram('errY;h_errY', type='TH1F',
        title=f'{n}-particle predicted #sigma_{{Y}};#sigma_{{Y}} [#mum];Entries',
        xbins=100, xmin=0, xmax=60)
    g.defineHistogram('eta,pullY;h_pullY_vs_eta', type='TH2F',
        title=f'{n}-particle pull Y vs #eta;#eta;pull_{{Y}}',
        xbins=30, xmin=-3, xmax=3, ybins=50, ymin=-5, ymax=5)
    g.defineHistogram('nCell,pullX;h_pullX_vs_nCell', type='TH2F',
        title=f'{n}-particle pull X vs cluster size;N_{{cells}};pull_{{X}}',
        xbins=15, xmin=0.5, xmax=15.5, ybins=50, ymin=-5, ymax=5)
    g.defineHistogram('nCell,pullY;h_pullY_vs_nCell', type='TH2F',
        title=f'{n}-particle pull Y vs cluster size;N_{{cells}};pull_{{Y}}',
        xbins=15, xmin=0.5, xmax=15.5, ybins=50, ymin=-5, ymax=5)


def PixelNNMonitorHistograms(helper, alg):
    """Pixel-NN performance histograms.

    NumberNet/   - number-of-particles (cluster-splitting) classification.
    PositionNet/ - per-particle position residual / pull vs the Geant4 truth,
                   split by true multiplicity (1/2/3 particle). The position net
                   is run with the track estimate as input (as in on-track reco)
                   but the residual is always against truth, never the track.
    """

    # ---------------- Number network ----------------
    num = helper.addGroup(alg, 'PixelNNNumber', '/PixelNNMonitor/NumberNet/')
    num.defineHistogram('predN;h_predN', type='TH1I',
        title='Predicted N particles;N_{pred};Clusters', xbins=3, xmin=0.5, xmax=3.5)
    for p in (1, 2, 3):
        num.defineHistogram(f'prob{p};h_prob{p}', type='TH1F',
            title=f'P({p} particle);P({p});Clusters', xbins=100, xmin=0, xmax=1)
    num.defineHistogram('nClusters;h_nClusters', type='TH1I',
        title='Clusters per event;N_{clusters};Events', xbins=200, xmin=0, xmax=20000)
    # truth-based classification performance
    num.defineHistogram('trueN,predNconf;h_confusion', type='TH2I',
        title='Number-net confusion;N_{true};N_{pred}',
        xbins=3, xmin=0.5, xmax=3.5, ybins=3, ymin=0.5, ymax=3.5)
    num.defineHistogram('isCorrect;h_correct', type='TH1I',
        title='Correctly classified;correct;Clusters', xbins=2, xmin=-0.5, xmax=1.5)
    num.defineHistogram('eta,isCorrect;h_acc_vs_eta', type='TProfile',
        title='Classification accuracy vs #eta;#eta;accuracy', xbins=50, xmin=-3, xmax=3)
    num.defineHistogram('nCell,isCorrect;h_acc_vs_nCell', type='TProfile',
        title='Classification accuracy vs cluster size;N_{cells};accuracy',
        xbins=15, xmin=0.5, xmax=15.5)
    num.defineHistogram('trueN,probMulti;h_probMulti_vs_trueN', type='TH2F',
        title='Split discriminant P(#geq2) vs N_{true};N_{true};P(#geq2)',
        xbins=3, xmin=0.5, xmax=3.5, ybins=50, ymin=0, ymax=1)

    # ---------------- Split fraction ----------------
    # Fraction of clusters classified as split (truth: trueN >= 2; NN: network
    # argmax multiplicity >= 2; reco: ambiguity-solver isSplit), vs the leading
    # track pT, the track incidence angles and the cluster global eta.
    sf = helper.addGroup(alg, 'PixelNNSplitFrac', '/PixelNNMonitor/SplitFraction/')
    ptEdges = [0.5 * 100.0 ** (i / 20.0) for i in range(21)]  # log, 0.5 to 50 GeV
    for var, edges, label in (
            ('trackPt',  ptEdges,                            'p_{T}^{lead} [GeV]'),
            ('trkPhi',   {'nb': 24, 'lo': -0.6, 'hi': 0.6},  'track incidence #phi [rad]'),
            ('trkTheta', {'nb': 24, 'lo': -1.2, 'hi': 1.2},  'track incidence #theta [rad]'),
            ('clusEta',  {'nb': 40, 'lo': -4.0, 'hi': 4.0},  'cluster #eta')):
        for curve, cname in (('truthSplit', 'Truth'), ('nnSplit', 'NN'),
                             ('recoSplit', 'Reco')):
            binning = ({'xbins': edges} if isinstance(edges, list) else
                       {'xbins': edges['nb'], 'xmin': edges['lo'],
                        'xmax': edges['hi']})
            sf.defineHistogram(f'{var},{curve};h_{curve}_vs_{var}',
                type='TProfile',
                title=f'{cname} split fraction;{label};split fraction',
                **binning)

    # ---------------- Position network: truth-free monitoring ----------------
    # Wide-range offsets against the cluster position, predicted uncertainties
    # and MDN precisions; catches pathological outputs on data and simulation.
    dq = helper.addGroup(alg, 'PixelNNPosDQ', '/PixelNNMonitor/PositionNet/DQ/')
    dq.defineHistogram('posDeltaXWide;h_deltaX_wide', type='TH1F',
        title='#DeltaX wide range;#DeltaX [mm];Entries', xbins=200, xmin=-1.0, xmax=1.0)
    dq.defineHistogram('posDeltaYWide;h_deltaY_wide', type='TH1F',
        title='#DeltaY wide range;#DeltaY [mm];Entries', xbins=200, xmin=-5.0, xmax=5.0)
    dq.defineHistogram('posErrXWide;h_errX_wide', type='TH1F',
        title='#sigma_{X} wide range;#sigma_{X} [mm];Entries', xbins=200, xmin=0, xmax=1.0)
    dq.defineHistogram('posErrYWide;h_errY_wide', type='TH1F',
        title='#sigma_{Y} wide range;#sigma_{Y} [mm];Entries', xbins=200, xmin=0, xmax=5.0)
    dq.defineHistogram('posPrecX;h_precX', type='TH1F',
        title='Precision X (1/#sigma^{2}_{X});Prec_{X} [mm^{-2}];Entries', xbins=200, xmin=0, xmax=1e6)
    dq.defineHistogram('posPrecY;h_precY', type='TH1F',
        title='Precision Y (1/#sigma^{2}_{Y});Prec_{Y} [mm^{-2}];Entries', xbins=200, xmin=0, xmax=1e5)

    # Per-event extremes of the position-net outputs.
    ex = helper.addGroup(alg, 'PixelNNExtremes', '/PixelNNMonitor/Extremes/')
    ex.defineHistogram('evtMinErrX;h_evtMinErrX', type='TH1F',
        title='Per-event min #sigma_{X};min #sigma_{X} [mm];Events', xbins=100, xmin=0, xmax=0.05)
    ex.defineHistogram('evtMaxErrX;h_evtMaxErrX', type='TH1F',
        title='Per-event max #sigma_{X};max #sigma_{X} [mm];Events', xbins=100, xmin=0, xmax=0.5)
    ex.defineHistogram('evtMinErrY;h_evtMinErrY', type='TH1F',
        title='Per-event min #sigma_{Y};min #sigma_{Y} [mm];Events', xbins=100, xmin=0, xmax=0.1)
    ex.defineHistogram('evtMaxErrY;h_evtMaxErrY', type='TH1F',
        title='Per-event max #sigma_{Y};max #sigma_{Y} [mm];Events', xbins=100, xmin=0, xmax=2.0)
    ex.defineHistogram('evtMaxAbsDeltaX;h_evtMaxAbsDeltaX', type='TH1F',
        title='Per-event max |#DeltaX|;max |#DeltaX| [mm];Events', xbins=100, xmin=0, xmax=0.5)
    ex.defineHistogram('evtMaxAbsDeltaY;h_evtMaxAbsDeltaY', type='TH1F',
        title='Per-event max |#DeltaY|;max |#DeltaY| [mm];Events', xbins=100, xmin=0, xmax=2.0)
    ex.defineHistogram('evtMaxProb2;h_evtMaxProb2', type='TH1F',
        title='Per-event max P(2);max P(2);Events', xbins=100, xmin=0, xmax=1)

    for n in (1, 2, 3):
        _bookPositionGroup(helper, alg, n)

    summ = helper.addGroup(alg, 'PixelNNPosSummary',
                          '/PixelNNMonitor/PositionNet/Summary/')
    summ.defineHistogram('posN,pullX;h_pullX_vs_N', type='TH2F',
        title='Pull X vs multiplicity;N_{true};pull_{X}',
        xbins=3, xmin=0.5, xmax=3.5, ybins=50, ymin=-5, ymax=5)
    summ.defineHistogram('posN,pullY;h_pullY_vs_N', type='TH2F',
        title='Pull Y vs multiplicity;N_{true};pull_{Y}',
        xbins=3, xmin=0.5, xmax=3.5, ybins=50, ymin=-5, ymax=5)
    summ.defineHistogram('posN,errX;h_errX_vs_N', type='TProfile',
        title='Mean #sigma_{X} vs multiplicity;N_{true};#sigma_{X} [#mum]',
        xbins=3, xmin=0.5, xmax=3.5)
    summ.defineHistogram('posN,errY;h_errY_vs_N', type='TProfile',
        title='Mean #sigma_{Y} vs multiplicity;N_{true};#sigma_{Y} [#mum]',
        xbins=3, xmin=0.5, xmax=3.5)


def ITkOnnxNnClusterizationFactoryCfg(flags, onnxPaths, useXPitches=False,
                                      name="ITkPixelNNMonitorFactory"):
    """NnClusterizationFactory for ITk, running the NN via local ONNX models.

    Mirrors ITkNnClusterizationFactoryCfg (ITk charge-calib conditions, ITk
    Lorentz tool, ITkPixelChargeCalibCondData key) but loads the networks from
    ONNX files instead of the PixelClusterNNJSON COOL folder, which is not
    available for the Run4 conditions tags.
    """
    from PixelConditionsAlgorithms.ITkPixelConditionsConfig import (
        ITkPixelChargeCalibCondAlgCfg)
    acc = ITkPixelChargeCalibCondAlgCfg(flags)

    from InDetConfig.SiClusterizationToolConfig import OnnxNNCondAlgCfg
    acc.merge(OnnxNNCondAlgCfg(flags, **onnxPaths))

    from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import (
        ITkPixelLorentzAngleToolCfg)
    lorentz = acc.popToolsAndMerge(ITkPixelLorentzAngleToolCfg(flags))

    acc.setPrivateTools(CompFactory.InDet.NnClusterizationFactory(
        name,
        PixelLorentzAngleTool=lorentz,
        useToT=False,
        useONNX=True,
        useXPitches=useXPitches,
        NnCollectionReadKey="",
        NnCollectionWithTrackReadKey="",
        NnCollectionJSONReadKey="",
        NnCollectionONNXReadKey="PixelClusterNNONNX",
        PixelChargeCalibCondData="ITkPixelChargeCalibCondData"))
    return acc


def PixelNNMonitorAlgCfg(flags, **kwargs):
    """Configure PixelNNMonitorAlg."""
    acc = ComponentAccumulator()
    helper = AthMonitorCfgHelper(flags, "PixelNNMonitoring")

    alg = helper.addAlgorithm(CompFactory.InDet.PixelNNMonitorAlg, 'PixelNNMonitorAlg')

    useXPitches = kwargs.pop("useXPitches", False)
    nnFactoryKwargs = {}
    for key in ["useONNX", "NumberNetworkPath", "PositionNetwork1Path",
                "PositionNetwork2Path", "PositionNetwork3Path"]:
        if key in kwargs:
            nnFactoryKwargs[key] = kwargs.pop(key)

    from AthenaConfiguration.Enums import LHCPeriod
    isITk = flags.GeoModel.Run >= LHCPeriod.Run4

    if isITk:
        # ITk: pick the ONNX models from local files, or fall back to the
        # production lwtnn networks (PixelClusterNNJSON) when no ONNX path given.
        if nnFactoryKwargs.get("NumberNetworkPath"):
            onnxPaths = {k: v for k, v in nnFactoryKwargs.items() if k != "useONNX"}
            nnFactory = acc.popToolsAndMerge(
                ITkOnnxNnClusterizationFactoryCfg(flags, onnxPaths,
                                                  useXPitches=useXPitches))
        else:
            from InDetConfig.SiClusterizationToolConfig import (
                ITkNnClusterizationFactoryCfg)
            nnFactory = acc.popToolsAndMerge(ITkNnClusterizationFactoryCfg(flags))
        alg.PixelClusterContainer = "ITkPixelClusters"
        alg.TrackCollection = "CombinedITkTracks"
        alg.SiHitCollection = "ITkPixelHits"
        alg.ClusterSplitProbContainer = "ITkAmbiguityProcessorSplitProb"
        from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import (
            ITkPixelLorentzAngleToolCfg)
        alg.PixelLorentzAngleTool = acc.popToolsAndMerge(
            ITkPixelLorentzAngleToolCfg(flags))
    else:
        from InDetConfig.SiClusterizationToolConfig import (
            NnClusterizationFactoryCfg)
        nnFactory = acc.popToolsAndMerge(
            NnClusterizationFactoryCfg(flags, **nnFactoryKwargs))
        alg.SiHitCollection = "PixelHits"
        alg.ClusterSplitProbContainer = "InDetAmbiguityProcessorSplitProb"
        from SiLorentzAngleTool.PixelLorentzAngleConfig import (
            PixelLorentzAngleToolCfg)
        alg.PixelLorentzAngleTool = acc.popToolsAndMerge(
            PixelLorentzAngleToolCfg(flags))
    alg.NnClusterizationFactory = nnFactory

    alg.doTruth = flags.Input.isMC

    PixelNNMonitorHistograms(helper, alg)

    acc.merge(helper.result())
    return acc

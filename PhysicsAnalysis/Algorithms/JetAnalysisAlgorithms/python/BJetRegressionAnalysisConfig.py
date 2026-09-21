# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock, logCPAlgCfgBlock
from JetAnalysisAlgorithms.JetAnalysisConfig import _parseJetCollection
import AthenaCommon.SystemOfUnits as Units


# The regression outputs, per jet radius. These are ABSOLUTE predictions in MeV,
# written by FlavorTagInference at derivation time. An empty mass entry means the
# model predicts the pT only and the jet mass is left alone.
_BJR_MODELS = {
    4:  {'pt': 'bJR4v01_pt',  'mass': ''},
    10: {'pt': 'bJR10v01_pt', 'mass': 'bJR10v01_mass'},
}

# The flat uncertainties covering the regression itself, per radius. These are tool
# PROPERTIES, so the central CVMFS uncertainty configs are used unmodified --
# switching the regression on must not require a private calibration area.
#
# Zero means no such nuisance parameter is built, which is how R=0.4 gets no mass
# uncertainty.
#
# Nothing else about the uncertainty tool is configured. Because the regression is
# applied after the tool runs, every lookup inside it happens at the standard
# calibrated kinematics, which is the same point an un-regressed analysis uses.
_BJR_UNCERTAINTIES = {
    4:  {'pt': 0.03, 'mass': 0.00},
    10: {'pt': 0.01, 'mass': 0.02},
}


def _instantiatedTool (alg, name):
    """Return alg.<name> if a tool was really created for it, else None.

    A private tool that was declared but never instantiated is still reachable as
    an attribute -- it is an EMPTY handle, not a missing attribute, so getattr
    succeeds and only the property assignment fails. That is exactly the case for
    uncertaintiesToolPD on data, where JetAnalysisConfig creates the main tool but
    not the pseudo-data one.

    Testing for the property we are about to set is the honest check: a real
    JetUncertaintiesTool declares it, an empty handle does not.
    """
    tool = getattr (alg, name, None)
    if tool is None or not hasattr (tool, 'BJetRegressionApplied'):
        return None
    return tool


def _configureBJetRegressionUncertainties (tool, radius):
    """Switch the flat regression nuisance parameters on for *tool*.

    Kept as one function over a single table because the settings otherwise have
    to be repeated at four call sites -- the main and pseudo-data tools for each
    radius -- and one of those four silently had the wrong radius' numbers.
    """
    cfg = _BJR_UNCERTAINTIES[radius]
    tool.BJetRegressionApplied = True
    tool.BJetRegressionPtUncertainty = cfg['pt']
    tool.BJetRegressionMassUncertainty = cfg['mass']


class BJetRegressionAnalysisConfig (ConfigBlock) :
    """the ConfigBlock for the b-jet energy regression

    Folds the b-jet energy regression into the jet four-momentum. The regression
    itself is evaluated in the derivation and stored as an absolute prediction;
    this schedules the algorithm that applies it on top of the standard
    calibration, and the flat uncertainties that go with it.

    This is a separate block, rather than a step inside the jet sequence, for one
    reason: it has to run AFTER flavour tagging and after the JVT and b-tagging
    efficiency scale factors. Those are parametrised in jet pT and are derived on
    standard-calibration jets, so they must be evaluated before the jet moves.
    Since those scale factors are themselves scheduled after overlap removal, so
    is this block -- which means overlap removal and MET also see the standard
    calibrated jet, and the regressed four-momentum is what reaches the output.
    See __init__ for why the placement cannot be expressed as a dependency.

    It must also run after the jet uncertainty algorithm. The regression output is
    an absolute number with no systematic dependence, so the relative
    uncertainties have to be evaluated on the standard calibrated jet and the
    regression applied on top. That ordering comes for free here, since the
    uncertainties are built inside the jet block.
    """

    def __init__ (self) :
        super (BJetRegressionAnalysisConfig, self).__init__ ()
        self.setBlockName('BJetRegression')
        # Ordering. Everything parametrised in jet pT and derived on
        # standard-calibration jets has to be evaluated BEFORE the regression
        # moves the jet: flavour tagging, and the b-tagging and JVT efficiency
        # scale factors.
        #
        # Dependencies alone cannot express that. A dependency is matched against
        # a block's blockName (ConfigBlock.__eq__), and the b-tagging event scale
        # factor block never calls setBlockName -- it prints as its factory name,
        # Jets.FlavourTaggingEventSF[N:FTagEventSFBlock] -- so its blockName is
        # empty and NO dependency can ever match it. BJetCalib's
        # addDependency('FTagJetSF') has the same problem and is a silent no-op.
        #
        # The ordering is therefore achieved by SCHEDULING POSITION: the analysis
        # appends this block after overlap removal, and reorderAlgs only ever
        # moves blocks FORWARD, so the scale-factor blocks -- themselves relocated
        # to just after OverlapRemoval -- land ahead of it. The dependencies below
        # are the ones that do match a real blockName. They are belt and braces,
        # and by construction they cannot pull this block earlier.
        self.addDependency('FTag', required=False)
        self.addDependency('JvtWorkingPointEventEfficiencyConfig', required=False)
        self.addDependency('FJvtWorkingPointEventEfficiencyConfig', required=False)
        # Not for ordering: pins BJetCalib ahead of us so the incompatibility
        # check in makeAlgs is deterministic.
        self.addDependency('BJetCalib', required=False)

        self.addOption ('containerName', '', type=str,
            noneAction='error',
            info="the name of the jet container to correct.")
        self.addOption ('jetCollection', '', type=str,
            noneAction='error',
            info="the jet collection the container was built from, e.g. "
            "'AntiKt4EMPFlowJets'. Used to pick the regression model and the "
            "uncertainty values.")
        self.addOption ('jetPreselection', '', type=str,
            info="an optional preselection restricting which jets are corrected, "
            "on top of the kinematic gate below. Leave empty to correct every jet "
            "carrying the regression decoration. Setting this to a b-tagging "
            "working point restricts the correction to tagged jets.")
        self.addOption ('gateMinPt', 20.*Units.GeV, type=float,
            info=r"jets below this $p_\mathrm{T}$ (in MeV) are left uncorrected. "
            "The regression is not derived at arbitrarily low pT. Harmless for "
            "large-R, which already starts far above it.")
        self.addOption ('gateMaxEta', 2.5, type=float,
            info=r"jets outside this $|\eta|$ are left uncorrected.")
        self.addOption ('onlyDecorate', False, type=bool,
            info="whether to write the regression ratios but leave the jet "
            "four-momentum alone. For measuring the correction without "
            "perturbing the selection; the jet is unchanged, so the accepted "
            "sample matches an un-regressed run exactly.")

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName

    def makeAlgs (self, config) :

        # The muon-in-jet correction and the regression both account for the
        # energy carried out of the calorimeter by muons in b-hadron decays, so
        # running both double counts it. The BJetCalib dependency above guarantees
        # that block is configured before this one, so this check is reliable.
        if config.getAlgorithm ('BJetCalibAlg') is not None:
            raise ValueError (
                'BJetCalib and the b-jet energy regression both correct the jet '
                'for muons inside it and must not be combined. Enable one of '
                'them: either drop the BJetCalib block, or do not schedule '
                'BJetRegression.')

        # Record the placement. Where this block sits in the sequence is the
        # whole reason it exists, and it cannot be read off the dependency list
        # (see __init__), so state it in the log where it can be checked.
        logCPAlgCfgBlock.info (
            'BJetRegression: scheduling for %s. Everything already configured -- '
            'flavour tagging, the JVT and b-tagging efficiency scale factors, '
            'overlap removal -- has seen the standard calibrated jet.',
            self.containerName)

        radius, jetInput, trim, hasBTag = _parseJetCollection (self.jetCollection)
        if radius not in _BJR_MODELS:
            raise ValueError (
                'no b-jet regression model for radius {0} (jet collection {1}); '
                'supported radii are {2}'.format (
                    radius, self.jetCollection, sorted (_BJR_MODELS)))
        model = _BJR_MODELS[radius]

        # The kinematic gate: which jets the regression is applied to at all.
        # Deliberately a selection on the jet rather than a window on the size of
        # the correction -- a cut on the correction would put a discontinuity in
        # the middle of a smooth distribution.
        preselection = config.getFullSelection (self.containerName,
                                                self.jetPreselection)
        if self.gateMinPt > 0 or self.gateMaxEta > 0:
            alg = config.createAlgorithm ('CP::AsgSelectionAlg', 'BJRPtEtaCutAlg')
            alg.selectionDecoration = 'selectPtEtaBJR,as_bits'
            config.addPrivateTool ('selectionTool', 'CP::AsgPtEtaSelectionTool')
            alg.selectionTool.minPt = self.gateMinPt
            alg.selectionTool.maxEta = self.gateMaxEta
            alg.particles = config.readName (self.containerName)
            alg.preselection = config.getPreselection (self.containerName, '')
            if preselection:
                preselection = 'selectPtEtaBJR,as_bits&&' + preselection
            else:
                preselection = 'selectPtEtaBJR,as_bits'

        # Fold the regression into the jet four-momentum.
        #
        # jetsNominal points at the same container as jets: the algorithm forms
        # the ratio against the NOMINAL instance, which is what makes the
        # systematic shifts survive. See BJetRegressionAlg.h for why a ratio taken
        # per systematic would cancel them exactly.
        alg = config.createAlgorithm ('CP::BJetRegressionAlg', 'BJetRegressionAlg')
        alg.jets = config.readName (self.containerName)
        alg.jetsNominal = config.readName (self.containerName)
        alg.jetsOut = config.copyName (self.containerName)
        alg.jetPreselection = preselection
        alg.regressedPtName = model['pt']
        alg.regressedMassName = model['mass']
        alg.onlyDecorate = self.onlyDecorate

        # The flat regression nuisance parameters live on JetUncertaintiesTool,
        # which is built inside the jet block. Reaching it from here keeps the
        # whole feature behind ONE switch, instead of asking an analysis to set a
        # flag in two places and keep them consistent. This is the same mechanism
        # ConfigBlock.applyConfigOverrides uses, and for the same reason: all of
        # this is Python configuration, so a property set now still takes effect.
        self._configureUncertainties (config, radius)

        # Re-decorate the jet energy. The jet block writes 'e' from the jet
        # four-momentum at the end of its own sequence, which is now BEFORE this
        # block, so that value would describe the pre-regression jet. Recompute it
        # here, under a distinct algorithm name. BJetCalib does the same thing for
        # the same reason. Large-R has no energy decoration, so nothing to do.
        if radius == 4 and not self.onlyDecorate:
            alg = config.createAlgorithm ('CP::AsgEnergyDecoratorAlg',
                                          'EnergyDecoratorBJR')
            alg.particles = config.readName (self.containerName)

    def _configureUncertainties (self, config, radius) :
        """Switch on the flat regression NPs in the jet uncertainty tools."""
        uncAlg = config.getAlgorithm ('JetUncertaintiesAlg')
        if uncAlg is None:
            # Legitimate: an analysis may run with no jet uncertainties at all.
            # Say so rather than silently producing a regressed jet with no
            # uncertainty covering the regression.
            logCPAlgCfgBlock.warning (
                'BJetRegression: no JetUncertaintiesAlg found for container %s, '
                'so no flat b-jet regression nuisance parameter will be built. '
                'The jet will still be regressed.', self.containerName)
            return

        tool = _instantiatedTool (uncAlg, 'uncertaintiesTool')
        if tool is None:
            raise ValueError (
                'JetUncertaintiesAlg for container {0} has no usable '
                'uncertaintiesTool. If JetUncertaintiesTool no longer declares '
                'BJetRegressionApplied, this block and that tool are out of '
                'step.'.format (self.containerName))
        _configureBJetRegressionUncertainties (tool, radius)

        # The pseudo-data tool is a second JetUncertaintiesTool instance that needs
        # the same settings -- missing it would silently drop a nuisance parameter
        # from the pseudo-data JER variations. It is only created when the JER model
        # is Full or All and we are not on data, so its absence is normal; say which
        # happened rather than leaving it to inference.
        toolPD = _instantiatedTool (uncAlg, 'uncertaintiesToolPD')
        if toolPD is None:
            logCPAlgCfgBlock.info (
                'BJetRegression: no pseudo-data uncertainty tool for container %s, '
                'so the regression nuisance parameter is built on the main tool '
                'only. Expected on data, and whenever the JER model is not Full '
                'or All.', self.containerName)
        else:
            _configureBJetRegressionUncertainties (toolPD, radius)

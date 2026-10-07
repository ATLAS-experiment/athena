# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Decoration suffixes, matching the slot names hardcoded in the algorithm.
LEAD = 'TruthTaus'
SUBLEAD = 'SubleadTruthTaus'

# Variables taken straight off the matched TruthTaus entry, by type.
DEFAULT_VARIABLES = {
    'Doubles': ['pt_vis', 'eta_vis', 'phi_vis', 'm_vis'],
    'Uints': ['classifierParticleOutCome', 'classifierParticleType'],
    'Ulongs': ['numCharged'],
    'Chars': ['IsHadronicTau'],
}


def _copy_properties(variables):
    props = {}
    for prefix, slot in (('lead', LEAD), ('sublead', SUBLEAD)):
        for kind, names in variables.items():
            props[f'{prefix}{kind}ToCopy'] = {
                name: f'{name}From{slot}' for name in names
            }
    return props


def TruthTauDecoratorCfg(flags,
                         target_jets,
                         ghost_tau_assoc_name='GhostTausFinal',
                         truth_taus='TruthTaus',
                         require_isolated_tau=True,
                         min_truth_tau_pt=0.0,
                         variables_to_copy=None,
                         name_suffix=''):
    """Decorate jets with the properties of their ghost-associated truth taus.

    The ghost association defines the match. The taus are ordered by pT and the
    leading and subleading one fill a decoration slot each, suffixed
    ``TruthTaus`` and ``SubleadTruthTaus``. Per slot the algorithm writes
    ``matchedTo``, ``deltaRTo``, ``deltaPtTo``, ``dEtaTo``, ``dPhiTo`` (all
    against the visible tau), ``ptFrom``, ``mFrom`` and ``chargeFrom`` (the
    total, neutrino-inclusive tau), plus the variables in `variables_to_copy`
    taken from the matched ``truth_taus`` entry as ``<name>From<slot>``. The
    number of taus considered is written once per jet as ``nGhostTaus``.

    Unmatched slots get NaN throughout, `matchedTo` 0 and the copied variables
    their per-type null value; gate on ``nGhostTaus`` or ``matchedTo``.

    Fails outright if the ghost tau association or ``truth_taus`` is missing.

    Small-R and large-R jets use the same defaults. The isolated tau
    requirement keeps taus from W, Z, Higgs and top decays and drops those from
    b and c hadron decays, so it stays on in both cases. Both slots are filled
    in both cases too, since a small-R jet can also catch two taus from a
    boosted boson decay.

    Parameters
    ----------
    target_jets : str
        Jet collection to decorate. Needs the tau ghost association, which
        small-R and VR track jets have.
    ghost_tau_assoc_name : str
        Ghost tau association on the jets.
    truth_taus : str
        Container providing the visible tau decays.
    require_isolated_tau : bool
        Keep only taus classified as IsoTau, dropping those from b- and
        c-hadron decays. Needs the MCTruthClassifier decorations on
        `truth_taus`.
    min_truth_tau_pt : float
        Minimum visible tau pT in MeV. The ghost association already cuts at
        5 GeV on the total tau pT, so this only tightens the selection.
    variables_to_copy : dict, optional
        Variable names to copy off the matched tau, keyed by type. Defaults to
        `DEFAULT_VARIABLES`.
    """
    variables = (DEFAULT_VARIABLES if variables_to_copy is None
                 else variables_to_copy)

    ca = ComponentAccumulator()
    ca.addEventAlgo(
        CompFactory.ftag.TruthTauDecoratorAlg(
            f'{target_jets}TruthTauDecoratorAlg{name_suffix}',
            jets=target_jets,
            ghostTauAssocName=ghost_tau_assoc_name,
            truthTaus=truth_taus,
            requireIsolatedTau=require_isolated_tau,
            minTruthTauPt=min_truth_tau_pt,
            **_copy_properties(variables),
        )
    )
    return ca

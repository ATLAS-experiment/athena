# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TruthTauDecoratorCfg(flags,
                         target_jets,
                         ghost_tau_assoc_name='GhostTausFinal',
                         truth_taus='TruthTaus',
                         name_suffix=''):
    """Decorate jets with their leading/subleading truth-tau kinematics.

    Orders the ghost-associated taus by pT and writes, per jet:

    - total (neutrino-inclusive): ``truthtau_{lead,sublead}_{pt,deta,dphi,m}``
    - visible:                    the same with a ``_vis`` suffix
    - prongness:                  ``truthtau_{lead,sublead}_numCharged``
    - charge:                     ``truthtau_{lead,sublead}_charge``
    - decay mode:                 ``truthtau_{lead,sublead}_isHadronic``
    - multiplicity:               ``nGhostTaus``

    The visible quantities come from the matched ``truth_taus`` entry, found via
    its ``originalTruthParticle`` back-link. Angles are the tau relative to the
    jet axis (``deta = tau.eta - jet.eta``); pt/m are absolute. Note the ghost
    association holds all final-state taus, so ``isHadronic`` separates hadronic
    from leptonic decays. With no tau, pt/m are 0, deta/dphi NaN, numCharged and
    isHadronic -1 and charge 0; gate on ``nGhostTaus``.

    Fails outright if the ghost-tau association is missing from the jets.

    Parameters
    ----------
    target_jets : str
        Jet collection to decorate.
    ghost_tau_assoc_name : str
        Ghost tau association on the jets.
    truth_taus : str
        Container providing the visible-tau 4-momenta and numCharged.
    """
    ca = ComponentAccumulator()
    ca.addEventAlgo(
        CompFactory.ftag.TruthTauDecoratorAlg(
            f'{target_jets}TruthTauDecoratorAlg{name_suffix}',
            jets=target_jets,
            ghostTauAssocName=ghost_tau_assoc_name,
            truthTaus=truth_taus,
        )
    )
    return ca

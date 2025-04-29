# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def JetMatchingCfg(flags, target, source=None,
                   floats_to_copy=[], ints_to_copy=[],
                   source_minimum_pt=0,
                   pt_priority_with_delta_r=-1,
                   particle_link_name=None,
                   name_suffix=''):

    if source is None:
        source = target

    dr_str = f'deltaRTo{source}'
    deta_str = f'deltaEtaTo{source}'
    dphi_str = f'deltaPhiTo{source}'
    dpt_str = f'deltaPtTo{source}'
    to_suffix = f'From{source}'
    match_str = f'matchedTo{source}'
    def to(f):
        return f + to_suffix

    ca = ComponentAccumulator()
    ca.addEventAlgo(
        CompFactory.ftag.JetMatcherAlg(
            f'{source}To{target}CopyAlg{name_suffix}',
            targetJet=target,
            sourceJets=[source],
            floatsToCopy={f: to(f) for f in floats_to_copy},
            intsToCopy={i: to(i) for i in ints_to_copy},
            dR=dr_str,
            dEta=deta_str,
            dPhi=dphi_str,
            dPt=dpt_str,
            match=match_str,
            ptPriorityWithDeltaR=pt_priority_with_delta_r,
            sourceMinimumPt=source_minimum_pt,
            particleLink=(particle_link_name or ""),
        )
    )
    return ca

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

import warnings


def JetMatchingCfg(flags, target, source=None,
                   sources=[],
                   source_name=None,
                   floats_to_copy=[],
                   ints_to_copy=[],
                   chars_to_copy=[],
                   iparticles_to_copy=[],
                   add_suffix_with_source=True,
                   source_minimum_pt=0,
                   pt_priority_with_delta_r=-1,
                   particle_link_name=None,
                   name_suffix=''):

    # handle some backward compatability stuff
    if source is not None:
        warnings.warn(
            "'source' option is deprecated, use 'sources'",
            FutureWarning,
            stacklevel=2
        )
        if sources:
            raise ValueError(
                "Can't use 'sources' and 'source' at the same time"
            )
        sources = [source]

    if not sources:
        sources = [target]

    if source_name is None:
        source_name = 'Or'.join(sources)

    dr_str = f'deltaRTo{source_name}'
    deta_str = f'deltaEtaTo{source_name}'
    dphi_str = f'deltaPhiTo{source_name}'
    dpt_str = f'deltaPtTo{source_name}'
    match_str = f'matchedTo{source_name}'
    n_match_str = f'numberOfMatchesTo{source_name}'

    def to(f):
        return f + f'From{source_name}' if add_suffix_with_source else f

    ca = ComponentAccumulator()
    ca.addEventAlgo(
        CompFactory.ftag.JetMatcherAlg(
            f'{source_name}To{target}CopyAlg{name_suffix}',
            targetJet=target,
            sourceJets=sources,
            floatsToCopy={f: to(f) for f in floats_to_copy},
            intsToCopy={i: to(i) for i in ints_to_copy},
            charsToCopy={c: to(c) for c in chars_to_copy},
            iparticlesToCopy={p: to(p) for p in iparticles_to_copy},
            dR=dr_str,
            dEta=deta_str,
            dPhi=dphi_str,
            dPt=dpt_str,
            isMatched=match_str,
            nMatch=n_match_str,
            ptPriorityWithDeltaR=pt_priority_with_delta_r,
            sourceMinimumPt=source_minimum_pt,
            particleLink=(particle_link_name or ""),
        )
    )
    return ca

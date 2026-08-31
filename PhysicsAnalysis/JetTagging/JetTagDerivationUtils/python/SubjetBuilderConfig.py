# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def SubjetBuilderCfg(flags,
                     target_jets,
                     output_container=None,
                     ghost_association='GhostTruth',
                     radius=0.2,
                     pt_min=5000,
                     link_name=None,
                     count_name=None,
                     name_suffix=''):
    """Recluster ghost-associated constituents of a jet into small-R subjets.

    Subjets above ``pt_min`` are written to their own container and linked back
    from the parent jet.

    Fails outright if the ghost association is missing from the jets.

    Parameters
    ----------
    target_jets : str
        Jet collection whose ghost constituents are reclustered.
    output_container : str, optional
        Output subjet container. Defaults to
        ``<target_jets><ghost_association>Subjets``.
    ghost_association : str
        Ghost-associated constituent links read off ``target_jets``.
    radius : float
        Anti-kt clustering radius.
    pt_min : float
        Minimum subjet pT in MeV.
    link_name, count_name : str, optional
        Decorations written on ``target_jets``. Bare names, not prefixed.
    """
    if output_container is None:
        output_container = f'{target_jets}{ghost_association}Subjets'
    if link_name is None:
        link_name = f'{output_container}Links'
    if count_name is None:
        count_name = f'{output_container}Count'

    ca = ComponentAccumulator()
    ca.addEventAlgo(
        CompFactory.ftag.SubjetBuilderAlg(
            f'{output_container}BuilderAlg{name_suffix}',
            targetJets=target_jets,
            outputContainer=output_container,
            linkName=link_name,
            countName=count_name,
            ghostAssociation=ghost_association,
            radius=radius,
            ptMin=pt_min,
        )
    )
    return ca

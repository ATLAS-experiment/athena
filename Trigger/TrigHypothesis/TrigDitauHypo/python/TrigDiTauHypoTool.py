# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
import re
from AthenaCommon.SystemOfUnits import GeV

from AthenaCommon.Logging import logging
log = logging.getLogger('TrigHLTDiTauHypoTool')


def TrigDiTauHypoToolFromDict(flags, chainDict):
    name = chainDict['chainName']
    chainPart = chainDict['chainParts'][0]
    cut_pt = float(chainPart['threshold']) * GeV
    ditau_tag_str = chainPart['ditauTag']
    pattern = r"ditauOmni([0-9]+)Trk([0-9]{2})"
    match = re.match(pattern, ditau_tag_str)
    if match is None:
        log.error(f"Invalid ditau tag: {ditau_tag_str}")
        raise ValueError(f"Invalid ditau tag: {ditau_tag_str}")
    id_cut = float(f'0.{match.group(1)}')
    n_trk_lead = int(match.group(2)[0])
    n_trk_subl = int(match.group(2)[1])
    if n_trk_lead < 3 or n_trk_subl < 3:
        log.error(f"Invalid ditau tag: {ditau_tag_str}")
        log.error("it doesn't make sense to require less than 3 tracks in a ditau subjet")
        raise ValueError(f"Invalid ditau tag: {ditau_tag_str}")

    from AthenaConfiguration.ComponentFactory import CompFactory
    currentHypo = CompFactory.TrigDiTauHypoTool(
        name,
        ditau_pt_threshold=cut_pt,
        ditau_id_score=id_cut,
        ditau_lead_max_trk=n_trk_lead,
        ditau_subl_max_trk=n_trk_subl,
    )
    
    return currentHypo

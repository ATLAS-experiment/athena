# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg


def MetadataAlgCfg(flags, h5_output=None, json_output=None,
                   h5_output_hists=None, enable_systematics=True):
    acc = ComponentAccumulator()
    acc.merge(MetaDataSvcCfg(flags))
    opts = {}
    if o := h5_output:
        opts |= dict(h5Output=o)
    if o := json_output:
        opts |= dict(jsonOutput=str(o))
    if o := h5_output_hists:
        opts |= dict(h5OutputHists=o)
    opts |= dict(enableSystematics=enable_systematics)
    if not (h5_output or json_output or h5_output_hists):
        raise ValueError('No outputs given for MetadataAlg')
    acc.addEventAlgo(
        CompFactory.ftag.MetadataAlg('MetadataAlg', **opts)
    )
    return acc

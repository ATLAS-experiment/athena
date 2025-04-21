# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg


def MetadataAlgCfg(flags, h5_output=None, json_output=None):
    acc = ComponentAccumulator()
    acc.merge(MetaDataSvcCfg(flags))
    opts = {}
    if o := h5_output:
        opts |= dict(h5Output=o)
    if o := json_output:
        opts |= dict(jsonOutput=str(o))
    if not opts:
        raise ValueError('No outputs given for MetadataAlg')
    acc.addEventAlgo(
        CompFactory.ftag.MetadataAlg(
            'MetadataAlg',
            **opts
        )
    )
    return acc

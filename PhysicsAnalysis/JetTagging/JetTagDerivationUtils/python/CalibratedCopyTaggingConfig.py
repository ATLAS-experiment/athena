# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from JetCalibTools.CalibratedJetCopyConfig import (
    CalibratedJetCopyCfg,
    sanitizeName,
)

_copy_link_name = 'calibratedJetLink'
_copy_suffix = 'FTAGInferenceOnlyJets'


def copyCollectionName(jetCollection):
    """Name of the frozen-calibration copy tagged instead of jetCollection."""
    return jetCollection.removesuffix('Jets') + _copy_suffix


def copySourceCollection(flags, jetCollection):
    """Source collection if jetCollection is a calibrated copy, else None."""
    for source in flags.BTagging.CalibratedCopies:
        if copyCollectionName(source) == jetCollection:
            return source
    return None


def CalibratedCopyCfg(flags, jetCollection, supportDecorations=None):
    """
    Record the frozen-calibration shallow copy of jetCollection.

    supportDecorations map the decorations written on jetCollection that
    the taggers read through the copy's parent store to the container
    type they are declared on. The scheduler cannot see them under the
    copy's name, so the copy declares them: it waits for the algorithms
    writing them and provides them to the taggers.
    """
    taggers = flags.BTagging.CalibratedCopies[jetCollection]
    calibrations = [cfg['calibration'] for cfg in taggers.values()]
    assert all(c == calibrations[0] for c in calibrations), \
        f'inconsistent frozen calibrations for {jetCollection}'
    calibration = calibrations[0]
    copy_collection = copyCollectionName(jetCollection)
    acc = CalibratedJetCopyCfg(
        flags,
        jetCollection=jetCollection,
        outputCollection=copy_collection,
        configFile=calibration['configFile'],
        calibSequence=calibration['calibSequence'],
        calibArea=calibration['calibArea'],
        calibrationScale='FrozenT0',
        # the derivation T0 context runs with IsData=True also on MC
        isData=True,
    )
    copy_alg = acc.getEventAlgo(
        f'CalibratedJetCopyAlg_{sanitizeName(copy_collection)}')
    copy_alg.ExtraInputs = [
        (container, f'StoreGateSvc+{jetCollection}.{name}')
        for name, container in sorted((supportDecorations or {}).items())
    ]
    copy_alg.ExtraOutputs = [
        ('xAOD::JetContainer', f'StoreGateSvc+{copy_collection}.{name}')
        for name in sorted(supportDecorations or {})
    ]
    return acc


def FtagScoreCopyCfg(flags, jetCollection):
    """Copy the opted-in tagger outputs from the copy back to jetCollection."""
    taggers = flags.BTagging.CalibratedCopies[jetCollection]
    copied = {key: [] for key in ('floats', 'uints', 'charVectors', 'trackLinks')}
    for cfg in taggers.values():
        for key, names in copied.items():
            names += cfg['copied_variables'].get(key, [])
    copy_collection = copyCollectionName(jetCollection)
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.ftag.JetLinkMatcherAlg(
        f'FtagScoreCopyAlg_{jetCollection}',
        targetJet=jetCollection,
        sourceJets=[copy_collection],
        linkName=_copy_link_name,
        floatsToCopy={n: n for n in copied['floats']},
        uintsToCopy={n: n for n in copied['uints']},
        charVectorsToCopy={n: n for n in copied['charVectors']},
        trackLinksToCopy={n: n for n in copied['trackLinks']},
    ))
    return acc

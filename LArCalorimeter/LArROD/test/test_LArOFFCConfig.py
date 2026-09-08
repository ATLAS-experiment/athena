#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Configuration test for LArOFFCRawChannelBuilderCfg.firstSample.

firstSample indexes the digit, not the reference shape, so the builder must be
given nPreceedingSamples itself. LArNNChannelBuilder and
LArRawChannelBuilderAlgConfig already do; LArOFFCChannelBuilder used
nPreceedingSamples - 1.
"""
import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import (defaultConditionsTags,
                                              defaultGeometryTags,
                                              defaultTestFiles)
from LArROD.LArOFFCChannelBuilder import LArOFFCRawChannelBuilderCfg


def firstSampleFor(nPreceeding, firstSampleFlag=0):
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RDO_RUN4
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
    flags.LAr.ROD.nSamples = 32
    flags.LAr.ROD.nPreceedingSamples = nPreceeding
    flags.LAr.ROD.FirstSample = firstSampleFlag
    flags.lock()
    acc = LArOFFCRawChannelBuilderCfg(flags)
    alg = acc.getEventAlgo("LArOFFCRawChannelBuilder")
    got = int(alg.firstSample)
    acc.wasMerged()
    return got


if __name__ == "__main__":
    failures = 0
    # nPreceedingSamples != 0 wins, and is passed through unchanged
    for nps, expected in ((24, 24), (15, 15), (1, 1)):
        got = firstSampleFor(nps)
        ok = got == expected
        failures += not ok
        print(f"nPreceedingSamples={nps:3d} -> firstSample={got:3d} "
              f"expected {expected:3d}  {'OK' if ok else 'FAIL'}")
    # at zero it falls back to the FirstSample flag
    got = firstSampleFor(0, firstSampleFlag=3)
    ok = got == 3
    failures += not ok
    print(f"nPreceedingSamples=  0 -> firstSample={got:3d} expected   3  "
          f"{'OK' if ok else 'FAIL'}")
    sys.exit(failures)

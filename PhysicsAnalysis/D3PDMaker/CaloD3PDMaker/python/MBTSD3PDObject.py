# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
# @file CaloD3PDMaker/python/MBTSD3PDObject.py
# @author scott snyder <snyder@bnl.gov>
# @date Nov, 2009
# @brief D3PD object for MBTS.
#

from D3PDMakerConfig.D3PDMakerFlags import D3PDMakerFlags
from D3PDMakerCoreComps.D3PDObject import make_SGDataVector_D3PDObject
from AthenaConfiguration.ComponentFactory import CompFactory

D3PD = CompFactory.D3PD


MBTSD3PDObject = \
           make_SGDataVector_D3PDObject ('TileContainer<TileCell>',
                                         D3PDMakerFlags.MBTSSGKey,
                                         'mb_', 'MBTSD3PDObject')

MBTSD3PDObject.defineBlock (0, 'MBTS', D3PD.MBTSFillerTool)


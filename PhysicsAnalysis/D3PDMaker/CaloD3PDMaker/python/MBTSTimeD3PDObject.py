# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
# @file CaloD3PDMaker/python/MBTSTimeD3PDObject.py
# @author scott snyder <snyder@bnl.gov>
# @date Mar, 2010
# @brief MBTS time info object.
#

from D3PDMakerCoreComps.D3PDObject import make_Void_D3PDObject
from AthenaConfiguration.ComponentFactory import CompFactory

D3PD = CompFactory.D3PD


MBTSTimeD3PDObject = \
           make_Void_D3PDObject ('mbtime_', 'MBTSTimeD3PDObject')
MBTSTimeD3PDObject.defineBlock (0, 'MBTSTime',
                                D3PD.MBTSTimeFillerTool)

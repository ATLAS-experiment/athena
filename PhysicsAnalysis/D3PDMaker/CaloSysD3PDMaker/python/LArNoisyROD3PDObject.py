# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
# @file CaloSysD3PDMaker/python/LArNoisROD3PDObject.py
# @author Laurent Duflot <duflot at lal.in2p3.fr>
# @date Apr, 2010
# @brief D3PD object for LAr noisy FEB
#


from D3PDMakerCoreComps.D3PDObject          import make_SG_D3PDObject
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD


LArNoisyROD3PDObject = \
           make_SG_D3PDObject ('LArNoisyROSummary',
                               'LArNoisyROSummary',
                               'larnro_',
                               'LArNoisyROD3PDObject',
                               default_allowMissing = True)

LArNoisyROD3PDObject.defineBlock (0,
                                  'Basic',
                                  D3PD.LArNoisyROFillerTool,
                                  SaveNB    = True,
                                  SaveFEBID = False,
                                  SavePAID  = False)

LArNoisyROD3PDObject.defineBlock (1,
                                  'FEBIDs',
                                  D3PD.LArNoisyROFillerTool,
                                  SaveNB    = False,
                                  SaveFEBID = True,
                                  SavePAID  = False)

LArNoisyROD3PDObject.defineBlock (2,
                                  'PAIDs',
                                  D3PD.LArNoisyROFillerTool,
                                  SaveNB    = False,
                                  SaveFEBID = False,
                                  SavePAID  = True)

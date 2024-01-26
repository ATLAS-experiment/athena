# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.D3PDObject          import make_SGDataVector_D3PDObject
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD

""" level of details:
0: digits
1: digit+SCA index
2: OF Iteration results

Note: 1 and 2 are not available in ESD.   They are available when running recon on raw data. 
"""

LArDigitD3PDObject = make_SGDataVector_D3PDObject( "LArDigitContainer",
                                                   "LArDigitContainer_Thinned",
                                                   "lardigit_", "LArDigitD3PDObject" )

LArDigitD3PDObject.defineBlock( 0, 'Digits',
                                D3PD.LArDigitFillerTool,
                                SaveDigit= True,
                                SaveId = True,
                                SaveSCAAddress= False,
                                DumpIterResults= False )

LArDigitD3PDObject.defineBlock( 1, 'SCA',
                                D3PD.LArDigitFillerTool,
                                SaveDigit= False,
                                SaveId = False,
                                SaveSCAAddress= True,
                                DumpIterResults= False )

LArDigitD3PDObject.defineBlock( 2, 'ITER',
                                D3PD.LArDigitFillerTool,
                                SaveDigit= False,
                                SaveId = False,
                                SaveSCAAddress= False,
                                DumpIterResults= True )

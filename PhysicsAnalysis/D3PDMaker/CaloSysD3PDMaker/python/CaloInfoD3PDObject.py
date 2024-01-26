# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


from D3PDMakerCoreComps.D3PDObject import make_SG_D3PDObject
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD


CaloInfoD3PDObject = make_SG_D3PDObject( "CaloCellContainer", "AllCalo", "ccc_",
                                         "CaloInfoD3PDObject" )

CaloInfoD3PDObject.defineBlock( 0, 'ALL',
                                D3PD.CaloInfoFillerTool,
                                PosNeg = 0 )

CaloInfoEtD3PDObject = make_SG_D3PDObject( "CaloCellContainer", "AllCalo", "cccEt_",
                                           "CaloInfoEtD3PDObject" )

CaloInfoEtD3PDObject.defineBlock( 0, 'AllEt',
                                  D3PD.CaloInfoFillerTool,
                                  prefix = 'Et_',
                                  PosNeg = 0,
                                  DoEt = 1 )

# if the energies are summed for each side 
CaloInfoEtD3PDObject.defineBlock( 0, 'Pos', 
                                  D3PD.CaloInfoFillerTool,
                                  prefix = 'p_',
                                  PosNeg = 1,
                                  DoEt = 1 )

CaloInfoEtD3PDObject.defineBlock( 0, 'Neg',
                                  D3PD.CaloInfoFillerTool,
                                  prefix = 'n_',
                                  PosNeg = -1,
                                  DoEt = 1 )

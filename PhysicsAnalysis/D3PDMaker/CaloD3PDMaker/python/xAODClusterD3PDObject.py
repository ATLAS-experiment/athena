# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from D3PDMakerCoreComps.D3PDObject import make_SGDataVector_D3PDObject
from D3PDMakerConfig.D3PDMakerFlags import D3PDMakerFlags
from D3PDMakerCoreComps.ContainedVectorMultiAssociation import ContainedVectorMultiAssociation
from AthenaConfiguration.ComponentFactory import CompFactory

D3PD = CompFactory.D3PD


xAODClusterD3PDObject = \
           make_SGDataVector_D3PDObject ('xAOD::CaloClusterContainer',
                                         D3PDMakerFlags.ClusterSGKey,
                                         'cl_', 'ClusterD3PDObject')


xAODClusterD3PDObject.defineBlock (0, 'Kinematics',
                                   # EventCommonD3PDMaker
                                   D3PD.FourMomFillerTool,
                                   WriteE = False,
                                   WriteM = False)

xAODClusterD3PDObject.defineBlock (0, 'SamplingBasics',
                                   D3PD.ClusterSamplingFillerTool)

xAODClusterD3PDObject.defineBlock (
    1, 'Moments',
    D3PD.AuxDataFillerTool,
    Vars = ['firstEdens = FIRST_ENG_DENS<float:0',
            'cellmaxfrac = ENG_FRAC_MAX<float:0',
            'longitudinal = LONGITUDINAL<float:0',
            'secondlambda = SECOND_LAMBDA<float:0',
            'lateral = LATERAL<float:0',
            'secondR = SECOND_R<float:0',
            'centerlambda = CENTER_LAMBDA<float:0',
            'eng_bad_cells = ENG_BAD_CELLS<float:0',
            'n_bad_cells = N_BAD_CELLS<float:0',
            'isolation = ISOLATION<float:0',
            ])
xAODClusterD3PDObject.defineBlock (
    1, 'CenterMagMoment',
    D3PD.AuxDataFillerTool,
    Vars = ['centermag = CENTER_MAG<float:0',
            ])
xAODClusterD3PDObject.defineBlock (
    1, 'Time',
    D3PD.AuxDataFillerTool,
    Vars = ['time'])

xAODClusterD3PDObject.defineBlock (2, 'SamplingEnergies',
                                   D3PD.ClusterSamplingFillerTool,
                                   EmHadEnergies = False, # don't duplicate this one!
                                   SamplingEnergies = True,
                                   SamplingEtaPhi = False,
                                   WriteRecoStatus = True)

xAODClusterD3PDObject.defineBlock (3, 'SamplingEtaPhi',
                                   D3PD.ClusterSamplingFillerTool,
                                   EmHadEnergies = False, # don't duplicate this one!
                                   SamplingEnergies = False, # don't duplicate this one!
                                   SamplingEtaPhi = True,
                                   WriteRecoStatus = False) # don't duplicate this one!


CaloCellInCluster = ContainedVectorMultiAssociation (
    xAODClusterD3PDObject,
    D3PD.CaloClusterCellAssociationTool,
    "cell_",
    4)

CaloCellInCluster.defineBlock (4, 'CellKinematics',
                               D3PD.FourMomFillerTool,
                               WriteE  = True,  WriteM = False)


CaloCellInCluster.defineBlock (5, 'Detail1',
                               D3PD.CaloCellDetailsFillerTool,
                               SaveCellQuality=True,
                               SaveTimeInfo=True,
                               SaveDetInfo=True,
                               SaveCellGain=True,
                               SaveBadCellStatus=False,
                                SaveId =False,
                               SavePositionInfo=False,
                                )


CaloCellInCluster.defineBlock (6, 'Detail2',
                               D3PD.CaloCellDetailsFillerTool,
                               SaveCellQuality=False,
                               SaveTimeInfo=False,
                               SaveDetInfo=False,
                               SaveCellGain=False,
                               SaveBadCellStatus=True,
                               SaveId =True,
                               SavePositionInfo=True,
                               )

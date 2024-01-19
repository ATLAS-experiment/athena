# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
# @file egammaD3PDMaker/python/egammaCluster.py
# @author scott snyder <snyder@bnl.gov>
# @date Mar, 2013
# @brief Add cluster information to a egamma D3PD object.
#


from D3PDMakerCoreComps.SimpleAssociation    import SimpleAssociation
from D3PDMakerCoreComps.ContainedVectorMultiAssociation import ContainedVectorMultiAssociation
from AthenaConfiguration.ComponentFactory   import CompFactory

D3PD = CompFactory.D3PD


def egammaCluster (egamma, allSamplings = False, fwdEVars = False):
    ClusterAssoc = SimpleAssociation \
      (egamma,
       D3PD.egammaClusterAssociationTool)
    ClusterAssoc.defineBlock \
      (2, 'ClusterKin', D3PD.FourMomFillerTool,
       prefix = 'cl_',
       WriteE = True,
       WriteM = False)
    ClusterAssoc.defineBlock (
        2, 'ClusterTime',
        D3PD.AuxDataFillerTool,
        Vars = ['time'],
        prefix = 'cl_')

    ClusterAssoc.defineBlock \
      (2, 'PositionInCalo',
       # CaloD3PDMaker
       D3PD.ClusterPositionInCaloFillerTool,
       prefix = 'cl_')
    ClusterAssoc.defineBlock \
      (2, 'Position0InCalo',
       # CaloD3PDMaker
       D3PD.ClusterPositionInCaloFillerTool,
       prefix = 'cl_',
       FillSeedCoordinates=True)
    ClusterAssoc.defineBlock \
      (2, 'Samplings',
       # CaloD3PDMaker
       D3PD.ClusterEMSamplingFillerTool)

    if allSamplings:
        ClusterAssoc.defineBlock \
          (2, 'AllSamplings',
           # CaloD3PDMaker
           D3PD.ClusterSamplingFillerTool,
           EmHadEnergies = False,
           SamplingEnergies = True)
    if fwdEVars:
        ClusterAssoc.defineBlock (
            1, 'FwdEVars',
            D3PD.AuxDataFillerTool,
            Vars = ['firstEdens = FIRST_ENG_DENS < float: 0',
                    'cellmaxfrac = ENG_FRAC_MAX < float: 0',
                    'longitudinal = LONGITUDINAL < float: 0',
                    'secondlambda = SECOND_LAMBDA < float: 0',
                    'lateral = LATERAL < float: 0',
                    'secondR = SECOND_R < float: 0',
                    'centerlambda = CENTER_LAMBDA < float: 0',
                    ])

    # Allow writing cluster cells.
    # Off by default.
    Cells = ContainedVectorMultiAssociation (
        ClusterAssoc,
        # CaloD3PDMaker
        D3PD.CaloClusterCellAssociationTool,
        "cell_",
        99,
        blockname = 'ClusterCells')
    Cells.defineBlock (4, 'CellKinematics',
                       D3PD.FourMomFillerTool,
                       WriteE  = True,  WriteM = False)
    Cells.defineBlock (4, 'CellRawPosition',
                       # CaloD3PDMaker
                       D3PD.CaloCellRawFillerTool)
    Cells.defineBlock (5, 'CellDetail1',
                       # CaloD3PDMaker
                       D3PD.CaloCellDetailsFillerTool,
                       SaveCellQuality=True,
                       SaveTimeInfo=True,
                       SaveDetInfo=True,
                       SaveCellGain=True,
                       SaveBadCellStatus=False,
                       SaveId =False,
                       SavePositionInfo=False,
                       )
    cellDetail2 = \
        Cells.defineBlock (6, 'CellDetail2',
                           # CaloD3PDMaker
                           D3PD.CaloCellDetailsFillerTool,
                           SaveCellQuality=False,
                           SaveTimeInfo=False,
                           SaveDetInfo=False,
                           SaveCellGain=False,
                           SaveBadCellStatus=True,
                           SaveId =True,
                           SavePositionInfo=True,
                           )
    def _cellDetail2Hook (c, flags, acc, *args, **kw):
        from CaloBadChannelTool.CaloBadChanToolConfig import CaloBadChanToolCfg
        c.BadChannelTool = acc.popToolsAndMerge (CaloBadChanToolCfg (flags))
        return
    cellDetail2.defineHook (_cellDetail2Hook)

    return ClusterAssoc

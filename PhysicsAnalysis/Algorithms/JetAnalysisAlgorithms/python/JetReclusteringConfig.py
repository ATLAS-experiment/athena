# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock
from AthenaCommon.SystemOfUnits import GeV

class JetReclusteringBlock(ConfigBlock):
    """ConfigBlock for the jet reclustering algorithm:
    run FastJet with small-R jets as input. The output
    is a new container containing the reclustered jets.
    The default output variables are the RC jet kinematics
    as well as a vector of indices pointing to the
    small-R jets that formed the RC jets.
    ---- to be updated with substructure variables!"""

    def __init__(self):
        super(JetReclusteringBlock, self).__init__()
        self.addOption ('containerName', None, type=str,
                        info='name of the output reclustered jets container.')
        self.addOption ('jets', None, type=str,
                        info='the input jet collection to recluster, with a possible selection, in the format `container` or `container.selection`.')
        self.addOption ('clusteringAlgorithm', 'AntiKt', type=str,
                        info='algorithm to use to recluster the jets: `AntiKt`, `Kt`, `CamKt`.')
        self.addOption ('reclusteredJetsRadius', 1.0, type=float,
                        info='radius parameter of the reclustering algorithm. The default is 1.0.')
        self.addOption ('minPt', 200*GeV, type=float,
                        info='minimum pT requirement (in MeV) on the reclustered jets, creating the selection `passed_sel`. The default is 200 GeV.')
        self.addOption ('maxEta', 0., type=float,
                        info='maximum eta requirement on the reclustered jets, creating the selection `passed_sel`. The default is 0.')
        self.addOption ('maxRapidity', 2.5, type=float,
                        info='maximum rapidity requirement on the reclustered jets, creating the selection `passed_sel`. The default is 2.5')
    def instanceName (self) :
        """Return the instance name for this block"""
        return self.containerName
                        
    def makeAlgs(self, config):

        alg = config.createAlgorithm('CP::JetReclusteringAlg', 'JetReclusteringAlg')

        alg.jets, alg.jetSelection = config.readNameAndSelection(self.jets)
        alg.reclusteredJets = config.writeName(self.containerName)
        alg.smallRjetIndices = 'smallRjetIndices_%SYS%'
        alg.rcJetEnergy = 'e_%SYS%'
        alg.clusteringAlgorithm = self.clusteringAlgorithm
        alg.reclusteredJetsRadius = self.reclusteredJetsRadius

        # prepare selection algorithm
        if self.minPt > 0 or self.maxEta > 0 or self.maxRapidity > 0 :
            selAlg = config.createAlgorithm('CP::AsgSelectionAlg', 'RCJetsMinPtAlg')
            selAlg.selectionDecoration = 'passed_sel,as_bits'
            config.addPrivateTool('selectionTool', 'CP::AsgPtEtaSelectionTool')
            selAlg.selectionTool.minPt = self.minPt
            selAlg.selectionTool.maxEta = self.maxEta
            selAlg.selectionTool.maxRapidity = self.maxRapidity
            selAlg.particles = config.readName(self.containerName)
            selAlg.preselection = config.getPreselection(self.containerName, '')
            config.addSelection(self.containerName, 'passed_sel', selAlg.selectionDecoration)

        config.addOutputVar(self.containerName, 'pt', 'pt')
        config.addOutputVar(self.containerName, 'eta', 'eta')
        config.addOutputVar(self.containerName, 'phi', 'phi')
        config.addOutputVar(self.containerName, 'm', 'm')
        config.addOutputVar(self.containerName, alg.rcJetEnergy, 'e')
        config.addOutputVar(self.containerName, alg.smallRjetIndices, 'small_r_jet_indices')

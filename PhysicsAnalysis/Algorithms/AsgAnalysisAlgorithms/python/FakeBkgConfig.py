# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AnalysisAlgorithmsConfig.ConfigBlock import ConfigBlock

class FakeBkgBlock(ConfigBlock):
    """ConfigBlock for the FakeBkgTools Calculator algorithm

    The source code and documentation for FakeBkgTools are available
    from https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/AnalysisCommon/FakeBkgTools
    """

    def __init__(self):
        super(FakeBkgBlock, self).__init__()
        self.addOption('setupName', '', type=str,
                       info='custom name for this instance of the algorithm and tool')
        self.addOption('electrons', None, type=str,
                       info='the input electron container, with a possible selection, in the format `container` or `container.selection`.')
        self.addOption('electronsTarget', None, type=str,
                       info='the input electron tight selection in the format `selection`.')
        self.addOption('muons', None, type=str,
                       info='the input muon container, with a possible selection, in the format `container` or `container.selection`.')
        self.addOption('muonsTarget', None, type=str,
                       info='the input muon tight selection in the format `selection`.')
        self.addOption('eventPreselection', '', type=str,
                       info='the event selection flag to estimate the fake background.')
        self.addOption('fakeTool', '', type=str,
                       info='the tool for fake lepton estimate among the different [FakeBkgTools](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/AnalysisCommon/FakeBkgTools). The currently available one is `CP::AsymptMatrixTool`.')
        self.addOption('config', '', type=str,
                       info='the input to the tool (ROOT or XML file). Some test files (deprecated and computed in Release 21) can be found in `dev/AnalysisTop/FakeBkgToolsData/`.')
        self.addOption('process', '', type=str,
                       info='this argument allows one to set what kind of processes, in terms of number of real and fake/non-prompt leptons in the final state, are estimated by the fake lepton background yields or weights computed by the tools. More details [here](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/AnalysisCommon/FakeBkgTools/doc/arg_process.md).')
        self.addOption('definition', '', type=str,
                       info='this argument allows the user to specify the definition of the region of interest, in terms of how many (tight) leptons are selected, and how extra leptons are treated (vetoed or not). This must describe the way events are selected in the main analysis. More details [here](https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/AnalysisCommon/FakeBkgTools/doc/arg_selection.md).')

    def instanceName (self) :
        """Return the instance name for this block"""
        return self.setupName

    def makeAlgs(self, config):

        alg = config.createAlgorithm('CP::FakeBkgCalculatorAlg', 'FakeBkgCalculatorAlg')

        alg.electrons, alg.electronSelection = config.readNameAndSelection(self.electrons)
        alg.electronSelectionTarget = config.getFullSelection(self.electrons.split(".")[0], self.electronsTarget)
        alg.muons, alg.muonSelection = config.readNameAndSelection(self.muons)
        alg.muonSelectionTarget = config.getFullSelection(self.muons.split(".")[0], self.muonsTarget)

        alg.eventPreselection = self.eventPreselection
        alg.Process = self.process
        alg.Definition = self.definition

        config.addPrivateTool('FakeTool', self.fakeTool)
        alg.FakeTool.InputFiles = [self.config]
        alg.FakeTool.Process = self.process
        alg.FakeTool.Selection = self.definition
        alg.FakeTool.EnergyUnit = 'GeV'
        alg.FakeTool.ConvertWhenMissing = True
        alg.FakeTool.TightDecoration = 'TightForFakeBkgCalculation,as_char'

        alg.FakeToolOutput = 'FakeToolOutput_' + self.setupName + '_%SYS%'
        config.addOutputVar('EventInfo', alg.FakeToolOutput, 'weight_fake_' + self.setupName, noSys=True)

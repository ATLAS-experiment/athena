# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

########################################################################
#
# SliceDef file for bphysics chains/signatures
#
#########################################################################
from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)
from ..Config.ChainConfigurationBase import ChainConfigurationBase
from ..Muon.MuonChainConfiguration import MuonChainConfiguration

from ..Muon.MuonMenuSequences import mul2IOOvlpRmSequenceGenCfg, mul2mtCBOvlpRmSequenceGenCfg, muEFCBSequenceGenCfg

from .BphysicsMenuSequences import dimuL2SequenceGenCfg, dimuEFSequenceGenCfg, bmumuxSequenceGenCfg
from TrigBphysHypo.TrigMultiTrkComboHypoConfig import StreamerDimuL2ComboHypoCfg, StreamerDimuL2IOComboHypoCfg, StreamerDimuL2MTComboHypoCfg, DimuEFComboHypoCfg, BmutrkComboHypoCfg, StreamerDimuEFComboHypoCfg, TrigMultiTrkComboHypoToolFromDict
from TrigBphysHypo.TrigBmumuxComboHypoConfig import BmumuxComboHypoCfg, TrigBmumuxComboHypoToolFromDict
from TrigBphysHypo.TrigBmuxComboHypoConfig import BmuxComboHypoCfg


#############################################
###  Class/function to configure muon chains
#############################################

class BphysicsChainConfiguration(MuonChainConfiguration):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)

    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleBphysChain(self, flags):

        log.debug("Assembling chain for %s", self.chainName)

        stepDictionary = self.getBphysStepDictionary()
        key = self.getBphysKey()
        steps=stepDictionary[key]

        chainSteps = []
        for step_level in steps:
            for step in step_level:
                chainStep = getattr(self, step)(flags)
                chainSteps+=[chainStep]

        chain = self.buildChain(chainSteps)
        return chain

    def getBphysStepDictionary(self):

        stepDictionary = {
            'dimu'   : [['getmuFast', 'getDimuL2'], ['getmuEFSA', 'getmuEFCB', 'getDimuEF']],
            'bmumux' : [['getmuFast', 'getDimuL2'], ['getmuEFSA', 'getDimuEFCB', 'getBmumux']],
            'bmutrk' : [['getmuFast', 'getmuCombIO'], ['getmuEFSA', 'getmuEFCB', 'getBmutrk']],
            'bmux'   : [['getmuFast', 'getmuCombIO'], ['getmuEFSA', 'getmuEFCB', 'getBmux']],
        }
        return stepDictionary

    def getBphysKey(self):

        the_topo = self.dict['topo'][0]

        topo_dict = {
            'bJpsimumu'  : 'dimu',
            'bJpsi'      : 'dimu',
            'bJpsimutrk' : 'bmutrk',
            'bUpsimumu'  : 'dimu',
            'bUpsi'      : 'dimu',
            'bBmumu'     : 'dimu',
            'bDimu'      : 'dimu',
            'bDimu2700'  : 'dimu',
            'bDimu6000'  : 'dimu',
            'bPhi'       : 'dimu',
            'bTau'       : 'dimu',
            'b3mu'       : 'dimu',
            'bBmux'      : 'bmux',
            'bBmumux'    : 'bmumux',
            'b0dRAB12vtx20' : 'dimu',
            'b0dRAB207invmAB22vtx20' : 'dimu',
            'b0dRAB127invmAB22vtx20' : 'dimu',
            'b7invmAB22vtx20' : 'dimu',
        }

        return topo_dict[the_topo]

    def getDimuL2(self, flags):
        if 'noL2Comb' in self.chainPart['extra']:
            return self.getStep(flags, 2, 'dimuL2', [dimuL2SequenceGenCfg], comboHypoCfg=StreamerDimuL2ComboHypoCfg)
        elif 'l2mt' in self.chainPart['l2AlgInfo']:
            return self.getStep(flags, 2, 'dimuL2MT', [mul2mtCBOvlpRmSequenceGenCfg], comboHypoCfg=StreamerDimuL2MTComboHypoCfg)
        else:
            return self.getStep(flags, 2, 'dimuL2IO', [mul2IOOvlpRmSequenceGenCfg], comboHypoCfg=StreamerDimuL2IOComboHypoCfg)

    def getDimuEF(self, flags):
        return self.getStep(flags, 5, 'dimuEF', [dimuEFSequenceGenCfg], comboHypoCfg=DimuEFComboHypoCfg, comboTools=[TrigMultiTrkComboHypoToolFromDict])

    def getDimuEFCB(self, flags):
        return self.getStep(flags, 4, 'dimuEFCB', [muEFCBSequenceGenCfg], comboHypoCfg=StreamerDimuEFComboHypoCfg)

    def getBmux(self, flags):
        return self.getStep(flags, 5, 'bmux', [bmumuxSequenceGenCfg], comboHypoCfg=BmuxComboHypoCfg, comboTools=[TrigBmumuxComboHypoToolFromDict])

    def getBmumux(self, flags):
        return self.getStep(flags, 5, 'bmumux', [bmumuxSequenceGenCfg], comboHypoCfg=BmumuxComboHypoCfg, comboTools=[TrigBmumuxComboHypoToolFromDict])

    def getBmutrk(self, flags):
        return self.getStep(flags, 5, 'bmutrk', [bmumuxSequenceGenCfg], comboHypoCfg=BmutrkComboHypoCfg, comboTools=[TrigMultiTrkComboHypoToolFromDict])

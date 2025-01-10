# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)


from ..Config.ChainConfigurationBase import ChainConfigurationBase

from TriggerMenuMT.CFtest.HLTSignatureConfig import  muMenuSequence, elMenuSequence, gamMenuSequence
from TriggerMenuMT.CFtest.HLTSignatureHypoTools import dimuDrComboHypoTool
from TriggerMenuMT.HLT.Config.MenuComponents import EmptyMenuSequenceCfg

#--------------------------------------------------------
# fragments generating config will be functions in new JO
#--------------------------------------------------------


# Muons
def muCfg(flags,step,reconame, hyponame):
    return muMenuSequence(flags,step,reconame, hyponame)

def muCfg111(flags):
    return muCfg(flags,step="1",reconame="v1", hyponame="v1")

def muCfg211(flags):
    return muCfg(flags,step="2",reconame="v1", hyponame="v1")

def muCfg311(flags):
    return muCfg(flags,step="3",reconame="v1", hyponame="v1")

def muCfg322(flags):
    return muCfg(flags,step="3",reconame="v2", hyponame="v2")

def muCfg411(flags):
    return muCfg(flags,step="4",reconame="v1", hyponame="v1")

def muCfg222(flags):
    return muCfg(flags,step="2",reconame="v2", hyponame="v2")


# Egamma
def elCfg(flags,step,reconame, hyponame):
    return elMenuSequence(flags,step,reconame, hyponame)

def gamCfg(flags,step,reconame, hyponame):
    return gamMenuSequence(flags,step,reconame, hyponame)

def elCfg111(flags):
    return elCfg(flags,step="1",reconame="v1", hyponame="v1")

def elCfg211(flags):
    return elCfg(flags,step="2",reconame="v1", hyponame="v1")

def elCfg222(flags):
    return elCfg(flags,step="2",reconame="v2", hyponame="v2")

def elCfg223(flags):
    return elCfg(flags,step="2",reconame="v2", hyponame="v3")

def elCfg311(flags):
    return elCfg(flags,step="3",reconame="v1", hyponame="v1")

def gamCfg111(flags):
    return gamCfg(flags,step="1",reconame="v1", hyponame="v1")


 
#----------------------------------------------------------------
# Class to configure chain
#----------------------------------------------------------------
class TestChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)
        
    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):
        chainSteps = []
        stepDictionary = self.getStepDictionary()
        key = self.chainPart['extra']

        log.debug('testChain key = %s', key)
        if key in stepDictionary:
            steps=stepDictionary[key]
        else:
            raise RuntimeError("Chain configuration unknown for electron chain with key: " + key )
        
        for step in steps:
            chainstep = getattr(self, step)(flags)
            chainSteps+=[chainstep]

            
        myChain = self.buildChain(chainSteps)
        return myChain


    
    def getStepDictionary(self):
          # --------------------
        # define names of the steps and obtain the chainStep configuration 
        # --------------------

        stepDictionary = {
            #muons
            'mv1step': ['Step_mu11'],
            'mv1':     ['Step_mu11', 'Step_mu21', 'Step_mu31', 'Step_mu41'], 
            'mv2':     ['Step_mu11', 'Step_mu22', 'Step_mu31'],
            'mEmpty1': ['Step_empty1', 'Step_mu21'], # empty step
            #'mEmpty1': ['Step_empty1', 'Step_mu11'], # try to break 'Step_mu21'],
            'mEmpty2': ['Step_mu11'  ,'Step_empty2' ,'Step_mu31', 'Step_mu41'], # same as mv1 with empty step
            'mEmpty3': ['Step_mu11'  ,'Step_empty2' ,'Step_empty3', 'Step_mu41'], # empty step + emtpy sequence
            'mv1dr' :  ['Step_mu11Dr', 'Step_mu21', 'Step_mu31', 'Step_mu41'],
            #egamma
            'ev1':     ['Step_em11', 'Step_em21', 'Step_em31'],
            'ev2':     ['Step_em11', 'Step_em22'], 
            'ev3':     ['Step_em11', 'Step_em23'],
            'gv1':     ['Step_gam11'],
            'ev1dr' :  ['Step_em11Dr', 'Step_em21Dr', 'Step_em31']
        }
        return stepDictionary

    ## Muons    
    
    def Step_mu11(self, flags):
        return self.getStep(flags, "mu11",[ muCfg111 ])

    def Step_mu21(self, flags):
        return self.getStep(flags, "mu21",[ muCfg211 ])

    def Step_mu11Dr(self, flags):
        return self.getStep(flags, "mu11",[ muCfg111 ], comboTools=[dimuDrComboHypoTool])

    def Step_mu21Dr(self, flags):
        return self.getStep(flags, "mu21",[ muCfg211 ], comboTools=[dimuDrComboHypoTool])

    def Step_mu22(self, flags):
        return self.getStep(flags, "mu22",[ muCfg222 ])

    def Step_mu31(self, flags):
        return self.getStep(flags, "mu31",[ muCfg311 ])

    def Step_mu32(self, flags):
        return self.getStep(flags, "mu32",[ muCfg322 ])
   
    def Step_mu41(self, flags):
        return self.getStep(flags, "mu41",[ muCfg411 ])

    def Step_empty1(self, flags):
        return self.getEmptyStep('empty')

    def Step_empty2(self, flags):
        return self.getEmptyStep('empty')

    def Step_empty3(self, flags):
        return self.getStep(flags,'emptySeq', [EmptyMenuSequenceCfg], name="EmptySequence")

    # Electrons

    def Step_em11(self, flags):
        return self.getStep(flags, "em11",[ elCfg111 ])
    
    def Step_em11Dr(self, flags):
        return self.getStep(flags, "em11",[ elCfg111 ], comboTools=[dimuDrComboHypoTool])

    def Step_em21(self, flags):
        return self.getStep(flags, "em21",[ elCfg211 ])

    def Step_em21Dr(self, flags):
        return self.getStep(flags, "em21",[ elCfg211 ], comboTools=[dimuDrComboHypoTool])

    def Step_em22(self, flags):
        return self.getStep(flags, "em22",[ elCfg222 ])

    def Step_em23(self, flags):
        return self.getStep(flags, "em23",[ elCfg223 ])

    def Step_em31(self, flags):
        return self.getStep(flags, "em31",[ elCfg311 ])

    def Step_gam11(self, flags):
        return self.getStep(flags, "gam11",[ gamCfg111 ])


#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
from ..Config.ChainConfigurationBase import ChainConfigurationBase
from ..CommonSequences.CaloSequences import fastCaloSequenceGenCfg
from ..Photon.FastPhotonMenuSequences import fastPhotonSequenceGenCfg
from ..Photon.PrecisionPhotonCaloIsoMenuSequences import precisionPhotonCaloIsoSequenceGenCfg
from ..Photon.PrecisionPhotonMenuSequences import precisionPhotonSequenceGenCfg
from ..Photon.PrecisionCaloMenuSequences import precisionCaloSequenceGenCfg
from ..Photon.HipTRTMenuSequences import TRTHitGeneratorSequenceGenCfg


def _diPhotonComboHypoToolFromDict(flags, chainDict, lowermass=80000,uppermass=-999,dphi=1.5,applymass=False,applydphi=False):
    name = chainDict['chainName']
    monTool = GenericMonitoringTool(flags, "MonTool_"+name,
                                    HistPath = 'EgammaMassHypo/'+name)
    monTool.defineHistogram('DphiOfAccepted', type='TH1F', path='EXPERT', title="PrecisionCalo Hypo entries per Phi;Phi", xbins=128, xmin=-3.2, xmax=3.2)
    monTool.defineHistogram('MassOfAccepted', type='TH1F', path='EXPERT', title="Mass in accepted combinations [MeV]", xbins=75, xmin=0, xmax=150000)

    tool= CompFactory.TrigEgammaTopoHypoTool(name,
                                             AcceptAll = False,
                                             ApplyMassCut = applymass,
                                             LowerMassEgammaClusterCut = lowermass,
                                             UpperMassEgammaClusterCut = uppermass,
                                             ApplyDPhiCut = applydphi,
                                             ThresholdDPhiCut = dphi,
                                             MonTool = monTool)
    return tool

def diphotonDPhiHypoToolFromDict(flags, chainDict):
    return _diPhotonComboHypoToolFromDict(flags,chainDict,lowermass=80000,uppermass=-999,dphi=1.5,applymass=False,applydphi=True)

def diphotonDPhiMassHypoToolFromDict(flags, chainDict):
    return _diPhotonComboHypoToolFromDict(flags,chainDict,lowermass=80000,uppermass=-999,dphi=1.5,applymass=True,applydphi=True)


#----------------------------------------------------------------
# Class to configure chain
#----------------------------------------------------------------
class PhotonChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)
        self.chainDict = chainDict

    # ----------------------
    # Prepare the sequence
    # ----------------------
    def prepareSequence(self):
        # This function prepares the list of step names from which assembleChainImpl would make the chain assembly from.
        
        # --------------------
        # define here the names of the steps and obtain the chainStep configuration
        # --------------------

        stepNames = [] # This will contain the name of the steps we will want to configure
        # Put first fast Calo. Two possible variants now: 
        # Step 1
        stepNames += ['getFastCalo']

        # OK now, unless its a HipTRT chain we need to do fastPhoton here:
        if self.chainPart['extra'] == 'hiptrt':
            stepNames += ['getHipTRT']
            # for hiptrt chains, there is noprecision Calo nor precision Photon so returning sequence here:
            return stepNames

        # Now we do fast Photon
        # Step 2
        stepNames += ['getFastPhoton']


        # After we need to place precisionCalo. There is no chain (except hiptrt) that does not require precision calo. Otherwise please insert logic here:
        # Step 3
        stepNames += ['getPrecisionCaloPhoton']

        # And we will do precisionPhoton UNLESS its an etcut chain
        if 'etcut' in self.chainPart['IDinfo']:
            # if its an etcut chain we return the sequence up to here
            return stepNames


        #Now its the turn of precision Photon. Here we apply cutbased identification
        #Step 4
        stepNames += ['getPrecisionPhoton']

        # Finally we need to run isolaton *IF* its an isolated chain
        # Step 5
        if 'noiso' in self.chainPart['isoInfo'] or 'icaloloose' in self.chainPart['isoInfo'] or 'icalomedium' in self.chainPart['isoInfo'] or 'icalotight' in self.chainPart['isoInfo']:
            stepNames += ['getPhotonCaloIso']

        return stepNames



    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):
        chainSteps = []
        log.debug("Assembling chain for %s", self.chainName)

        # This will contain the name of the steps we will want to configure
        steps = self.prepareSequence()

        # This is it, lets print the list of stepNames
        log.debug("stepNames: %s", steps)
        for step in steps:
            log.debug('Adding photon trigger step %s', step)
            is_probe_leg = self.chainPart['tnpInfo']=='probe'
            chainstep = getattr(self, step)(flags, is_probe_leg=is_probe_leg)
            chainSteps+=[chainstep]

        myChain = self.buildChain(chainSteps)

        return myChain

    # --------------------
    # Configuration of steps
    # --------------------
    def getFastCalo(self, flags, is_probe_leg=False):
        stepName = "PhotonFastCalo"
        
        return self.getStep(flags,1,stepName,[fastCaloSequenceGenCfg], name='Photon', is_probe_leg=is_probe_leg)

    def getFastPhoton(self, flags, is_probe_leg=False):
        stepName = "FastPhoton"
        return self.getStep(flags,2,stepName,[fastPhotonSequenceGenCfg], is_probe_leg=is_probe_leg)

    def getPrecisionCaloPhoton(self, flags, is_probe_leg=False):
        do_ion = 'ion' in self.chainPart['extra']
        if do_ion:
            stepName = "PhotonPrecisionHICalo"
        else:
            stepName = "PhotonPrecisionCalo"

        return self.getStep(flags,3,stepName,[precisionCaloSequenceGenCfg], ion=do_ion, is_probe_leg=is_probe_leg)
    
    def getHipTRT(self, flags, is_probe_leg=False):
        stepName = "hipTRT"
        return self.getStep(flags,2,stepName,[TRTHitGeneratorSequenceGenCfg], is_probe_leg=is_probe_leg)

    def getPrecisionPhoton(self, flags, is_probe_leg=False):

        stepName = "precision_photon"
        do_ion = 'ion' in self.chainPart['extra'] == 'ion'

        if do_ion:
            stepName += '_ion'

        return self.getStep(flags,4,stepName,sequenceCfgArray=[precisionPhotonSequenceGenCfg], ion=do_ion, is_probe_leg=is_probe_leg)

    def getPhotonCaloIso(self, flags, is_probe_leg=False):

        stepName = "precision_photon_CaloIso"
        comboTools = []
        do_ion = 'ion' in self.chainPart['extra']

        if do_ion:
            stepName += '_ion'
        
        if "dPhi15" in self.chainDict["topo"]:
            stepName+='_dPhi15'
            if "m80" in self.chainDict["topo"]:
                stepName+= '_m80'
                comboTools.append(diphotonDPhiMassHypoToolFromDict)
            else:
                comboTools.append(diphotonDPhiHypoToolFromDict)

        return self.getStep(flags,5,stepName,sequenceCfgArray=[precisionPhotonCaloIsoSequenceGenCfg], name='Photon', comboTools=comboTools, ion=do_ion, is_probe_leg=is_probe_leg)

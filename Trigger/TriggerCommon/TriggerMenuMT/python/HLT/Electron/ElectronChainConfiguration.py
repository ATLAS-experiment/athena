#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from ..Config.ChainConfigurationBase import ChainConfigurationBase
from ..CommonSequences.CaloSequences import fastCaloMenuSequenceCfg
from ..CommonSequences.CaloSequences_FWD import fastCaloMenuSequence_FWDCfg
from ..Electron.FastElectronMenuSequences import fastElectronSequenceCfg, fastElectronSequence_LRTCfg
from ..Electron.FastTrackingMenuSequences import fastTrackingSequenceCfg, fastTrackingSequence_LRTCfg
from ..Electron.PrecisionCaloMenuSequences import precisionCaloSequenceCfg, precisionCaloSequence_LRTCfg
from ..Electron.PrecisionElectronMenuSequences import precisionElectronSequenceCfg, precisionElectronSequence_LRTCfg
from ..Electron.PrecisionElectronMenuSequences_GSF import precisionElectronSequence_GSFCfg, precisionElectronSequence_GSF_LRTCfg
from TrigBphysHypo.TrigMultiTrkComboHypoConfig import NoMuonDiElecPrecisionGSFComboHypoCfg, DiElecPrecisionGSFComboHypoCfg, TrigMultiTrkComboHypoToolFromDict
from ..Electron.PrecisionTrackingMenuSequences import precisionTrackingSequenceCfg, precisionTrackingSequence_LRTCfg
from ..Electron.PrecisionTracks_GSFRefittedMenuSequences   import precisionTracks_GSFRefittedSequenceCfg, precisionTracks_GSFRefittedSequence_LRTCfg

from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool



def _diElectronMassComboHypoToolFromDict(flags, chainDict, mass_range):
    name = chainDict['chainName']
    monTool = GenericMonitoringTool(flags, "MonTool_"+name,
                                    HistPath = 'EgammaMassHypo/'+name)
    monTool.defineHistogram('DphiOfProcessed', type='TH1F', path='EXPERT', title="PrecisionCalo Hypo entries per Phi;Phi", xbins=128, xmin=-3.2, xmax=3.2)
    monTool.defineHistogram('MassOfProcessed', type='TH1F', path='EXPERT', title="Mass in accepted combinations [MeV]", xbins=75, xmin=0, xmax=150000)
    monTool.defineHistogram('DphiOfAccepted', type='TH1F', path='EXPERT', title="PrecisionCalo Hypo entries per Phi;Phi", xbins=128, xmin=-3.2, xmax=3.2)
    monTool.defineHistogram('MassOfAccepted', type='TH1F', path='EXPERT', title="Mass in accepted combinations [MeV]", xbins=75, xmin=0, xmax=150000)

    tool = CompFactory.TrigEgammaTopoHypoTool(name,
                                              AcceptAll = False,
                                              ApplyMassCut = True,
                                              LowerMassEgammaClusterCut = mass_range[0],
                                              UpperMassEgammaClusterCut = mass_range[1],
                                              MonTool = monTool)
    return tool


def diElectronZeeMassComboHypoToolFromDict(flags, chainDict):
    return _diElectronMassComboHypoToolFromDict(flags, chainDict, (50000, 130000))

def diElectronJpsieeMassComboHypoToolFromDict(flags, chainDict):
    return _diElectronMassComboHypoToolFromDict(flags, chainDict, (1000, 5000))

def diEgammaHegMassComboHypoToolFromDict(flags, chainDict):
    return _diElectronMassComboHypoToolFromDict(flags, chainDict, (90000, 1400000))


#----------------------------------------------------------------
# Class to configure chain
#----------------------------------------------------------------
class ElectronChainConfiguration(ChainConfigurationBase):

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

        # Step1
        # Put first fast Calo. Two possible variants now: 
        # getFastCalo 
        # getFastCalo_fwd
        # But first, we need to check whether this is a chain that will run smth at fast or precision. So if no L2IDAlg or IDinfo defined, just return this here

        if not self.chainPart['IDinfo'] and not self.chainPart['L2IDAlg'] and not self.chainPart['isoInfo'] and not self.chainPart['addInfo']:
            return stepNames

        if "fwd" in self.chainPart['addInfo']:
            stepNames += ['getFastCalo_fwd']
            
            # Actually, if its fwd and etcut we will stop here (for now)
            if "etcut" in self.chainPart['addInfo']:
                return stepNames
        else:
            stepNames += ['getFastCalo']

        # Step2
        # Now lets do Fast Electron. Possible Flavours:
        # getFastTracking
        # getFastTracking_lrt

        if self.chainPart['lrtInfo']:
            stepNames += ['getFastTracking_lrt']
        else:
            stepNames += ['getFastTracking'] 


        # Step3
        # Now lets do Fast Electron. Possible Flavours:
        # getFastElectron
        # getFastElectron_lrt

        if self.chainPart['lrtInfo']:
            stepNames += ['getFastElectron_lrt']
        else:
            stepNames += ['getFastElectron']


        # Step4
        # After Fast Electron we have to build PrecisionCalo for electorns. Current available variantas are:
        # getPrecisionCaloElectron
        # getPrecisionCaloElectron_lrt
        # Now, we will add this if there is any IDInfo (i.e. any of dnn* or lh* in the chain name). Otherwise we wont run precision steps
        
        if not self.chainPart['IDinfo'] and not self.chainPart['isoInfo'] and not self.chainPart['addInfo']: 
            log.debug("No IDInfo, no isoInfo and no addInfo. Returning here up to fastElectron")
            return stepNames

        if self.chainPart['lrtInfo']:
            stepNames += ['getPrecisionCaloElectron_lrt']
        else:
            stepNames += ['getPrecisionCaloElectron']

        # If its an etcut chain, we will not run precision Tracking Electron. Just precision Calo. So returning here if its an etcut chain unless its an etcut_idperf chaiin:
        
        if 'etcut' in self.chainPart['addInfo'] and 'idperf' not in self.chainPart['idperfInfo']:
            log.debug("This is an etcut chain. Returning here")
            return stepNames

        # Step5
        # After precisionCalo Electron we have to do precision tracking next. Current available variantas are:
        # getPrecisionTracking
        # getPrecisionTracking_lrt

        if self.chainPart['lrtInfo']:
            stepNames += ['getPrecisionTracking_lrt']
        else:
            stepNames += ['getPrecisionTracking']

        # Step6
        # Now if a chain is configured to do gsf refitting we need to add another tracking step for the GSF refitting:
        # getPrecisionTrack_GSFRefitted
        # getPrecisionTrack_GSFRefitted_lrt

        if "" in self.chainPart['gsfInfo'] and 'nogsf' not in self.chainPart['gsfInfo']:
            if self.chainPart['lrtInfo']:
                stepNames += ['getPrecisionTrack_GSFRefitted_lrt']
            else:
                stepNames += ['getPrecisionTrack_GSFRefitted']
        # if its nogsf, then we need to add an addtional empty step to keep aligned the gsf chains (with the additional refitting)     
        else:
            if 'idperf' not in self.chainPart['idperfInfo']:
                # Only add an empty refiting step if its not an idperf - nonGSF. Otherwise the last step will be an empty step and that doesnt work
                stepNames += ['getEmptyRefitStep']

           
            
        # If its an idperf chain, we will not run precision Electron. Just precision Calo and Precision Tracking so returning here if its an etcut chain
        if 'idperf' in self.chainPart['idperfInfo']:
            log.debug("This is an idperf chain. Returning here")
            return stepNames


        # Step7
        # and Finally! once we have precision tracking adn precision calo, we can build our electrons!. Current available variantas are:
        # getPrecisionElectron
        # getPrecisionGSFElectron
        # getPrecisionElectron_lrt

        if "nogsf" in self.chainPart['gsfInfo']:
            if self.chainPart['lrtInfo']:
                stepNames += ['getPrecisionElectron_lrt']
            else:
                stepNames += ['getPrecisionElectron']
        
        else:
            if self.chainPart['lrtInfo']:
                stepNames += ['getPrecisionGSFElectron_lrt']
            else:
                stepNames += ['getPrecisionGSFElectron']

        log.debug("Returning chain with all steps in the sequence")
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
            log.debug('Adding electron trigger step %s', step)
            is_probe_leg = self.chainPart['tnpInfo']=='probe'
            chainstep = getattr(self, step)(flags, is_probe_leg=is_probe_leg)
            chainSteps+=[chainstep]

        myChain = self.buildChain(chainSteps) 
        
        return myChain

    # -------------------------------
    # Configuration of electron steps
    # -------------------------------

    def getFastCalo(self, flags, is_probe_leg=False):
        stepName       = "FastCalo_electron"
        return self.getStep(flags,1,stepName,[fastCaloMenuSequenceCfg], name='Electron', is_probe_leg=is_probe_leg)

    def getFastTracking(self, flags, is_probe_leg=False):
        stepName = "fast_tracking"
        return self.getStep(flags,2,stepName,[ fastTrackingSequenceCfg],is_probe_leg=is_probe_leg)

    def getFastTracking_lrt(self, flags, is_probe_leg=False):
        stepName = "fast_tracking_lrt"
        return self.getStep(flags,2,stepName,[ fastTrackingSequence_LRTCfg],is_probe_leg=is_probe_leg)

    def getFastElectron(self, flags, is_probe_leg=False):
        if self.chainPart['idperfInfo']:
            stepName = "fast_electron_empty"
            return self.getEmptyStep(3,stepName)
        else:
            stepName = "fast_electron"
            return self.getStep(flags,3,stepName,[fastElectronSequenceCfg],is_probe_leg=is_probe_leg)

    def getFastElectron_lrt(self, flags, is_probe_leg=False):
        if self.chainPart['idperfInfo']:
            stepName = "fast_electron_lrt_empty"
            return self.getEmptyStep(3,stepName)
        else:
            stepName = "fast_electron_lrt"
            return self.getStep(flags,3,stepName,[fastElectronSequence_LRTCfg],is_probe_leg=is_probe_leg)

    def getPrecisionCaloElectron(self, flags, is_probe_leg=False):
        if self.chainPart['extra'] == 'ion':
            stepName = 'precisionCalo_ion_electron'
            return self.getStep(flags,4, stepName, [precisionCaloSequenceCfg], ion=True, is_probe_leg=is_probe_leg)

        stepName = "precisionCalo_electron"
        return self.getStep(flags,4,stepName,[precisionCaloSequenceCfg], is_probe_leg=is_probe_leg)
    
    def getPrecisionCaloElectron_lrt(self, flags, is_probe_leg=False):
        stepName = "precisionCalo_electron_lrt"
        return self.getStep(flags,4,stepName,[precisionCaloSequence_LRTCfg],is_probe_leg=is_probe_leg)

    def getPrecisionTracking(self, flags, is_probe_leg=False):
        if self.chainPart['extra'] == 'ion':
            stepName = 'precisionTracking_ion_electron'
            return self.getStep(flags,5, stepName, [precisionTrackingSequenceCfg], ion=True, is_probe_leg=is_probe_leg)

        stepName = "precisionTracking_electron"
        return self.getStep(flags,5,stepName,[precisionTrackingSequenceCfg], is_probe_leg=is_probe_leg)

    def getPrecisionTracking_lrt(self, flags, is_probe_leg=False):
        stepName = "precisionTracking_electron_lrt"
        return self.getStep(flags,5,stepName,[precisionTrackingSequence_LRTCfg],is_probe_leg=is_probe_leg)

    def getPrecisionTrack_GSFRefitted(self, flags, is_probe_leg=False):
        stepName = "PrecisionTrack_GSFRefitted_electron"
        return self.getStep(flags,6,stepName,[precisionTracks_GSFRefittedSequenceCfg], is_probe_leg=is_probe_leg)
 
    def getPrecisionTrack_GSFRefitted_lrt(self, flags, is_probe_leg=False):
        stepName = "PrecisionTrack_GSFRefitted_electron_lrt"
        return self.getStep(flags,6,stepName,[precisionTracks_GSFRefittedSequence_LRTCfg], is_probe_leg=is_probe_leg)

    def getPrecisionElectron(self, flags, is_probe_leg=False):

        isocut = self.chainPart['isoInfo']
        log.debug(' isolation cut = %s', isocut)
         
        if "Zee" in self.chainDict['topo']:
            stepName = "precision_electron_Zee"+str(isocut)
            return self.getStep(flags,7,stepName,sequenceCfgArray=[precisionElectronSequenceCfg], comboTools=[diElectronZeeMassComboHypoToolFromDict], is_probe_leg=is_probe_leg)
        elif "Jpsiee" in self.chainDict['topo']:
            stepName = "precision_topoelectron_Jpsiee"+str(isocut)
            return self.getStep(flags,7,stepName,sequenceCfgArray=[precisionElectronSequenceCfg], comboTools=[diElectronJpsieeMassComboHypoToolFromDict], is_probe_leg=is_probe_leg)
        elif "Heg" in  self.chainDict['topo']:
            stepName = "precision_electron_Heg"+str(isocut)
            return self.getStep(flags,7,stepName,sequenceCfgArray=[precisionElectronSequenceCfg], comboTools=[diEgammaHegMassComboHypoToolFromDict], is_probe_leg=is_probe_leg)
        elif self.chainPart['extra'] == 'ion':
            stepName = "precision_ion_electron" + str(isocut)
            return self.getStep(flags,7,stepName,[precisionElectronSequenceCfg], ion=True, is_probe_leg=is_probe_leg)
        else:
            stepName = "precision_electron_nominal"+str(isocut)
            return self.getStep(flags,7,stepName,[ precisionElectronSequenceCfg ], is_probe_leg=is_probe_leg)     

    def getPrecisionGSFElectron(self, flags, is_probe_leg=False):

        isocut = self.chainPart['isoInfo']
        log.debug(' isolation cut = %s', isocut)
       
        if "Zee" in  self.chainDict['topo']:
            stepName = "precision_topoelectron_Zee_GSF"+str(isocut)
            return self.getStep(flags,7,stepName,sequenceCfgArray=[precisionElectronSequence_GSFCfg], comboTools=[diElectronZeeMassComboHypoToolFromDict], is_probe_leg=is_probe_leg)
        elif "Jpsiee" in  self.chainDict['topo']:
            stepName = "precision_topoelectron_Jpsiee_GSF"+str(isocut)
            return self.getStep(flags,7,stepName,sequenceCfgArray=[precisionElectronSequence_GSFCfg], comboTools=[diElectronJpsieeMassComboHypoToolFromDict], is_probe_leg=is_probe_leg)
        elif "bBeeM6000" in  self.chainDict['topo']:
            signatures = self.chainDict['signatures']
            if signatures.count(signatures[0]) == len(signatures):
                stepName = "noMuon_precision_electron_bBee_GSF"+str(isocut)
                return self.getStep(flags,7,stepName,sequenceCfgArray=[precisionElectronSequence_GSFCfg], comboHypoCfg=NoMuonDiElecPrecisionGSFComboHypoCfg, comboTools=[TrigMultiTrkComboHypoToolFromDict], is_probe_leg=is_probe_leg)
            else:
                stepName = "precision_electron_bBee_GSF"+str(isocut)
                return self.getStep(flags,7,stepName,sequenceCfgArray=[precisionElectronSequence_GSFCfg], comboHypoCfg=DiElecPrecisionGSFComboHypoCfg, comboTools=[TrigMultiTrkComboHypoToolFromDict], is_probe_leg=is_probe_leg)
        else:
            stepName = "precision_electron_GSF"+str(isocut)
            return self.getStep(flags,7,stepName,[precisionElectronSequence_GSFCfg], is_probe_leg=is_probe_leg)

    def getPrecisionGSFElectron_lrt(self, flags, is_probe_leg=False):

        isocut = self.chainPart['isoInfo']
        log.debug(' isolation cut = %s', isocut)
        stepName = "precision_electron_LRTGSF"+str(isocut)
        return self.getStep(flags,7,stepName,[precisionElectronSequence_GSF_LRTCfg], is_probe_leg=is_probe_leg)

    def getPrecisionElectron_lrt(self, flags, is_probe_leg=False):

        isocut = self.chainPart['isoInfo']
        log.debug(' isolation cut = %s', isocut)
        stepName = "precision_electron_lrt"+str(isocut)
        return self.getStep(flags,7,stepName,[ precisionElectronSequence_LRTCfg],is_probe_leg=is_probe_leg)

    def getFastCalo_fwd(self, flags, is_probe_leg=False):
        stepName       = "FastCalo_FWD_electron"
        return self.getStep(flags,1, stepName, [fastCaloMenuSequence_FWDCfg], name='Electron', is_probe_leg=is_probe_leg)

    def getEmptyRefitStep(self, flags,  is_probe_leg=False):
        return self.getEmptyStep(6,'nonGSFEmptyRefit')



# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import re
from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from ..Config.ChainConfigurationBase import ChainConfigurationBase

from .JetRecoSequencesConfig import JetRecoDataDeps

from .JetMenuSequencesConfig import (
    jetCaloHypoMenuSequenceGenCfg,
    jetRoITrackJetTagHypoMenuSequenceGenCfg,
    jetFSTrackingHypoMenuSequenceGenCfg,
    jetCaloRecoMenuSequenceGenCfg, 
    jetCaloPreselMenuSequenceGenCfg,
    jetHICaloHypoMenuSequenceGenCfg,
)
from .ExoticJetSequencesConfig import jetEJsMenuSequenceGenCfg, jetCRVARMenuSequenceGenCfg,jetCRMenuSequenceGenCfg

from . import JetRecoCommon
from . import JetPresel

from TrigEDMConfig.TriggerEDM import recordable

import copy

#----------------------------------------------------------------
# Class to configure chain
#----------------------------------------------------------------
class JetChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        # we deliberately don't call base class constructore, since this assumes a single chain part
        # which is not the case for jets

        self.dict = copy.deepcopy(chainDict)
        
        self.chainName = self.dict['chainName']
        self.chainL1Item = self.dict['L1item']

        self.chainPart = self.dict['chainParts']
        self.L1Threshold = ''
        self.mult = 1 # from the framework point of view I think the multiplicity is 1, internally the jet hypo has to figure out what to actually do

        # these properties are in the base class, but I don't think we need them for jets
        #self.chainPartName = ''
        #self.chainPartNameNoMult = ''
        #self.chainPartNameNoMultwL1 = ''

        # expect that the L1 seed is the same for all jet parts, otherwise we have a problem
        jChainParts = JetRecoCommon.jetChainParts(self.chainPart)
        # Register if this is a performance chain, in which case the HLT should be exactly j0_perf
        self.isPerf = False
        # Exotic hypo (emerging-jets, trackless jets)
        self.exotHypo = ''
        # Check if we intend to preselect events with calo jets in step 1
        self.trkpresel = "nopresel"

        for ip,p in enumerate(jChainParts):
            # Check if there is exactly one exotic hypothesis defined
            if len(p['exotHypo']) > 1:
                raise RuntimeError('emerging chains currently not configurable with more than one emerging selection!')
            if p['exotHypo']:
                self.exotHypo = p['exotHypo'][0]

            if 'perf' in p['addInfo']:
                # Need to ensure a few conditions to ensure there is no selection bias:
                # No preselection, no special hypo
                # Only one chainPart is permitted
                verify_null_str = [ p[k]!='' for k in ['jvt','momCuts','timing','bsel','bTag', 'tausel']]
                verify_null_list = [ p[k]!=[] for k in ['prefilters','exotHypo'] ]
                if (
                    p['trkpresel']!='nopresel' or p['hypoScenario']!='simple'
                    or any(verify_null_str) or any(verify_null_list)
                    or p['smc']!='nosmc'
                    or ip>0
                ):
                    raise RuntimeError(f'Invalid jet \'perf\' chain "{self.chainName}": No additional selection is permitted!')
                self.isPerf = True
            l1th = p['L1threshold']
            if self.L1Threshold != '' and self.L1Threshold != l1th:
                raise RuntimeError('Cannot configure a jet chain with different L1 thresholds')
            self.L1Threshold = l1th
            # Verify that the preselection is defined only once
            if p["trkpresel"]!="nopresel" and ip+1!=len(jChainParts): # Last jet chainPart, presel should go here
                log.error("Likely inconsistency encountered in preselection specification for %s",self.chainName)
                raise RuntimeError("Preselection %s specified earlier than in the last chainPart!",p["trkpresel"])

        self.trkpresel = JetPresel.extractPreselection(self.dict)
        self.trkpresel_parsed_reco = {key:p[key] for key in ['recoAlg']} #Storing here the reco options from last chain part that we want to propagate to preselection (e.g. jet radius)

        self.recoDict = JetRecoCommon.extractRecoDict(jChainParts)
        self.jetName = None
        self.jetDefDict = None


    # ----------------------
    # Precompute the data dependencies i.e. jet collections to generate
    # ----------------------
    def prepareDataDependencies(self, flags):
        self.jetDefDict = JetRecoDataDeps(flags, **self.recoDict)
        jetsOut, jetDef = self.jetDefDict['final']
        self.jetName = recordable(jetDef.fullname())


    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):                            
        log.debug("Assembling chain %s", self.chainName)

        # --------------------
        # define here the names of the steps and obtain the chainStep configuration 
        # --------------------
        chainSteps = []
        if self.recoDict["ionopt"]=="ion":
            jetHICaloHypoStep = self.getJetHICaloHypoChainStep(flags)
            chainSteps.append( jetHICaloHypoStep )
        elif self.recoDict["trkopt"]=="roiftf":
            # Can't work w/o calo presel jets to seed RoIs
            if self.trkpresel=="nopresel":
                raise RuntimeError("RoI FTF jet tracking requested with no jet preselection to provide RoIs")
            # Set up calo preselection step first
            preselJetDef, jetPreselStep = self.getJetCaloPreselChainStep(flags)
            chainSteps.append( jetPreselStep )
            # Standard tracking step, configure the tracking instance differently

            jetRoITrackJetTagHypoStep = self.getJetRoITrackJetTagHypoChainStep(flags, preselJetDef)
            chainSteps.append( jetRoITrackJetTagHypoStep )
        elif self.recoDict["trkopt"]=="ftf":
            if self.trkpresel=="nopresel":
                # Passthrough calo reco -- only for prescaled chains due to CPU cost
                caloRecoStep = self.getJetCaloRecoChainStep(flags)
                chainSteps.append( caloRecoStep )
                #Add empty step to align with preselection step
                roitrkPreselStep = self.getEmptyStep('RoIFTFEmptyStep')
            else:
                # Add calo preselection step
                preselJetDef, jetPreselStep = self.getJetCaloPreselChainStep(flags)
                chainSteps.append( jetPreselStep )

                if re.match(r'.*(b\d\d|bg\d\d|bgtwo\d\d)|.*Z|.*gntau', self.trkpresel):
                    # Preselection with super-RoI tracking for b-tagging, tau-tagging or pileup tagging
                    roitrkPreselStep = self.getJetRoITrackJetTagPreselChainStep(flags, preselJetDef)
                else:
                    # Empty step for alignment if no roiftf preselection defined
                    roitrkPreselStep=self.getEmptyStep('RoIFTFEmptyStep')

            chainSteps.append(roitrkPreselStep)
            # Final selection with FS tracking
            jetFSTrackingHypoStep = self.getJetFSTrackingHypoChainStep(flags)
            chainSteps.append( jetFSTrackingHypoStep )
        else:
            # No special options, just EMTopo jet reconstruction going straight to final rejection
            jetCaloHypoStep = self.getJetCaloHypoChainStep(flags)
            chainSteps.append( jetCaloHypoStep )

        # Add exotic jets hypo
        if self.exotHypo != '' and ("emerging" in self.exotHypo or "trackless" in self.exotHypo):
            EJsStep = self.getJetEJsChainStep(flags, self.jetName, self.exotHypo)
            chainSteps+= [EJsStep]
        elif self.exotHypo != '' and ("calratiovar" in self.exotHypo):
            CRVARStep = self.getJetCRVARChainStep(flags, self.jetName, self.exotHypo)
            chainSteps+= [ CRVARStep]
        elif self.exotHypo != '' and ("calratio" in self.exotHypo):
            CRStep = self.getJetCRChainStep(flags, self.jetName, self.exotHypo)
            chainSteps+= [self.getEmptyStep('RoIFTFEmptyStep'), CRStep]

        myChain = self.buildChain(chainSteps)

        return myChain
        

    # --------------------
    # Configuration of steps
    # --------------------

    # These ChainStep generators all pass information between steps
    def getJetCaloHypoChainStep(self, flags):
        stepName = f"MainStep_jet_{self.recoDict['jetDefStr']}"

        if self.isPerf:
            stepName += '_perf'

        jetStep = self.getStep(flags, stepName, [jetCaloHypoMenuSequenceGenCfg], isPerf=self.isPerf, **self.jetDefDict)
        return jetStep

    def getJetHICaloHypoChainStep(self, flags):
        stepName = f"MainStep_HIjet_{self.recoDict['jetDefStr']}"
        if self.isPerf:
            stepName += '_perf'

        jetStep = self.getStep(
            flags, stepName, [jetHICaloHypoMenuSequenceGenCfg],
            isPerf=self.isPerf, **self.recoDict,
        )
        return jetStep

    def getJetFSTrackingHypoChainStep(self, flags):
        stepName = "MainStep_jet_"+self.recoDict['jetDefStr']
        if self.isPerf:
            stepName += '_perf'

        jetStep = self.getStep(
            flags, stepName, [jetFSTrackingHypoMenuSequenceGenCfg],
            isPerf=self.isPerf,
            **self.jetDefDict
        )
        return jetStep

    def getJetCaloRecoChainStep(self, flags):
        clusterCalib = self.recoDict["clusterCalib"]
        stepName = "CaloRecoPTStep_jet_"+clusterCalib

        return self.getStep(flags, stepName, [jetCaloRecoMenuSequenceGenCfg], clusterCalib=clusterCalib)

    def getJetCaloPreselChainStep(self, flags):

        #Find if a a4 or a10 calo jet needs to be used in the pre-selection from the last chain dict
        assert 'recoAlg' in self.trkpresel_parsed_reco.keys(), "Impossible to find \'recoAlg\' key in last chain dictionary for preselection"
        #Want to match now only a4 and a10 in the original reco algorithm. We don't want to use a10sd or a10t in the preselection
        matched_reco = re.match(r'^a\d?\d?', self.trkpresel_parsed_reco['recoAlg'])
        assert matched_reco is not None, "Impossible to get matched reco algorithm for jet trigger preselection The reco expression {0} seems to be impossible to be parsed.".format(self.trkpresel_parsed_reco['recoAlg'])

        #Getting the outcome of the regex reco option (it should correspond to a4 or a10 depending by which chain you are configuring)
        preselRecoDict = JetPresel.getPreselRecoDict(matched_reco.group())

        preselJetDefDict = JetRecoDataDeps(flags, **preselRecoDict)
        preselJetsOut, preselJetDef = preselJetDefDict["final"]

        stepName = "PreselStep_jet_"+preselRecoDict['jetDefStr']
        jetStep = self.getStep(flags, stepName, [jetCaloPreselMenuSequenceGenCfg], **preselJetDefDict)

        return preselJetDef, jetStep

    # From here, only return ChainStep, no passing around of data dependencies
    def getJetRoITrackJetTagHypoChainStep(self, flags, preselJetDef):
        stepName = "RoIFTFStep_jet_sel_"+self.recoDict['jetDefStr']
        return self.getStep(flags, stepName, [jetRoITrackJetTagHypoMenuSequenceGenCfg], jetDef=preselJetDef, isPresel=False)

    def getJetRoITrackJetTagPreselChainStep(self, flags, jetDef):

        #Find if a a4 or a10 calo jet needs to be used in the pre-selection from the last chain dict
        assert 'recoAlg' in self.trkpresel_parsed_reco.keys(), "Impossible to find \'recoAlg\' key in last chain dictionary for preselection"
        #Want to match now only a4 and a10 in the original reco algorithm. We don't want to use a10sd or a10t in the preselection
        matched_reco = re.match(
            r'^a\d?\d?', self.trkpresel_parsed_reco['recoAlg'])
        assert matched_reco is not None, "Impossible to get matched reco algorithm for jet trigger preselection The reco expression {0} seems to be impossible to be parsed.".format(
            self.trkpresel_parsed_reco['recoAlg'])

        #Getting the outcome of the regex reco option (it should correspond to a4 or a10 depending by which chain you are configuring)
        preselRecoDict = JetPresel.getPreselRecoDict(matched_reco.group(),roiftf=True)

        assert preselRecoDict['trkopt'] == 'roiftf', 'getJetRoITrackJetTagPreselChainStep: you requested a RoI tracking preselection but the reco dictionary has \'trkopt\' set to {0}'.format(preselRecoDict['trkopt'])

        stepName = "RoIFTFStep_jet_"+self.recoDict['jetDefStr']
        return self.getStep(flags, stepName, [jetRoITrackJetTagHypoMenuSequenceGenCfg], jetDef=jetDef, isPresel=True)

    def getJetEJsChainStep(self, flags, jetCollectionName, exotdictstring):

        # Must be configured similar to : emergingPTF0p0dR1p2 or tracklessdR1p2
        if 'emerging' in exotdictstring and ('dR' not in exotdictstring \
           or 'PTF' not in exotdictstring):
            log.error('Misconfiguration of exotic jet chain - need dR and PTF options')
            exit(1)
        if 'trackless' in exotdictstring and 'dR' not in exotdictstring:
            log.error('Misconfiguration of trackless exotic jet chain - need dR option')
            exit(1)

        trackless = int(0)
        if 'emerging' in exotdictstring:
            ptf = float(exotdictstring.split('PTF')[1].split('dR')[0].replace('p', '.'))
            dr  = float(exotdictstring.split('dR')[1].split('_')[0].replace('p', '.'))
        elif 'trackless' in exotdictstring:
            trackless = int(1)
            ptf = 0.0
            dr = float(exotdictstring.split('dR')[1].split('_')[0].replace('p', '.'))
        else:
            log.error('Misconfiguration of trackless exotic jet chain - need emerging or trackless selection')
            exit(1)

        log.debug("Running exotic jets with ptf: " + str(ptf) + "\tdR: " + str(dr) + "\ttrackless: " + str(trackless) + "\thypo: " + exotdictstring)

        stepName = "EJsStep"

        return self.getStep(flags, stepName, [jetEJsMenuSequenceGenCfg], jetsIn=jetCollectionName)

    def getJetCRVARChainStep(self, flags, jetCollectionName, exotdictstring):
        
        if 'calratiovar' in exotdictstring:
            MinjetlogR = 1.2
            if 'calratiovarrmbib' in exotdictstring:
               doBIBremoval = int(1)
            else:
               doBIBremoval = int(0)
        else:
            log.error('Misconfiguration of trackless exotic jet chain - need calratiovar selection')
            exit(1)

        log.debug("Running exotic jets with MinjetlogR: " + str(MinjetlogR) + "\t BIB rm " + str(doBIBremoval) + "\thypo: " + exotdictstring)

        return self.getStep(flags, "CRVARStep", [jetCRVARMenuSequenceGenCfg], jetsIn=jetCollectionName)

    def getJetCRChainStep(self, flags, jetCollectionName, exotdictstring):
        
        if 'calratio' in exotdictstring  and  ('calratiovar' not in exotdictstring):
            MinjetlogR = 1.2
            if 'calratiormbib' in exotdictstring:
               doBIBremoval = int(1)
            else:
               doBIBremoval = int(0)
        else:
            log.error('Misconfiguration of trackless exotic jet chain - need calratio selection')
            exit(1)

        log.debug("Running exotic jets with MinjetlogR: " + str(MinjetlogR) + "\t BIB rm " + str(doBIBremoval) + "\thypo: " + exotdictstring)

        return self.getStep(flags, "CRStep", [jetCRMenuSequenceGenCfg], jetsIn=jetCollectionName)

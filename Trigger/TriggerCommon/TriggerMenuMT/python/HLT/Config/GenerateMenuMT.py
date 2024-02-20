# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import importlib
import string

from TriggerMenuMT.HLT.Config.ControlFlow.HLTCFTools import NoCAmigration
from TriggerMenuMT.HLT.Config.Utility.HLTMenuConfig import HLTMenuConfig

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

_isCAMenu = True
def isCAMenu():
  return _isCAMenu


def calibCosmicMonSignatures():
    return ['Streaming','Monitor','Beamspot','Cosmic', 'Calib', 'EnhancedBias']

def combinedSignatures():
    return ['MinBias','Electron','Photon','Muon','Tau','Jet', 'Bjet','MET','UnconventionalTracking','HeavyIon']

def jointSignatures():
    return ['Bjet', 'Egamma', 'Combined']

def defaultSignatures():
    return ['Streaming']

def testSignatures():
    return ['Test']

def bphysicsSignatures():
    return ['Bphysics']

def allSignatures():
    return set(calibCosmicMonSignatures() + combinedSignatures() + jointSignatures() + bphysicsSignatures() + defaultSignatures() + testSignatures())

class Singleton(type):
    _instances = {}
    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            cls._instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)
        return cls._instances[cls]

class FilterChainsToGenerate(object):
    """
    class to use filters for chains
    """
    def __init__(self,flags):
        self.enabledSignatures  = flags.Trigger.enabledSignatures  if flags.hasFlag("Trigger.enabledSignatures") else []
        self.disabledSignatures = flags.Trigger.disabledSignatures if flags.hasFlag("Trigger.disabledSignatures") else []
        self.selectChains       = flags.Trigger.selectChains       if flags.hasFlag("Trigger.selectChains") else []
        self.disableChains      = flags.Trigger.disableChains      if flags.hasFlag("Trigger.disableChains") else []          
    def __call__(self, signame, chain):            
        return ((signame in self.enabledSignatures and signame not in self.disabledSignatures) and \
            (not self.selectChains or chain in self.selectChains) and chain not in self.disableChains)
  

# for now we make this a singleton because calling menu generation twice leads to problems
class GenerateMenuMT(object, metaclass=Singleton):

    # Applicable to all menu instances
    calibCosmicMonSigs = calibCosmicMonSignatures()
    combinedSigs = combinedSignatures()
    defaultSigs = defaultSignatures()  # for noalg chains

    # Define which signatures (folders) are required for each slice
    def getRequiredSignatures(theslice):
        allSigs = allSignatures()
        signatureDeps = {sig:[sig] for sig in allSigs}
        # Special cases
        signatureDeps.update({
            # Bjet always requires jet
            'Bjet': ['Bjet','Jet'],
            # Egamma contains two signatures
            'Egamma': ['Electron','Photon'],
            'Combined': GenerateMenuMT.combinedSigs,
        })
        return set(signatureDeps[theslice]+GenerateMenuMT.defaultSigs) # always allow streamers

    def __init__(self):
        self.chainsInMenu = {}  # signature : [chains]

        self.allChainsForAlignment = []
        self.chainDicts = []
        self.combinationsInMenu = []
        self.alignmentGroupsToAlign = set()
        self.configLengthDict = {}
        
        self.signaturesOverwritten = False
        self.L1Prescales = None
        self.HLTPrescales = None

        self.chainFilter = None
        self.availableSignatures = []

        self.sigDicts = {}

        self.chainDefModule = {}   # Generate[SIG]ChainDefs module for each SIGnature

    def setChainFilter(self, f):
        """Set chain filter for menu generation.

           This can be any callable object taking two
           arguments for signature and chain name and returning a boolean.
           E.g. to only generate Egamma chains:
                menu.setChainFilter(lambda slice,chain : slice=='Egamma').

           In the special case that f is a functor with the list attributes
           selectChains and/or disableChains, the contents will be explicitly
           checked to be in the menu.
        """
        fname = f.__class__.__name__ if isinstance(f,object) else f.__name__
        import inspect
        if len(inspect.signature(f).parameters)!=2:
            log.error('%s is not a valid chain filter. Function/callable needs take two arguments '
                      'for signature and chain name and return a boolean', fname)
        else:
            log.warning('Setting chain filter to %s', fname)
            self.chainFilter = f


    def getChainDicts(self, flags):

        def validSignature(currentSig, chainSig):
            """Check if chain is assigned to the correct signature"""
            reqd = GenerateMenuMT.getRequiredSignatures(currentSig)
            isValid = chainSig.issubset( reqd )
            log.debug("Chain signatures: %s, required signatures: %s",chainSig,reqd)
            if not isValid:
                log.error("Chain signatures %s not a subset of required signatures %s",set(chainSig),reqd)
            return isValid

        from TriggerMenuMT.HLT.Config.Utility.DictFromChainName import dictFromChainName

        chainCounter = 0
        invalid = False
        for sig, chains in self.chainsInMenu.items():
            for chain in chains:
                log.debug("Now processing chain: %s from signature %s", chain, sig)
                chainCounter += 1
                chainDict = dictFromChainName(flags, chain)
                chainDict['chainCounter'] = chainCounter
                chainDict['prescale'] = 1  # set default chain prescale

                # Pick out the folder and subsignature directories to import
                for sigfo, subsig in chainDict['sigDicts'].items():
                    if sigfo not in self.sigDicts:
                        self.sigDicts[sigfo] = subsig
                    else:
                        for ss in subsig:
                            if ss not in self.sigDicts[sigfo]:
                                self.sigDicts[sigfo].append(ss)

                self.chainDicts.append(chainDict)

                if not validSignature(sig, set(chainDict['signatures'])):
                    invalid=True
                    log.error('Chain %s assigned to signature %s but creates %s',
                              chainDict['chainName'], sig, set(chainDict['signatures']))
        if invalid:
            raise RuntimeError('Incorrect assignment of chains to slices -- see preceding messages.')

    def importSignaturesToGenerate(self):
        """check if all the signature files can be imported and then import them"""

        for sig, subSigs in self.sigDicts.items():
            try:
                for ss in subSigs:
                    import_module = 'TriggerMenuMT.HLT.' + sig +'.Generate' + ss + 'ChainDefs'
                    self.chainDefModule[ss] = importlib.import_module(import_module)

                    if ss not in self.availableSignatures:
                        self.availableSignatures.append(ss)

            except ImportError:
                log.exception('Problems when importing ChainDef generating code for %s', sig)
                import traceback
                traceback.print_exc()

        log.info('Available signature(s) for chain generation: %s', self.availableSignatures)

        return

    def generateChains(self, flags):       
        all_chains = []
        combinations_in_menu = []
        alignmentGroups_to_align = set()
        length_of_configs = {}
        
        for chainDict in self.chainDicts:
            log.debug("Next: getting chain configuration for chain %s ", chainDict['chainName'])
            chainConfig,lengthOfChainConfigs = self.__generateChainConfig(flags, chainDict)
            if isCAMenu():
                # skip chain generation if no ChainConfig was found
                if chainConfig is None:
                    continue
                log.debug("Found CA configuration for Chain %r",chainConfig)
                
            all_chains += [(chainDict,chainConfig,lengthOfChainConfigs)]
            
            
            #update the alignment group length dictionary if we have a longer number of steps
            #or the signature isn't registered in the dictionary yet
            for config_length, config_grp in lengthOfChainConfigs:
                if config_grp in length_of_configs:
                    if config_length > length_of_configs[config_grp]:
                        length_of_configs[config_grp] = config_length
                else:
                    length_of_configs[config_grp] = config_length

            # find the chains that contain more than one alignment group, to keep track
            # of what combinations do we need to deal with.
            # we're using sets here so we don't end up with duplicates
            if len(set(chainDict['alignmentGroups'])) > 1:  
                combinations_in_menu += [list(set(chainDict['alignmentGroups']))]
                for align_group in list(set(chainDict['alignmentGroups'])):
                    alignmentGroups_to_align.update([align_group])
                    
        self.allChainsForAlignment = all_chains
        self.combinationsInMenu = combinations_in_menu
        self.alignmentGroupsToAlign = alignmentGroups_to_align
        self.configLengthDict = length_of_configs

        return 


    def generateAllChainConfigs(self, flags):
        """
        == Obtains chain configs for all chains in menu
        """

        from TriggerMenuMT.HLT.Config.Utility.MenuAlignmentTools import MenuAlignment
        from TriggerMenuMT.HLT.CommonSequences import EventBuildingSequences, TLABuildingSequences

        # get all chain names from menu
        log.info("Will now get chains from the menu")
        self.getChainsFromMenu(flags)

        # decoding of the chain name
        log.info("Will now get chain dictionaries for each chain")
        self.getChainDicts(flags)
        
        if flags.Trigger.disableCPS:
            log.warning('Removing all CPS group because the flag Trigger.disableCPS is set')
            for chainDict in self.chainDicts:
                chainDict['groups'] = [g for g in chainDict['groups'] if not g.startswith('RATE:CPS_')]

        #import the necessary signatures
        log.info("Importing the necessary signatures")
        self.importSignaturesToGenerate()

        log.info("Will now generate the chain configuration for each chain")
        self.generateChains(flags)

        log.info("Will now calculate the alignment parameters")
        #dict of signature: set it belongs to
        #e.g. {'Electron': ['Electron','Muon','Photon']}        
        menuAlignment = MenuAlignment(self.combinationsInMenu,
                                      self.alignmentGroupsToAlign,
                                      self.configLengthDict)
        menuAlignment.analyse_combinations()

        # alignmentGroups_to_align = menuAlignment.groupsToAlign
        # lengthOfChainConfigs = self.configLengthDict
        # combinationsInMenu = menuAlignment.combinationsInMenu
        # alignmentGroup_sets_to_align = menuAlignment.setsToAlign

        log.info('Aligning the following signatures: %s',sorted(menuAlignment.sets_to_align))
        log.debug('Length of each of the alignment groups: %s',self.configLengthDict)


        for chainDict,chainConfig,lengthOfChainConfigs in self.allChainsForAlignment:

              # start by ordering electron, photon, muon by having e+mu, g+mu, e+g chains
              # desired ordering: electron, photon, muon, tau, jet, met, b-jet
              
              # lengthOfChainConfigs is something like this: [(4, 'Photon'), (5, 'Muon')]
              # needs to match up with the maximum number of steps in a signature in the menu (length_of_configs)
              # start with electron! Only need to add post-steps for combined electron chains if the max length in a combined chain
              # is greater than the number of electron steps combined chain. Assume that the max length of an electron chain occurs 
              # in a combined chain.

              log.debug("[generateAllChainConfigs] chain %s has config lengths %s and alignment groups %s", chainDict['chainName'], lengthOfChainConfigs, chainDict['alignmentGroups'])
 
              alignmentGroups = chainDict['alignmentGroups']
            
              #parallel-merged single-signature chains or single signature chains. Anything that needs no splitting!
              if len(set(alignmentGroups)) == 1: 
                  alignedChainConfig = menuAlignment.single_align(chainDict, chainConfig)
                  HLTMenuConfig.registerChain( chainDict, alignedChainConfig )

              elif len(alignmentGroups) >= 2:
                  alignedChainConfig = menuAlignment.multi_align(chainDict, chainConfig, lengthOfChainConfigs)

                  HLTMenuConfig.registerChain( chainDict, alignedChainConfig )              

              else: 
                  log.error("Menu can't deal with combined chains with more than two alignmentGroups at the moment. oops...")
                  raise NotImplementedError("more than three alignment groups still needs implementing in ChainMerging.py, ATR-22206")

              if not HLTMenuConfig.isChainRegistered(chainDict['chainName']):
                log.error("Chain %s has not been registered in the menu!", chainDict['chainName'])
                import pprint
                pp = pprint.PrettyPrinter(indent=4, depth=8)
                log.error('The chain dictionary is: %s', pp.pformat(chainDict))
                raise Exception("Please fix the menu or the chain.")
        
        
        
        # align event building sequences
        log.info("[generateAllChainConfigs] general alignment complete, will now align TLA chains")
        TLABuildingSequences.alignTLASteps(HLTMenuConfig.configs(), HLTMenuConfig.dicts())    
        log.info("[generateAllChainConfigs] general and TLA alignment complete, will now align PEB chains")
        EventBuildingSequences.alignEventBuildingSteps(HLTMenuConfig.configs(), HLTMenuConfig.dicts())
        
         
        log.info("[generateAllChainConfigs] all chain configurations have been generated.")
        return HLTMenuConfig.configsList()


    def getChainsFromMenu(self, flags):
        """
        == Returns the list of chain names that are in the menu
        """       
        from TriggerMenuMT.HLT.Menu.MenuPrescaleConfig import MenuPrescaleConfig

        # go over the slices and put together big list of signatures requested
        (self.L1Prescales, self.HLTPrescales, self.chainsInMenu) = MenuPrescaleConfig(HLTMenuConfig, flags)

        log.debug("Setup HLT menu with prescales: %s", self.HLTPrescales)

        # Filter chains if requested
        if self.chainFilter is not None:
            self.signaturesOverwritten = True

            # Verify that if the chain filter has lists of chains
            # they are all in the menu
            chainsToCheck = []
            if hasattr(self.chainFilter,'selectChains'):
                chainsToCheck += self.chainFilter.selectChains
            if hasattr(self.chainFilter,'disableChains'):
                chainsToCheck += self.chainFilter.disableChains
            for chain in chainsToCheck:
                inMenu = False
                for signame in self.chainsInMenu:
                    if chain in [c.name for c in self.chainsInMenu[signame]]:
                        inMenu = True
                        break
                if not inMenu:
                    raise RuntimeError(f'Request to enable/disable chain {chain} that is not in menu')

            for signame in self.chainsInMenu:
                self.chainsInMenu[signame] = [c for c in self.chainsInMenu[signame]
                                              if self.chainFilter(signame, c.name)]

        if not self.chainsInMenu:
            log.warning("There seem to be no chains in the menu - please check")
        elif log.isEnabledFor(logging.DEBUG):
            import pprint
            log.debug("The following chains were found in the menu:")
            pprint.pprint(self.chainsInMenu)


    def __generateChainConfig(self, flags, mainChainDict):
        """
        # Assembles the chain configuration and returns a chain object with (name, L1see and list of ChainSteps)
        """

        from TriggerMenuMT.HLT.Config.Utility.ChainDictTools import splitInterSignatureChainDict
        from TriggerMenuMT.HLT.Config.Utility.ComboHypoHandling import addTopoInfo, comboConfigurator, topoLegIndices
        from TriggerMenuMT.HLT.Config.Utility.ChainMerging import mergeChainDefs
        from TriggerMenuMT.HLT.CommonSequences import EventBuildingSequences, TLABuildingSequences

        # split the the chainDictionaries for each chain and print them in a pretty way
        chainDicts = splitInterSignatureChainDict(mainChainDict)

        if log.isEnabledFor(logging.DEBUG):
            import pprint
            pp = pprint.PrettyPrinter(indent=4, depth=8)
            log.debug('dictionary is: %s', pp.pformat(chainDicts))

        # Loop over all chainDicts and send them off to their respective assembly code
        listOfChainConfigs = []
        perSig_lengthOfChainConfigs = []
        not_migrated=False

        for chainPartDict in chainDicts:
            chainPartConfig = None
            currentSig = chainPartDict['signature']
            currentAlignGroup = None
            if len(chainPartDict['chainParts']) == 1:
                currentAlignGroup = chainPartDict['chainParts'][0]['alignmentGroup']
            
            chainName = chainPartDict['chainName']
            log.debug('Checking chainDict for chain %s in signature %s, alignment group %s' , chainName, currentSig, currentAlignGroup)

            if currentSig in self.availableSignatures:
                try:
                    log.debug("[__generateChainConfigs] Trying to get chain config for %s", currentSig)
                    if currentSig in ['Electron', 'Photon', 'Muon', 'Tau', 'Bphysics'] :
                        chainPartConfig, perSig_lengthOfChainConfigs = self.chainDefModule[currentSig].generateChainConfigs(flags, chainPartDict, perSig_lengthOfChainConfigs)
                    else:
                        chainPartConfig = self.chainDefModule[currentSig].generateChainConfigs(flags, chainPartDict)
                        if currentSig == 'Test' and isinstance(chainPartConfig, tuple):
                            chainPartConfig = chainPartConfig[0]
                except Exception:
                    log.error('[__generateChainConfigs] Problems creating ChainDef for chain %s ', chainName)
                    log.error('[__generateChainConfigs] I am in chain part\n %s ', chainPartDict)
                    log.exception('[__generateChainConfigs] Full chain dictionary is\n %s ', mainChainDict)
                    raise Exception('[__generateChainConfigs] Stopping menu generation. Please investigate the exception shown above.')
            else:
                log.error('Chain %s cannot be generated - Signature "%s" not available', chainPartDict['chainName'], currentSig)
                log.error('Available signature(s): %s', self.availableSignatures)
                raise Exception('Stopping the execution. Please correct the configuration.')

            log.debug("Chain %s \n chain config: %s",chainPartDict['chainName'],chainPartConfig)
            import itertools   
            
            # check if there are not migrated steps between migrated ones
            # if built-up steps are not consecutive, do not build the chain because it's incomplete
            leg_not_migrated = ( (chainPartConfig is None ) or  \
                len([k for k, g in itertools.groupby(["_MissingCA" in step.name for step in chainPartConfig.steps]) if k==0])!=1) \
                and 'noalg' not in chainPartDict['chainName'] # no alg chains should be excluded as they do not have any configureation

            not_migrated |= leg_not_migrated

            if isCAMenu() and leg_not_migrated:
                if chainPartConfig is None:
                    log.debug(str(NoCAmigration("[__generateChainConfigs] Chain {0} chainPartConfig is None, because of failure of merging chains".format(chainPartDict['chainName'])) ))                    
                else:
                    listOfChainConfigs.append(chainPartConfig)                                        
            else:
                listOfChainConfigs.append(chainPartConfig)
                log.debug("[__generateChainConfigs] adding to the perSig_lengthOfChainConfigs list (%s, %s)",chainPartConfig.nSteps,chainPartConfig.alignmentGroups)
                perSig_lengthOfChainConfigs.append((chainPartConfig.nSteps,chainPartConfig.alignmentGroups))
                
        # this will be a list of lists for inter-sig combined chains and a list with one 
        # multi-element list for intra-sig combined chains
        # here, we flatten it accordingly (works for both cases!)
        lengthOfChainConfigs = []
        if isCAMenu() and not_migrated: 
            log.debug(str(NoCAmigration("[__generateChainConfigs] Chain {0} removed because is incomplete".format(chainPartDict['chainName'])) ))                           
        else:        
            for nSteps, aGrps in perSig_lengthOfChainConfigs:
                if len(nSteps) != len(aGrps):
                    log.error("Chain part has %s steps and %s alignment groups - these don't match!",nSteps,aGrps)
                else:
                    for a,b in zip(nSteps,aGrps):
                        lengthOfChainConfigs.append((a,b))
            
       
        # This part is to deal with combined chains between different signatures
        try:
            if isCAMenu() and not_migrated:
                raise NoCAmigration("[__generateChainConfigs] chain {0} generation missed configuration".format(mainChainDict['chainName']))                               

            if len(listOfChainConfigs) == 0:
                raise Exception('[__generateChainConfigs] No Chain Configuration found for {0}'.format(mainChainDict['chainName']))                    
            else:
                if len(listOfChainConfigs)>1:
                    log.debug("Merging strategy from dictionary: %s", mainChainDict["mergingStrategy"])
                    theChainConfig, perSig_lengthOfChainConfigs = mergeChainDefs(listOfChainConfigs, mainChainDict, perSig_lengthOfChainConfigs)
                    if isCAMenu() and perSig_lengthOfChainConfigs is None:
                       raise NoCAmigration("[__generateChainConfigs] chain {0} generation missed configuration during merging".format(mainChainDict['chainName']))
                    lengthOfChainConfigs = [] 
                    for nSteps, aGrps in perSig_lengthOfChainConfigs:
                        if len(nSteps) != len(aGrps):
                            log.error("Post-merged chain part has %s steps and %s alignment groups - these don't match!",nSteps,aGrps)
                        else:
                            for a,b in zip(nSteps,aGrps):
                                lengthOfChainConfigs.append((a,b))
                else:
                    theChainConfig = listOfChainConfigs[0]
                
                for topoID in range(len(mainChainDict['extraComboHypos'])):
                    thetopo = mainChainDict['extraComboHypos'][topoID].strip(string.digits).rstrip(topoLegIndices)                    
                    theChainConfig.addTopo((comboConfigurator[thetopo],thetopo))
                                    
                # Now we know where the topos should go, we can insert them in the right steps
                if len(theChainConfig.topoMap) > 0:                    
                    log.debug("Trying to add extra ComboHypoTool for %s",mainChainDict['extraComboHypos'])
                    addTopoInfo(theChainConfig,mainChainDict,listOfChainConfigs,lengthOfChainConfigs)
        except RuntimeError:
            log.error('[__generateChainConfigs] Problems creating ChainDef for chain %s ', chainName)
            log.error('[__generateChainConfigs] I am in the extraComboHypos section, for %s ', mainChainDict['extraComboHypos'])
            log.exception('[__generateChainConfigs] Full chain dictionary is\n %s ', mainChainDict)
            raise Exception('[__generateChainConfigs] Stopping menu generation. Please investigate the exception shown above.')
        except AttributeError:                    
            if isCAMenu():
                log.warning(str(NoCAmigration("[__generateChainConfigs] addTopoInfo failed with CA configurables") )  )  
                return None,[]                       
            raise Exception('[__generateChainConfigs] Stopping menu generation. Please investigate the exception shown above.')
        except NoCAmigration as e:
            log.warning(str(e))
            # flag as merged all CAs created , but not used
            [seq.ca.wasMerged() for chainPartConfig in listOfChainConfigs for step in chainPartConfig.steps for seq in step.sequences  ]                      
            return None,[]

        # Configure event building strategy
        eventBuildType = mainChainDict['eventBuildType']
        if eventBuildType:
            try:
                if 'PhysicsTLA' in eventBuildType:
                    log.debug("Adding TLA Step for chain %s", mainChainDict['chainName'])
                    TLABuildingSequences.addTLAStep(flags, theChainConfig, mainChainDict)
                log.debug('Configuring event building sequence %s for chain %s', eventBuildType, mainChainDict['chainName'])
                EventBuildingSequences.addEventBuildingSequence(flags, theChainConfig, eventBuildType, mainChainDict)
            except TypeError as ex:
                    log.error(ex)
                    raise Exception('[__generateChainConfigs] Stopping menu generation for EventBuilding/TLA sequences. Please investigate the exception shown above.')

        log.debug('[__generateChainConfigs] lengthOfChainConfigs %s, ChainConfigs  %s ', lengthOfChainConfigs, theChainConfig)
        return theChainConfig,lengthOfChainConfigs
 
    
    def resolveEmptySteps(self,chainConfigs):
        max_steps = max([len(cc.steps) for cc in chainConfigs], default=0)    
        steps_are_empty = [True for i in range(0,max_steps)]
        emptySteps = []
        for cc in chainConfigs:
            for istep, the_step in enumerate(cc.steps):
                if not the_step.isEmpty:
                    steps_are_empty[istep] = False
                else: 
                    emptySteps.append(the_step)
                            
        log.debug("Are there any fully empty steps? %s", steps_are_empty)
        log.debug("The empty step(s) and associated chain(s) are: %s", emptySteps)
        empty_step_indices = [i for i,is_empty in enumerate(steps_are_empty) if is_empty]
        
        if len(empty_step_indices) == 0:
            return chainConfigs
        
        special_test_menu = self.chainFilter and (  getattr(self.chainFilter, "selectChains", False) or \
                                                    getattr(self.chainFilter, "disableChains", False) or \
                                                    getattr(self.chainFilter, "disabledSignatures", False) or \
                                                    getattr(self.chainFilter, "enabledSignatures", False) )
                                                
        
        if len(self.availableSignatures) != 1 and not special_test_menu:                
            raise Exception("[resolveEmptySteps] Please find the reason for this empty step and resolve it / remove it from the menu: %s", emptySteps)  

        log.info("Will now delete steps %s (indexed from zero)",empty_step_indices)
        
        for cc in chainConfigs:
            new_steps = []
            #only add non-empty steps to the new steps list!
            for istep,step in enumerate(cc.steps):
                if istep not in empty_step_indices:
                    new_steps += [step]
            cc.steps = new_steps

        return chainConfigs 
 

def generateMenuMT(flags): 
    """
    == Main function to generates L1, L1Topo and HLT menu CA, using class GenerateMenuMT
    """         
    # generate L1 menu
    # This probably will go to TriggerConfig.triggerRunCfg
    from TrigConfigSvc.TrigConfigSvcCfg import generateL1Menu, createL1PrescalesFileFromMenu
    from TriggerMenuMT.HLT.Menu.MenuPrescaleConfig import MenuPrescaleConfig
    generateL1Menu(flags)
    createL1PrescalesFileFromMenu(flags)

    # Generate the menu, stolen from HLT_standalone
    from TriggerMenuMT.HLT.Config.GenerateMenuMT import GenerateMenuMT
    menu = GenerateMenuMT() 

    chainsToGenerate = FilterChainsToGenerate(flags)
    menu.setChainFilter(chainsToGenerate)
    log.debug("Filtering chains = %d", (menu.chainFilter is not None))
    finalListOfChainConfigs = menu.generateAllChainConfigs(flags)
    log.info("Length of FinalListOfChainConfigs %s", len(finalListOfChainConfigs))

    # Add prescales for disabling items (e.g. MC production)
    log.info("Applying HLT prescales")
    
    (menu.L1Prescales, menu.HLTPrescales, menu.chainsInMenu) = MenuPrescaleConfig(HLTMenuConfig, flags)
    from TriggerMenuMT.HLT.Menu.MenuPrescaleConfig import applyHLTPrescale
    applyHLTPrescale(HLTMenuConfig, menu.HLTPrescales, menu.signaturesOverwritten)
 
    # make sure that we didn't generate any steps that are fully empty in all chains
    # if there are empty steps, remove them
    finalListOfChainConfigs = menu.resolveEmptySteps(finalListOfChainConfigs)

    log.debug("finalListOfChainConfig %s", finalListOfChainConfigs)
    log.info("Making the HLT configuration tree")
    menuAcc=makeHLTTree(flags)

    # Configure ChainFilters for ROBPrefetching
    from TriggerJobOpts.TriggerConfigFlags import ROBPrefetching
    if ROBPrefetching.InitialRoI in flags.Trigger.ROBPrefetchingOptions:
        from TrigGenericAlgs.TrigGenericAlgsConfig import prefetchingInitialRoIConfig
        menuAcc.merge( prefetchingInitialRoIConfig(flags, HLTMenuConfig.configsList()), 'HLTBeginSeq')

    log.info("Checking the L1HLTConsistency...")
    from TriggerMenuMT.HLT.Config.Validation.CheckL1HLTConsistency import checkL1HLTConsistency
    checkL1HLTConsistency(flags)

    log.info("Checking the Coherent Prescale assignments...")
    from TriggerMenuMT.HLT.Config.Validation.CheckCPSGroups import checkCPSGroups
    checkCPSGroups(HLTMenuConfig.dictsList())


    return menuAcc
    

def makeHLTTree(flags):
    """
    Generate appropriate Control Flow Graph wiht all HLT algorithms
    """
    from TriggerMenuMT.HLT.Config.ControlFlow.HLTCFConfig import decisionTreeFromChains, sequenceScanner
    from TriggerJobOpts.TriggerConfig import collectViewMakers
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaCommon.CFElements import compName 
    from AthenaCommon.CFElements import seqAND  

    acc = ComponentAccumulator()    
    steps = seqAND('HLTAllSteps')
    finalDecisions, menuAcc = decisionTreeFromChains(flags, steps, HLTMenuConfig.configsList(), HLTMenuConfig.dictsList())
    if log.getEffectiveLevel() <= logging.DEBUG:
        menuAcc.printConfig()

    acc.merge(menuAcc)
    successful_scan = sequenceScanner( steps )
    if not successful_scan:
        raise Exception("[makeHLTTree] At least one sequence is expected in more than one step. Check error messages and fix!")    

    flatDecisions=[]
    for step in finalDecisions:
        flatDecisions.extend (step)
 
    viewMakers = collectViewMakers(steps)
    viewMakerMap = {compName(vm):vm for vm in viewMakers}
    for vmname, vm in viewMakerMap.items():
        log.debug(f"[makeHLTTree] {vmname} InputMakerOutputDecisions: {vm.InputMakerOutputDecisions}")
        if vmname.endswith("_probe"):
            try:
                log.debug(f"Setting InputCachedViews on {vmname} to read decisions from tag leg {vmname[:-6]}: {vm.InputMakerOutputDecisions}")
                vm.InputCachedViews = viewMakerMap[vmname[:-6]].InputMakerOutputDecisions
            except KeyError: # We may be using a probe leg that has different reco from the tag
                log.debug(f"Tag leg does not match probe: '{vmname[:-6]}', will not use cached views")

    
    # generate JSON representation of the config
    from TriggerMenuMT.HLT.Config.JSON.HLTMenuJSON import generateJSON_newJO
    generateJSON_newJO(flags, HLTMenuConfig.dictsList(), HLTMenuConfig.configsList(), menuAcc.getSequence("HLTAllSteps"))

    from TriggerMenuMT.HLT.Config.JSON.HLTPrescaleJSON import generateJSON_newJO as generatePrescaleJSON_newJO
    generatePrescaleJSON_newJO(flags, HLTMenuConfig.dictsList(), HLTMenuConfig.configsList())

    from TriggerMenuMT.HLT.Config.JSON.HLTMonitoringJSON import generateDefaultMonitoringJSON_newJO
    generateDefaultMonitoringJSON_newJO(flags, HLTMenuConfig.dictsList())

    from AthenaCommon.CFElements import checkSequenceConsistency 
    checkSequenceConsistency(steps)
    return acc

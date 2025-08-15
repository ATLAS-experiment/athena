# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# ********************* All Tools/Functions for the TriggerEDM **********************
# Keeping all functions from the original TriggerEDM.py (Run 2 EDM) in this file
# with this name to not break backwards compatibility
# Where possible, functions will be adapted to also work with Run 3, they will then be moved
# to the Run 3 section
# ***********************************************************************************

from TrigEDMConfig.TriggerEDMRun1 import TriggerL2List,TriggerEFList,TriggerResultsRun1List
from TrigEDMConfig.TriggerEDMRun2 import TriggerResultsList,TriggerLvl1List,TriggerIDTruth,TriggerHLTList,EDMDetails,EDMLibraries,TriggerL2EvolutionList,TriggerEFEvolutionList
from TrigEDMConfig.TriggerEDMRun3 import TriggerHLTListRun3,varToRemoveFromAODSLIM,EDMDetailsRun3,getSafeEDMInsertPosition
from TrigEDMConfig.TriggerEDMRun4 import TriggerHLTListRun4
from TrigEDMConfig.TriggerEDMDefs import allowTruncation
from CLIDComps.clidGenerator import clidGenerator
from collections import defaultdict
import itertools
import re

from AthenaCommon.Logging import logging
log = logging.getLogger('TriggerEDM')

#************************************************************
#
#  For Run 3 and Run 4
#
#************************************************************

# ------------------------------------------------------------
# AllowedOutputFormats
# ------------------------------------------------------------
AllowedOutputFormats = ['BS', 'ESD', 'AODFULL', 'AODSLIM', 'AODBLSSLIM' ]
from TrigEDMConfig import DataScoutingInfo
AllowedOutputFormats.extend(DataScoutingInfo.getAllDataScoutingIdentifiers())

_allowedEDMPrefixes = ['HLT_', 'L1_', 'LVL1']
def recordable( arg, runVersion=3 ):
    """
    Verify that the name is in the list of recorded objects and conform to the name convention

    In Run 2 it was a delicate process to configure correctly what got recorded
    as it had to be set in the algorithm that produced it as well in the TriggerEDM.py in a consistent manner.

    For Run 3 every alg input/output key can be crosschecked against the list of objects to record which is defined here.
    I.e. in the configuration alg developer would do this:
    from TriggerEDM.TriggerEDMRun3 import recordable

    alg.outputKey = recordable("SomeKey")
    If the names are correct the outputKey is assigned with SomeKey, if there is a missmatch an exception is thrown.

    """

    # Allow passing DataHandle as argument - convert to string and remove store name
    name = str(arg).replace('StoreGateSvc+','')

    if "HLTNav_" in name:
        log.error( "Don't call recordable({0}), or add any \"HLTNav_\" collection manually to the EDM. See:collectDecisionObjects.".format( name ) )
        pass
    else: #negative filtering
        if not any([name.startswith(p) for p in _allowedEDMPrefixes]):
            raise RuntimeError( f"The collection name {name} does not start with any of the allowed prefixes: {_allowedEDMPrefixes}" )
        if "Aux" in name and not name[-1] != ".":
            raise RuntimeError( f"The collection name {name} is Aux but the name does not end with the '.'" )

    if runVersion >= 3:
        for entry in TriggerHLTListRun3:
            if entry[0].split( "#" )[1] == name:
                return arg
        msg = "The collection name {0} is not declared to be stored by HLT. Add it to TriggerEDMRun3.py".format( name )
        log.error("ERROR in recordable() - see following stack trace.")
        raise RuntimeError( msg )

def _addExtraCollectionsToEDMList(edmList, extraList):
    """
    Extend edmList with extraList, keeping track whether a completely new
    collection is being added, or a dynamic variable is added to an existing collection, or new targets are added to an existing collection.
    The format of extraList is the same as those of TriggerHLTListRun3.
    """
    existing_collections = [(c[0].split("#")[1]).split(".")[0] for c in edmList]
    insert_idx = getSafeEDMInsertPosition(edmList)
    for item in extraList:
        colname = (item[0].split("#")[1]).split(".")[0]
        if colname not in existing_collections:
            # a new collection and its Aux container are inserted in front of 'allowTruncation' items.
            if 'Aux' in colname:
                edmList.insert(insert_idx+1,item)
            else:
                edmList.insert(insert_idx,item)
            log.info("added new item to Trigger EDM: {}".format(item))
        else:
            # Maybe extra dynamic variables or EDM targets are added
            isAux = "Aux." in item[0]
            # find the index of the existing item
            existing_item_nr = [i for i,s in enumerate(edmList) if colname == (s[0].split("#")[1]).split(".")[0]]
            if len(existing_item_nr) != 1:
                log.error("Found {} existing edm items corresponding to new item {}, but it must be exactly one!".format(len(existing_item_nr), item))
            existingItem = edmList[existing_item_nr[0]]
            if isAux:
                dynVars = (item[0].split("#")[1]).split(".")[1:]
                existing_dynVars = (existingItem[0].split("#")[1]).split(".")[1:]
                existing_dynVars.extend(dynVars)
                dynVars = list(dict.fromkeys(existing_dynVars))
                if '' in dynVars:
                    dynVars.remove('')
                newVars = '.'.join(dynVars)
            edmTargets = item[1].split(" ") if len(item) > 1 else []
            existing_edmTargets = existingItem[1].split(" ")
            edmTargets.extend(existing_edmTargets)
            edmTargets = list(dict.fromkeys(edmTargets))
            newTargets = " ".join(edmTargets)
            typename = item[0].split("#")[0]
            log.info("old item in Trigger EDM    : {}".format(existingItem))
            signature = existingItem[2] # NOT updated at the moment
            tags = existingItem[3] if len(existingItem) > 3 else None  # NOT updated at the moment
            edmList.pop(existing_item_nr[0])
            combName = typename + "#" + colname
            if isAux:
                combName += "." + newVars
            if tags:
                edmList.insert(existing_item_nr[0], (combName, newTargets, signature, tags))
            else:
                edmList.insert(existing_item_nr[0] , (combName, newTargets, signature))
            log.info("updated item in Trigger EDM: {}".format(edmList[existing_item_nr[0]]))

    if testEDMList(edmList, error_on_edmdetails=False):
        log.error("edmList contains inconsistencies!")

def getRawTriggerEDMList(flags, runVersion=-1):
    """
    The static EDM list does still need some light manipulation before it can be used commonly.
    Never import TriggerHLTListRun3 or TriggerHLTListRun4 directly, always fetch them via this function
    """
    if runVersion == -1:
        runVersion = flags.Trigger.EDMVersion

    if runVersion == 3:
        edmListCopy = TriggerHLTListRun3.copy()
    elif runVersion == 4:
        edmListCopy  = TriggerHLTListRun3.copy()
        log.debug("Removing duplicated item between TriggerHLTListRun3 and TriggerHLTListRun4.")
        for i in range(len(edmListCopy)-1, -1, -1): # Back iterate by index, we might be removing as we go
            for r4item in TriggerHLTListRun4:
                if edmListCopy[i][0] == r4item[0]:
                    del edmListCopy[i]
                    log.debug(f"- Dupe: {r4item} is removed from TriggerHLTListRun3 to be updated by TriggerHLTListRun4")
                    break
        lenPreMerge = len(edmListCopy)
        edmListCopy.extend(TriggerHLTListRun4)
        lenPostMerge = len(edmListCopy)
        log.info(f"Added TriggerHLTListRun4 to TriggerHLTListRun3. EDM entries {lenPreMerge} -> {lenPostMerge}")
    else:
        errMsg="ERROR the getRawTriggerEDMList function supports runs 3 and 4."
        log.error(errMsg)
        raise RuntimeError(errMsg)

    if flags and flags.Trigger.ExtraEDMList:
        log.info( "Adding extra collections to EDM %i: %s", runVersion, str(flags.Trigger.ExtraEDMList))
        _addExtraCollectionsToEDMList(edmListCopy, flags.Trigger.ExtraEDMList)

    return edmListCopy

def getTriggerEDMList(flags, key, runVersion=-1):
    """
    List (Literally Python dict) of trigger objects to be placed with flags:
    flags is the CA flag container
    key can be" 'ESD', 'AODSLIM', 'AODFULL'
    runVersion can be: '-1 (Auto-configure)', '1 (Run1)', '2 (Run2)', '3' (Run 3), '4' (Run 4)
    """
    # We allow for this to be overriden as Run1 bytestream actually need to request the Run2 EDM due to it additionally undergoing a transient xAOD migration. 
    if runVersion == -1:
        runVersion = flags.Trigger.EDMVersion

    if runVersion == 1:
        return _getTriggerRun1Run2ObjList(key, [TriggerL2List,TriggerEFList, TriggerResultsRun1List])

    elif runVersion == 2:
        edmList = _getTriggerRun1Run2ObjList(key, [TriggerHLTList, TriggerResultsList])
        return _getTriggerRun2EDMSlimList(key, edmList) if 'SLIM' in key else edmList

    elif runVersion >= 3:
        RawEDMList = getRawTriggerEDMList(flags=flags, runVersion=runVersion)

        if key not in AllowedOutputFormats: # AllowedOutputFormats is the entire list of output formats including ESD         
            log.warning('Output format: %s is not in list of allowed formats, please check!', key)
            return _getRun3TrigObjList(key, [RawEDMList])
            
        # this keeps only the dynamic variables that have been specified in TriggerEDMRun3
        Run3TrigEDM = {}
        Run3TrigEDMSLIM = {}

        if "AODFULL" in key: 
            Run3TrigEDM.update(_getRun3TrigEDMSlimList(key, RawEDMList))

        elif "AODSLIM" in key:
            # remove the variables that are defined in TriggerEDMRun3.varToRemoveFromAODSLIM from the containers
            
            # get all containers from list that are marked with AODSLIM
            if len(varToRemoveFromAODSLIM) == 0:
                Run3TrigEDM.update(_getRun3TrigEDMSlimList(key, RawEDMList))
                log.info("No decorations are listed to be removed from AODSLIM")
            else:
                Run3TrigEDMSLIM.update(_getRun3TrigEDMSlimList(key, RawEDMList))
                log.info("The following decorations are going to be removed from the listed collections in AODSLIM {}".format(varToRemoveFromAODSLIM))
                # Go through all container values and remove the variables to remove
                # Format of Run3TrigEDMSLIM is {'xAOD::Cont': ['coll1.varA.varB', 'coll2.varD',...],...} 
                for cont, values in Run3TrigEDMSLIM.items():
                    if (isinstance(values, list)):
                        newValues = []
                        for value in values:                       
                            newValue = value+'.'
                            coll = value.split('.')[0]
                            
                            varRemovedFlag = False
                            for myTuple in varToRemoveFromAODSLIM:
                                var = myTuple[0]
                                
                                if var in value and coll in myTuple:
                                    varRemovedFlag = True
                                    removeVar =  '.'+var+'.'
                                    newValue = newValue.replace(removeVar, '.')
                                    
                            if newValue[-1:] == '.':
                                newValue = newValue[:-1]

                            if varRemovedFlag is False: 
                                newValues.append(value)
                            elif varRemovedFlag is True:
                                newValues.append(newValue)                                        
                            else:
                                raise RuntimeError("Decoration removed but no new Value was available, not sure what to do...")

                        # Filling the Run3TrigEDM dictionary with the new set of values for each cont
                        Run3TrigEDM[cont] = newValues
                    else:
                        raise RuntimeError("Value in Run3TrigEDM dictionary is not a list")

        else: # ESD
            Run3TrigEDM.update(_getRun3TrigEDMSlimList(key, RawEDMList))

        log.debug('TriggerEDM for EDM set {} contains the following collections: {}'.format(key, Run3TrigEDM) ) 
        return Run3TrigEDM


    else:
        raise RuntimeError("Invalid runVersion=%s supplied to getTriggerEDMList" % runVersion)



def _getRun3TrigObjProducedInView(theKey, trigEDMList):
    """
    Run 3 only
    Finds a given key from within the trigEDMList.
    Returns true if this collection is produced inside EventViews
    (Hence, has the special viewIndex Aux decoration applied by steering)
    """
    from TrigEDMConfig.TriggerEDMRun3 import InViews

    return any(coll for coll in itertools.chain(*trigEDMList) if
               len(coll)>3 and theKey==coll[0].split('#')[1] and
               any(isinstance(v, InViews) for v in coll[3]))


def _handleRun3ViewContainers( el, HLTList ):
    if 'Aux.' in el:
        # Get equivalent non-aux string (fragile!!!)
        keyNoAux = el.split('.')[0].replace('Aux','')
        # Check if this interface container is produced inside a View
        inView = _getRun3TrigObjProducedInView(keyNoAux, [HLTList])
        if el.split('.')[1] == '':
            # Aux lists zero dynamic vars to save ...
            if inView:
                # ... but it was produced in a View, so we need to add the viewIndex dynamic aux
                return el.split('.')[0]+'.viewIndex'
            else:
                # ... and was not in a View, strip all dynamic
                return el.split('.')[0]+'.-'
        else:
            # Aux lists one or more dynamic vars to save ...
            if inView:
                # ... and was produced in a View, so add the viewIndex dynamic as well
                return el+'.viewIndex'
            else:
                # ... and was not produced in a View, keep user-supplied list
                return el
    else: # no Aux
        return el


def getRun3BSList(flags, keys):
    """
    The keys should contain BS and all the identifiers used for scouting.
    Returns list of tuples (typename#key, [keys], [properties]).
    """

    from TrigEDMConfig.TriggerEDMRun3 import persistent
    keys = set(keys[:])
    collections = []
    _HLTList = getRawTriggerEDMList(flags, 3)
    for definition in _HLTList:

        typename,collkey = definition[0].split("#")
        # normalise collection name and the key (decorations)
        typename = persistent(typename)
        collkey  = _handleRun3ViewContainers( collkey, _HLTList )
        destination = keys & set(definition[1].split())
        if len(destination) > 0:
            collections.append( (typename+"#"+collkey, list(destination),
                                 definition[3] if len(definition)>3 else []) )

    return collections


def _getRun3TrigObjList(destination, trigEDMList):
    """
    Run 3 version
    Gives back the Python dictionary  with the content of ESD/AOD (dst) which can be inserted in OKS.
    """
    dset = set(destination.split())
    toadd = defaultdict(list)

    for item in itertools.chain(*trigEDMList):
        if item[1] == '': # no output has been defined
            continue

        confset = set(item[1].split())

        if dset & confset: # intersection of the sets
            colltype, k = _getTypeAndKey(item[0])

            if k not in toadd[colltype]:
                toadd[colltype] += [k]

    return toadd


def _getRun3TrigEDMSlimList(key, HLTList):
    """
    Run 3 version
    Modified EDM list to remove all dynamic variables
    Requires changing the list to have 'Aux.-'
    """
    _edmList = _getRun3TrigObjList(key,[HLTList])

    output = {}
    for k,v in _edmList.items():
        output[k] = [_handleRun3ViewContainers( el, HLTList ) for el in v]
    return output

#************************************************************
#
#  For Run 1 and Run 2 (not modified (so far))
#
#************************************************************
def _getTriggerRun2EDMSlimList(key, edmList):
    """
    Run 2 version
    Modified EDM list to remove all dynamic variables
    Requires changing the list to have 'Aux.-'
    """
    output = {}
    for k,v in edmList.items():
        newnames = []
        for el in v:
            if 'Aux' in el:
                newnames+=[el.split('.')[0]+'.-']
            else:
                newnames+=[el]
            output[k] = newnames
    return output

def getCategory(s):
    """ From name of object in AOD/ESD found by checkFileTrigSize.py, return category """

    """ Clean up object name """
    s = s.strip()

    # To-do
    # seperate the first part of the string at the first '_'
    # search in EDMDetails for the key corresponding to the persistent value
    # if a key is found, use this as the first part of the original string
    # put the string back together

    if s.count('.') : s = s[:s.index('.')]
    if s.count('::'): s = s[s.index(':')+2:]
    if s.count('<'):  s = s[s.index('<')+1:]
    if s.count('>'):  s = s[:s.index('>')]
    if s.count('.') : s = s[:s.index('.')]
    if s.count('Dyn') : s = s[:s.index('Dyn')]

    # containers from Run 1-2 and 3 require different preprocessing
    # s12 is for Run 1-2, s is for Run 3
    s12 = s

    if s12.startswith('HLT_xAOD__') or s12.startswith('HLT_Rec__') or s12.startswith('HLT_Analysis__') :
        s12 = s12[s12.index('__')+2:]
        s12 = s12[s12.index('_')+1:]
        #if s12.count('.') : s12 = s12[:s12.index('.')]
        s12 = "HLT_"+s12
    elif s12.startswith('HLT_'):
        #if s.count('Dyn') : s = s[:s.index('Dyn')]
        if s12.count('_'): s12 = s12[s12.index('_')+1:]
        if s12.count('_'): s12 = s12[s12.index('_')+1:]
        s12 = "HLT_"+s12

    TriggerListRun1 = TriggerL2List + TriggerEFList + TriggerResultsRun1List
    TriggerListRun2 = TriggerResultsList + TriggerLvl1List + TriggerIDTruth + TriggerHLTList
    TriggerListRun3 = getRawTriggerEDMList(flags=None, runVersion=3)

    category = ''
    bestMatch = ''

    """ Loop over all objects already defined in lists (and hopefully categorized!!) """
    for item in TriggerListRun1+TriggerListRun2:
        t,k = _getTypeAndKey(item[0])

        """ Clean up type name """
        if t.count('::'): t = t[t.index(':')+2:]
        if t.count('<'):  t = t[t.index('<')+1:]
        if t.count('>'):  t = t[:t.index('>')]
        if (s12.startswith(t) and s12.endswith(k)) and (len(t) > len(bestMatch)):
            bestMatch = t
            category = item[2]

        if k.count('.'): k = k[:k.index('.')]
        if (s12 == k):
            bestMatch = k
            category = item[2]

    for item in TriggerListRun3:
        t,k = _getTypeAndKey(item[0])

        """ Clean up type name """
        if t.count('::'): t = t[t.index(':')+2:]
        if t.count('<'):  t = t[t.index('<')+1:]
        if t.count('>'):  t = t[:t.index('>')]

        if (s.startswith(t) and s.endswith(k)) and (len(t) > len(bestMatch)):
            bestMatch = t
            category = item[2]

        if k.count('.'): k = k[:k.index('.')]
        if (s == k):
            bestMatch = k
            category = item[2]

    if category == '' and 'HLTNav' in s:
        category = 'HLTNav'

    if category == '': return 'NOTFOUND'
    return category



def _getTypeAndKey(s):
    """ From the strings containing type and key of trigger EDM extract type and key
    """
    return s[:s.index('#')], s[s.index('#')+1:]

def _keyToLabel(key):
    """ The key is usually HLT_*, this function returns second part of it or empty string
    """
    if '_' not in key:
        return ''
    else:
        return key[key.index('_'):].lstrip('_')

def _getTriggerRun1Run2ObjList(destination, lst):
    """
    Gives back the Python dictionary  with the content of ESD/AOD (dst) which can be inserted in OKS.
    """
    dset = set(destination.split())

    toadd = {}

    for item in itertools.chain(*lst):
        if item[1] == '':
            continue
        confset = set(item[1].split())
        if dset & confset: # intersection of the sets
            t,k = _getTypeAndKey(item[0])
            colltype = t
            if 'collection' in EDMDetails[t]:
                colltype = EDMDetails[t]['collection']
            if colltype in toadd:
                if k not in toadd[colltype]:
                    toadd[colltype] += [k]
            else:
                toadd[colltype] = [k]
    return _InsertContainerNameForHLT(toadd)


def getTrigIDTruthList(dst):
    """
    Gives back the Python dictionary  with the truth trigger content of ESD/AOD (dst) which can be inserted in OKS.
    """
    return _getTriggerRun1Run2ObjList(dst,[TriggerIDTruth])

def getLvl1ESDList():
    """
    Gives back the Python dictionary  with the lvl1 trigger result content of ESD which can be inserted in OKS.
    """
    return _getTriggerRun1Run2ObjList('ESD',[TriggerLvl1List])

def getLvl1AODList():
    """
    Gives back the Python dictionary  with the lvl1 trigger result content of AOD which can be inserted in OKS.
    """
    return _getTriggerRun1Run2ObjList('AODFULL',[TriggerLvl1List])



def _getL2PreregistrationList():
    """
    List (Literally Python list) of trigger objects to be preregistered i.e. this objects we want in every event for L2
    """
    l = []
    for item in TriggerL2List:
        if len (item[1]) == 0: continue
        t,k = _getTypeAndKey(item[0])
        if('Aux' in t):
            continue #we don't wat to preregister Aux containers
        l += [t+"#"+_keyToLabel(k)]
    return l

def _getEFPreregistrationList():
    """
    List (Literally Python list) of trigger objects to be preregistered i.e. this objects we want in every event for EF
    """
    l = []
    for item in TriggerEFList:
        if len (item[1]) == 0: continue
        t,k = _getTypeAndKey(item[0])
        if('Aux' in t):
            continue #we don't wat to preregister Aux containers
        l += [t+"#"+_keyToLabel(k)]
    return l

def _getHLTPreregistrationList():
    """
    List (Literally Python list) of trigger objects to be preregistered i.e. this objects we want in every event for merged L2/EF in addition to default L2 and EF
    """
    l = []
    for item in TriggerHLTList:
        if len (item[1]) == 0: continue
        t,k = _getTypeAndKey(item[0])
        if('Aux' in t):
            continue #we don't wat to preregister Aux containers
        l += [t+"#"+_keyToLabel(k)]
    return l


def getPreregistrationList(version=2, doxAODConversion=True):
    """
    List (Literally Python list) of trigger objects to be preregistered i.e. this objects we want for all levels
    version can be: '1 (Run1)', '2 (Run2)'
    """

    l=[]
    if version==2:
        l = _getHLTPreregistrationList()
    elif version==1:
        # remove duplicates while preserving order
        objs=_getL2PreregistrationList()+_getEFPreregistrationList()
        if doxAODConversion:
            objs += _getHLTPreregistrationList()
        l=list(dict.fromkeys(objs))
    else:
        raise RuntimeError("Invalid version=%s supplied to getPreregistrationList" % version)
    return l


def _getL2BSTypeList():
    """ List of L2 types to be read from BS, used by the TP
    """
    l = []
    for item in TriggerL2List:
        t,k = _getTypeAndKey(item[0])
        ctype = t
        if 'collection' in EDMDetails[t]:
            ctype = EDMDetails[t]['collection']
        l += [ctype]
    return l

def _getEFBSTypeList():
    """ List of EF types to be read from BS, used by the TP
    """
    l = []
    for item in TriggerEFList:
        t,k = _getTypeAndKey(item[0])
        ctype = t
        if 'collection' in EDMDetails[t]:
            ctype = EDMDetails[t]['collection']
        l += [ctype]
    return l

def _getHLTBSTypeList():
    """ List of HLT types to be read from BS, used by the TP
    """
    l = []
    for item in TriggerHLTList:
        t,k = _getTypeAndKey(item[0])
        ctype = t
        if 'collection' in EDMDetails[t]:
            ctype = EDMDetails[t]['collection']
        l += [ctype]
    return l

def getTPList(version=2):
    """
    Mapping  of Transient objects to Peristent during serialization (BS creation)
    version can be: '1 (Run1)', '2 (Run2)'
    """
    l = {}
    if version==2:
        bslist = _getHLTBSTypeList()
    elif version==1:
        bslist = list(set(_getL2BSTypeList() + _getEFBSTypeList()))
    else:
        raise RuntimeError("Invalid version=%s supplied to getTPList" % version)
        
    for t,d in EDMDetails.items():
        colltype = t
        if 'collection' in d:
            colltype = EDMDetails[t]['collection']
        if colltype in bslist:
            l[colltype] = d['persistent']
    return l


def getEDMLibraries():
    return EDMLibraries

def _InsertContainerNameForHLT(typedict):
    import re
    output = {}
    for k,v in typedict.items():
        newnames = []
        for el in v:
            if el.startswith('HLT_') or el == 'HLT':
                prefixAndLabel = el.split('_',1) #only split on first underscore
                containername = k if 'Aux' not in k else EDMDetails[k]['parent'] #we want the type in the Aux SG key to be the parent type #104811
                #maybe this is not needed anymore since we are now versionless with the CLIDs but it's not hurting either
                containername = re.sub('::','__',re.sub('_v[0-9]+$','',containername))
                newnames+=['_'.join([prefixAndLabel[0],containername]+([prefixAndLabel[1]] if len(prefixAndLabel) > 1 else []))]
            else:
                newnames+=[el]
            output[k] = newnames
    return output

def getEFRun1BSList():
    """
    List of EF trigger objects that were written to ByteStream in Run 1
    """
    l = []
    for item in TriggerEFEvolutionList:
        if len (item[1]) == 0: continue
        t,k = _getTypeAndKey(item[0])
        l += [t+"#"+_keyToLabel(k)]
    return l

def getEFRun2EquivalentList():
    """
    List of Run-2 containers equivalent to Run-1 EF containers
    """
    l = []
    for item in TriggerEFEvolutionList:
        if len (item[1]) == 0: continue
        t,k = _getTypeAndKey(item[1])
        l += [t+"#"+_keyToLabel(k)]
    return l

def getL2Run1BSList():
    """
    List of L2 trigger objects that were written to ByteStream in Run 1
    """
    l = []
    for item in TriggerL2EvolutionList:
        if len (item[1]) == 0: continue
        t,k = _getTypeAndKey(item[0])
        l += [t+"#"+_keyToLabel(k)]
    return l

def getL2Run2EquivalentList():
    """
    List of Run-2 containers equivalent to Run-1 L2 containers
    """
    l = []
    for item in TriggerL2EvolutionList:
        if len (item[1]) == 0: continue
        t,k = _getTypeAndKey(item[1])
        l += [t+"#"+_keyToLabel(k)]
    return l

def isCLIDDefined(cgen,typename):
  """
  Checks container type name is hashable
  """
  c = cgen.genClidFromName(typename)
  return (cgen.getNameFromClid(c) is not None)

def testEDMList(edm_list, error_on_edmdetails = True):
    """
    Checks EDM list entries for serialization and configuration compliance.
    """

    #xAOD types that don't expect accompanying Aux containers
    _noAuxList = ["xAOD::" + contType for contType in ["TrigConfKeys", "BunchConfKey"]]

    cgen = clidGenerator("", False)
    return_code = 0
    found_allow_truncation = False
    serializable_names = []
    serializable_names_no_label = []
    serializable_names_no_properties = []

    for i, edm in enumerate(edm_list):

        #check has sufficient entries
        if len(edm) < 3:
            log.error("EDM entry too short for " + edm[0])
            return_code = 1
            continue

        serializable_name = edm[0]
        serializable_name_no_label = re.sub(r"\#.*", "", serializable_name)
        serializable_name_no_properties = serializable_name.split('.')[0]

        serializable_names.append(serializable_name)
        serializable_names_no_label.append(serializable_name_no_label)
        serializable_names_no_properties.append(serializable_name_no_properties)

        #check container type name is hashable
        if not isCLIDDefined(cgen,serializable_name_no_label):
            log.error("no CLID for " + serializable_name)
            return_code = 1

        #check that no '.' in entries _not_ containing 'Aux'
        if "Aux" not in serializable_name and "." in serializable_name:
            log.error("A '.' found in non-Aux container name " + serializable_name)
            return_code = 1

        #check for Aux "."
        if "Aux" in serializable_name and "Aux." not in serializable_name:
            log.error("no final Aux. in label for " + serializable_name)
            return_code = 1

        #check contains exactly one #
        if serializable_name.count("#") != 1:
            log.error("Invalid naming structure for " + serializable_name)
            return_code = 1
        else: # only proceed if type and name can be separated
            #check that every interface xAOD container directly followed by matching Aux container.
            if serializable_name.startswith("xAOD") and "Aux" not in serializable_name:
                cont_type,cont_name = serializable_name.split("#")
                if cont_type not in _noAuxList:
                    auxmismatch = False
                    if len(edm_list) == i+1:
                        auxmismatch = True
                        cont_to_test = "nothing"
                    else:
                        cont_to_test = edm_list[i+1][0].split(".")[0]+"."
                        cont_type_short = cont_type.replace("Container","")
                        pattern = re.compile(cont_type_short+r".*Aux.*\#"+cont_name+"Aux.")
                        if not pattern.match(cont_to_test):
                          #test if shallow container type.
                          shallow_pattern = re.compile("xAOD::ShallowAuxContainer#"+cont_name+"Aux.")
                          if not shallow_pattern.match(cont_to_test):
                            auxmismatch = True
                    if auxmismatch:
                        log.error("Expected relevant Aux container following interface container %s, but found %s instead.",serializable_name,cont_to_test)
                        return_code = 1
                    else:
                        #check that targets of interface and aux container match
                        targets = set(edm[1].split())
                        if len(edm_list[i+1]) > 1 and cont_to_test.count("#") == 1:
                            contname_to_test = cont_to_test.split("#")[1]
                            targets_to_test = set(edm_list[i+1][1].split())
                            if len(targets ^ targets_to_test) > 0:
                                log.error("Targets of %s (%s) and %s (%s) do not match",cont_name,targets,contname_to_test,targets_to_test)
                                return_code = 1

        #check that Aux always follows non-Aux (our deserialiser relies on that)
        if i>0 and "Aux" in serializable_name and "Aux" in edm_list[i-1][0]:
            log.error(f"Aux container {serializable_name} needs to folow the "
                      "associated interface container in the EDM list")
            return_code = 1

        #check target types are valid. Check that target lists containing higher level targets
        #have required low level targets (ESD for AOD targets, AODFULL for SLIM targets).
        file_types = edm[1].split() # might return empty list, this is fine - 0 output file types allowed for an EDM entry (in case of obsolete containers)
        for file_type in file_types:
          if file_type not in AllowedOutputFormats:
              log.error("unknown file type " + file_type + " for " + serializable_name)
              return_code = 1
          for higher_level,required_lower_level in [('AOD','ESD'), ('SLIM','AODFULL')]:
              if higher_level in file_type and required_lower_level not in file_types:
                  log.error("Target list for %s containing '%s' must also contain lower level target '%s'.",serializable_name,file_type,required_lower_level)
                  return_code = 1

        # Check allowTuncation is only at the end
        tags = edm[3] if len(edm) > 3 else None
        allow_truncation_flag = False
        if tags:
            allow_truncation_flag = allowTruncation in tags
            if allow_truncation_flag:
                found_allow_truncation = True

        if found_allow_truncation and not allow_truncation_flag:
            log.error("All instances of 'allowTruncation' need to be at the END of the EDM serialisation list")
            return_code = 1

    #end of for loop over entries. Now do complete list checks.

    #check for duplicates:
    #check that no two EDM entries match after stripping out characters afer '.'
    if not len(set(serializable_names_no_properties)) == len(serializable_names_no_properties):
        log.error("Duplicates in EDM list! Duplicates found:")
        import collections.abc
        for item, count in collections.Counter(serializable_names_no_properties).items():
            if count > 1:
                log.error(str(count) + "x: " + str(item))
        return_code = 1

    #check EDMDetails
    for EDMDetail in EDMDetailsRun3.keys():
        if EDMDetail not in serializable_names_no_label:
            msg = "EDMDetail for " + EDMDetail + " does not correspond to any name in TriggerList"
            if error_on_edmdetails:
                log.error(msg)
                return_code = 1
            else:
                log.warning(msg)

    return return_code

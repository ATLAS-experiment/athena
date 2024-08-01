# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# ********************* All Tools/Functions for the TriggerEDM **********************
# Keeping all functions from the original TriggerEDM.py (Run 2 EDM) in this file
# with this name to not break backwards compatibility
# Where possible, functions will be adapted to also work with Run 3, they will then be moved
# to the Run 3 section
# ***********************************************************************************

from TrigEDMConfig.TriggerEDMRun1 import TriggerL2List,TriggerEFList,TriggerResultsRun1List
from TrigEDMConfig.TriggerEDMRun2 import TriggerResultsList,TriggerLvl1List,TriggerIDTruth,TriggerHLTList,EDMDetails,EDMLibraries,TriggerL2EvolutionList,TriggerEFEvolutionList
from TrigEDMConfig.TriggerEDMRun3 import TriggerHLTListRun3,varToRemoveFromAODSLIM
from TrigEDMConfig.TriggerEDMRun4 import TriggerHLTListRun4
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
    for item in extraList:
        colname = (item[0].split("#")[1]).split(".")[0]
        if colname not in existing_collections:
            # a new collection is added
            edmList.append(item)
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

def getRawTriggerEDMList(flags, runVersion=-1):
    """
    The static EDM list does still need some light manipulation before it can be used commonly.
    Never import TriggerHLTListRun3 or TriggerHLTListRun4 directly, always fetch them via this function
    """
    if runVersion == -1:
        runVersion = flags.Trigger.EDMVersion

    if runVersion <= 2 or runVersion > 4:
        errMsg="ERROR the getRawTriggerEDMList function supports runs 3 and 4."
        log.error(errMsg)
        raise RuntimeError(errMsg)

    if runVersion == 3:
        edmListCopy = TriggerHLTListRun3.copy()
    else:
        edmListCopy = TriggerHLTListRun4.copy()

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
        RawEDMList = getRawTriggerEDMList(flags, 3)

        # Run 4 will eventually use only its own distinct EDM, but for now we will append the Run 4 specific items to the Run 3 list.
        if runVersion >= 4:
            # Replace if existing
            for i in range(len(RawEDMList)-1, -1, -1): # Back iterate by index, we might be removing as we go
                for r4item in TriggerHLTListRun4:
                    if RawEDMList[i][0] == r4item[0]:
                        del RawEDMList[i]
                        break
            RawEDMList.extend(TriggerHLTListRun4)

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
    import itertools

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
    from collections import OrderedDict
    toadd = OrderedDict()
    import itertools

    for item in itertools.chain(*trigEDMList):
        if item[1] == '': # no output has been defined
            continue

        confset = set(item[1].split())

        if dset & confset: # intersection of the sets
            t,k = _getTypeAndKey(item[0])
            colltype = t

            if colltype in toadd:
                if k not in toadd[colltype]:
                    toadd[colltype] += [k]
            else:
                toadd[colltype] = [k]

    return toadd


def _getRun3TrigEDMSlimList(key, HLTList):
    """
    Run 3 version
    Modified EDM list to remove all dynamic variables
    Requires changing the list to have 'Aux.-'
    """
    _edmList = _getRun3TrigObjList(key,[HLTList])
    from collections import OrderedDict
    output = OrderedDict()
    for k,v in _edmList.items():
        newnames = []
        for el in v:
            newnames.append( _handleRun3ViewContainers( el, HLTList ) )
        output[k] = newnames
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
    import itertools

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

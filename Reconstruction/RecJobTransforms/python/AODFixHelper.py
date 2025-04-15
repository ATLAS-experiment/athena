# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import re
from AthenaCommon.Logging import logging

#Helper method to determine if the input release is in particular release range. 
#The boundaries rel1 and rel2 are inclusive.

def releaseInRange(flags,rel1,rel2):
    msg=logging.getLogger("releaseInRange")

    #This regex matches 3 and 4 digit release numbers
    #It also matches only something that start with Athena-xx
    relPattern=re.compile(r"^Athena-(\d+(\.\d+){2,3}(\.\*)?)$")

    for r in (rel1,rel2):
        if not relPattern.match(r):
            raise RuntimeError("Release number %s doesn't match the expected format"%r)

    inputRelease=flags.Input.Release
    
    # protection against AOD with very old releases or not read correctly by MetaReader.py
    if inputRelease == "" or not inputRelease.startswith("Athena-"):
        msg.debug("flags.Input.Release is read as empty or it does not start with 'Athena-'")
        return False
    
    if not relPattern.match(inputRelease):
        raise RuntimeError("Input release number %s doesn't match the expected format"%inputRelease)
   
    # convert 3 digits releases to 4 digits
    if rel1.count(".") == 2:         rel1=f"{rel1}.0"
    if rel2.count(".") == 2:         rel2=f"{rel2}.0"
    if inputRelease.count(".") == 2: inputRelease=f"{inputRelease}.0"

    #By Atlas convention, the first number denotes the major release, the second one the purpose (Tier0, Generation, ... ) 
    #the third one is the running version number, the fourth one the patch number
    #We request that the first two numbers are identical and the (third*10000 + forth) ones are in range

    def identifyMajorAndMinorRel(rel):
        idx = rel.rfind(".", 0, rel.rfind("."))
        return rel[:idx]
    
    def mangleRunningAndPatch(rel):
        rel_split = rel.split(".")
        return int(rel_split[2])*10000 + int(rel_split[3]) # running*10000 should be safely high enough

    if identifyMajorAndMinorRel(rel1) != identifyMajorAndMinorRel(rel2):
        raise RuntimeError("Boundary releases not from the same release series, got %s and %s"%(rel1,rel2))

    if identifyMajorAndMinorRel(rel1) != identifyMajorAndMinorRel(inputRelease):
        msg.info("Input release not from the same release series.")
        return False

    #convert number to int for comparison
    lower=mangleRunningAndPatch(rel1)
    upper=mangleRunningAndPatch(rel2)
    current=mangleRunningAndPatch(inputRelease)

    if (lower > upper): 
        raise RuntimeError("Lower boundary releases %s larger then upper boundary release %s" % (rel1,rel2))

    if (current >= lower and current <= upper):
        return True
    else:
        return False

#!/usr/bin/env python

#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

# Script to print out default test files, conditions and geometry tags

from AthenaConfiguration import TestDefaults

def printValues(testDefaultsClass):
    instance_vars = vars(testDefaultsClass)
    for var_name, var_value in instance_vars.items():
        #Ignore functions
        if str(var_name).startswith("__"):
            continue
        if str(var_value).startswith("__"):
            continue
        if str(var_value).startswith("<"):
            continue
        #Print only value, not ['value']
        if type(var_value) is list:
            var_value = var_value[0]
        print(testDefaultsClass.__name__, var_name, var_value)

if __name__ == "__main__":
    printValues(TestDefaults.defaultTestFiles)
    printValues(TestDefaults.defaultConditionsTags)
    printValues(TestDefaults.defaultGeometryTags)

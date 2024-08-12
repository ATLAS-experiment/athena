# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Run this file in order to print out the empty slots

#
# ALL CHANGES TO THE RUN 4 MENU SHOULD BE MADE TO RELEASE 25 OR GREATER
#

from TriggerMenuMT.L1.Base.L1MenuFlags import L1MenuFlags
from TriggerMenuMT.L1.Menu.MenuCommon import print_available, RequiredL1Items, defineCommonL1Flags

def defineMenu():
    # We do not use physics menu in Run4 development at the moment
    defineCommonL1Flags(L1MenuFlags)
    L1MenuFlags.items = RequiredL1Items

if __name__ == "__main__":
    defineMenu()
    print_available(L1MenuFlags)



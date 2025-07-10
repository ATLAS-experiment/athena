#!/bin/sh
#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

# This is a unit test of HLT Control Flow  in CA mode

athena.py --menuType 'menuManual' --config-only=config.pkl --imf --threads=1  --evtMax 4 --filesInput /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data17_13TeV.00327265.physics_EnhancedBias.merge.RAW._lb0100._SFO-1._0001.1_50evt TriggerMenuMT/CFtest/test_menu_cf.py

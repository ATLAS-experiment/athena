# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# CI test definitions for the AthSimulation project
# --> README.md before you modify this file
#

atlas_add_citest( DuplicateClass
   SCRIPT python -c 'import ROOT'
   PROPERTIES FAIL_REGULAR_EXPRESSION "class .* is already in" )

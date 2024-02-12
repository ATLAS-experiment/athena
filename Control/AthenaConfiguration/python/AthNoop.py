# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Dummy script for the purpose of printing the --help text w/o any further script
from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()
flags.fillFromArgs()

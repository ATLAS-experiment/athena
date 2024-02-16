# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#Job options needed to enable LAr alignment

from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
from LArGeoAlgsNV.LArGMConfig import LArGMCfg
from AthenaConfiguration.OldFlags2NewFlags import getNewConfigFlags
flags=getNewConfigFlags()
flags.lock()
CAtoGlobalWrapper(LArGMCfg,flags)


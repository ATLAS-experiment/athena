# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaCommon import Logging
logger = Logging.logging.getLogger("PowhegControl")

from PowhegControl.parameters import powheg_atlas_common
powheg_atlas_common.usePDGvalues()
logger.warning("Using PDG values as 'ATLAS default' parameters for Powheg")

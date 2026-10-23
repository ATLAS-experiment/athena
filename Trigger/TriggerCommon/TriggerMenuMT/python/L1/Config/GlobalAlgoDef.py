# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from .GlobalHypoAlgoDef import GlobalHypoAlgoDef

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

class GlobalAlgoDef:

    @staticmethod
    def registerGlobalAlgos(tm):
        GlobalHypoAlgoDef.registerGlobalHypoAlgos(tm)
        GlobalAlgoDef.registerGlobalPreprocessingAlgos(tm)

    @staticmethod
    def registerGlobalPreprocessingAlgos(tm):
        pass

#! /usr/bin/env python

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
import logging
from AnaAlgorithm.DualUseConfig import isAthena

if __name__ == '__main__':
    if isAthena:
        from AnalysisAlgorithmsConfig.AthenaCPRunScript import AthenaCPRunScript
        script = AthenaCPRunScript()
    else:
        from AnalysisAlgorithmsConfig.EventLoopCPRunScript import EventLoopCPRunScript
        script = EventLoopCPRunScript()
        
    script.logger.setLevel(logging.INFO)
    script.run()

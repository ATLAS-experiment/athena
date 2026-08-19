#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import sys
from AthenaCommon.Logging import logging
log = logging.getLogger('athenaHLT')

log.error("athenaHLT.py is obsolete, use athenaEF.py instead (same command line interface)")
sys.exit(1)

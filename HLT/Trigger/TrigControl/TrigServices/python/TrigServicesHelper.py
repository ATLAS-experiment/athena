# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

"""
TrigServicesHelper - Python interface to call ITrigEventLoopMgr methods directly.

This module provides a way to call prepareForStart(), prepareForRun(), and
hltUpdateAfterFork() on the HltEventLoopMgr without going through PSC/HLTMPPU.

Usage in athenaEF.py:
   
   from TrigServices.TrigServicesHelper import TrigServicesHelper
   
   # After app.initialize() but before app.start():
   helper = TrigServicesHelper()
   success = helper.prepareForStart(
       run_number=args.run_number,
       det_mask=args.detector_mask,
       sor_time=args.sor_time,
       lb_number=args.lb_number,
       beam_type=0,
       beam_energy=0,
       toroids_current=20400.0,
       solenoid_current=7730.0
   )
"""

from libTrigServicesHelper import TrigServicesHelper

__all__ = ['TrigServicesHelper']

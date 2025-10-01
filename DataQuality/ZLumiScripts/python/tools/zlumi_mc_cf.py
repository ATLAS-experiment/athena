#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def correction(mu, runmode, campaign, run=None):
    if runmode == "Zee":
        if campaign == "mc16a":
            return 0.907628 - 0.000328652*mu - 3.0512e-06*mu*mu
        elif campaign == "mc16d":
            return 0.904096 - 0.000172139*mu - 4.35328e-06*mu*mu
        elif campaign == "mc16e":
            return 0.90238 - 8.75767e-05*mu - 5.79201e-06*mu*mu
        elif campaign == "mc21":
            return 0.889336 - 0.000191184*mu - 4.11419e-06*mu*mu 
        elif campaign == "mc23a":
            return 0.888218 - 0.0000213381*mu - 0.00000507315*mu*mu
        elif campaign == "mc23d":
            return 1.04742 - 0.0102056*mu + 0.000197393*mu*mu - 0.00000132012*mu*mu*mu
        elif campaign == "mc23e":
            return 0.883555 + 0.00000150059*mu - 0.00000750447*mu*mu
    elif runmode == "Zmumu":
        if campaign == "mc16a":
            return 9.90074e-01 - 5.34716e-06*mu - 3.23366e-06*mu*mu
        elif campaign == "mc16d":
            return 9.91619e-01 - 1.21674e-04*mu - 1.58362e-06*mu*mu
        elif campaign == "mc16e":
            return 9.90808e-01 - 9.99749e-05*mu - 1.40241e-06*mu*mu
        elif campaign == "mc21":
            return 0.987 - 6.11277e-05*mu - 2.59671e-06*mu*mu 
        elif campaign == "mc23a":
            return 0.994803 - 0.000269879*mu + 0.000000169232*mu*mu
        elif campaign == "mc23d":
            return 0.976218 + 0.000148547*mu - 0.00000464437*mu*mu
        elif campaign == "mc23e":
            return 0.984034 - 0.000186118*mu - 0.000000939314*mu*mu


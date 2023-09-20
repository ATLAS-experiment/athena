#!/usr/bin/env python

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
        elif campaign == "mc23a" and run < 451896:
            return 0.8896 - 0.0000115*mu - 0.0000081*mu*mu
        elif campaign == "mc23a" and run >= 451896:
            return 0.8923 - 0.0001822*mu - 0.0000038*mu*mu
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
            return 0.9914 - 0.0001093*mu - 0.0000018*mu*mu
        #elif campaign == "mc23a" and run >= 451896:
        #    return 0.9913- 0.0001052*mu - 0.0000019*mu*mu


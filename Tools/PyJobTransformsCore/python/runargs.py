# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

__author__ = "clat@hep.ph.bham.ac.uk"

import os

class RunArguments:
    """Dynamic class that holds the run arguments as named members with values."""
    def __str__(self):
        myself = 'RunArguments:'
        for arg in dir(self):
            if not arg.startswith('__'):
                myself += '%s   %s = %s' % (os.linesep, arg, repr(getattr(self,arg)))
        return myself

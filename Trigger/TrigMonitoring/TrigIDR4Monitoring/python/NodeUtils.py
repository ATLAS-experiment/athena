## noqa: ATL902
##
##    @file  NodeUtils.py
##
##           Node class to represent flags as an actual tree
##
##   @author  sutt
##   @date    Sat Aug 15 15:08:59 CEST 2026
##
##  $Id: NodeUtils.py, v0.0  Sat Aug 15 15:08:59 CEST 2026  sutt $
##
## Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
##

from TrigIDR4Monitoring.Node import decode_flags
from TrigIDR4Monitoring.Node import update_flags

import datetime as dt

from functools import wraps

def timerwrapper(thefunction):
    @wraps(thefunction)
    def wrapper(*args, **kwargs):
            """ timer wrapper around function """
            t = dt.datetime.now()
            result = thefunction( *args, **kwargs )
            t2 = dt.datetime.now()
            duration = str( round((t2 - t).total_seconds()*1000, 4))
            print( f"{thefunction.__name__} timer: elapsed time: {duration} ms")
            return result
    return wrapper


def timer( thefunction, *args, **kwargs):
    return timerwrapper(thefunction)(*args,**kwargs)


@timerwrapper
def decodeFlags( flags, domain=None ) :
    return decode_flags( flags, domain )

@timerwrapper
def updateFlags( flags, node, domain=None ) :
    return update_flags( flags, node, domain )


    

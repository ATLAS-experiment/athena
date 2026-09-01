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

def decodeFlags( flags, domain=None ) :
    """ timer wrapper around decode_flags """
    t = dt.datetime.now()
    root = decode_flags( flags, domain )
    t2 = dt.datetime.now()
    duration = str( round((t2 - t).total_seconds()*1000, 4))
    print( f"decode flags: elapsed time: {duration} ms")
    return root


def updateFlags( flags, node, domain="" ) :    
    """ timer wrapper around update_flags """
    t = dt.datetime.now()
    update_flags( flags, node, domain )
    t2 = dt.datetime.now()
    duration = str( round((t2 - t).total_seconds()*1000, 4))    
    print( f"update flags: elapsed time: {duration} ms")

    

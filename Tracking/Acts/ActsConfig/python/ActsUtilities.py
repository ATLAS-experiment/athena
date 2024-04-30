#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def extractChildKwargs(*,
                       prefix: str,
                       **kwargs: dict) -> dict:
    """ Extract subset of kwargs with common prefix """
    assert isinstance(kwargs, dict)
    assert isinstance(prefix, str)
    
    args = dict()
    for (k,v) in kwargs.items():
        if len(k) <= len(prefix):
            continue
        if k[:len(prefix)] != prefix:
            continue
        args[ k[len(prefix):] ] = v
    return args

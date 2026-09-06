# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from pickle import dump, loads
from functools import wraps
from os import environ
from importlib.resources import files

def invariant(test_data_name):
    def wrapper(func):
        testfile = "%s.%s" % (func.__name__, test_data_name)
        @wraps(func)
        def check_invariant(*args, **kwargs):
            result = func(*args, **kwargs)
            if "DQU_UPDATE_TEST_DATA" in environ:
                with open(test_data_name, "wb") as fd:
                    dump(result, fd)
                print("Wrote updated test data")
            else:
                test_data = loads(files("testdata").joinpath(testfile).read_bytes())
                assert result == test_data, "Data considered 'invariant' has changed"
        return check_invariant           
            
    return wrapper

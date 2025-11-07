# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import os

class TransformError( Exception ):
    """Base class for PyJobTransform Exception classes"""
    def __init__(self,message=None,error='TRF_UNKNOWN',**kwargs): # noqa: B042
        Exception.__init__(self,error,message)
        self.message = message
        self.error = error
        self.extras = kwargs

    def __str__(self):
        header = '%s: ' % self.error
        msg = header + self.message
        indent = os.linesep + ' '*len(header)
        for n,v in self.extras.items():
            msg += '%s%s=%s' % (indent,n,v)
        return msg

    def setError(self,error):
        self.error = error
        self.args = (self.error,self.message)

    def setMessage(self,message):
        self.message = message
        self.args = (self.error,self.message)


class TransformConfigError( TransformError ):
    """Exception raised in case of an error in the transform configuration"""
    def __init__(self,message=None,error='TRF_CONFIG',**kwargs): # noqa: B042
        TransformError.__init__(self,message,error,**kwargs)


class JobOptionsNotFoundError( TransformError ):
    """Exception raised in case a joboptions file can not be found"""
    def __init__(self,filename,message=None,error='ATH_JOP_NOTFOUND'): # noqa: B042
        mess = "JobOptions file %s not found" % filename
        if message: mess += '. ' + message
        TransformError.__init__(self,mess,error)

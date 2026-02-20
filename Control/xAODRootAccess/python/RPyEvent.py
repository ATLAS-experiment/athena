# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Module holding the RPyEvent Python class
#

# Pull in ROOT:
import ROOT

## A Python wrapper around xAOD::RPyEvent
#
# In order to make the semi-templated functions of xAOD::RPyEvent more
# convenient to use from PyROOT, the user is supposed to use an instance
# of this class, and not ROOT.xAOD.RPyEvent directly.
#
class RPyEvent( ROOT.xAOD.Experimental.RPyEvent ):

    ## Constructor for the class
    def __init__( self ):

        # Forward the call to the base class's constructor:
        ROOT.xAOD.Experimental.RPyEvent.__init__( self )
        return

    ## Convenient shorthand for retrieving an object.
    def __getitem__( self, key ):
        return self.pyRetrieve( key )

    ## Convenient version of the base class's contains function
    #
    # This function allows the user to, instead of figuring out the exact
    # type name of some C++ type, to rather write code like:
    #
    # <code>
    #   if event.contains( "Electrons", ROOT.xAOD.ElectronContainer_v1 ):
    # </code>
    #
    # @param key  The string key of the object to check for
    # @param type The type of the object we are looking for
    # @returns <code>True</code> if the object is available in the event,
    #          <code>False</code> if it's not
    #
    def contains( self, key, type ):
        # Determine the class name:
        clname = type.__name__
        if hasattr( type, "__cpp_name__" ):
            clname = type.__cpp_name__
            pass
        # Call the parent class's function:
        return super( RPyEvent, self ).pyContains( key, clname )

    ## Convenient version of the base class's transientContains function
    #
    # This function allows the used to, instead of figuring out the exact
    # type name of some C++ type, to rather write code like:
    #
    # <code>
    #   if event.transientContains( "MyElectrons",<br/>
    #                               ROOT.xAOD.ElectronContainer_v1 ):
    # </code>
    #
    # @param key  The string key of the object to check for
    # @param type The type of the object we are looking for
    # @returns <code>True</code> if the object is available in the event in a
    #          modifiable form, <code>False</code> if it's not
    #
    def transientContains( self, key, type ):
        # Determine the class name:
        clname = type.__name__
        if hasattr( type, "__cpp_name__" ):
            clname = type.__cpp_name__
            pass
        # Call the parent class's function:
        return super( RPyEvent,
                      self ).pyTransientContains( key, clname )


# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

isAthena = False

try:
    from AthenaConfiguration.ComponentFactory import CompFactory
    isAthena = True
except ImportError:
    pass


def createComponent( typeName, instanceName, componentType ):
    """Create a generic configurable

    This function is used to create an component "configurable" in a
    dual-use way, either returning an actual Athena configurable, or
    an appropriately configured PythonConfig instance.

    Keyword arguments:
      typeName      -- The C++ type name of the component
      instanceName  -- The instance name of the component to create
      componentType -- The type of component in AnalysisBase

    """

    if isAthena:
        # Try to get a configurable for this C++ class "from Athena".
        # If this succeeds, we're obviously in an Athena environment.

        # Look up the Athena configurable of this component:
        componentClass = CompFactory.getComp(typeName)

        # Return the object:
        return componentClass( instanceName )

    else:
        # If that didn't work, then apparently we're in an EventLoop
        # environment, so we need to use PythonConfig as the base class
        # for the user's class.
        from AnaAlgorithm.PythonConfig import PythonConfig
        component = PythonConfig( '%s/%s' % ( typeName, instanceName ) )
        component.setComponentType( componentType )
        return component

    pass


def createAlgorithm( typeName, instanceName ):
    """Create an algorithm configurable

    This function is used to create an algorithm "configurable" in a dual-use
    way, either returning an actual Athena configurable, or an appropriately
    configured EL::AnaAlgorithmConfig instance.

    Keyword arguments:
      typeName     -- The C++ type name of the algorithm
      instanceName -- The instance name of the algorithm to create
    """
    return createComponent( typeName, instanceName, 'AnaAlgorithm' )


def createReentrantAlgorithm( typeName, instanceName ):
    """Create a reentrant algorithm configurable

    This function is used to create an algorithm "configurable" in a dual-use
    way, either returning an actual Athena configurable, or an appropriately
    configured EL::AnaAlgorithmConfig instance.

    Keyword arguments:
      typeName     -- The C++ type name of the algorithm
      instanceName -- The instance name of the algorithm to create
    """
    return createComponent( typeName, instanceName, 'AnaReentrantAlgorithm' )


def createPublicTool( typeName, toolName ):
    """Helper function for setting up a public tool for a dual-use algorithm

    This function is meant to be used in the analysis algorithm sequence
    configurations for setting up public tools on the analysis algorithms.
    Public tools that could then be configured with a syntax shared between
    Athena and EventLoop.

    Keyword arguments:
      typeName -- The C++ type name of the private tool
      toolName -- The property name with which the tool handle was declared on
                  the algorithm. Also the instance name of the tool.
    """

    if isAthena:
        # Look up the Athena configurable of this tool:
        return CompFactory.getComp( typeName )( toolName )

    else:
        # If that didn't work, then apparently we're in an EventLoop
        # environment, so let's use the EventLoop specific formalism.
        return createComponent( typeName, toolName, 'AsgTool' )


def createService( typeName, serviceName, sequence=None ):
    """Helper function for setting up a service for a dual-use algorithm

    This function is meant to be used to set up services in a dual-use
    manner, particularly for the common CP algorithms.  This allows to
    use the same syntax in EventLoop and Athena, hiding the
    differences internally.  Since in EventLoop the service gets added
    to a sequence (but in Athena does not), that sequence needs to be
    passed into this function.

    Keyword arguments:
      typeName -- The C++ type name of the service
      serviceName -- The name with which the service handle was configured on
                    the algorithm. Also the instance name of the service.
      sequence -- an optional argument of an algorithm sequence to add it to
                  in EventLoop (ignored in Athena)

    """

    if isAthena:
        # Look up the Athena configurable of this serivce:
        return CompFactory.getComp( typeName )( serviceName )

    else:
        # If that didn't work, then apparently we're in an EventLoop
        # environment, so let's use the EventLoop specific formalism.
        service = createComponent( typeName, serviceName, 'AsgService' )
        if sequence is not None :
            sequence += service
            pass
        return service


def addPrivateTool( alg, toolName, typeName ):
    """Helper function for declaring a private tool for a dual-use algorithm

    This function is meant to be used in the analysis algorithm sequence
    configurations for setting up private tools on the analysis algorithms.
    Private tools that could then be configured with a syntax shared between
    Athena and EventLoop.

    Keyword arguments:
      alg      -- The algorithm to set up the private tool on
      toolName -- The property name with which the tool handle was declared on
                  the algorithm. Also the instance name of the tool.
      typeName -- The C++ type name of the private tool
    """

    if isAthena:

        # First try to set up the private tool in an "Athena way".

        # Tokenize the tool's name. In case it is a subtool of a tool, or
        # something possibly even deeper.
        toolNames = toolName.split( '.' )

        # Look up the component that we need to set up the private tool on:
        component = alg
        for tname in toolNames[ 0 : -1 ]:
            component = getattr( component, tname )
            pass

        # Now look up the Athena configurable describing this tool:
        toolClass = CompFactory.getComp(typeName)

        # Finally, set up the tool handle property:
        setattr( component, toolNames[ -1 ], toolClass( toolNames[ -1 ] ) )

    else:

        # If that failed, then we should be in an EventLoop environment. So
        # let's rely on the standalone specific formalism for setting up the
        # private tool.
        alg.addPrivateTool( toolName, typeName )
        pass

    return


def addPrivateToolInArray( alg, toolName, typeName ):
    """Helper function for declaring a private tool in an array for a
    dual-use algorithm

    This function is meant to be used in the analysis algorithm
    sequence configurations for setting up private tools in arrays on
    the analysis algorithms.  Private tools that could then be
    configured with a syntax shared between Athena and EventLoop.

    Keyword arguments:
      alg      -- The algorithm to set up the private tool on
      toolName -- The property name with which the tool handle was declared on
                  the algorithm. Also the instance name of the tool.
      typeName -- The C++ type name of the private tool

    """

    if isAthena:

        # First try to set up the private tool in an "Athena way".

        # Tokenize the tool's name. In case it is a subtool of a tool, or
        # something possibly even deeper.
        toolNames = toolName.split( '.' )

        # Look up the component that we need to set up the private tool on:
        component = alg
        for tname in toolNames[ 0 : -1 ]:
            component = getattr( component, tname )
            pass

        # Now look up the Athena configurable describing this tool:
        toolClass = CompFactory.getComp(typeName)

        # Finally, set up the tool handle property:
        getattr( component, toolNames[ -1 ] ).append (toolClass( toolNames[ -1 ] ) )
        return getattr( component, toolNames[ -1 ] )

    else:

        # If that failed, then we should be in an EventLoop environment. So
        # let's rely on the standalone specific formalism for setting up the
        # private tool.
        return alg.addPrivateToolInArray( toolName, typeName )

    return

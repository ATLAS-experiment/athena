#!/usr/bin/env athena.py
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def HelloWorldToolCfg(flags, name="HelloTool", **kwargs):
    result = ComponentAccumulator()
    # one can use kwargs to set properties
    kwargs.setdefault("MyMessage", "Hello World!")
    # create and add the tool to the result (and pass arguments)
    result.setPrivateTools(CompFactory.HelloTool(name, **kwargs))
    return result


def HelloWorldPublicToolCfg(flags, name="PublicHello", **kwargs):
    result = ComponentAccumulator()
    # one can also set properties directly with creation
    tool = CompFactory.HelloTool(name, MyMessage="Hello!", **kwargs)
    # ...or after creation
    tool.MyMessage = "A public Message!"
    # add the tool to the result
    result.addPublicTool(tool, primary=True)
    return result


def HelloWorldCfg(flags):
    result = ComponentAccumulator()

    alg = CompFactory.HelloAlg("HelloWorld",
                               # Set an int from a flag
                               MyInt=flags.Input.JobNumber,
                               # Set a boolean property (False, True, 0, 1)
                               MyBool=True,
                               # Set a double property
                               MyDouble=3.14159,
                               # Set a vector of strings property
                               MyStringVec=["Welcome", "to",
                                            "Athena", "Framework", "Tutorial"],
                               # Set a map of strings to strings property
                               MyDict={"Bonjour": "Guten Tag",
                                       "Good Morning": "Bonjour",
                                       "one": "uno"},
                               # Set a table (a vector of pairs of doubles)
                               MyTable=[(1, 1), (2, 4), (3, 9)],
                               # Set a matrix (a vector of vectors) ...
                               MyMatrix=[[1, 2, 3],
                                         [4, 5, 6]],
                               # set a private tool
                               MyPrivateHelloTool=result.popToolsAndMerge(
                                   HelloWorldToolCfg(flags)),
                               # set a public tool
                               MyPublicHelloTool=result.getPrimaryAndMerge(
                                   HelloWorldPublicToolCfg(flags)),
                               )

    # add the algorithm to the result
    result.addEventAlgo(alg)
    return result


if __name__ == "__main__":
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Exec.MaxEvents = 10
    flags.fillFromArgs()
    flags.lock()

    cfg = MainServicesCfg(flags)
    cfg.merge(HelloWorldCfg(flags))

    import sys
    sys.exit(cfg.run().isFailure())

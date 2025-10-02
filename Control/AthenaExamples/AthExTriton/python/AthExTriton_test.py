# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon import Constants


def AthExTritonCfg(flags, name="AthExTritonExample", **kwargs):
    acc = ComponentAccumulator()

    from AthTritonComps.TritonToolConfig import TritonToolCfg
    kwargs.setdefault("InferenceTool", acc.popToolsAndMerge(
        TritonToolCfg(flags, "MNIST_testModel", "localhost", name="EvaluateModelTritonTool")
    ))


    input_data = "dev/MLTest/2020-03-31/t10k-images-idx3-ubyte"
    kwargs.setdefault("BatchSize", 2)
    kwargs.setdefault("InputDataPixel", input_data)
    kwargs.setdefault("OutputLevel", Constants.DEBUG)
    acc.addEventAlgo(CompFactory.AthInfer.ExampleMLInferenceWithTriton(name, **kwargs))

    return acc

if __name__ == "__main__":
    from AthenaCommon.Logging import log as msg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    msg.setLevel(Constants.DEBUG)

    flags = initConfigFlags()
    flags.lock()

    acc = MainServicesCfg(flags)
    acc.merge(AthExTritonCfg(flags))
    acc.printConfig(withDetails=True, summariseProps=True)

    acc.store(open('test_AthExTritonCfg.pkl','wb'))

    import sys
    sys.exit(acc.run(2).isFailure())

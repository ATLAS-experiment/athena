# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon import Constants
from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType
from AthOnnxComps.OnnxRuntimeInferenceConfig import OnnxRuntimeInferenceToolCfg


def AthExOnnxRuntimeExampleCfg(flags, name="AthOnnxExample", **kwargs):
    acc = ComponentAccumulator()

    model_fname = "dev/MLTest/2020-03-02/MNIST_testModel.onnx"
    execution_provider = OnnxRuntimeType.CPU
    kwargs.setdefault("ORTInferenceTool", acc.popToolsAndMerge(
        OnnxRuntimeInferenceToolCfg(flags, model_fname, execution_provider)
    ))

    input_data = "dev/MLTest/2020-03-31/t10k-images-idx3-ubyte"
    kwargs.setdefault("BatchSize", 100)
    kwargs.setdefault("InputDataPixel", input_data)
    kwargs.setdefault("OutputLevel", Constants.DEBUG)
    acc.addEventAlgo(
        CompFactory.AthOnnx.EvaluateModelWithAsyncInfer(name, **kwargs))

    return acc


if __name__ == "__main__":
    from AthenaCommon.Logging import log as msg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    msg.setLevel(Constants.DEBUG)

    flags = initConfigFlags()
    flags.AthOnnx.ExecutionProvider = OnnxRuntimeType.CPU
    flags.Concurrency.NumThreads = 3
    flags.Concurrency.NumOffloadThreads = 1
    flags.Exec.FPE = -1
    flags.lock()

    acc = MainServicesCfg(flags)
    acc.merge(AthExOnnxRuntimeExampleCfg(flags))
    acc.printConfig(withDetails=True, summariseProps=True)

    acc.store(open('test_AsyncInferORTExampleCfg.pkl', 'wb'))

    import sys
    sys.exit(acc.run(2).isFailure())

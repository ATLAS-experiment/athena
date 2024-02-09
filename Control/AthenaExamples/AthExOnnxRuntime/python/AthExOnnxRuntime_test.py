# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon import Constants
from AthOnnxComps.OnnxRuntimeFlags import OnnxRuntimeType 


def AthExOnnxRuntimeExampleCfg(flags, name="AthOnnxExample", **kwargs):
    acc = ComponentAccumulator()

    model_fname = "/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/MLTest/2020-03-02/MNIST_testModel.onnx"
    execution_provider = OnnxRuntimeType.CPU
    from AthOnnxComps.OnnxRuntimeInferenceConfig import OnnxRuntimeInferenceToolCfg
    kwargs.setdefault("ORTInferenceTool", acc.popToolsAndMerge(
        OnnxRuntimeInferenceToolCfg(flags, model_fname, execution_provider)
    ))

    input_data = "/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/MLTest/2020-03-31/t10k-images-idx3-ubyte"
    kwargs.setdefault("BatchSize", 3)
    kwargs.setdefault("InputDataPixel", input_data)
    kwargs.setdefault("OutputLevel", Constants.DEBUG)
    acc.addEventAlgo(CompFactory.AthOnnx.EvaluateModel(name, **kwargs))

    return acc

if __name__ == "__main__":
    from AthenaCommon.Logging import log as msg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    msg.setLevel(Constants.DEBUG)

    flags = initConfigFlags()
    flags.AthOnnx.ExecutionProvider = OnnxRuntimeType.CPU
    flags.lock()

    acc = MainServicesCfg(flags)
    acc.merge(AthExOnnxRuntimeExampleCfg(flags))
    acc.printConfig(withDetails=True, summariseProps=True)

    acc.store(open('test_AthExOnnxRuntimeExampleCfg.pkl','wb'))

    import sys
    sys.exit(acc.run(2).isFailure())

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.Enums import FlagEnum

class OnnxRuntimeType(FlagEnum):
    CPU = 'CPU'
    CUDA = 'CUDA'
# possible future backends. Uncomment when implemented.
    # DML = 'DML'
    # DNNL = 'DNNL'
    # NUPHAR = 'NUPHAR'
    # OPENVINO = 'OPENVINO'
    # ROCM = 'ROCM'
    # TENSORRT = 'TENSORRT'
    # VITISAI = 'VITISAI'
    # VULKAN = 'VULKAN'

def createOnnxRuntimeFlags():
    icf = AthConfigFlags()

    icf.addFlag("AthOnnx.ExecutionProvider", OnnxRuntimeType.CPU, type=OnnxRuntimeType)

    return icf

if __name__ == "__main__":

    flags = createOnnxRuntimeFlags()
    flags.dump()

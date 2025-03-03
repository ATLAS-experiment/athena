// The nominal setup for this package require OPENCL, which is only currently avaliable in docker images
// And not distributed within normal athena setup. However, this tool and alg are not dependant on that
// and are useful to be implemented in FPGATrackSim for FPGA test vector generation

#include "../FPGADataFormatAlg.h"
#include "../FPGADataFormatTool.h"
#include "../TestVectorTool.h"
#include "../OutputConversionTool.h"
#include "../xAODContainerMaker.h"
#include "../FPGAOutputValidationAlg.h"

DECLARE_COMPONENT(FPGADataFormatAlg)
DECLARE_COMPONENT(FPGADataFormatTool)
DECLARE_COMPONENT(TestVectorTool)
DECLARE_COMPONENT(OutputConversionTool)
DECLARE_COMPONENT(xAODContainerMaker)
DECLARE_COMPONENT(FPGAOutputValidationAlg)

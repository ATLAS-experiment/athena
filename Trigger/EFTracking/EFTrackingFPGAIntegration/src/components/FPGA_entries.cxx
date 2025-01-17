// The nominal setup for this package require OPENCL, which is only currently avaliable in docker images
// And not distributed within normal athena setup. However, this tool and alg are not dependant on that
// and are useful to be implemented in FPGATrackSim for FPGA test vector generation

#include "../FPGADataFormatAlg.h"
#include "../FPGADataFormatTool.h"

DECLARE_COMPONENT(FPGADataFormatAlg)
DECLARE_COMPONENT(FPGADataFormatTool)

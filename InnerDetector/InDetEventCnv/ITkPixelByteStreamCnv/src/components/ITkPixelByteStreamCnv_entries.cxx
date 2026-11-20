/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "src/ITkPixelEncodingAlg.h"
#include "src/ITkPixelEncodingTool.h"
#include "src/ITkPixelHitSortingTool.h"
#include "src/ITkPixelDecodingAlg.h"
#include "src/ITkPixelDecodingPhaseIIRDOAlg.h"
#include "src/ITkPixelRawContBytestreamCnv.h"
#include "src/ITkPixelTranslatorAlg.h"
#include "src/ITkPixelCnvTool.h"
#include "src/ITkPixelDataRateMonTool.h"
#include "src/ITkPixelCsvWaferIdAlg.h"
#include "src/PixelRDOValidationAlg.h"
#include "src/PixelPhaseIIRDOValidationAlg.h"
#include "src/PixelPhaseIIRDOVsPhaseIIRDOValidationAlg.h"
#ifdef ITKPIXEL_WITH_CUDA
#include "src/cuda/ITkPixelDecodingTracccCellsCUDAAlg.h"
#endif

//This is a converter - needs special macro
DECLARE_CONVERTER( ITkPixelRawContByteStreamCnv )

//Rest are usual "components"
DECLARE_COMPONENT( ITkPixelEncodingAlg )
DECLARE_COMPONENT( ITkPixelEncodingTool )
DECLARE_COMPONENT( ITkPixelHitSortingTool)
DECLARE_COMPONENT( ITkPixelDecodingAlg )
DECLARE_COMPONENT( ITkPixelDecodingPhaseIIRDOAlg )
DECLARE_COMPONENT( ITkPixelTranslatorAlg )
DECLARE_COMPONENT( ITkPixelCnvTool )
DECLARE_COMPONENT( ITkPixelDataRateMonTool )
DECLARE_COMPONENT( ITkPixelCsvWaferIdAlg )
DECLARE_COMPONENT( PixelRDOValidationAlg )
DECLARE_COMPONENT( PixelPhaseIIRDOValidationAlg )
DECLARE_COMPONENT( PixelPhaseIIRDOVsPhaseIIRDOValidationAlg )
#ifdef ITKPIXEL_WITH_CUDA
DECLARE_COMPONENT( ITkPixelDecodingTracccCellsCUDAAlg )
#endif

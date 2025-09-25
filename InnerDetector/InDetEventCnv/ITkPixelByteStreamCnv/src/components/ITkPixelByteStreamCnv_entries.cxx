/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/ITkPixelEncodingAlg.h"
#include "src/ITkPixelEncodingTool.h"
#include "src/ITkPixelHitSortingTool.h"
#include "src/ITkPixelDecodingAlg.h"
#include "src/ITkPixelRawContBytestreamCnv.h"
#include "src/ITkPixelTranslatorAlg.h"
#include "src/ITkPixelCnvTool.h"

//This is a converter - needs special macro
DECLARE_CONVERTER( ITkPixelRawContByteStreamCnv )

//Rest are usual "components"
DECLARE_COMPONENT( ITkPixelEncodingAlg )
DECLARE_COMPONENT( ITkPixelEncodingTool )
DECLARE_COMPONENT( ITkPixelHitSortingTool)
DECLARE_COMPONENT( ITkPixelDecodingAlg )
DECLARE_COMPONENT( ITkPixelTranslatorAlg )
DECLARE_COMPONENT( ITkPixelCnvTool )
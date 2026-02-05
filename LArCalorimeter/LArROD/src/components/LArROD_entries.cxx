#include "../ReadLArRaw.h"
#include "../ReadLArDigits.h"
#include "../LArRawChannelSimpleBuilder.h"
#include "../LArCalibDigitsAccumulator.h"
#include "../LArDigitsAccumulator.h"
#include "../LArDigitThinner.h"
#include "../LArFebErrorSummaryMaker.h"
#include "../LArCalibDigitsAccumulatorFreeGain.h"
#include "../LArRawChannelBuilderAlg.h"
#include "../LArRawChannelBuilderSCAlg.h"
#include "../LArLATOMEBuilderAlg.h"
#include "../LArRawChannelBuilderIterAlg.h"
#include "../LArNNRawChannelBuilder.h"
#include "../LArOFFCRawChannelBuilder.h"
#include "../LArSCSimpleMaker.h"
#include "../LArSuperCellBCIDEmAlg.h"
#include "../LArSuperCellBCIDAlg.h"
#include "../LArHITtoCell.h"


DECLARE_COMPONENT( ReadLArRaw )
DECLARE_COMPONENT( ReadLArDigits )
DECLARE_COMPONENT( LArRawChannelSimpleBuilder )
DECLARE_COMPONENT( LArCalibDigitsAccumulator )
DECLARE_COMPONENT( LArDigitsAccumulator )
DECLARE_COMPONENT( LArDigitThinner )
DECLARE_COMPONENT( LArFebErrorSummaryMaker )
DECLARE_COMPONENT( LArCalibDigitsAccumulatorFreeGain )
DECLARE_COMPONENT( LArRawChannelBuilderAlg )
DECLARE_COMPONENT( LArRawChannelBuilderSCAlg )
DECLARE_COMPONENT( LArLATOMEBuilderAlg )
DECLARE_COMPONENT( LArRawChannelBuilderIterAlg )
DECLARE_COMPONENT( LArNNRawChannelBuilder )
DECLARE_COMPONENT( LArOFFCRawChannelBuilder )
DECLARE_COMPONENT( LArSCSimpleMaker )
DECLARE_COMPONENT( LArSuperCellBCIDEmAlg )
DECLARE_COMPONENT( LArSuperCellBCIDAlg )
DECLARE_COMPONENT( LArHITtoCell )

#include "../tests/SuperCellVsCaloCellTestAlg.h"
DECLARE_COMPONENT( SuperCellVsCaloCellTestAlg )


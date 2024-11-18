#include "LArCalibUtils/LArAutoCorrMaker.h"
#include "LArCalibUtils/LArAutoCorrExtrapolate.h"
#include "LArCalibUtils/LArAutoCorrDecoderTool.h"
#include "LArCalibUtils/LArCalibDigitMaker.h"
#include "LArCalibUtils/LArCaliWaveAverage.h"
#include "LArCalibUtils/LArCaliWaveBuilder.h"
#include "LArCalibUtils/LArCaliWaveBuilderXtalk.h"
#include "LArCalibUtils/LArCaliWaveSelector.h"
#include "LArCalibUtils/LArDeltaRespPredictor.h"
#include "LArCalibUtils/LArDeltaRespTool.h"
#include "LArCalibUtils/LArDigitOscillationCorrTool.h"
#include "LArCalibUtils/LArMasterWaveBuilder.h"
#include "LArCalibUtils/LArOFCAlg.h"
#include "LArCalibUtils/LArPedestalMaker.h"
#include "LArCalibUtils/LArPhysWaveShifter.h"
#include "LArCalibUtils/LArPhysWaveTool.h"
#include "LArCalibUtils/LArPhysWaveHECTool.h"
#include "../LArRampBuilder.h"
#include "LArCalibUtils/LArRTMParamExtractor.h"
#include "LArCalibUtils/LArStripsCrossTalkCorrector.h"
#include "LArCalibUtils/LArPhysWavePredictor.h"
#include "LArCalibUtils/LArWFParamTool.h"
#include "LArCalibUtils/LArDumpShapes.h"
#include "LArCalibUtils/LArRampCorr.h"
#include "LArCalibUtils/LArAccumulatedCalibDigitContSplitter.h"
#include "LArCalibUtils/LArOFPhaseFill.h"
#include "LArCalibUtils/LArOFPhasePicker.h"
#include "LArCalibUtils/LArHVCorrMaker.h"
#include "LArCalibUtils/LArCalibShortCorrector.h"
#include "LArCalibUtils/LArTimePhysPrediction.h"
#include "LArCalibUtils/LArPedestalAutoCorrBuilder.h"
#include "LArCalibUtils/LArRampAdHocPatchingAlg.h"
#include "LArCalibUtils/LArShapeCorrector.h"
#include "LArCalibUtils/LArAutoCorrAlgToDB.h"
#include "LArCalibUtils/LArDuplicateConstants.h"
#include "LArCalibUtils/LArCalibPatchingAlg.h"
#include "LArCalibUtils/LArCalibCopyAlg.h"
#include "../LArConditionsMergerAlg.h"


using LArRampPatcher = LArCalibPatchingAlg<LArRampComplete>;
using LArCaliWavePatcher = LArCalibPatchingAlg<LArCaliWaveContainer>;
using LArMphysOverMcalPatcher = LArCalibPatchingAlg<LArMphysOverMcalComplete>;
using LArACPatcher = LArCalibPatchingAlg<LArAutoCorrComplete>;

//typedef LArCalibCopyAlg<LArPedestalComplete> LArPedestalCopyAlg;
using LArPhysWaveCopyAlg = LArCalibCopyAlg<LArPhysWaveContainer>;
using LArDAC2uAMCCopyAlg = LArCalibCopyAlg<LArDAC2uAMC>;
using LArTdriftCompleteCopyAlg = LArCalibCopyAlg<LArTdriftComplete>;


DECLARE_COMPONENT( LArAutoCorrMaker )
DECLARE_COMPONENT( LArAutoCorrExtrapolate )
DECLARE_COMPONENT( LArCalibDigitMaker )
DECLARE_COMPONENT( LArCaliWaveAverage )
DECLARE_COMPONENT( LArCaliWaveBuilder )
DECLARE_COMPONENT( LArCaliWaveBuilderXtalk )
DECLARE_COMPONENT( LArCaliWaveSelector )
DECLARE_COMPONENT( LArDeltaRespPredictor )
DECLARE_COMPONENT( LArMasterWaveBuilder )
DECLARE_COMPONENT( LArOFCAlg )
DECLARE_COMPONENT( LArPedestalMaker )
DECLARE_COMPONENT( LArPedestalAutoCorrBuilder )
DECLARE_COMPONENT( LArPhysWaveShifter )
DECLARE_COMPONENT( LArRampBuilder )
DECLARE_COMPONENT( LArRTMParamExtractor )
DECLARE_COMPONENT( LArStripsCrossTalkCorrector )
DECLARE_COMPONENT( LArPhysWavePredictor )
DECLARE_COMPONENT( LArDumpShapes )
DECLARE_COMPONENT( LArRampCorr )
DECLARE_COMPONENT( LArAccumulatedCalibDigitContSplitter )
DECLARE_COMPONENT( LArRampPatcher )
DECLARE_COMPONENT( LArACPatcher )
DECLARE_COMPONENT( LArCaliWavePatcher )
DECLARE_COMPONENT( LArOFPhaseFill )
DECLARE_COMPONENT( LArOFPhasePicker )
DECLARE_COMPONENT( LArHVCorrMaker )
DECLARE_COMPONENT( LArCalibShortCorrector )
DECLARE_COMPONENT( LArTimePhysPrediction )
DECLARE_COMPONENT( LArRampAdHocPatchingAlg )
DECLARE_COMPONENT( LArPhysWaveCopyAlg )
DECLARE_COMPONENT( LArDAC2uAMCCopyAlg )
DECLARE_COMPONENT( LArTdriftCompleteCopyAlg )
DECLARE_COMPONENT( LArShapeCorrector )
DECLARE_COMPONENT( LArAutoCorrAlgToDB )
DECLARE_COMPONENT( LArDuplicateConstants )

DECLARE_COMPONENT( LArAutoCorrDecoderTool )
DECLARE_COMPONENT( LArDigitOscillationCorrTool )
DECLARE_COMPONENT( LArDeltaRespTool )
DECLARE_COMPONENT( LArPhysWaveTool )
DECLARE_COMPONENT( LArPhysWaveHECTool )
DECLARE_COMPONENT( LArWFParamTool )

DECLARE_COMPONENT( LArPedestalMerger )
DECLARE_COMPONENT( LArOFCMerger )
DECLARE_COMPONENT( LArAutoCorrMerger )
DECLARE_COMPONENT( LArMphysOverMcalMerger )
DECLARE_COMPONENT( LArShapeMerger )
DECLARE_COMPONENT( LArRampMerger )
DECLARE_COMPONENT( LArDetCellParamsMerger )
DECLARE_COMPONENT( LArCaliPulseParamsMerger )
DECLARE_COMPONENT( LArPhysWaveMerger )
DECLARE_COMPONENT( LArCaliWaveMerger )

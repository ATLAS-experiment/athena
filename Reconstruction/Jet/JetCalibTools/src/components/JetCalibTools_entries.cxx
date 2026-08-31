#include "JetCalibTools/BJetCorrectionTool.h"
#include "JetCalibTools/EtaJESCalibStep.h"
#include "JetCalibTools/GSCCalibStep.h"
#include "JetCalibTools/Generic4VecCorrectionStep.h"
#include "JetCalibTools/InSituCalibStep.h"
#include "JetCalibTools/InSituJMSCalibStep.h"
#include "JetCalibTools/JetCalibrationTool.h"
#include "JetCalibTools/JetCalibTestAlg.h"
#include "JetCalibTools/JetCalibTool.h"
#include "JetCalibTools/JetResponseTool.h"
#include "JetCalibTools/JMSCalibStep.h"
#include "JetCalibTools/MuonInJetCorrectionTool.h"
#include "JetCalibTools/Pileup1DResidualCalibStep.h"
#include "JetCalibTools/PileupCalibStep.h"
#include "JetCalibTools/SmearingCalibStep.h"

#ifndef XAOD_STANDALONE
#include "CalibratedJetCopyAlg.h"
#include "JetCalibrationDecoratorAlg.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT( BJetCorrectionTool )
DECLARE_COMPONENT( EtaJESCalibStep )
DECLARE_COMPONENT( GSCCalibStep )
DECLARE_COMPONENT( Generic4VecCorrectionStep )
DECLARE_COMPONENT( InSituCalibStep )
DECLARE_COMPONENT( InSituJMSCalibStep )
DECLARE_COMPONENT( JetCalibrationTool )
DECLARE_COMPONENT( JetCalibTestAlg )
DECLARE_COMPONENT( JetCalibTool )
DECLARE_COMPONENT( JetResponseTool )
DECLARE_COMPONENT( JMSCalibStep )
DECLARE_COMPONENT( PileupCalibStep )
DECLARE_COMPONENT( Pileup1DResidualCalibStep )
DECLARE_COMPONENT( MuonInJetCorrectionTool )
DECLARE_COMPONENT( SmearingCalibStep )

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT( CalibratedJetCopyAlg )
DECLARE_COMPONENT( JetCalibrationDecoratorAlg )
#endif

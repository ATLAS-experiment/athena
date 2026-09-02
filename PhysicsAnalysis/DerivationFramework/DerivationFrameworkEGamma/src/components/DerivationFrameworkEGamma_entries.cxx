#include "DerivationFrameworkEGamma/PhotonsDirectionAlg.h"
#include "DerivationFrameworkEGamma/EGInvariantMassTool.h"
#include "DerivationFrameworkEGamma/EGTransverseMassTool.h"
#include "DerivationFrameworkEGamma/EGSelectionToolWrapper.h"
#include "DerivationFrameworkEGamma/EGElectronLikelihoodToolWrapper.h"
#include "DerivationFrameworkEGamma/EGPhotonCleaningWrapper.h"
#include "DerivationFrameworkEGamma/BkgElectronClassification.h"
#include "DerivationFrameworkEGamma/EGElectronAmbiguityAlg.h"
#include "DerivationFrameworkEGamma/PhotonVertexSelectionWrapper.h"
#include "DerivationFrameworkEGamma/EGammaCookieCutClusterAlg.h"
#include "DerivationFrameworkEGamma/EGammaGSFCalo.h"
#include "DerivationFrameworkEGamma/EGammaEnergyCalibrationWrapper.h"
#include "DerivationFrameworkEGamma/EGPhotonBDTToolWrapper.h"
#include "DerivationFrameworkEGamma/EGPhotonBDTToolDecorator.h"
#include "DerivationFrameworkEGamma/EGammaFudgeAlgorithm.h"

using namespace DerivationFramework;
DECLARE_COMPONENT( PhotonsDirectionAlg )
DECLARE_COMPONENT( EGInvariantMassTool )
DECLARE_COMPONENT( EGTransverseMassTool )
DECLARE_COMPONENT( EGSelectionToolWrapper )
DECLARE_COMPONENT( EGElectronLikelihoodToolWrapper )
DECLARE_COMPONENT( EGPhotonCleaningWrapper )
DECLARE_COMPONENT( BkgElectronClassification )
DECLARE_COMPONENT( EGElectronAmbiguityAlg )
DECLARE_COMPONENT( PhotonVertexSelectionWrapper )
DECLARE_COMPONENT( EGammaCookieCutClusterAlg )
DECLARE_COMPONENT( EGammaGSFCalo )
DECLARE_COMPONENT( EGammaEnergyCalibrationWrapper )
DECLARE_COMPONENT( EGPhotonBDTToolWrapper )
DECLARE_COMPONENT( EGPhotonBDTToolDecorator )
DECLARE_COMPONENT( EGammaFudgeAlgorithm )

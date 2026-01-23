#include "DerivationFrameworkMCTruth/TruthDressingTool.h"
#include "DerivationFrameworkMCTruth/TruthIsolationTool.h"
#include "DerivationFrameworkMCTruth/MenuTruthThinning.h"
#include "DerivationFrameworkMCTruth/GenericTruthThinning.h"
#include "DerivationFrameworkMCTruth/TruthCollectionMakerBase.h"
#include "DerivationFrameworkMCTruth/TruthCollectionMakerBoson.h"
#include "DerivationFrameworkMCTruth/TruthCollectionMakerBottom.h"
#include "DerivationFrameworkMCTruth/TruthCollectionMakerBSM.h"
#include "DerivationFrameworkMCTruth/TruthCollectionMakerCharm.h"
#include "DerivationFrameworkMCTruth/TruthCollectionMakerElectron.h"
#include "DerivationFrameworkMCTruth/TruthCollectionMakerForwardProton.h"
#include "DerivationFrameworkMCTruth/TruthCollectionMaker.h"
#include "DerivationFrameworkMCTruth/TruthCollectionMakerMuon.h"
#include "DerivationFrameworkMCTruth/TruthCollectionMakerNeutrino.h"
#include "DerivationFrameworkMCTruth/TruthCollectionMakerPhoton.h"
#include "DerivationFrameworkMCTruth/TruthCollectionMakerPhotonSim.h"
#include "DerivationFrameworkMCTruth/TruthCollectionMakerTau.h"
#include "DerivationFrameworkMCTruth/TruthCollectionMakerTop.h"
#include "DerivationFrameworkMCTruth/TruthClassificationDecorator.h"
#include "DerivationFrameworkMCTruth/CompactHardTruth.h"
#include "DerivationFrameworkMCTruth/HardTruthThinning.h"
#include "DerivationFrameworkMCTruth/HadronOriginDecorator.h"
#include "DerivationFrameworkMCTruth/HadronOriginClassifier.h"
#include "DerivationFrameworkMCTruth/TruthDecayCollectionMaker.h"
#include "src/TruthNavigationDecorator.h"
#include "DerivationFrameworkMCTruth/TruthD2Decorator.h"
#include "DerivationFrameworkMCTruth/TruthQGDecorationTool.h"
#include "src/TruthBornLeptonCollectionMaker.h"
#include "src/TruthLinkRepointTool.h"
#include "DerivationFrameworkMCTruth/TruthPVCollectionMaker.h"
#include "src/GenFilterTool.h"
#include "src/TruthEDDecorator.h"
#include "src/TruthMetaDataWriter.h"
#include "DerivationFrameworkMCTruth/ClassifyAndCalculateHFAugmentation.h"
#include "DerivationFrameworkMCTruth/ClassifyAndCalculateHFTool.h"
#include "DerivationFrameworkMCTruth/HadronOriginClassifier.h"
#include "DerivationFrameworkMCTruth/JetMatchingTool.h"

using namespace DerivationFramework;

DECLARE_COMPONENT( TruthDressingTool )
DECLARE_COMPONENT( TruthIsolationTool )
DECLARE_COMPONENT( MenuTruthThinning )
DECLARE_COMPONENT( GenericTruthThinning )
DECLARE_COMPONENT( TruthCollectionMaker )
DECLARE_COMPONENT( TruthCollectionMakerBase )
DECLARE_COMPONENT( TruthCollectionMakerBoson )
DECLARE_COMPONENT( TruthCollectionMakerBottom )
DECLARE_COMPONENT( TruthCollectionMakerBSM )
DECLARE_COMPONENT( TruthCollectionMakerCharm )
DECLARE_COMPONENT( TruthCollectionMakerElectron )
DECLARE_COMPONENT( TruthCollectionMakerForwardProton )
DECLARE_COMPONENT( TruthCollectionMakerMuon )
DECLARE_COMPONENT( TruthCollectionMakerNeutrino )
DECLARE_COMPONENT( TruthCollectionMakerPhoton )
DECLARE_COMPONENT( TruthCollectionMakerPhotonSim )
DECLARE_COMPONENT( TruthCollectionMakerTau )
DECLARE_COMPONENT( TruthCollectionMakerTop )
DECLARE_COMPONENT( TruthClassificationDecorator )
DECLARE_COMPONENT( CompactHardTruth )
DECLARE_COMPONENT( HardTruthThinning )
DECLARE_COMPONENT( HadronOriginDecorator )
DECLARE_COMPONENT( HadronOriginClassifier )
DECLARE_COMPONENT( TruthDecayCollectionMaker )
DECLARE_COMPONENT( TruthNavigationDecorator )
DECLARE_COMPONENT( TruthD2Decorator )
DECLARE_COMPONENT( TruthQGDecorationTool )
DECLARE_COMPONENT( TruthBornLeptonCollectionMaker )
DECLARE_COMPONENT( TruthLinkRepointTool )
DECLARE_COMPONENT( TruthPVCollectionMaker )
DECLARE_COMPONENT( GenFilterTool )
DECLARE_COMPONENT( TruthEDDecorator )
DECLARE_COMPONENT( TruthMetaDataWriter )
DECLARE_COMPONENT( ClassifyAndCalculateHFAugmentation )
DECLARE_COMPONENT( ClassifyAndCalculateHFTool )
DECLARE_COMPONENT( HadronOriginClassifier )
DECLARE_COMPONENT( JetMatchingTool )

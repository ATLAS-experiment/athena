//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: G4AtlasHadronPhysicsFTFP_BERT_ATL_noDiffraction.cc 83699 2014-09-10 07:18:25Z gcosmo $
//
//---------------------------------------------------------------------------
// Author: Alberto Ribon
// Date:   October 2017
//
// Hadron physics for the new physics list FTFP_BERT_ATL_noDiffraction.
// This is a modified version of the FTFP_BERT_ATL hadron physics for ATLAS,
// which has the target diffraction for hadron-nucleus interactions
// switched off (note that FTFP_BERT_ATL has already the projectile
// diffraction for hadron-nucleus switched off, but the target diffraction
// is switched on).
//----------------------------------------------------------------------------
//
#include <iomanip>   

#include "G4AtlasHadronPhysicsFTFP_BERT_ATL_noDiffraction.hh"

#include "globals.hh"
#include "G4ios.hh"
#include "G4SystemOfUnits.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"

#include "G4MesonConstructor.hh"
#include "G4BaryonConstructor.hh"
#include "G4ShortLivedConstructor.hh"

#include "G4ChipsKaonMinusInelasticXS.hh"
#include "G4ChipsKaonPlusInelasticXS.hh"
#include "G4ChipsKaonZeroInelasticXS.hh"
#include "G4CrossSectionDataSetRegistry.hh"

#include "G4NeutronRadCapture.hh"
#include "G4NeutronInelasticXS.hh"
#include "G4NeutronCaptureXS.hh"

#include "G4ProcessManager.hh"
#include "G4BGGNucleonInelasticXS.hh"
#if G4VERSION_NUMBER < 1100
#include "G4PiNuclearCrossSection.hh"
#include "G4CrossSectionPairGG.hh"
#else
#include "G4BGGPionInelasticXS.hh"
#endif	
#include "G4ComponentAntiNuclNuclearXS.hh"
#include "G4CrossSectionInelastic.hh"

#include "G4PhysListUtil.hh"

// factory
#include "G4PhysicsConstructorFactory.hh"
//
G4_DECLARE_PHYSCONSTR_FACTORY(G4AtlasHadronPhysicsFTFP_BERT_ATL_noDiffraction);

G4AtlasHadronPhysicsFTFP_BERT_ATL_noDiffraction::G4AtlasHadronPhysicsFTFP_BERT_ATL_noDiffraction(G4int)
    :  G4VPhysicsConstructor("hInelastic FTFP_BERT_ATL_noDiffraction")
    , theNeutronCaptureModel(0)
    , theHandler(0)
    , theModel1(0)
    , theModel2(0)
    , theModel3(0)
    , theBertini1(0)
    , theBertini2(0)
    , theNeutronCaptureProcess(0)
    , theNeutronInelastic(0)
    , theProtonInelastic(0)
    , thePionMinusInelastic(0)
    , thePionPlusInelastic(0)
    , theKaonMinusInelastic(0)
    , theKaonPlusInelastic(0)
    , theKaonZeroLInelastic(0)
    , theKaonZeroSInelastic(0)
    , theLambdaInelastic(0)
    , theAntiLambdaInelastic(0)
    , theSigmaMinusInelastic(0)
    , theAntiSigmaMinusInelastic(0)
    , theSigmaPlusInelastic(0)
    , theAntiSigmaPlusInelastic(0)
    , theXiZeroInelastic(0)
    , theAntiXiZeroInelastic(0)
    , theXiMinusInelastic(0)
    , theAntiXiMinusInelastic(0)
    , theOmegaMinusInelastic(0)
    , theAntiOmegaMinusInelastic(0)
    , theAntiProtonInelastic(0)
    , theAntiNeutronInelastic(0)
    , theAntiDeuteronInelastic(0)
    , theAntiTritonInelastic(0)
    , theAntiHe3Inelastic(0)
    , theAntiAlphaInelastic(0)
    , thePiPlusXS(0)
    , thePiMinusXS(0)
    , theChipsHyperonInelasticXS(0)
    , theAntiNucleonXS(0)
    , theChipsKaonMinusXS(0)
    , theChipsKaonPlusXS(0)
    , theChipsKaonZeroXS(0)
    , theNeutronInelasticXS(0)
    , theNeutronCaptureXS(0)
{}

G4AtlasHadronPhysicsFTFP_BERT_ATL_noDiffraction::G4AtlasHadronPhysicsFTFP_BERT_ATL_noDiffraction(const G4String& name, G4bool /*quasiElastic*/)
    :  G4VPhysicsConstructor(name) 
    , theNeutronCaptureModel(0)
    , theHandler(0)
    , theModel1(0)
    , theModel2(0)
    , theModel3(0)
    , theBertini1(0)
    , theBertini2(0)
    , theNeutronCaptureProcess(0)
    , theNeutronInelastic(0)
    , theProtonInelastic(0)
    , thePionMinusInelastic(0)
    , thePionPlusInelastic(0)
    , theKaonMinusInelastic(0)
    , theKaonPlusInelastic(0)
    , theKaonZeroLInelastic(0)
    , theKaonZeroSInelastic(0)
    , theLambdaInelastic(0)
    , theAntiLambdaInelastic(0)
    , theSigmaMinusInelastic(0)
    , theAntiSigmaMinusInelastic(0)
    , theSigmaPlusInelastic(0)
    , theAntiSigmaPlusInelastic(0)
    , theXiZeroInelastic(0)
    , theAntiXiZeroInelastic(0)
    , theXiMinusInelastic(0)
    , theAntiXiMinusInelastic(0)
    , theOmegaMinusInelastic(0)
    , theAntiOmegaMinusInelastic(0)
    , theAntiProtonInelastic(0)
    , theAntiNeutronInelastic(0)
    , theAntiDeuteronInelastic(0)
    , theAntiTritonInelastic(0)
    , theAntiHe3Inelastic(0)
    , theAntiAlphaInelastic(0)
    , thePiPlusXS(0)
    , thePiMinusXS(0)
    , theChipsHyperonInelasticXS(0)
    , theAntiNucleonXS(0)
    , theChipsKaonMinusXS(0)
    , theChipsKaonPlusXS(0)
    , theChipsKaonZeroXS(0)
    , theNeutronInelasticXS(0)
    , theNeutronCaptureXS(0)
{}

void G4AtlasHadronPhysicsFTFP_BERT_ATL_noDiffraction::CreateModels()
{

  G4double minFTFP =  9.0 * GeV;
  G4double maxBERT = 12.0 * GeV;
  G4cout << " FTFP_BERT_ATL_noDiffraction : similar to FTFP_BERT_ATL but with" << G4endl
         << " target diffraction for hadron-nucleus interaction switched off." << G4endl;

  theStringModel = std::make_unique<G4FTFModel2>();

  //***********************************
  theStringModel->TurnOffDiffraction();  // Switch off projectile and target diffraction
  //***********************************

  theLund = std::make_unique<G4LundStringFragmentation>();
  theStringDecay = std::make_unique<G4ExcitedStringDecay>( theLund.get() );
  theStringModel->SetFragmentationModel( theStringDecay.get() );
  thePreEquilib = std::make_unique<G4PreCompoundModel>( theHandler = new G4ExcitationHandler );
  theCascade = std::make_unique<G4GeneratorPrecompoundInterface>( thePreEquilib.get() );

  // FTF for neutrons, protons, pions, and kaons
  theModel1 = new G4TheoFSGenerator( "FTFP" );
  theModel1->SetMinEnergy( minFTFP );
  theModel1->SetMaxEnergy( 100.0*TeV );
  theModel1->SetTransport( theCascade.get() );
  theModel1->SetHighEnergyGenerator( theStringModel.get() );
 
  // BERT for neutrons, protons, pions, and kaons
  theBertini1 = new G4CascadeInterface;
  theBertini1->SetMinEnergy( 0.0*GeV );
  theBertini1->SetMaxEnergy( maxBERT );

  // FTF for hyperons
  theModel2 = new G4TheoFSGenerator( "FTFP" );
  theModel2->SetMinEnergy( 2.0*GeV );
  theModel2->SetMaxEnergy( 100.0*TeV );
  theModel2->SetTransport( theCascade.get() );
  theModel2->SetHighEnergyGenerator( theStringModel.get() );
  
  // BERT for hyperons
  theBertini2 = new G4CascadeInterface;
  theBertini2->SetMinEnergy( 0.0*GeV );
  theBertini2->SetMaxEnergy( 6.0*GeV );

  // FTF for Antibaryons  
  theModel3 = new G4TheoFSGenerator( "FTFP" );
  theModel3->SetMinEnergy( 0.0*GeV );
  theModel3->SetMaxEnergy( 100.0*TeV );
  theModel3->SetTransport( theCascade.get() );
  theModel3->SetHighEnergyGenerator( theStringModel.get() );

  // Neutron Capture
  theNeutronCaptureModel = new G4NeutronRadCapture;
  theNeutronCaptureModel->SetMinEnergy( 0.0 );
  theNeutronCaptureModel->SetMaxEnergy( 100.0*TeV );

  // Cross sections
#if G4VERSION_NUMBER < 1100
  thePiPlusXS = new G4CrossSectionPairGG( new G4PiNuclearCrossSection(), 91*GeV );
  thePiMinusXS = thePiPlusXS;
#else
  thePiPlusXS = new G4BGGPionInelasticXS( G4PionPlus::Definition() );
  thePiMinusXS = new G4BGGPionInelasticXS( G4PionMinus::Definition() );
#endif	
  theAntiNucleonXS = new G4CrossSectionInelastic( new G4ComponentAntiNuclNuclearXS() );
  theChipsHyperonInelasticXS = G4CrossSectionDataSetRegistry::Instance()->GetCrossSectionDataSet( G4ChipsHyperonInelasticXS::Default_Name() );
  theChipsKaonMinusXS        = G4CrossSectionDataSetRegistry::Instance()->GetCrossSectionDataSet( G4ChipsKaonMinusInelasticXS::Default_Name() );
  theChipsKaonPlusXS         = G4CrossSectionDataSetRegistry::Instance()->GetCrossSectionDataSet( G4ChipsKaonPlusInelasticXS::Default_Name() );
  theChipsKaonZeroXS         = G4CrossSectionDataSetRegistry::Instance()->GetCrossSectionDataSet( G4ChipsKaonZeroInelasticXS::Default_Name() );
  theNeutronInelasticXS      = G4CrossSectionDataSetRegistry::Instance()->GetCrossSectionDataSet( G4NeutronInelasticXS::Default_Name() );
  theNeutronCaptureXS        = G4CrossSectionDataSetRegistry::Instance()->GetCrossSectionDataSet( G4NeutronCaptureXS::Default_Name() );
}

G4AtlasHadronPhysicsFTFP_BERT_ATL_noDiffraction::~G4AtlasHadronPhysicsFTFP_BERT_ATL_noDiffraction()
{
}

void G4AtlasHadronPhysicsFTFP_BERT_ATL_noDiffraction::ConstructParticle()
{
  G4MesonConstructor pMesonConstructor;
  pMesonConstructor.ConstructParticle();

  G4BaryonConstructor pBaryonConstructor;
  pBaryonConstructor.ConstructParticle();

  G4ShortLivedConstructor pShortLivedConstructor;
  pShortLivedConstructor.ConstructParticle();  
}

void G4AtlasHadronPhysicsFTFP_BERT_ATL_noDiffraction::ConstructProcess()
{
  CreateModels();

  G4ProcessManager * aProcMan = 0;

#if G4VERSION_NUMBER < 1100
  theNeutronInelastic = new G4NeutronInelasticProcess();
#else
  theNeutronInelastic = new G4HadronInelasticProcess( "NeutronInelastic", G4Neutron::Definition() );
#endif  
  theNeutronInelastic->RegisterMe( theModel1 );
  theNeutronInelastic->RegisterMe( theBertini1 );
  theNeutronInelastic->AddDataSet( new G4BGGNucleonInelasticXS( G4Neutron::Neutron() ) );
  theNeutronInelastic->AddDataSet( theNeutronInelasticXS );
  aProcMan = G4Neutron::Neutron()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theNeutronInelastic );

#if G4VERSION_NUMBER < 1100
  theNeutronCaptureProcess = new G4HadronCaptureProcess();
#else
  theNeutronCaptureProcess = new G4NeutronCaptureProcess();
#endif
  theNeutronCaptureProcess->RegisterMe( theNeutronCaptureModel );
  theNeutronCaptureProcess->AddDataSet( theNeutronCaptureXS );
  aProcMan->AddDiscreteProcess( theNeutronCaptureProcess );

#if G4VERSION_NUMBER < 1100
  theProtonInelastic = new G4ProtonInelasticProcess();
#else
  theProtonInelastic = new G4HadronInelasticProcess( "ProtonInelastic", G4Proton::Definition() );
#endif
  theProtonInelastic->RegisterMe( theModel1 );
  theProtonInelastic->RegisterMe( theBertini1 );
  theProtonInelastic->AddDataSet( new G4BGGNucleonInelasticXS( G4Proton::Proton() ) );
  aProcMan = G4Proton::Proton()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theProtonInelastic );

#if G4VERSION_NUMBER < 1100
  thePionMinusInelastic = new G4PionMinusInelasticProcess();
#else
  thePionMinusInelastic = new G4HadronInelasticProcess( "PionMinusInelastic", G4PionMinus::Definition() );
#endif
  thePionMinusInelastic->RegisterMe( theModel1 );
  thePionMinusInelastic->RegisterMe( theBertini1 );
  thePionMinusInelastic->AddDataSet( thePiMinusXS );
  aProcMan = G4PionMinus::PionMinus()->GetProcessManager();
  aProcMan->AddDiscreteProcess( thePionMinusInelastic );

#if G4VERSION_NUMBER < 1100
  thePionPlusInelastic = new G4PionPlusInelasticProcess();
#else
  thePionPlusInelastic = new G4HadronInelasticProcess( "PionPlusInelastic", G4PionPlus::Definition() );
#endif
  thePionPlusInelastic->RegisterMe( theModel1 );
  thePionPlusInelastic->RegisterMe( theBertini1 );
  thePionPlusInelastic->AddDataSet( thePiPlusXS );
  aProcMan = G4PionPlus::PionPlus()->GetProcessManager();
  aProcMan->AddDiscreteProcess( thePionPlusInelastic );

#if G4VERSION_NUMBER < 1100
  theKaonMinusInelastic = new G4KaonMinusInelasticProcess();
#else
  theKaonMinusInelastic = new G4HadronInelasticProcess( "KaonMinusInelastic", G4KaonMinus::Definition() );
#endif
  theKaonMinusInelastic->RegisterMe( theModel1 );
  theKaonMinusInelastic->RegisterMe( theBertini1 );
  theKaonMinusInelastic->AddDataSet( theChipsKaonMinusXS );
  aProcMan = G4KaonMinus::KaonMinus()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theKaonMinusInelastic );

#if G4VERSION_NUMBER < 1100
  theKaonPlusInelastic = new G4KaonPlusInelasticProcess();
#else
  theKaonPlusInelastic = new G4HadronInelasticProcess( "KaonPlusInelastic", G4KaonPlus::Definition() );
#endif
  theKaonPlusInelastic->RegisterMe( theModel1 );
  theKaonPlusInelastic->RegisterMe( theBertini1 );
  theKaonPlusInelastic->AddDataSet( theChipsKaonPlusXS );
  aProcMan = G4KaonPlus::KaonPlus()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theKaonPlusInelastic );

#if G4VERSION_NUMBER < 1100
  theKaonZeroLInelastic = new G4KaonZeroLInelasticProcess();
#else
  theKaonZeroLInelastic = new G4HadronInelasticProcess( "KaonZeroLInelastic", G4KaonZeroLong::Definition() );
#endif
  theKaonZeroLInelastic->RegisterMe( theModel1 );
  theKaonZeroLInelastic->RegisterMe( theBertini1 );
  theKaonZeroLInelastic->AddDataSet( theChipsKaonZeroXS );
  aProcMan = G4KaonZeroLong::KaonZeroLong()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theKaonZeroLInelastic );

#if G4VERSION_NUMBER < 1100
  theKaonZeroSInelastic = new G4KaonZeroSInelasticProcess();
#else
  theKaonZeroSInelastic = new G4HadronInelasticProcess( "KaonZeroSInelastic", G4KaonZeroShort::Definition() );
#endif
  theKaonZeroSInelastic->RegisterMe( theModel1 );
  theKaonZeroSInelastic->RegisterMe( theBertini1 );
  theKaonZeroSInelastic->AddDataSet( theChipsKaonZeroXS );
  aProcMan = G4KaonZeroShort::KaonZeroShort()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theKaonZeroSInelastic );

#if G4VERSION_NUMBER < 1100
  theLambdaInelastic = new G4LambdaInelasticProcess();
#else
  theLambdaInelastic = new G4HadronInelasticProcess( "LambdaInelastic", G4Lambda::Definition() );
#endif
  theLambdaInelastic->RegisterMe( theModel2 );
  theLambdaInelastic->RegisterMe( theBertini2 );
  theLambdaInelastic->AddDataSet( theChipsHyperonInelasticXS );
  aProcMan = G4Lambda::Lambda()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theLambdaInelastic );

#if G4VERSION_NUMBER < 1100
  theAntiLambdaInelastic = new G4AntiLambdaInelasticProcess();
#else
  theAntiLambdaInelastic = new G4HadronInelasticProcess( "AntiLambdaInelastic", G4AntiLambda::Definition() );
#endif
  theAntiLambdaInelastic->RegisterMe( theModel3 );
  theAntiLambdaInelastic->AddDataSet( theChipsHyperonInelasticXS );
  aProcMan = G4AntiLambda::AntiLambda()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theAntiLambdaInelastic );

#if G4VERSION_NUMBER < 1100
  theSigmaMinusInelastic = new G4SigmaMinusInelasticProcess();
#else
  theSigmaMinusInelastic = new G4HadronInelasticProcess( "SigmaMinusInelastic", G4SigmaMinus::Definition() );
#endif
  theSigmaMinusInelastic->RegisterMe( theModel2 );
  theSigmaMinusInelastic->RegisterMe( theBertini2 );
  theSigmaMinusInelastic->AddDataSet( theChipsHyperonInelasticXS );
  aProcMan = G4SigmaMinus::SigmaMinus()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theSigmaMinusInelastic );

#if G4VERSION_NUMBER < 1100
  theAntiSigmaMinusInelastic = new G4AntiSigmaMinusInelasticProcess();
#else
  theAntiSigmaMinusInelastic = new G4HadronInelasticProcess( "AntiSigmaMinusInelastic", G4AntiSigmaMinus::Definition() );
#endif
  theAntiSigmaMinusInelastic->RegisterMe( theModel3 );
  theAntiSigmaMinusInelastic->AddDataSet( theChipsHyperonInelasticXS );
  aProcMan = G4AntiSigmaMinus::AntiSigmaMinus()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theAntiSigmaMinusInelastic );

#if G4VERSION_NUMBER < 1100
  theSigmaPlusInelastic = new G4SigmaPlusInelasticProcess();
#else
  theSigmaPlusInelastic = new G4HadronInelasticProcess( "SigmaPlusInelastic", G4SigmaPlus::Definition() );
#endif
  theSigmaPlusInelastic->RegisterMe( theModel2 );
  theSigmaPlusInelastic->RegisterMe( theBertini2 );
  theSigmaPlusInelastic->AddDataSet( theChipsHyperonInelasticXS );
  aProcMan = G4SigmaPlus::SigmaPlus()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theSigmaPlusInelastic );

#if G4VERSION_NUMBER < 1100
  theAntiSigmaPlusInelastic = new G4AntiSigmaPlusInelasticProcess();
#else
  theAntiSigmaPlusInelastic = new G4HadronInelasticProcess( "AntiSigmaPlusInelastic", G4AntiSigmaPlus::Definition() );
#endif
  theAntiSigmaPlusInelastic->RegisterMe( theModel3 );
  theAntiSigmaPlusInelastic->AddDataSet( theChipsHyperonInelasticXS );
  aProcMan = G4AntiSigmaPlus::AntiSigmaPlus()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theAntiSigmaPlusInelastic );

#if G4VERSION_NUMBER < 1100
  theXiMinusInelastic = new G4XiMinusInelasticProcess();
#else
  theXiMinusInelastic = new G4HadronInelasticProcess( "XiMinusInelastic", G4XiMinus::Definition() );
#endif
  theXiMinusInelastic->RegisterMe( theModel2 );
  theXiMinusInelastic->RegisterMe( theBertini2 );
  theXiMinusInelastic->AddDataSet( theChipsHyperonInelasticXS );
  aProcMan = G4XiMinus::XiMinus()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theXiMinusInelastic );

#if G4VERSION_NUMBER < 1100
  theAntiXiMinusInelastic = new G4AntiXiMinusInelasticProcess();
#else
  theAntiXiMinusInelastic = new G4HadronInelasticProcess( "AntiXiMinusInelastic", G4AntiXiMinus::Definition() );
#endif
  theAntiXiMinusInelastic->RegisterMe( theModel3 );
  theAntiXiMinusInelastic->AddDataSet( theChipsHyperonInelasticXS );
  aProcMan = G4AntiXiMinus::AntiXiMinus()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theAntiXiMinusInelastic );

#if G4VERSION_NUMBER < 1100
  theXiZeroInelastic = new G4XiZeroInelasticProcess();
#else
  theXiZeroInelastic = new G4HadronInelasticProcess( "XiZeroInelastic", G4XiZero::Definition() );
#endif
  theXiZeroInelastic->RegisterMe( theModel2 );
  theXiZeroInelastic->RegisterMe( theBertini2 );
  theXiZeroInelastic->AddDataSet( theChipsHyperonInelasticXS );
  aProcMan = G4XiZero::XiZero()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theXiZeroInelastic );

#if G4VERSION_NUMBER < 1100
  theAntiXiZeroInelastic = new G4AntiXiZeroInelasticProcess();
#else
  theAntiXiZeroInelastic = new G4HadronInelasticProcess( "AntiXiZeroInelastic", G4AntiXiZero::Definition() );
#endif
  theAntiXiZeroInelastic->RegisterMe( theModel3 );
  theAntiXiZeroInelastic->AddDataSet( theChipsHyperonInelasticXS );
  aProcMan = G4AntiXiZero::AntiXiZero()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theAntiXiZeroInelastic );

#if G4VERSION_NUMBER < 1100
  theOmegaMinusInelastic = new G4OmegaMinusInelasticProcess();
#else
  theOmegaMinusInelastic = new G4HadronInelasticProcess( "OmegaMinusInelastic", G4OmegaMinus::Definition() );
#endif
  theOmegaMinusInelastic->RegisterMe( theModel2 );
  theOmegaMinusInelastic->RegisterMe( theBertini2 );
  theOmegaMinusInelastic->AddDataSet( theChipsHyperonInelasticXS );
  aProcMan = G4OmegaMinus::OmegaMinus()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theOmegaMinusInelastic );

#if G4VERSION_NUMBER < 1100
  theAntiOmegaMinusInelastic = new G4AntiOmegaMinusInelasticProcess();
#else
  theAntiOmegaMinusInelastic = new G4HadronInelasticProcess( "AntiOmegaMinusInelastic", G4AntiOmegaMinus::Definition() );
#endif
  theAntiOmegaMinusInelastic->RegisterMe( theModel3 );
  theAntiOmegaMinusInelastic->AddDataSet( theChipsHyperonInelasticXS );
  aProcMan = G4AntiOmegaMinus::AntiOmegaMinus()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theAntiOmegaMinusInelastic );

#if G4VERSION_NUMBER < 1100
  theAntiProtonInelastic = new G4AntiProtonInelasticProcess();
#else
  theAntiProtonInelastic = new G4HadronInelasticProcess( "AntiProtonInelastic", G4AntiProton::Definition() );
#endif
  theAntiProtonInelastic->RegisterMe( theModel3 );
  theAntiProtonInelastic->AddDataSet( theAntiNucleonXS );
  aProcMan = G4AntiProton::AntiProton()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theAntiProtonInelastic );

#if G4VERSION_NUMBER < 1100
  theAntiNeutronInelastic = new G4AntiNeutronInelasticProcess();
#else
  theAntiNeutronInelastic = new G4HadronInelasticProcess( "AntiNeutronInelastic", G4AntiNeutron::Definition() );
#endif
  theAntiNeutronInelastic->RegisterMe( theModel3 );
  theAntiNeutronInelastic->AddDataSet( theAntiNucleonXS );
  aProcMan = G4AntiNeutron::AntiNeutron()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theAntiNeutronInelastic );

#if G4VERSION_NUMBER < 1100
  theAntiDeuteronInelastic = new G4AntiDeuteronInelasticProcess();
#else
  theAntiDeuteronInelastic = new G4HadronInelasticProcess( "AntiDeuteronInelastic", G4AntiDeuteron::Definition() );
#endif
  theAntiDeuteronInelastic->RegisterMe( theModel3 );
  theAntiDeuteronInelastic->AddDataSet( theAntiNucleonXS );
  aProcMan = G4AntiDeuteron::AntiDeuteron()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theAntiDeuteronInelastic );

#if G4VERSION_NUMBER < 1100
  theAntiTritonInelastic = new G4AntiTritonInelasticProcess();
#else
  theAntiTritonInelastic = new G4HadronInelasticProcess( "AntiTritonInelastic", G4AntiTriton::Definition() );
#endif
  theAntiTritonInelastic->RegisterMe( theModel3 );
  theAntiTritonInelastic->AddDataSet( theAntiNucleonXS );
  aProcMan = G4AntiTriton::AntiTriton()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theAntiTritonInelastic );

#if G4VERSION_NUMBER < 1100
  theAntiHe3Inelastic = new G4AntiHe3InelasticProcess();
#else
  theAntiHe3Inelastic = new G4HadronInelasticProcess( "AntiHe3Inelastic", G4AntiHe3::Definition() );
#endif
  theAntiHe3Inelastic->RegisterMe( theModel3 );
  theAntiHe3Inelastic->AddDataSet( theAntiNucleonXS );
  aProcMan = G4AntiHe3::AntiHe3()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theAntiHe3Inelastic );

#if G4VERSION_NUMBER < 1100
  theAntiAlphaInelastic = new G4AntiAlphaInelasticProcess();
#else
  theAntiAlphaInelastic = new G4HadronInelasticProcess( "AntiAlphaInelastic", G4AntiAlpha::Definition() );
#endif
  theAntiAlphaInelastic->RegisterMe( theModel3 );
  theAntiAlphaInelastic->AddDataSet( theAntiNucleonXS );
  aProcMan = G4AntiAlpha::AntiAlpha()->GetProcessManager();
  aProcMan->AddDiscreteProcess( theAntiAlphaInelastic );
}

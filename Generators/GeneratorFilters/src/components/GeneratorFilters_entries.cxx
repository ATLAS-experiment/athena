// xAOD filters accepted for 22.6
#include "GeneratorFilters/QCDTruthJetFilter.h"
#include "GeneratorFilters/xAODBSignalFilter.h"
#include "GeneratorFilters/xAODChargedTracksFilter.h"
#include "GeneratorFilters/xAODChargedTracksWeightFilter.h"
#include "GeneratorFilters/xAODDecaysFinalStateFilter.h"
#include "GeneratorFilters/xAODDecayTimeFilter.h"
#include "GeneratorFilters/xAODDiLeptonMassFilter.h"
#include "GeneratorFilters/xAODDirectPhotonFilter.h"
#include "GeneratorFilters/xAODElectronFilter.h"
#include "GeneratorFilters/xAODForwardProtonFilter.h"
#include "GeneratorFilters/xAODFourLeptonMassFilter.h"
#include "GeneratorFilters/xAODHeavyFlavorHadronFilter.h"
#include "GeneratorFilters/xAODHTFilter.h"
#include "GeneratorFilters/xAODJetFilter.h"
#include "GeneratorFilters/xAODLeptonFilter.h"
#include "GeneratorFilters/xAODLeptonPairFilter.h"
#include "GeneratorFilters/xAODM4MuIntervalFilter.h"
#include "GeneratorFilters/xAODMETFilter.h"
#include "GeneratorFilters/xAODMuDstarFilter.h"
#include "GeneratorFilters/xAODMultiBjetFilter.h"
#include "GeneratorFilters/xAODMultiCjetFilter.h"
#include "GeneratorFilters/xAODMultiElecMuTauFilter.h"
#include "GeneratorFilters/xAODMultiElectronFilter.h"
#include "GeneratorFilters/xAODMultiLeptonFilter.h"
#include "GeneratorFilters/xAODMultiMuonFilter.h"
#include "GeneratorFilters/xAODMuonFilter.h"
#include "GeneratorFilters/xAODParentChildFilter.h"
#include "GeneratorFilters/xAODParentTwoChildrenFilter.h"
#include "GeneratorFilters/xAODParticleFilter.h"
#include "GeneratorFilters/xAODParticleDecayFilter.h"
#include "GeneratorFilters/xAODPhotonFilter.h"
#include "GeneratorFilters/xAODSameParticleHardScatteringFilter.h"
#include "GeneratorFilters/xAODTauFilter.h"
#include "GeneratorFilters/xAODTTbarWithJpsimumuFilter.h"
#include "GeneratorFilters/xAODTTbarWToLeptonFilter.h"
#include "GeneratorFilters/xAODVBFForwardJetsFilter.h"
#include "GeneratorFilters/xAODVBFMjjIntervalFilter.h"
#include "GeneratorFilters/xAODXtoVVDecayFilterExtended.h"
#include "GeneratorFilters/xAODSplitPhotonFilter.h"
#include "GeneratorFilters/xAODDiPhotonFilter.h"

// slimmers for 22.6
#include "GeneratorFilters/PileupTruthParticleSlimmer.h"
#include "GeneratorFilters/xAODTruthParticleSlimmerElectron.h"
#include "GeneratorFilters/xAODTruthParticleSlimmerGen.h" 
#include "GeneratorFilters/xAODTruthParticleSlimmerLightLepton.h"
#include "GeneratorFilters/xAODTruthParticleSlimmerMET.h"
#include "GeneratorFilters/xAODTruthParticleSlimmerMuon.h"
#include "GeneratorFilters/xAODTruthParticleSlimmerPhoton.h"
#include "GeneratorFilters/xAODTruthParticleSlimmerTau.h"

// old format of filters accepted for 22.6
#include "GeneratorFilters/BSignalFilter.h"
#include "GeneratorFilters/ChargedTracksFilter.h"
#include "GeneratorFilters/ChargedTracksWeightFilter.h"
#include "GeneratorFilters/DecaysFinalStateFilter.h"
#include "GeneratorFilters/DecayTimeFilter.h"
#include "GeneratorFilters/DiLeptonMassFilter.h"
#include "GeneratorFilters/DirectPhotonFilter.h"
#include "GeneratorFilters/ElectronFilter.h"
#include "GeneratorFilters/ForwardProtonFilter.h"
#include "GeneratorFilters/FourLeptonMassFilter.h"  
#include "GeneratorFilters/HeavyFlavorHadronFilter.h"
#include "GeneratorFilters/JetFilter.h"
#include "GeneratorFilters/LeptonFilter.h" 
#include "GeneratorFilters/LeptonPairFilter.h"
#include "GeneratorFilters/M4MuIntervalFilter.h"
#include "GeneratorFilters/MissingEtFilter.h"
#include "GeneratorFilters/MultiBjetFilter.h"
#include "GeneratorFilters/MultiCjetFilter.h"
#include "GeneratorFilters/MultiElecMuTauFilter.h"
#include "GeneratorFilters/MultiLeptonFilter.h"
#include "GeneratorFilters/MultiMuonFilter.h"
#include "GeneratorFilters/MuonFilter.h"
#include "GeneratorFilters/ParentChildFilter.h"
#include "GeneratorFilters/ParentTwoChildrenFilter.h"
#include "GeneratorFilters/PhotonFilter.h"
#include "GeneratorFilters/SameParticleHardScatteringFilter.h"
#include "GeneratorFilters/SplitPhotonFilter.h"
#include "GeneratorFilters/TauFilter.h"
#include "GeneratorFilters/TTbarWToLeptonFilter.h"
#include "GeneratorFilters/TTbarWithJpsimumuFilter.h"
#include "GeneratorFilters/VBFForwardJetsFilter.h"
#include "GeneratorFilters/VBFMjjIntervalFilter.h"
#include "GeneratorFilters/XtoVVDecayFilterExtended.h"

// other
#include "GeneratorFilters/BoostedHadTopAndTopPair.h"
#include "GeneratorFilters/DiBjetFilter.h"
#include "GeneratorFilters/DiPhotonFilter.h"
#include "GeneratorFilters/FourLeptonInvMassFilter.h"
#include "GeneratorFilters/HiggsFilter.h" 
#include "GeneratorFilters/HTFilter.h"
#include "GeneratorFilters/MassRangeFilter.h"
#include "GeneratorFilters/MuDstarFilter.h"
#include "GeneratorFilters/MultiElectronFilter.h"
#include "GeneratorFilters/MultiParticleFilter.h"
#include "GeneratorFilters/ParentChildwStatusFilter.h"
#include "GeneratorFilters/ParticleDecayFilter.h"
#include "GeneratorFilters/ParticleFilter.h"
#include "GeneratorFilters/QCDTruthMultiJetFilter.h"
#include "GeneratorFilters/TTbarMassFilter.h"
#include "GeneratorFilters/TTbarPlusHeavyFlavorFilter.h"
#include "GeneratorFilters/TrimuMassRangeFilter.h"
#include "GeneratorFilters/TruthJetFilter.h"
#include "GeneratorFilters/VBFHbbEtaSortingFilter.h"  
#include "GeneratorFilters/ZtoLeptonFilter.h"

//filters accepted for 22.6 (xAOD format)
DECLARE_COMPONENT( QCDTruthJetFilter )
DECLARE_COMPONENT( xAODBSignalFilter )
DECLARE_COMPONENT( xAODChargedTracksFilter )
DECLARE_COMPONENT( xAODChargedTracksWeightFilter )
DECLARE_COMPONENT( xAODDecaysFinalStateFilter )
DECLARE_COMPONENT( xAODDecayTimeFilter )
DECLARE_COMPONENT( xAODDiLeptonMassFilter )
DECLARE_COMPONENT( xAODDirectPhotonFilter )
DECLARE_COMPONENT( xAODElectronFilter )
DECLARE_COMPONENT( xAODForwardProtonFilter )
DECLARE_COMPONENT( xAODFourLeptonMassFilter)
DECLARE_COMPONENT( xAODHeavyFlavorHadronFilter )
DECLARE_COMPONENT( xAODHTFilter )     
DECLARE_COMPONENT( xAODJetFilter)
DECLARE_COMPONENT( xAODLeptonFilter ) 
DECLARE_COMPONENT( xAODLeptonPairFilter )
DECLARE_COMPONENT( xAODM4MuIntervalFilter )
DECLARE_COMPONENT( xAODMETFilter)
DECLARE_COMPONENT( xAODMuDstarFilter)
DECLARE_COMPONENT( xAODMultiBjetFilter )
DECLARE_COMPONENT( xAODMultiCjetFilter )
DECLARE_COMPONENT( xAODMultiElecMuTauFilter )
DECLARE_COMPONENT( xAODMultiElectronFilter )
DECLARE_COMPONENT( xAODMultiLeptonFilter)
DECLARE_COMPONENT( xAODMultiMuonFilter)
DECLARE_COMPONENT( xAODMuonFilter )
DECLARE_COMPONENT( xAODParentChildFilter )
DECLARE_COMPONENT( xAODParentTwoChildrenFilter )
DECLARE_COMPONENT( xAODParticleFilter)
DECLARE_COMPONENT( xAODParticleDecayFilter)
DECLARE_COMPONENT( xAODPhotonFilter )
DECLARE_COMPONENT( xAODSameParticleHardScatteringFilter )
DECLARE_COMPONENT( xAODTauFilter )
DECLARE_COMPONENT( xAODTTbarWithJpsimumuFilter )
DECLARE_COMPONENT( xAODTTbarWToLeptonFilter )
DECLARE_COMPONENT( xAODVBFForwardJetsFilter )
DECLARE_COMPONENT( xAODVBFMjjIntervalFilter )
DECLARE_COMPONENT( xAODXtoVVDecayFilterExtended )
DECLARE_COMPONENT( xAODSplitPhotonFilter )
DECLARE_COMPONENT( xAODDiPhotonFilter )
//slimmers accepted for 22.6
DECLARE_COMPONENT( PileupTruthParticleSlimmer )
DECLARE_COMPONENT( xAODTruthParticleSlimmerElectron )
DECLARE_COMPONENT( xAODTruthParticleSlimmerGen)  
DECLARE_COMPONENT( xAODTruthParticleSlimmerLightLepton )
DECLARE_COMPONENT( xAODTruthParticleSlimmerMET)
DECLARE_COMPONENT( xAODTruthParticleSlimmerMuon )
DECLARE_COMPONENT( xAODTruthParticleSlimmerPhoton)
DECLARE_COMPONENT( xAODTruthParticleSlimmerTau)

// old format of filters accepted for 22.6
DECLARE_COMPONENT( BSignalFilter )
DECLARE_COMPONENT( ChargedTracksFilter )
DECLARE_COMPONENT( ChargedTracksWeightFilter )
DECLARE_COMPONENT( DecaysFinalStateFilter ) 
DECLARE_COMPONENT( DecayTimeFilter )
DECLARE_COMPONENT( DiLeptonMassFilter )
DECLARE_COMPONENT( DirectPhotonFilter )
DECLARE_COMPONENT( ElectronFilter ) 
DECLARE_COMPONENT( ForwardProtonFilter )
DECLARE_COMPONENT( FourLeptonMassFilter )
DECLARE_COMPONENT( HeavyFlavorHadronFilter )
DECLARE_COMPONENT( JetFilter )
DECLARE_COMPONENT( LeptonFilter )
DECLARE_COMPONENT( LeptonPairFilter )
DECLARE_COMPONENT( M4MuIntervalFilter )
DECLARE_COMPONENT( MissingEtFilter )
DECLARE_COMPONENT( MultiBjetFilter )
DECLARE_COMPONENT( MultiCjetFilter )
DECLARE_COMPONENT( MultiElecMuTauFilter )
DECLARE_COMPONENT( MultiLeptonFilter )   
DECLARE_COMPONENT( MultiMuonFilter )
DECLARE_COMPONENT( MuDstarFilter )
DECLARE_COMPONENT( MuonFilter )  
DECLARE_COMPONENT( ParentChildFilter )
DECLARE_COMPONENT( ParentTwoChildrenFilter )
DECLARE_COMPONENT( ParticleDecayFilter ) 
DECLARE_COMPONENT( ParticleFilter )
DECLARE_COMPONENT( PhotonFilter ) 
DECLARE_COMPONENT( SameParticleHardScatteringFilter )
DECLARE_COMPONENT( SplitPhotonFilter )
DECLARE_COMPONENT( TauFilter )
DECLARE_COMPONENT( TTbarWToLeptonFilter )
DECLARE_COMPONENT( TTbarWithJpsimumuFilter )
DECLARE_COMPONENT( VBFForwardJetsFilter )
DECLARE_COMPONENT( VBFMjjIntervalFilter )
DECLARE_COMPONENT( XtoVVDecayFilterExtended )

// others
DECLARE_COMPONENT( BoostedHadTopAndTopPair )
DECLARE_COMPONENT( DiBjetFilter )
DECLARE_COMPONENT( DiPhotonFilter )
DECLARE_COMPONENT( FourLeptonInvMassFilter )
DECLARE_COMPONENT( HiggsFilter ) 
DECLARE_COMPONENT( HTFilter )
DECLARE_COMPONENT( MassRangeFilter )
DECLARE_COMPONENT( MultiElectronFilter )
DECLARE_COMPONENT( MultiParticleFilter )
DECLARE_COMPONENT( ParentChildwStatusFilter )
DECLARE_COMPONENT( QCDTruthMultiJetFilter )
DECLARE_COMPONENT( TTbarMassFilter )
DECLARE_COMPONENT( TTbarPlusHeavyFlavorFilter )
DECLARE_COMPONENT( TrimuMassRangeFilter )
DECLARE_COMPONENT( TruthJetFilter )
DECLARE_COMPONENT( VBFHbbEtaSortingFilter )
DECLARE_COMPONENT( ZtoLeptonFilter )


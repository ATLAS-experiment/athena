/**
 **     @file    AnalysisConfig_Ntuple.cxx
 **
 **     @author  mark sutton
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/


#include <cstdio>

#include <sys/time.h> 

#include "McParticleEvent/TruthParticleContainer.h"

#include "GeneratorObjects/McEventCollection.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"


#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"
#include "xAODEventInfo/EventInfo.h"


#include "TrigInDetAnalysis/TIDDirectory.h"
#include "TrigInDetAnalysisUtils/TIDARoiDescriptorBuilder.h"

#include "TrigInDetAnalysis/Filter_AcceptAll.h"

#include "TrigInDetAnalysisUtils/Filter_etaPT.h"
#include "TrigInDetAnalysisUtils/Filter_RoiSelector.h"
#include "TrigInDetAnalysisUtils/Filters.h"

#include "TrigInDetAnalysisExample/AnalysisConfig_Ntuple.h"
// #include "TrigInDetAnalysisUtils/OfflineObjectSelection.h"

#include "TrkParameters/TrackParameters.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTrack/Track.h"


#include "VxVertex/VxContainer.h"

#include "muonEvent/MuonContainer.h"

#include "egammaEvent/ElectronContainer.h"

#include "tauEvent/TauJetContainer.h"

//#include "JetEvent/JetCollection.h"

#include "TrigSteeringEvent/HLTResult.h"
#include "TrigDecisionTool/ExpertMethods.h"

#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"


#define endmsg endmsg



std::string date() { 
  time_t t;
  time(&t);
  char buf[26];
  std::string mtime = ctime_r(&t, buf);
  mtime.erase( std::remove(mtime.begin(), mtime.end(), '\n'), mtime.end() );
  return mtime;
}


//function to find true taus
HepMC::ConstGenParticlePtr fromParent( int pdg_id, HepMC::ConstGenParticlePtr p, bool printout=false ) { 

  if ( p==0 ) return 0;
  if (std::abs(p->pdg_id())==11 || std::abs(p->pdg_id())==13 ) return 0; //don't want light leptons from tau decays
  if ( std::abs(p->pdg_id())==pdg_id ) return p;   /// recursive stopping conditions
    
  auto vertex = p->production_vertex();
  if ( !vertex) return 0; // has no production vertex !!!

#ifdef HEPMC3
  if ( vertex->particles_in().size() < 1 ) return 0;  /// recursive stopping conditions

  /// useful debug
  //  if ( printout ) { 
  //     TruthParticle t(p);
  //     std::cout << "particle " << *p << "  " << t.pdgId() << "\tparent " << p << std::endl;
  //  }
  
  for ( auto in: vertex->particles_in()) {
    auto parent = fromParent( pdg_id, in, printout );
    TruthParticle t(in);
    if ( parent && std::abs(parent->pdg_id())==pdg_id) { 
      return parent;
    }  /// recursive stopping conditions
  }
#else  
  if ( vertex->particles_in_size() < 1 ) return 0;  /// recursive stopping conditions

  HepMC::GenVertex::particles_in_const_iterator in  = vertex->particles_in_const_begin();
  HepMC::GenVertex::particles_in_const_iterator end = vertex->particles_in_const_end();
  while ( in!=end ) {
    const HepMC::GenParticle* parent = fromParent( pdg_id, *in, printout );
    TruthParticle t(*in);
    // if ( printout ) std::cout << "\tvalue for particle " << *in << "  " << t.pdgId() << "\tparent " << parent << std::endl;
    if ( parent && std::abs(parent->pdg_id())==pdg_id) { 
      //if ( printout ) std::cout << "found tau! - in parents" << std::endl; 
      return parent;
    }   /// recursive stopping conditions
    in++;
  }
#endif
  
  return 0;
}
  


template<class T>
void remove_duplicates(std::vector<T>& vec) {
  std::sort(vec.begin(), vec.end());
  vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}


/// retrieve the jets  from the Roi

size_t AnalysisConfig_Ntuple::get_jets( Trig::FeatureContainer::combination_const_iterator citr, 
				      std::vector<TrackTrigObject>& objects, const std::string& key ) {

  objects.clear();

  const std::vector< Trig::Feature<xAOD::JetContainer> >  jetfeatures = citr->get<xAOD::JetContainer>( key, TrigDefs::alsoDeactivateTEs );
  
  if ( jetfeatures.empty() ) return 0; 

  for ( size_t ifeature=0 ; ifeature<jetfeatures.size() ; ifeature++ ) { 
    Trig::Feature<xAOD::JetContainer> jetfeature = jetfeatures.at(ifeature);

    if ( jetfeature.empty() ) continue;

    const xAOD::JetContainer* jets = jetfeature.cptr();
    
    if ( jets == 0 ) continue;

    xAOD::JetContainer::const_iterator jitr = jets->begin();

    for ( ; jitr!=jets->end() ; ++jitr ) {
      
      const xAOD::Jet* ajet = (*jitr);

      long unsigned jetid  = (unsigned long)ajet;

      TrackTrigObject jet = TrackTrigObject( ajet->eta(), ajet->phi(), ajet->pt(), 0, ajet->type(), jetid );

      objects.push_back( jet );
 
    } 
  }
  
  return objects.size();

}




void AnalysisConfig_Ntuple::loop() {

  m_provider->msg(MSG::INFO) << "[91;1m" << "AnalysisConfig_Ntuple::loop() for " << m_analysisInstanceName 
			     << " compiled " << __DATE__ << " " << __TIME__ << "\t: " << date() << "[m" << endmsg;

  m_provider->msg(MSG::ERROR) << "[91;1m" << "This should no longer be called" << m_analysisInstanceName << endmsg; 

}




/// setup the analysis the analysis, retrieve the tools etc 

void AnalysisConfig_Ntuple::book() { 

	m_provider->msg(MSG::INFO) << "AnalysisConfig_Ntuple::book() name " << name() << endmsg;   

	/// flag should be called m_fileIsNotOpen really, so is 
	/// if m_fileIsNotOpen open file, 
	/// if !m_fileIsNotOpen, then close file etc 
	if ( !m_finalised ) { 
		m_provider->msg(MSG::INFO) << "AnalysisConfig_Ntuple::book() not booking " << name() << endmsg;   
		return;
	}



	// get the TriggerDecisionTool

	if( m_tdt->retrieve().isFailure() ) {
		m_provider->msg(MSG::FATAL) << " Unable to retrieve the TrigDecisionTool: Please check job options file" << endmsg;
		//    return StatusCode::FAILURE;
		return;
	}

	m_provider->msg(MSG::INFO) << "[91;1m" << " Successfully retrieved the TrigDecisionTool" << "[m" << endmsg;
	m_provider->msg(MSG::INFO) << "[91;1m" << " booking ntuple" << "[m" << endmsg;
	m_provider->msg(MSG::INFO) << "[91;1m" << " trying to create new ntple file" << "[m" << endmsg;

	/// save the current directory so we can return there after
	TDirectory* dir = gDirectory;

	static std::atomic<bool> first_open = true;

	std::string outputFileName = m_outputFileName;

	if ( genericFlag() ) { 
		static std::atomic<int> file_index = 0;
		std::string::size_type pos = outputFileName.find(".root");
		if ( pos != std::string::npos ) outputFileName.erase(pos, outputFileName.size());
		char file_label[64];
		sprintf( file_label, "-%04d.root", file_index++ );
		outputFileName += file_label;
	} 

	m_provider->msg(MSG::INFO) << "book() Writing to file " << outputFileName << endmsg;

	if ( first_open || genericFlag() ) {
		/// create a brand new ntple
		m_File = new TFile( outputFileName.c_str(), "recreate");

		TTree*  dataTree = new TTree("dataTree", "dataTree");
		TString releaseData(m_releaseData.c_str());
		dataTree->Branch( "ReleaseMetaData", "TString", &releaseData);
		dataTree->Fill();
		dataTree->Write("", TObject::kOverwrite);
		delete dataTree;


		m_Tree = new TTree("tree", "tree");
		m_Tree->Branch( "TIDA::Event", "TIDA::Event", m_event, 6400, 1 );

		
	}
	else { 
		/// update the ntple from the file  
		m_File = new TFile( outputFileName.c_str(), "update");
		m_Tree = (TTree *)m_File->Get("tree");
		m_Tree->SetBranchAddress( "TIDA::Event", &m_event );
	}

	m_Dir = gDirectory;

	first_open = false;


	m_provider->msg(MSG::DEBUG) << "change directory " << name() << "  " << dir->GetName() << endmsg;

	//	std::cout << "change directory " << name() << "  " << dir->GetName() << std::endl;
	/// go back to original directory
	dir->cd();

	//  gDirectory->pwd();

	m_finalised = false; // flag we have an open file that is not yet finalised

	m_provider->msg(MSG::INFO) << "AnalysisConfig_Ntuple::book() exiting" << endmsg;

}



/// finalise the analysis - take ratios for efficiencies etc

void AnalysisConfig_Ntuple::finalize() { 

	//  gDirectory->pwd();


	/// NB: flag this round the other way for multiple files
	if ( m_finalised ) { 
		m_provider->msg(MSG::INFO) << "AnalysisConfig_Ntuple::finalise() flagged, not finalising  " << m_provider->name() << "\t" << m_Tree->GetEntries() << " entries" << endmsg;
		return;
	}

	m_provider->msg(MSG::INFO) << "AnalysisConfig_Ntuple::finalise() writing " << m_provider->name() << "\t" << m_Tree->GetEntries() << " entries" << endmsg;

	TDirectory* directory = gDirectory; 

	//	std::cout << "change directory " << name() << "  " << m_Dir->GetName() << std::endl;

	m_provider->msg(MSG::DEBUG) << "change directory " << name() << "  " << m_Dir->GetName() << endmsg;


	m_Dir->cd();

	//  gDirectory->pwd();

	m_Tree->Write("", TObject::kOverwrite);

	//  m_File->Write();
	if ( m_File ) m_File->Close();


	m_finalised = true; /// flag that we have finalised and closed this file

	// m_Tree "belongs" to the m_File so was (possibly) deleted on the m_File->Close();
	// so don't delete it ! 
	// delete m_Tree;
	delete m_File;

	m_Tree = 0;
	m_File = 0;

	//  f.Write();
	//  f.Close();

	//	std::cout << "change directory " << name() << "  " << directory->GetName() << std::endl;

	directory->cd();

	//  gDirectory->pwd();

}




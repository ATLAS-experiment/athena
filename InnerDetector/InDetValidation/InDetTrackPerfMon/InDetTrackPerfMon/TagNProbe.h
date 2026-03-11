/// emacs: this is -*- c++ -*-
/**
 **     @file    TagNProbe.h
 **
 **     @author  mark sutton
 **     @date    Sat Apr  9 12:55:17 CEST 2022
 **
 **     Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef TIDAUTILS_TAGNPROBE_H
#define TIDAUTILS_TAGNPROBE_H

#include <vector> 

#include "TrigInDetAnalysis/TIDAChain.h"
#include "TrigInDetAnalysis/TIDARoiDescriptor.h"
#include "TrigInDetAnalysis/TrigObjectMatcher.h"
#include "TrigInDetAnalysis/Track.h" 

#include "TrigInDetAnalysis/TrackSelector.h" 
#include "TrigInDetAnalysisUtils/Filters.h" 

#include "TLorentzVector.h"


template<typename T, typename S=T>
class TagNProbe {

public:

  TagNProbe( const std::string& refName,  double massMin, double massMax, bool unique_flag=true );

  TagNProbe( const std::string& refName0, const std::string& refName1, double massMin, double massMax, bool unique_flag=true );

  virtual ~TagNProbe() { }

  // constructir common

  void construct();
  
  /// getters and setters

  /// could be moved to the constructor now ...
  void   tag( const std::string& chainName ) { m_tagChainName   = chainName; }
  void probe( const std::string& chainName ) { m_probeChainName = chainName; }

  const std::string&   tag() const { return m_tagChainName; }
  const std::string& probe() const { return m_probeChainName; }

  const std::string& type0() const { return m_particleType0; };
  const std::string& type1() const { return m_particleType1; };

  double mass0() const { return m_mass0; };
  double mass1() const { return m_mass1; };

public:

  /// Fixme: This needs to be heavily modified to store and extract the relevant information
  /// from the athena event record, rather than the far simpler TrigInDetAnalysis record.
  /// In addition, this also fille histograms of the invariant mass, which may - || may not,
  /// end up being desired
  
  template<typename T>
  std::vector<TIDA::Roi*> GetRois( std::vector<TIDA::Chain>& chains, 
				   const TrackSelector*  selector, 
				   TrackFilter*          filter,
				   T* hmass,  
				   T* hmass_obj,
				   TrigObjectMatcher* tom=0 ) const {  
    return GetRois( chains, selector, filter, selector, filter, hmass, hmass_obj, tom, tom );
  }

  template<typename T>
  std::vector<TIDA::Roi*> GetRois( std::vector<TIDA::Chain>& chains, 
				   const TrackSelector*  selector_tag, 
				   TrackFilter*          filter_tag,
				   const TrackSelector*  selector_probe, 
				   TrackFilter*          filter_probe,
				   T* hmass,  
				   T* hmass_obj,
				   TrigObjectMatcher* tom_tag=0,
				   TrigObjectMatcher* tom_probe=0 ) const {  

    std::vector<TIDA::Roi*> probes;

    TIDA::Chain* chain_tag   = findChain( tag(), chains ); 
    TIDA::Chain* chain_probe = findChain( probe(), chains ); 

    if ( chain_tag==0 || chain_probe==0 ) return probes;

    // loop for possible probes
    for ( size_t ip=0 ; ip<chain_probe->size() ; ip++ ) {
      
      TIDA::Roi& proi = chain_probe->rois()[ip];
    
      TIDARoiDescriptor roi_probe( proi.roi() );

      bool found_tnp = false;

      // loop for possible tags
      for ( size_t it=0 ; it<chain_tag->size() ; it++ ) {  
	
	TIDA::Roi& troi = chain_tag->rois()[it];
	TIDARoiDescriptor roi_tag( troi.roi() );

	/// tag and probe are the same: skip this tag
	if ( roi_probe == roi_tag ) continue;
	
	if ( selection( troi, proi, selector_tag, filter_tag, selector_probe, filter_probe, 
			hmass, hmass_obj, tom_tag, tom_probe ) ) { 

	  found_tnp = true;
	  if ( m_unique ) break;
	}
	
      } // end loop on tags
      
      if ( found_tnp ) probes.push_back( &proi );

    } // end loop on probes
    
    return probes;
    
  }

protected:
  
  double pt( const TIDA::Track* t )     const { return t->pT(); }   
  double pt( const TrackTrigObject* t ) const { return t->pt(); }   
  
  template<typename T1, typename T2>
  double mass( const T1* t1, const T2* t2 ) const {
    TLorentzVector v1;
    v1.SetPtEtaPhiM( pt(t1)*0.001, t1->eta(), t1->phi(), m_mass0 );
    TLorentzVector v2;
    v2.SetPtEtaPhiM( pt(t2)*0.001, t2->eta(), t2->phi(), m_mass1 );
    return (v1+v2).M();
  }


  template<typename T>
  bool selection( const TIDA::Roi& troi, const TIDA::Roi& proi, 
		  const TrackSelector* selector, 
		  TrackFilter*         filter,
		  T* hmass, 
		  T* hmass_obj,
		  TrigObjectMatcher*  tom=0) const {
    return selection( troi, proi, selector, filter, selector, filter, hmass, hmass_obj, tom, tom ); 
  }

  template<typename T>
  bool selection( const TIDA::Roi& troi, const TIDA::Roi& proi, 
		  const TrackSelector* selector_tag, 
		  TrackFilter*         filter_tag,
		  const TrackSelector* selector_probe, 
		  TrackFilter*         filter_probe,
		  T* hmass, 
		  T* hmass_obj,
		  TrigObjectMatcher*  tom_tag=0,
		  TrigObjectMatcher*  tom_probe=0) const {
  
    /// get reference tracks from the tag roi                                                                                                                                                                           
    TIDARoiDescriptor roi_tag( troi.roi() );
  
    dynamic_cast<Filter_Combined*>(filter_tag)->setRoi( &roi_tag );

    std::vector<TIDA::Track*> refp_tag = selector_tag->tracks( filter_tag );

    /// get reference tracks from the probe roi 
                                                                                                                                                                     
    TIDARoiDescriptor roi_probe( proi.roi() );

    dynamic_cast<Filter_Combined* >( filter_probe )->setRoi( &roi_probe );

    std::vector<TIDA::Track*> refp_probe = selector_probe->tracks( filter_probe );

    /// loop over tag ref tracks                                                                                                                                                                                           
    bool found = false;

    for ( size_t it=0; it<refp_tag.size() ; it++ ) {

      /// loop over probe ref tracks
      for ( size_t ip=0; ip<refp_probe.size() ; ip++ ) {

	/// check compatibility of the track z and invariant mass ...
	double invmass     = mass( refp_tag[it], refp_probe[ip] );
	double invmass_obj = mass_obj( refp_tag[it], refp_probe[ip], tom_tag, tom_probe  );
	double deltaz0     = std::fabs(refp_tag[it]->z0() - refp_probe[ip]->z0() );
	
	if ( invmass_obj>m_massMin && invmass_obj<m_massMax && deltaz0<5 ) {
	  hmass->Fill( invmass );
	  hmass_obj->Fill( invmass_obj );
	  found = true;
	}
      }
    }
    
    return found;

  }
  

  double   mass_obj( const TIDA::Track* t1, const TIDA::Track* t2, TrigObjectMatcher* tom=0 ) const;
  double   mass_obj( const TIDA::Track* t1, const TIDA::Track* t2, TrigObjectMatcher* tom_tag, TrigObjectMatcher* tom_probe ) const;

  TIDA::Chain* findChain( const std::string& chainname, std::vector<TIDA::Chain>& chains ) const;


private:

  std::string m_particleType0;
  std::string m_particleType1;

  double m_mass0;
  double m_mass1;

  double m_massMin;
  double m_massMax;

  bool   m_unique;

  std::string  m_probeChainName ;
  std::string  m_tagChainName ;

};


inline std::ostream& operator<<( std::ostream& s, const TagNProbe& t ) { 
  s << "[ TagNProbe: chains tag: " << t.tag()    << "\tprobe: " << t.probe() << "\n";
  s << "                   type: " << t.type0()  << "\t       " << t.type1() << "\n";
  s << "                   mass: " << t.mass0()  << "\t       " << t.mass1() << " ]";
  return s;
}


#endif ///  TIDAUTILS_TAGNPROBE_H



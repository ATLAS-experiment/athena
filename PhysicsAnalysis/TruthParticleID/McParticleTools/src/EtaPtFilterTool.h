///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EtaPtFilterTool.h 
// Header file for class EtaPtFilterTool
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef MCPARTICLETOOLS_ETAPTFILTERTOOL_H 
#define MCPARTICLETOOLS_ETAPTFILTERTOOL_H 1

// STL includes
#include <string>
#include <map>

// McParticleTools includes
#include "TruthParticleFilterBaseTool.h"

// Forward declaration

class EtaPtFilterTool : public TruthParticleFilterBaseTool
{ 

 public: 

  // Copy constructor: 

  /// Constructor with parameters: 
  EtaPtFilterTool( const std::string& type,
		   const std::string& name, 
		   const IInterface* parent );

  /// Destructor: 
  virtual ~EtaPtFilterTool(); 


  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /** This method will check the validity of the input McEventCollection 
   *  and build a filtered one from the strategy implemented by this
   *  concrete tool.
   */
  StatusCode buildMcAod( const McEventCollection* in, McEventCollection* out );

  /////////////////////////////////////////////////////////////////// 
  // Protected methods: 
  /////////////////////////////////////////////////////////////////// 
 protected:
  using ParticleMap_t = std::unordered_map<const HepMC::GenParticle*,
                                           HepMC::GenParticlePtr>;
  using VertexMap_t = std::unordered_map<const HepMC::GenVertex*,
                                         HepMC::GenVertexPtr>;

  /** Default constructor: 
   */
  EtaPtFilterTool();

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /** This method will check the validity of the input HepMC::GenEvent
   *  and build a filtered one from the strategy implemented by this
   *  concrete tool.
   */
  StatusCode buildGenEvent( const HepMC::GenEvent* in, HepMC::GenEvent* out );

  /** Check if a given particle is within the acceptance (pt+eta)
   */
  bool isAccepted( const HepMC::ConstGenParticlePtr& mcPart ) const;

  /** Check if a given vertex has at least one in-going
   *  or out-going particle within the acceptance (pt+eta)
   */
  bool isAccepted( const HepMC::ConstGenVertexPtr& vtx ) const;

  /** Helper method to copy a given vertex and add it to a GenEvent
   */
  StatusCode addVertex( const HepMC::ConstGenVertexPtr& srcVtx, HepMC::GenEvent* evt,
                        VertexMap_t& vmap,
                        ParticleMap_t& pmap,
			bool isSignalVertex = false ) const;


  bool isPartonVertex( const HepMC::ConstGenVertexPtr& vtx ) const;
  bool isFromHardScattering( const HepMC::ConstGenVertexPtr& vtx ) const;
  
  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /** Method to initialize the tool: we need to check the validity of 
   *  the parameters given for the inner and outer eta regions
   */
  StatusCode initializeTool();

  /** Callback function to setup the "InnerEtaRegionCuts" property
   */
  void setupInnerEtaRegionCuts( Gaudi::Details::PropertyBase& innerEtaRegionCuts );

  /** Callback function to setup the "OuterEtaRegionCuts" property
   */
  void setupOuterEtaRegionCuts( Gaudi::Details::PropertyBase& outerEtaRegionCuts );

  /////////////////////////////////////////////////////////////////// 
  // Protected data: 
  /////////////////////////////////////////////////////////////////// 
 protected: 

  /** vector of cuts parameters for the inner region delimited in |eta|
   *   0-th element is the minimum |eta| of this region
   *   1-st element is the maximum |eta| of this region
   *   2-nd element is the minimum pt for the stable particles to be accepted
   */
  DoubleArrayProperty m_innerEtaRegionCuts;

  /** vector of cuts parameters for the outer region delimited in |eta|
   *   0-th element is the minimum |eta| of this region
   *   1-st element is the maximum |eta| of this region
   *   2-nd element is the minimum pt for the stable particles to be accepted
   */
  DoubleArrayProperty m_outerEtaRegionCuts;

  /** Switch to only include particles from generation and reject particles 
   *  from detector simulation (Geant4)
   */
  BooleanProperty m_onlyGenerator;

  /** Switch to keep *all* stable generator particles (IsGenStable) regardless 
   *  what eta or pt cuts are defined
   */
  BooleanProperty m_butKeepAllGeneratorStable;

  /** Switch to keep *all* generator particles which are documentaries
   *  (statuscode == 3)
   */
  BooleanProperty m_keepDocumentaries;

  /** Switch to keep *all* leptons - i.e. do not apply kinematic cuts on them
   */
  BooleanProperty m_keepAllLeptons;

  
  /* For the isolation */
  std::map<int,double> m_isol_energies;

}; 


#endif //> MCPARTICLETOOLS_ETAPTFILTERTOOL_H

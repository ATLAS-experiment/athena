/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EVENTUTILS_PARTICLEREMOVERALG_H
#define EVENTUTILS_PARTICLEREMOVERALG_H 1

#include "AthenaBaseComps/AthAlgorithm.h"

#include "xAODBase/IParticleContainer.h"



class ParticleRemoverAlg: public ::AthAlgorithm {
 public:
  /// Standard constructor
  ParticleRemoverAlg( const std::string& name, ISvcLocator* pSvcLocator );

  /// Standard destructor
  virtual ~ParticleRemoverAlg();

  /// Standard Gaudi initialize method called once before the event loop
  virtual StatusCode  initialize();

  /// Standard Gaudi execute method called once for every event
  virtual StatusCode  execute();

  /// Standard Gaudi finalize method called once after the event loop
  virtual StatusCode  finalize();

 private:
   /// Private function to perform the actualy work
   template<class CONT>
   StatusCode removeParticles( const std::vector<bool>& keepParticleVec );

 private:

  /// @name The properties that can be defined via the python job options
  /// @{

  /// The input container name
  Gaudi::Property<std::string> m_inCont{this, "Input", "", "Input container name"};

  /// The string separator between the output container name and the sytematic variation (default="___")
  Gaudi::Property<std::string> m_separator{this, "Separator", "___", "The string separator between the output container name and the sytematic variation"};

  /// The output container name
  Gaudi::Property<std::string> m_outCont{this, "Output", "", "The name of the output container with the deep copy of input objects"};

  /// The names of all suffixes for the input and output container names
  Gaudi::Property<std::vector<std::string>> m_suffixes{this, "Suffixes", {}, "The names of all suffixes for the input and output container names"};

  /// The names of all view containers that contain particles that we want to retain
  Gaudi::Property<std::vector<std::string>> m_viewContNames{this, "SelectedViewContainers", {}, "The names of all view containers that contain particles that we want to retain"};

  /// Boolean to decide if the existing view containers should be re-mapped (default: true)
  Gaudi::Property<bool> m_resetViewConts{this, "RemapViewContainers", true, "Boolean to decide if the existing view containers should be re-mapped"};

  /// Prefix to be used for all created output view containers
  Gaudi::Property<std::string> m_outPrefix{this, "OutputViewContainerPrefix", "", "Prefix to be used for all created output view containers"};

  /// @}


  /// @name Internal members
  /// @{

  /// Vector of all input container names
  std::vector<std::string> m_inContNameList;

  /// Vector of all output container names
  std::vector<std::string> m_outContNameList;

  /// Vector of all input containers
  std::vector< const xAOD::IParticleContainer* > m_inContList;

  /// Vector of all output containers
  std::vector< xAOD::IParticleContainer* > m_outContList;

  /// Vector of all input view container names
  std::vector< std::vector<std::string> > m_inViewContNameListList;

  /// Vector of all output view container names
  std::vector< std::vector<std::string> > m_outViewContNameListList;


  /// An enumaration for the actual container type
  enum contType_t {
    UNKNOWN,
    PHOTON,
    ELECTRON,
    MUON,
    TAU,
    JET,
    PARITCLEFLOW,
    NEUTRALPARTICLE,
    TRACKPARTICLE,
    TRUTHPARTICLE,
    COMPOSITEPARTICLE,
    PARTICLE,
    CALOCLUSTER
  };

  /// The variable that holds the value that we find for the input container
  contType_t m_contType{UNKNOWN};

  /// @}

};

// Include the templated code here. This must be done from this header file.
#include "ParticleRemoverAlg.icc"

#endif //> !EVENTUTILS_PARTICLEREMOVERALG_H

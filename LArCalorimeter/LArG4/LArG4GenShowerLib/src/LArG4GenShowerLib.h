/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARG4GENSHOWERLIB_LARG4GENSHLIB_H
#define LARG4GENSHOWERLIB_LARG4GENSHLIB_H

// STL includes
#include <string>
#include <list>
#include <map>

#include "AthenaBaseComps/AthAlgorithm.h"

// CLHEP include(s)
#include "CLHEP/Vector/ThreeVector.h"

// forward includes in namespaces
namespace ShowerLib {
  class IShowerLib;
  class StepInfoCollection;
  class StepInfo;
  typedef std::list<StepInfo*> StepInfoList;
}
#include "AtlasHepMC/GenParticle_fwd.h"


  /**
   *
   *   @short Class for shower library generation algorithm
   *
   *      Create shower library using geant hits
   *
   *  @author Wolfgang Ehrenfeld, University of Hamburg, Germany
   *  @author Sasha Glazov, DESY Hamburg, Germany
   *
   * @version $Id: LArG4GenShowerLib.h 711210 2015-11-27 15:56:00Z jchapman $
   *
   */
class LArG4GenShowerLib : public AthAlgorithm {

 public:
  using AthAlgorithm::AthAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual StatusCode execute() override;

 private:

  void clusterize(ShowerLib::StepInfoList* stepinfo);

  const ShowerLib::StepInfoCollection* getStepInfo();

  ShowerLib::StepInfoList* copyStepInfo(const ShowerLib::StepInfoCollection* stepinfo);

  ShowerLib::StepInfoList* copyStepInfoZeroCleanup(const ShowerLib::StepInfoCollection* stepinfo);

  void truncate(ShowerLib::StepInfoList* stepinfo);

  //! return first MC truth particle for event
  HepMC::ConstGenParticlePtr getParticleFromMC();
  //! calculate moments from StepInfoCollection
  void calculateMoments(const ShowerLib::StepInfoCollection&  eventSteps,
			double& weights, double& xavfra, double& yavfra, double& ravfra);
  //! adding tag information (release, detector description, ...) to library comment
  void addingTagsToLibrary();

  /* data members */
  DoubleProperty m_maxDistance {this, "MaxDistance", 50000.
    , "max distance squared after which the hits will be truncated"}; //!< property, see @link LArG4GenShowerLib::LArG4GenShowerLib @endlink
  DoubleProperty m_maxRadius {this, "MaxRadius", 25.
    , "maximal radius squared until two hits will be combined"};      //!< property, see @link LArG4GenShowerLib::LArG4GenShowerLib @endlink
  DoubleProperty m_minEnergy {this, "MinEnergy", .99
    , "energy border, that truncation won't cross"};                  //!< property, see @link LArG4GenShowerLib::LArG4GenShowerLib @endlink
  DoubleProperty m_containmentEnergy {this, "ContainmentEnergy", 0.95
    , "energy fraction that will be inside containment borders"};     //!< property, see @link LArG4GenShowerLib::LArG4GenShowerLib @endlink
  DoubleProperty m_energyFraction {this, "EnergyFraction", .02
    ,  "the allowed amount of energy that can be deposited outside calorimeter region"}; //!< property, see @link LArG4GenShowerLib::LArG4GenShowerLib @endlink
  StringProperty m_physicslist_name {this, "PhysicsList", "FTFP_BERT"
    , "Geant4 PhysicsList used in the simulation"};
  StringArrayProperty m_lib_struct_files {this, "LibStructFiles", {}
    , "List of files to read library structures from"};

  typedef std::map<std::string, ShowerLib::IShowerLib*> libMap;
  libMap m_libraries;                  //!< pointer to shower library
  libMap m_libraries_by_filename;

  int m_stat_numshowers{0};
  int m_stat_valid{0};
  int m_stat_invalid{0};
  int m_stat_nolib{0};

  std::map<ShowerLib::IShowerLib*, int> m_stat_lib_saved;
  std::map<ShowerLib::IShowerLib*, int> m_stat_lib_notsaved;
}; // class LArG4GenShowerLib

#endif // LARG4GENSHOWERLIB_LARG4GENSHLIB_H

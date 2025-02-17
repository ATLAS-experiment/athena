/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SHERPA_I_SHERPA_I_H
#define SHERPA_I_SHERPA_I_H

#include "GaudiKernel/Algorithm.h"
#include "GeneratorModules/GenModule.h"

#include "AtlasHepMC/GenEvent.h"

namespace SHERPA {
  class Sherpa;
}


class Sherpa_i : public GenModule {
public:
  Sherpa_i(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode genInitialize();
  StatusCode callGenerator();
  StatusCode genFinalize();
  StatusCode fillEvt(HepMC::GenEvent* evt);
  #ifndef IS_SHERPA_3
  void getParameters(int &argc, char** &argv);
  #endif
  void compilePlugin(const std::string&);

protected:
  
  SHERPA::Sherpa * p_sherpa{};

  #ifdef IS_SHERPA_3
  /// Sherpa base settings (read from base fragment file) and run card snippet (from JO file)
  std::map<std::string,std::string> m_inputfiles;
  #else
  /// Sherpa base settings (read from base fragment file)
  std::string m_basefragment; /// FIXME unused?

  /// Sherpa run card snippet (from JO file)
  StringProperty m_runcard{this, "RunCard", ""};

  /// List of additional Sherpa parameters beyond run card snippet (from JO file)
  StringArrayProperty m_params{this, "Parameters", {} };
  #endif

  /// List of needed OpenLoops process libraries (from JO file)
  StringArrayProperty m_openloopslibs{this, "OpenLoopsLibs", {} };

  /// List of any additional needed files, e.g. custom libraries, PDF sets (from JO file)
  StringArrayProperty m_extrafiles{this, "ExtraFiles", {} };

  /// Number of cores recommended for multi-core integration file
  IntegerProperty m_ncores{this, "NCores", 1};

  /// Memory required for integration/evgen
  DoubleProperty m_memorymb{this, "MemoryMB", 2500.};

  /// Optional code for plugin library to compile and load at run time
  StringProperty m_plugincode{this, "PluginCode", ""};

  /// Variation weight cap factor
  DoubleProperty m_variation_weight_cap{this, "VariationWeightCap", 10.0};

  DoubleProperty m_xsscale{this, "CrossSectionScaleFactor", 1.0};
  BooleanProperty m_cleanup{this, "CleanupGeneratedFiles", true};

  //Gen_tf run args.
  IntegerProperty m_dsid{this, "Dsid", 999999, "Dataset ID number"};
};



#include "ATOOLS/Math/Random.H"

class Atlas_RNG: public ATOOLS::External_RNG {
  CLHEP::HepRandomEngine* p_engine;
  std::string m_filename;
  std::once_flag m_once_flag_atlas_rng;

public:
  Atlas_RNG(CLHEP::HepRandomEngine*);
  ~Atlas_RNG();
  double Get();
  bool CanRestoreStatus() const { return true; }
  void SaveStatus();
  void RestoreStatus();
  const std::string GenerateUID() const;

};


#endif // SHERPA_I_SHERPA_I_H



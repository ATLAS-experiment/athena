// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LUMIBLOCKCOMPS_BUNCHCROSSINGAVERAGECONDTEST_H
#define LUMIBLOCKCOMPS_BUNCHCROSSINGAVERAGECONDTEST_H

// Gaudi/Athena include(s):
#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Property.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "LumiBlockData/BunchCrossingAverageCondData.h"

#include <fstream>

class BunchCrossingAverageCondTest : public AthAlgorithm {

public:
  /// Regular Athena algorithm constructor
  using AthAlgorithm::AthAlgorithm;

  /// Initialization run before the event loop
  virtual StatusCode initialize();
  /// Function called once per event
  virtual StatusCode execute();

private:
  /// Function for printing detailed info about a given bunch crossing
  static void printInfo(const BunchCrossingAverageCondData* bccd, unsigned int channel, std::ostream& out);
  
  SG::ReadCondHandleKey<BunchCrossingAverageCondData> m_inputKey{this,"InputKey","BunchCrossingAverageData",""};

  Gaudi::Property<std::string> m_fileName{this,"FileName",""};

  // Gaudi::Property<bool> m_compact{this,"compact",false};

  std::ofstream m_fileOut;

}; 



#endif 

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FATRASINTERFACES_IPROCESSSAMPLINGTOOL_H
#define ISF_FATRASINTERFACES_IPROCESSSAMPLINGTOOL_H

// Gaudi
#include "GaudiKernel/IAlgTool.h"
#include "CxxUtils/checker_macros.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"

namespace Trk{
  struct PathLimit;
}

namespace CLHEP {
  class HepRandomEngine;
}

namespace iFatras {

  /**
   @class IProcessSamplingTool

   sampling the process/free path

   @author Sarka.Todorova -at- cern.ch

   */

  class IProcessSamplingTool : virtual public IAlgTool {
     public:

       /** Virtual destructor */
       virtual ~IProcessSamplingTool(){}

       /// Creates the InterfaceID and interfaceID() method
       DeclareInterfaceID(IProcessSamplingTool, 1, 0);

       /** Process, path limit */
       virtual Trk::PathLimit sampleProcess(CLHEP::HepRandomEngine *randomEngine, double momentum, double charge, Trk::ParticleHypothesis pHypothesis) const=0;

  };

} // end of namespace

#endif

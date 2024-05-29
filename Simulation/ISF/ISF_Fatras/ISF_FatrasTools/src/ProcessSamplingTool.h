/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FATRASTOOLS_PROCESSSAMPLINGTOOL_H
#define ISF_FATRASTOOLS_PROCESSSAMPLINGTOOL_H

// Athena Base
#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"

// iFatras
#include "ISF_FatrasInterfaces/IProcessSamplingTool.h"

// Tracking
#include "TrkEventPrimitives/PdgToParticleHypothesis.h"
#include "TrkParameters/TrackParameters.h"

namespace iFatras
{
  /** @class ProcessSamplingTool

      Fatras AlgTool to sample process and free path

      @author Sharka Todorova Sarka.Todorova -at- cern.ch
  */

  class ProcessSamplingTool : public extends<AthAlgTool, IProcessSamplingTool>
  {
  public:
    /** Constructor */
    ProcessSamplingTool(const std::string&,const std::string&,const IInterface*);

    /** Destructor */
    virtual ~ProcessSamplingTool () = default;

    /** Process pre-sampling : to be moved into material updators eventually */
    Trk::PathLimit sampleProcess(CLHEP::HepRandomEngine *randomEngine, double mom, double charge, Trk::ParticleHypothesis pHypothesis) const;

  private:
    /*---------------------------------------------------------------------
     *  Private members
     *---------------------------------------------------------------------*/
    /** hadronic interaction */
    BooleanProperty m_hadInt{this, "HadronicInteraction", true, ""};
  };
}

#endif // FATRASTOOLS_PROCESSSSAMPLINGTOOL_H

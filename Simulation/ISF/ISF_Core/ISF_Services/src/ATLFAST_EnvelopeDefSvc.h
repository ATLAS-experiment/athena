/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ATLFAST_EnvelopeDefSvc.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
#ifndef ISF_SERVICES_ATLFAST_ENVELOPEDEFSVC_H
#define ISF_SERVICES_ATLFAST_ENVELOPEDEFSVC_H

// STL includes
#include <string>
#include <vector>
#include <utility>

// GaudiKernel & Athena
#include "AthenaBaseComps/AthService.h"

// interface header file
#include "SubDetectorEnvelopes/IEnvelopeDefSvc.h"

namespace ISF {

  class ATLFAST_EnvelopeDefSvc : public extends<AthService, IEnvelopeDefSvc> {

    public:
      /** public AthService constructor */
      ATLFAST_EnvelopeDefSvc(const std::string& name, ISvcLocator* svc);

      /** Destructor */
      ~ATLFAST_EnvelopeDefSvc();

      /** AthService initialize method.*/
      StatusCode initialize();
      /** AthService finalize method */
      StatusCode finalize();

      /** return a vector of (r,z) pairs, defining the respective envelope */
      const RZPairVector& getRZBoundary( AtlasDetDescr::AtlasRegion region ) const;

      /** return a vector of (r,z) pairs, defining the envelope on the z>0 region */
      const RZPairVector &getRPositiveZBoundary( AtlasDetDescr::AtlasRegion region ) const;

    private:
      /** return boundary with shifted z values */
      RZPairVector getShiftedBoundary( AtlasDetDescr::AtlasRegion region, double shiftFromZ, double shiftToZ ) const;

      /** ServiceHandle to the standard ISF envelope definition service */
      ServiceHandle<IEnvelopeDefSvc>            m_isfEnvDefSvc;

      /** internal tolerance on coordinates */
      double                                    m_tolerance;

      /** maximum desired extent (halfz) of the modified inner detector volume */
      double                                    m_idMaxExtentZ;

      /** internal (r,z) representation for BeamPipe, InnerDetector and calo volumes */
      RZPairVector                              m_rzBeamPipe;
      RZPairVector                              m_rzInDet;
      RZPairVector                              m_rzCalo;
      /** internal (r,z) representation for the positive z-side only,
       *  one RZPairVector for BeamPipe and InnerDetector each */
      RZPairVector                              m_rposzBeamPipe;
      RZPairVector                              m_rposzInDet;
      RZPairVector                              m_rposzCalo;

  };


} // namespace ISF

#endif // ISF_SERVICES_ATLFAST_ENVELOPEDEFSVC_H


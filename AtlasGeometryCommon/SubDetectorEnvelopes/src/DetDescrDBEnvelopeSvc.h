/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  DetDescrDBEnvelopeSvc.h
 * @class DetDescrDBEnvelopeSvc
 */

#ifndef SUBDETECTORENVELOPES_DETDESCRDBENVELOPESVC_H
#define SUBDETECTORENVELOPES_DETDESCRDBENVELOPESVC_H

// STL includes
#include <string>
#include <vector>
#include <utility>
#include <array>

// GaudiKernel & Athena
#include "AthenaBaseComps/AthService.h"

// interface header file
#include "SubDetectorEnvelopes/IEnvelopeDefSvc.h"

// Database includes
#include "RDBAccessSvc/IRDBAccessSvc.h"

// GeoModel
#include "GeoModelInterfaces/IGeoModelSvc.h"

/** datatype used for fallback solution */
using FallbackDoubleVector = std::vector<double>;

class DetDescrDBEnvelopeSvc : public extends<AthService, IEnvelopeDefSvc>
{
public:
  /** public AthService constructor */
  DetDescrDBEnvelopeSvc(const std::string& name, ISvcLocator* svc);

  /** Destructor */
  ~DetDescrDBEnvelopeSvc();

  /** AthService initialize method.*/
  virtual StatusCode initialize() override;

  /** return a vector of (r,z) pairs, defining the respective envelope */
  virtual const RZPairVector& getRZBoundary( AtlasDetDescr::AtlasRegion region ) const override { return m_rz[region]; }

  /** return a vector of (r,z) pairs, defining the envelope on the z>0 region */
  virtual const RZPairVector &getRPositiveZBoundary( AtlasDetDescr::AtlasRegion region ) const override { return m_rposz[region]; }

private:
  /** retrieve and store the (r,z) values locally for the given DB node.
      if there are problems with retrieving this from DDDB,
      try the fallback approach if allowed */
  StatusCode retrieveRZBoundaryOptionalFallback( const std::string           &dbNode,
						 const FallbackDoubleVector  &r,
						 const FallbackDoubleVector  &z,
						 RZPairVector                &rzVec);

  /** retrieve and store the (r,z) values locally for the given DB node */
  StatusCode retrieveRZBoundary( const std::string &node, RZPairVector &rzVec);

  /** use the fallback approach (python arguments) to set the (r,z) values */
  StatusCode fallbackRZBoundary( const FallbackDoubleVector  &r,
				 const FallbackDoubleVector  &z,
				 RZPairVector                &rzVec);

  /** enable fallback solution:
   *  @return true if fallback mode is allowed, false if no fallback allowed */
  bool enableFallback();

  /** the DetectorDescription database access method */
  ServiceHandle<IRDBAccessSvc>       m_dbAccess{this, "RDBAccessSvc", "RDBAccessSvc"};

  /** ATLAS GeoModel */
  ServiceHandle<IGeoModelSvc>        m_geoModelSvc{this, "GeoModelSvc", "GeoModelSvc"};

  /** main DDDB node for the ATLAS detector */
  std::string                        m_atlasNode{"ATLAS"};
  std::string                        m_atlasVersionTag{"AUTO"};

  /** the names of the DB nodes for the respective AtlasRegion */
  std::array<StringProperty,AtlasDetDescr::fNumAtlasRegions> m_node{{
      {this, "DBUndefinedNode", ""} // Dummy
      , {this, "DBInDetNode","InDetEnvelope"}
      , {this, "DBBeamPipeNode", "BeamPipeEnvelope"}
      , {this, "DBCaloNode", "CaloEnvelope"}
      , {this, "DBMSNode", "MuonEnvelope"}
      , {this, "DBCavernNode", "CavernEnvelope"}
    }};

  /** internal (r,z) representation, one RZPairVector for each AtlasRegion */
  RZPairVector                       m_rz[AtlasDetDescr::fNumAtlasRegions]{};
  /** internal (r,z) representation for the positive z-side only,
   *  one RZPairVector for each AtlasRegion */
  RZPairVector                       m_rposz[AtlasDetDescr::fNumAtlasRegions]{};

  /** fallback solution, in case something goes wrong with the DB */
  Gaudi::Property<bool> m_allowFallback{this, "EnableFallback", false};
  bool                  m_doFallback{false};

  std::array<DoubleArrayProperty,AtlasDetDescr::fNumAtlasRegions> m_fallbackR{{
      {this, "FallbackUndefinedR", {}} //Dummy
      , {this, "FallbackInDetR", {}}
      , {this, "FallbackBeamPipeR", {}}
      , {this, "FallbackCaloR", {}}
      , {this, "FallbackMuonR", {}}
      , {this, "FallbackCavernR", {}}
    }};

  std::array<DoubleArrayProperty,AtlasDetDescr::fNumAtlasRegions> m_fallbackZ{{
      {this, "FallbackUndefinedZ", {}} //Dummy
      , {this, "FallbackInDetZ", {}}
      , {this, "FallbackBeamPipeZ", {}}
      , {this, "FallbackCaloZ", {}}
      , {this, "FallbackMuonZ", {}}
      , {this, "FallbackCavernZ", {}}
    }};
};

#endif // DETDESCRDBENVELOPESVC_H

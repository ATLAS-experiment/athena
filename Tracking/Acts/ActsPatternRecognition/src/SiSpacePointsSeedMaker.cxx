/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/SiSpacePointsSeedMaker.h"
#include "GaudiKernel/EventContext.h"

#include "Acts/Definitions/Units.hpp"
#include "Acts/MagneticField/MagneticFieldContext.hpp"
#include "ActsGeometry/ATLASMagneticFieldWrapper.h"

#include "InDetPrepRawData/SiCluster.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "SiSPSeededTrackFinderData/SiSpacePointsSeedMakerEventData.h"

#include "ActsEvent/Seed.h"
#include "SiSpacePoint/SCT_SpacePoint.h"
#include "SiSpacePoint/PixelSpacePoint.h"
#include "InDetPrepRawData/PixelClusterCollection.h"
#include "InDetPrepRawData/SCT_ClusterCollection.h"
#include "MagFieldElements/AtlasFieldCache.h"
//for validation
#include "TrkTrack/Track.h"
#include "TrkParameters/TrackParameters.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ITHistSvc.h"
#include "SiSPSeededTrackFinderData/ITkSiSpacePointForSeed.h"

#include "TTree.h"

#include <cmath>


namespace ActsTrk {

  SiSpacePointsSeedMaker::SiSpacePointsSeedMaker(const std::string &t, const std::string &n, const IInterface *p)
    : base_class(t, n, p)
  {}

  StatusCode SiSpacePointsSeedMaker::initialize()
  {
    ATH_MSG_DEBUG( "Initializing " << name() << "..." );

    ATH_MSG_DEBUG( "Properties Summary:" );
    ATH_MSG_DEBUG( "   " << m_pixel );
    ATH_MSG_DEBUG( "   " << m_strip );
    ATH_MSG_DEBUG( "   " << m_useOverlap );
    ATH_MSG_DEBUG( "   " << m_writeNtuple );
    ATH_MSG_DEBUG( "   " << m_fastTracking );

    if (not m_pixel and not m_strip) {
      ATH_MSG_ERROR("Activate seeding on at least one between Pixel and Strip space point collections!");
      return StatusCode::FAILURE;
    }

    ATH_CHECK( m_actsSpacepointsPixel.initialize(m_pixel) );
    ATH_CHECK( m_actsSpacepointsStrip.initialize(m_strip) );
    ATH_CHECK( m_actsSpacepointsOverlap.initialize(m_strip and m_useOverlap) );

    ATH_CHECK( m_seedsToolPixel.retrieve(EnableTool{m_pixel}) );
    ATH_CHECK( m_seedsToolStrip.retrieve(EnableTool{m_strip}) );

    ATH_CHECK( m_prdToTrackMap.initialize(SG::AllowEmpty) );

    ATH_CHECK( m_beamSpotKey.initialize() );
    ATH_CHECK( m_fieldCondObjInputKey.initialize() );

    // Validation
    if (m_writeNtuple) 
      ATH_CHECK( InitTree() );

    return StatusCode::SUCCESS;
  }

  // ===================================================================== //

  StatusCode
  SiSpacePointsSeedMaker::InitTree()
  {
    ATH_CHECK( service("THistSvc", m_thistSvc)  );
    std::string tree_name = std::string("SeedTree_") + name();
    std::replace( tree_name.begin(), tree_name.end(), '.', '_' );

    m_outputTree = new TTree( tree_name.c_str() , "ActsSeedMakerValTool");

    m_outputTree->Branch("eventNumber",    &m_eventNumber,"eventNumber/L");
    m_outputTree->Branch("d0",             &m_d0);
    m_outputTree->Branch("z0",             &m_z0);
    m_outputTree->Branch("pt",             &m_pt);
    m_outputTree->Branch("eta",            &m_eta);
    m_outputTree->Branch("x1",             &m_x1);
    m_outputTree->Branch("x2",             &m_x2);
    m_outputTree->Branch("x3",             &m_x3);
    m_outputTree->Branch("y1",             &m_y1);
    m_outputTree->Branch("y2",             &m_y2);
    m_outputTree->Branch("y3",             &m_y3);
    m_outputTree->Branch("z1",             &m_z1);
    m_outputTree->Branch("z2",             &m_z2);
    m_outputTree->Branch("z3",             &m_z3);
    m_outputTree->Branch("r1",             &m_r1);
    m_outputTree->Branch("r2",             &m_r2);
    m_outputTree->Branch("r3",             &m_r3);
    m_outputTree->Branch("quality",        &m_quality);
    m_outputTree->Branch("seedType",       &m_type);
    m_outputTree->Branch("givesTrack",     &m_givesTrack);
    m_outputTree->Branch("dzdr_b",         &m_dzdr_b);
    m_outputTree->Branch("dzdr_t",         &m_dzdr_t);
    m_outputTree->Branch("track_pt",       &m_trackPt);
    m_outputTree->Branch("track_eta",      &m_trackEta);

    std::string full_tree_name = "/" + m_treeFolder + "/" + tree_name;
    ATH_CHECK( m_thistSvc->regTree( full_tree_name.c_str(), m_outputTree ) );

    return StatusCode::SUCCESS;
  }

  void
  SiSpacePointsSeedMaker::newSpacePoint(InDet::SiSpacePointsSeedMakerEventData& data,
                                        const xAOD::SpacePoint* const& sp) const
  {
    if (m_fastTracking and skipSpacePoint(sp->x()-data.xbeam[0], sp->y()-data.ybeam[0], sp->z()-data.zbeam[0]))
      return;

    data.v_ActsSpacePointForSeed.emplace_back(sp);
    data.ns++;
    data.nsaz++;
  }

  bool SiSpacePointsSeedMaker::skipSpacePoint(float x, float y, float z) const {
    float R = std::hypotf(x,y);
    // At small R, we remove space points beyond |z|=200
    if (std::abs(z) > 200. && R < 50.)
      return true;
    // We also remove space points beyond eta=4. if their z is larger
    // than the max seed z0 (150.)
    float cotTheta = 27.2899;  // (4.0 eta) --> 27.2899 = 1/tan(2*arctan(exp(-4)))
    if (std::abs(z) - 150. > cotTheta * R)
      return true;
    return false;
  }

  void SiSpacePointsSeedMaker::pixInform(const Trk::SpacePoint* const& sp,
					     float *r)
  {
    const InDet::SiCluster *cl = static_cast<const InDet::SiCluster *>(sp->clusterList().first);
    const InDetDD::SiDetectorElement *de = cl->detectorElement();
    const Amg::Transform3D &Tp = de->surface().transform();
    r[3] = static_cast<float>(Tp(0, 2));
    r[4] = static_cast<float>(Tp(1, 2));
    r[5] = static_cast<float>(Tp(2, 2));
  }


  void SiSpacePointsSeedMaker::stripInform(InDet::SiSpacePointsSeedMakerEventData& data,
					       const Trk::SpacePoint* const& sp, 
					       float *r)
  {
    const InDet::SiCluster *c0 = static_cast<const InDet::SiCluster *>(sp->clusterList().first);
    const InDet::SiCluster *c1 = static_cast<const InDet::SiCluster *>(sp->clusterList().second);
    const InDetDD::SiDetectorElement *d0 = c0->detectorElement();
    const InDetDD::SiDetectorElement *d1 = c1->detectorElement();

    Amg::Vector2D lc0 = c0->localPosition();
    Amg::Vector2D lc1 = c1->localPosition();

    std::pair<Amg::Vector3D, Amg::Vector3D> e0 =
        (d0->endsOfStrip(InDetDD::SiLocalPosition(lc0.y(), lc0.x(), 0.)));
    std::pair<Amg::Vector3D, Amg::Vector3D> e1 =
        (d1->endsOfStrip(InDetDD::SiLocalPosition(lc1.y(), lc1.x(), 0.)));

    Amg::Vector3D s0(.5 * (e0.first + e0.second));
    Amg::Vector3D s1(.5 * (e1.first + e1.second));

    Amg::Vector3D b0(.5 * (e0.second - e0.first));
    Amg::Vector3D b1(.5 * (e1.second - e1.first));
    Amg::Vector3D d02(s0 - s1);

    // b0
    r[3] = static_cast<float>(b0[0]);
    r[4] = static_cast<float>(b0[1]);
    r[5] = static_cast<float>(b0[2]);

    // b1
    r[6] = static_cast<float>(b1[0]);
    r[7] = static_cast<float>(b1[1]);
    r[8] = static_cast<float>(b1[2]);

    // r0-r2
    r[9] = static_cast<float>(d02[0]);
    r[10] = static_cast<float>(d02[1]);
    r[11] = static_cast<float>(d02[2]);

    // r0
    r[12] = static_cast<float>(s0[0]) - data.xbeam[0];
    r[13] = static_cast<float>(s0[1]) - data.ybeam[0];
    r[14] = static_cast<float>(s0[2]) - data.zbeam[0];
  }

  StatusCode 
  SiSpacePointsSeedMaker::retrievePixel(const EventContext& ctx,
					InDet::SiSpacePointsSeedMakerEventData& data,
					const Trk::PRDtoTrackMap* /*prd_to_track_map_cptr*/) const
  {
    // get the xAOD::SpacePointContainer and loop on entries to check which space point
    // you want to use for seeding
    
    SG::ReadHandle< xAOD::SpacePointContainer > inputSpacePointContainer( m_actsSpacepointsPixel, ctx );
    if (not inputSpacePointContainer.isValid()){
      ATH_MSG_FATAL("xAOD::SpacePointContainer with key " << m_actsSpacepointsPixel.key() << " is not available...");
      return StatusCode::FAILURE;
    }
    const xAOD::SpacePointContainer* inputSpacePointCollection = inputSpacePointContainer.cptr();
    // TODO: here you need to write some lines to implement the
    // check on the used PDRs in previous tracking passes
    for (const xAOD::SpacePoint * sp : *inputSpacePointCollection) {
      newSpacePoint(data, sp);
    }
    
    return StatusCode::SUCCESS;
  }

  StatusCode
  SiSpacePointsSeedMaker::retrieveStrip(const EventContext& ctx,
					InDet::SiSpacePointsSeedMakerEventData& data,
					const Trk::PRDtoTrackMap* /*prd_to_track_map_cptr*/) const
  {
    // get the xAOD::SpacePointContainer and loop on entries to check which space point
    // you want to use for seeding
    
    SG::ReadHandle< xAOD::SpacePointContainer > inputSpacePointContainer( m_actsSpacepointsStrip, ctx );
    if (!inputSpacePointContainer.isValid()){
      ATH_MSG_FATAL("xAOD::SpacePointContainer with key " << m_actsSpacepointsStrip.key() << " is not available...");
      return StatusCode::FAILURE;
    }
    const xAOD::SpacePointContainer* inputSpacePointCollection = inputSpacePointContainer.cptr();
    // TODO: here you need to write some lines to implement the
    // check on the used PDRs in previous tracking passes
    for (const xAOD::SpacePoint * sp : *inputSpacePointCollection) {
      newSpacePoint(data, sp);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode 
  SiSpacePointsSeedMaker::retrieveOverlap(const EventContext& ctx,
					  InDet::SiSpacePointsSeedMakerEventData& data,
					  const Trk::PRDtoTrackMap* /*prd_to_track_map_cptr*/) const
  {
    // get the xAOD::SpacePointContainer and loop on entries to check which space point
    // you want to use for seeding
    
    SG::ReadHandle< xAOD::SpacePointContainer > inputSpacePointContainer( m_actsSpacepointsOverlap, ctx );
    if (!inputSpacePointContainer.isValid()){
      ATH_MSG_FATAL("xAOD::SpacePointContainer with key " << m_actsSpacepointsOverlap.key() << " is not available...");
      return StatusCode::FAILURE;
    }
    const xAOD::SpacePointContainer* inputSpacePointCollection = inputSpacePointContainer.cptr();
    // TODO: here you need to write some lines to implement the
    // check on the used PDRs in previous tracking passes
    for (const xAOD::SpacePoint * sp : *inputSpacePointCollection) {
      newSpacePoint(data, sp);
    }
    
    return StatusCode::SUCCESS;
  }

  void
  SiSpacePointsSeedMaker::buildBeamFrameWork(const EventContext& ctx,
						 InDet::SiSpacePointsSeedMakerEventData& data) const
  {
    SG::ReadCondHandle< InDet::BeamSpotData > beamSpotHandle { m_beamSpotKey, ctx };
    
    const Amg::Vector3D &cb = beamSpotHandle->beamPos();
    double tx = std::tan(beamSpotHandle->beamTilt(0));
    double ty = std::tan(beamSpotHandle->beamTilt(1));

    double phi = std::atan2(ty, tx);
    double theta = std::acos(1. / std::sqrt(1. + tx * tx + ty * ty));
    double sinTheta = std::sin(theta);
    double cosTheta = std::cos(theta);
    double sinPhi = std::sin(phi);
    double cosPhi = std::cos(phi);

    data.xbeam[0] = static_cast<float>(cb.x());
    data.xbeam[1] = static_cast<float>(cosTheta * cosPhi * cosPhi + sinPhi * sinPhi);
    data.xbeam[2] = static_cast<float>(cosTheta * sinPhi * cosPhi - sinPhi * cosPhi);
    data.xbeam[3] = -static_cast<float>(sinTheta * cosPhi);

    data.ybeam[0] = static_cast<float>(cb.y());
    data.ybeam[1] = static_cast<float>(cosTheta * cosPhi * sinPhi - sinPhi * cosPhi);
    data.ybeam[2] = static_cast<float>(cosTheta * sinPhi * sinPhi + cosPhi * cosPhi);
    data.ybeam[3] = -static_cast<float>(sinTheta * sinPhi);

    data.zbeam[0] = static_cast<float>(cb.z());
    data.zbeam[1] = static_cast<float>(sinTheta * cosPhi);
    data.zbeam[2] = static_cast<float>(sinTheta * sinPhi);
    data.zbeam[3] = static_cast<float>(cosTheta);

    // Acts Seed Tool requires both MagneticFieldContext and BeamSpotData
    // we need to retrieve them both from StoreGate
    // Read the b-field information
    SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle { m_fieldCondObjInputKey, ctx };
    if ( not readHandle.isValid() ) {
      throw std::runtime_error("Error while retrieving Atlas Field Cache Cond DB");
    }
    const AtlasFieldCacheCondObj* fieldCondObj{ *readHandle };
    if (fieldCondObj == nullptr) {
      throw std::runtime_error("Failed to retrieve AtlasFieldCacheCondObj with key " + m_fieldCondObjInputKey.key());
    }

    // Get the magnetic field
    // Using ACTS classes in order to be sure we are consistent
    Acts::MagneticFieldContext magFieldContext(fieldCondObj);

    // Beam Spot Position
    Acts::Vector3 beamPos( data.xbeam[0] * Acts::UnitConstants::mm,
                           data.ybeam[0] * Acts::UnitConstants::mm,
                           0. );

    // Magnetic Field
    ATLASMagneticFieldWrapper magneticField;
    Acts::MagneticFieldProvider::Cache magFieldCache = magneticField.makeCache( magFieldContext );
    Acts::Vector3 bField = *magneticField.getField( beamPos,
                                                    magFieldCache );

    data.bField[0] = bField[0];
    data.bField[1] = bField[1];
    data.bField[2] = bField[2];

    data.K = 2. * s_toTesla / (300. * bField[2]);

  }


  // ===================================================================== //  
  // Interface Methods
  // ===================================================================== //  

  void
  SiSpacePointsSeedMaker::newEvent(const EventContext& ctx,
				       InDet::SiSpacePointsSeedMakerEventData& data,
				       int iteration) const
  {
    // Store iteration into data for use in other methods
    data.iteration = iteration;
    if (iteration < 0)
      data.iteration = 0;

    // At the beginning of each iteration
    // clearing list of space points to be used for seeding
    InDet::SiSpacePointsSeedMakerEventData::clearPoolList(data.l_ITkSpacePointForSeed);
    data.i_ITkSpacePointForSeed = data.l_ITkSpacePointForSeed.begin();
    data.v_ActsSpacePointForSeed.clear();


    // clearing list of produced seeds
    InDet::SiSpacePointsSeedMakerEventData::clearPoolList(data.i_ITkSeeds);
    data.i_ITkSeed = data.i_ITkSeeds.begin();

    // First iteration:
    // -- event-specific configuration, i.e. beamspot and magnetic field
    // -- for default case: producing SSS
    // -- for fast tracking: producing PPP
    // Second iteration:
    // -- for default case: producing PPP
    // -- no additional iteration is foreseen for fast tracking case

    bool isPixel = (m_fastTracking or data.iteration == 1) and m_pixel;
    bool isStrip = not m_fastTracking and data.iteration == 0 and m_strip;

    // The Acts Seed tool requires beamspot information for the space points already here
    if (data.iteration == 0) 
      buildBeamFrameWork(ctx, data);

    // initialising the number of space points as well
    data.ns = 0;
    data.nsaz = 0;
    data.nsazv = 0;

    // Retrieve the Trk::PRDtoTrackMap
    const Trk::PRDtoTrackMap *prd_to_track_map_cptr = nullptr;
    if ( not m_prdToTrackMap.empty() ) {
      SG::ReadHandle<Trk::PRDtoTrackMap> prd_handle = SG::makeHandle( m_prdToTrackMap, ctx );
      if ( not prd_handle.isValid() ) {
        ATH_MSG_ERROR("Failed to read PRD to track association map: " << m_prdToTrackMap.key());
      }
      prd_to_track_map_cptr = prd_handle.get();
    }

    // Only retrieving the collections needed for the seed formation,
    // depending on the tool configuration and iteration

    // Retrieve Pixels
    if (isPixel and not retrievePixel(ctx, data, prd_to_track_map_cptr).isSuccess() ) {
      ATH_MSG_ERROR("Error while retrieving Pixel space points with key " << m_actsSpacepointsPixel.key());
    }

    // Retrieve Strips
    if (isStrip and not retrieveStrip(ctx, data, prd_to_track_map_cptr).isSuccess() ) {
      ATH_MSG_ERROR("Error while retrieving Strip space points with key " << m_actsSpacepointsStrip.key());
    }

    // Retrieve Overlaps, will go into Strip collection
    if ((isStrip and m_useOverlap) and not retrieveOverlap(ctx, data, prd_to_track_map_cptr).isSuccess() ) {
      ATH_MSG_ERROR("Error while retrieving Strip Overlap space points with key " <<  m_actsSpacepointsOverlap.key());
    }    

    data.initialized = true;
  }

  void
  SiSpacePointsSeedMaker::find3Sp(const EventContext& ctx,
				  InDet::SiSpacePointsSeedMakerEventData& data,
				  const std::list<Trk::Vertex>& /*lv*/) const
  {
    // Fast return if no sps are collected
    if ( data.v_ActsSpacePointForSeed.empty() )
      return;

    // select the ACTS seeding tool to call, if for PPP or SSS
    bool isPixel = (m_fastTracking or data.iteration == 1) and m_pixel;

    // this is use for a fast retrieval of the space points later
    std::vector<ITk::SiSpacePointForSeed*> sp_storage;

    // We can now run the Acts Seeding
    std::unique_ptr< ActsTrk::SeedContainer > seedPtrs = std::make_unique< ActsTrk::SeedContainer >();
    //cppcheck-suppress unreadVariable
    std::string combinationType = isPixel ? "PPP" : "SSS";
    ATH_MSG_DEBUG("Running Seed Finding (" << combinationType << ") ...");

    // Beam Spot Position
    Acts::Vector3 beamPos( data.xbeam[0] * Acts::UnitConstants::mm,
                           data.ybeam[0] * Acts::UnitConstants::mm,
                           data.zbeam[0] * Acts::UnitConstants::mm);

    Acts::Vector3 bField( data.bField[0], data.bField[1], data.bField[2] );

    StatusCode sc;

    if (isPixel)
      sc = m_seedsToolPixel->createSeeds( ctx,
                                          data.v_ActsSpacePointForSeed,
                                          beamPos,
                                          bField,
                                          *seedPtrs.get() );
    else {
      sc = m_seedsToolStrip->createSeeds( ctx,
                                          data.v_ActsSpacePointForSeed,
                                          beamPos,
                                          bField,
                                          *seedPtrs.get() );
    }

    if (sc == StatusCode::FAILURE) {
      ATH_MSG_ERROR("Error during seed production (" << combinationType << ")");
      return;
    }

    ATH_MSG_DEBUG("    \\__ Created " << seedPtrs->size() << " seeds");
    data.nsazv = seedPtrs->size();

    // Store seeds to data
    // We need now to convert the output to Athena object once again (i.e. ITk::SiSpacePointForSeed)
    // The seeds will be stored in data.i_ITkSeeds (both PPP and SSS seeds)
    if (m_doSeedConversion) {
      
      if (isPixel) {
	if (not convertPixelSeed(ctx, data, *seedPtrs.get())) {
	  ATH_MSG_FATAL("Error in pixel seed conversion");
	  return;
	}
      } else {
	if (not convertStripSeed(ctx, data, *seedPtrs.get())) {
	  ATH_MSG_FATAL("Error in strip seed conversion");
	  return;
	}
      }
    }

    data.i_ITkSeed = data.i_ITkSeeds.begin();
    data.nprint = 1;
    dump(data, msg(MSG::DEBUG));
  }

  const InDet::SiSpacePointsSeed*
  SiSpacePointsSeedMaker::next(const EventContext& /*ctx*/,
			       InDet::SiSpacePointsSeedMakerEventData& data) const
  {

    do {
      if (data.i_ITkSeed == data.i_ITkSeeds.end())
        return nullptr;

    // iterate until we find a valid seed satisfying certain quality cuts in set3
    } while (!(*data.i_ITkSeed++).set3(data.seedOutput, 1./(1000. * data.K)));

    /// then return this next seed candidate
    return &data.seedOutput;

  }

  void
  SiSpacePointsSeedMaker::writeNtuple(const InDet::SiSpacePointsSeed* seed,
				      const Trk::Track* track, 
				      int seedType,
				      long eventNumber) const
  {
    if (not m_writeNtuple) return;

    std::lock_guard<std::mutex> lock(m_mutex);

    if(track != nullptr) {
      m_trackPt = (track->trackParameters()->front()->pT())/1000.f;
      m_trackEta = std::abs(track->trackParameters()->front()->eta());
    }
    else {
      m_trackPt = -1.;
      m_trackEta = -1.; 
    }
    m_d0           =   seed->d0();
    m_z0           =   seed->zVertex();
    m_eta          =   seed->eta();
    m_x1           =   seed->x1();
    m_x2           =   seed->x2();
    m_x3           =   seed->x3();
    m_y1           =   seed->y1();
    m_y2           =   seed->y2();
    m_y3           =   seed->y3();      
    m_z1           =   seed->z1();
    m_z2           =   seed->z2();
    m_z3           =   seed->z3();
    m_r1           =   seed->r1();
    m_r2           =   seed->r2();
    m_r3           =   seed->r3();
    m_type         =   seedType;
    m_dzdr_b       =   seed->dzdr_b();
    m_dzdr_t       =   seed->dzdr_t();
    m_pt           =   seed->pt();
    m_givesTrack   =   !(track == nullptr);
    m_eventNumber  =   eventNumber;

    // Ok: protected by mutex.
    TTree* outputTree ATLAS_THREAD_SAFE = m_outputTree;
    outputTree->Fill();
  }

  bool
  SiSpacePointsSeedMaker::getWriteNtupleBoolProperty() const
  { return m_writeNtuple; }

  MsgStream&
  SiSpacePointsSeedMaker::dump(InDet::SiSpacePointsSeedMakerEventData& data,
				   MsgStream& out) const
  {
    if (data.nprint)
      return dumpEvent(data, out);
    return dumpConditions(data, out);
  }


  ///////////////////////////////////////////////////////////////////
  // Dumps conditions information into the MsgStream
  ///////////////////////////////////////////////////////////////////

  MsgStream &SiSpacePointsSeedMaker::dumpConditions(InDet::SiSpacePointsSeedMakerEventData& data, MsgStream &out) const
  {
    std::string s2, s3, s4, s5;

    int n = 42-m_actsSpacepointsPixel.key().size();
    s2.append(n,' ');
    s2.append("|");
    n     = 42-m_actsSpacepointsStrip.key().size();
    s3.append(n,' ');
    s3.append("|");
    n     = 42-m_actsSpacepointsOverlap.key().size();
    s4.append(n,' ');
    s4.append("|");
    n     = 42-m_beamSpotKey.key().size();
    s5.append(n,' ');
    s5.append("|");

    out<<"|---------------------------------------------------------------------|"
       <<endmsg;
    out<<"| Pixel    space points   | "<<m_actsSpacepointsPixel.key() <<s2
       <<endmsg;
    out<<"| Strip    space points   | "<<m_actsSpacepointsStrip.key()<<s3
       <<endmsg;
    out<<"| Overlap  space points   | "<<m_actsSpacepointsOverlap.key()<<s4
       <<endmsg;
    out<<"| BeamConditionsService   | "<<m_beamSpotKey.key()<<s5
       <<endmsg;
    out<<"| usePixel                | "
       <<std::setw(12)<<m_pixel 
       <<"                              |"<<endmsg;
    out<<"| useStrip                | "
       <<std::setw(12)<<m_strip
       <<"                              |"<<endmsg;
    out<<"| useStripOverlap         | "
       <<std::setw(12)<<m_useOverlap
       <<"                              |"<<endmsg;
    out<<"|---------------------------------------------------------------------|"
       <<endmsg;
    out<<"| Beam X center           | "
       <<std::setw(12)<<std::setprecision(5)<<data.xbeam[0]
       <<"                              |"<<endmsg;
    out<<"| Beam Y center           | "
       <<std::setw(12)<<std::setprecision(5)<<data.ybeam[0]
       <<"                              |"<<endmsg;
    out<<"| Beam Z center           | "
       <<std::setw(12)<<std::setprecision(5)<<data.zbeam[0]
       <<"                              |"<<endmsg;
    out<<"| Beam X-axis direction   | "
       <<std::setw(12)<<std::setprecision(5)<<data.xbeam[1]
       <<std::setw(12)<<std::setprecision(5)<<data.xbeam[2]
       <<std::setw(12)<<std::setprecision(5)<<data.xbeam[3]
       <<"      |"<<endmsg;
    out<<"| Beam Y-axis direction   | "
       <<std::setw(12)<<std::setprecision(5)<<data.ybeam[1]
       <<std::setw(12)<<std::setprecision(5)<<data.ybeam[2]
       <<std::setw(12)<<std::setprecision(5)<<data.ybeam[3]
       <<"      |"<<endmsg;
    out<<"| Beam Z-axis direction   | "
       <<std::setw(12)<<std::setprecision(5)<<data.zbeam[1]
       <<std::setw(12)<<std::setprecision(5)<<data.zbeam[2]
       <<std::setw(12)<<std::setprecision(5)<<data.zbeam[3]
       <<"      |"<<endmsg;
    out<<"|---------------------------------------------------------------------|"
       <<endmsg;
    return out;

  }

  ///////////////////////////////////////////////////////////////////
  // Dumps event information into the MsgStream
  ///////////////////////////////////////////////////////////////////

  MsgStream &SiSpacePointsSeedMaker::dumpEvent(InDet::SiSpacePointsSeedMakerEventData& data,
						   MsgStream &out) 
  {
    out<<"|---------------------------------------------------------------------|"
       <<endmsg;
    out<<"| ns                       | "
       <<std::setw(12)<<data.ns
       <<"                              |"<<endmsg;
    out<<"| nsaz                     | "
       <<std::setw(12)<<data.nsaz
       <<"                              |"<<endmsg;
    out<<"| seeds                    | "
       <<std::setw(12)<< data.nsazv
       <<"                              |"<<endmsg;
    out<<"|---------------------------------------------------------------------|"
       <<endmsg;
    return out;

  }

  bool
  SiSpacePointsSeedMaker::convertStripSeed(const EventContext& /*ctx*/,
					   InDet::SiSpacePointsSeedMakerEventData& data,
					   const ActsTrk::SeedContainer& seedPtrs) const {
    // If the read handle key for clusters is not empty, that means the xAOD->InDet Cluster link is available
    // Else we can use xAOD->Trk Space Point link
    // If none of these is available, we have a JO misconfguration!
    static const SG::AuxElement::Accessor< ElementLink< ::SpacePointCollection > > linkAcc("sctSpacePointLink");
    static const SG::AuxElement::Accessor< ElementLink< ::SpacePointOverlapCollection > > overlaplinkAcc("stripOverlapSpacePointLink");
    static const SG::AuxElement::Accessor< ElementLink< InDet::SCT_ClusterCollection > > stripLinkAcc("sctClusterLink");

    if (m_useClusters) {
      data.v_StripSpacePointForSeed.clear();
    }

    std::array<const Trk::SpacePoint*, 3> spacePoints {};
    std::array<ITk::SiSpacePointForSeed*, 3> stripSpacePointsForSeeds {};

    for (const ActsTrk::Seed* seed : seedPtrs) {
      // Retrieve/make space points
      if (not m_useClusters) {
	// Get the space point from the element link
	if (not linkAcc.isAvailable(*seed->sp()[0]) and
	    not overlaplinkAcc.isAvailable(*seed->sp()[0])) {
	  ATH_MSG_FATAL("no sctSpacePointLink/stripOverlapSpacePointLink for bottom sp!");
	  return false;
	}
	if (not linkAcc.isAvailable(*seed->sp()[1]) and
	    not overlaplinkAcc.isAvailable(*seed->sp()[1])) {
	  ATH_MSG_FATAL("no sctSpacePointLink/stripOverlapSpacePointLink for middle sp!");
	  return false;
	}
	if (not linkAcc.isAvailable(*seed->sp()[2])  and
	    not overlaplinkAcc.isAvailable(*seed->sp()[2])) {
	  ATH_MSG_FATAL("no sctSpacePointLink/stripOverlapSpacePointLink for top sp!");
	  return false;
	}
	
	for (std::size_t nsp(0ul); nsp < 3ul; ++nsp) {
	  spacePoints[nsp] = linkAcc.isAvailable(*seed->sp()[nsp])
	    ? *linkAcc(*seed->sp()[nsp])
	    : *overlaplinkAcc(*seed->sp()[nsp]);
	}
      } 
      else {
	// Get the clusters from the xAOD::Clusters and then make the space points
	const auto& bottom_idx = seed->sp()[0]->measurements();
	const auto& medium_idx = seed->sp()[1]->measurements();
	const auto& top_idx    = seed->sp()[2]->measurements();

	std::array<const xAOD::StripCluster*, 6> strip_cluster { 
	  reinterpret_cast<const xAOD::StripCluster*>(bottom_idx[0]),
	  reinterpret_cast<const xAOD::StripCluster*>(bottom_idx[1]),
	  reinterpret_cast<const xAOD::StripCluster*>(medium_idx[0]),
	  reinterpret_cast<const xAOD::StripCluster*>(medium_idx[1]),
	  reinterpret_cast<const xAOD::StripCluster*>(top_idx[0]),
	  reinterpret_cast<const xAOD::StripCluster*>(top_idx[1])
	    };
	
	if (not stripLinkAcc.isAvailable(*strip_cluster[0]) or 
	    not stripLinkAcc.isAvailable(*strip_cluster[1])){
	  ATH_MSG_FATAL("no sctClusterLink for clusters associated to bottom sp!");
	  return false;
	}
	if (not stripLinkAcc.isAvailable(*strip_cluster[2]) or 
	    not stripLinkAcc.isAvailable(*strip_cluster[3])){
	  ATH_MSG_FATAL("no sctClusterLink for clusters associated to middle sp!");
	  return false;
	}
	if (not stripLinkAcc.isAvailable(*strip_cluster[4]) or 
	    not stripLinkAcc.isAvailable(*strip_cluster[5])){
	  ATH_MSG_FATAL("no sctClusterLink for clusters associated to top sp!");
	  return false;
	}

	std::pair<std::size_t, std::size_t> key_b = std::make_pair(strip_cluster[0]->index(), strip_cluster[1]->index());
	std::pair<std::size_t, std::size_t> key_m = std::make_pair(strip_cluster[2]->index(), strip_cluster[3]->index());
	std::pair<std::size_t, std::size_t> key_t = std::make_pair(strip_cluster[4]->index(), strip_cluster[5]->index());

        std::unique_ptr<InDet::SCT_SpacePoint>& ptr_b = data.v_StripSpacePointForSeed[key_b];
	if (!ptr_b) {
	  ptr_b = std::make_unique<InDet::SCT_SpacePoint>(std::make_pair<IdentifierHash, IdentifierHash>(strip_cluster[0]->identifierHash(), strip_cluster[1]->identifierHash()),
											 Amg::Vector3D(seed->sp()[0]->x(), seed->sp()[0]->y(), seed->sp()[0]->z()),
											 std::make_pair<const Trk::PrepRawData*, const Trk::PrepRawData*>(*(stripLinkAcc(*strip_cluster[0])), *(stripLinkAcc(*strip_cluster[1]))));
	}
        std::unique_ptr<InDet::SCT_SpacePoint>& ptr_m = data.v_StripSpacePointForSeed[key_m];
	if (!ptr_m) {
	  ptr_m = std::make_unique<InDet::SCT_SpacePoint>(std::make_pair<IdentifierHash, IdentifierHash>(strip_cluster[2]->identifierHash(), strip_cluster[3]->identifierHash()),
											 Amg::Vector3D(seed->sp()[1]->x(), seed->sp()[1]->y(), seed->sp()[1]->z()),
											 std::make_pair<const Trk::PrepRawData*, const Trk::PrepRawData*>(*(stripLinkAcc(*strip_cluster[2])), *(stripLinkAcc(*strip_cluster[3]))));
	}
        std::unique_ptr<InDet::SCT_SpacePoint>& ptr_t = data.v_StripSpacePointForSeed[key_t];
	if (!ptr_t) {
	  ptr_t = std::make_unique<InDet::SCT_SpacePoint>(std::make_pair<IdentifierHash, IdentifierHash>(strip_cluster[4]->identifierHash(), strip_cluster[5]->identifierHash()),
											 Amg::Vector3D(seed->sp()[2]->x(), seed->sp()[2]->y(), seed->sp()[2]->z()),
											 std::make_pair<const Trk::PrepRawData*, const Trk::PrepRawData*>(*(stripLinkAcc(*strip_cluster[4])), *(stripLinkAcc(*strip_cluster[5]))));
	}
	
	spacePoints = {
	  data.v_StripSpacePointForSeed[key_b].get(),
	  data.v_StripSpacePointForSeed[key_m].get(),
	  data.v_StripSpacePointForSeed[key_t].get()
	};

      }

      for (unsigned int index = 0; index<3; index++) {
	float r[15];
	r[0] = seed->sp()[index]->x();
	r[1] = seed->sp()[index]->y();
	r[2] = seed->sp()[index]->z();
	stripInform(data, spacePoints[index], r);
	stripSpacePointsForSeeds[index] = new ITk::SiSpacePointForSeed(spacePoints[index], r);
	stripSpacePointsForSeeds[index]->setQuality(-seed->seedQuality());
      }
      
      data.i_ITkSeeds.emplace_back(stripSpacePointsForSeeds[0], stripSpacePointsForSeeds[1],
				   stripSpacePointsForSeeds[2], seed->z());
      data.i_ITkSeeds.back().setQuality(-seed->seedQuality());
    }
    
    return true;
  }

  bool 
  SiSpacePointsSeedMaker::convertPixelSeed(const EventContext& /*ctx*/,
					   InDet::SiSpacePointsSeedMakerEventData& data,
					   const ActsTrk::SeedContainer& seedPtrs) const {
    // If the read handle key for clusters is not empty, that means the xAOD->InDet Cluster link is available
    // Else we can use xAOD->Trk Space Point link
    // If none of these is available, we have a JO misconfguration!
    static const SG::AuxElement::Accessor< ElementLink< InDet::PixelClusterCollection > > pixelLinkAcc("pixelClusterLink");
    static const SG::AuxElement::Accessor< ElementLink< ::SpacePointCollection > > linkAcc("pixelSpacePointLink");

    if (m_useClusters) {
      data.v_PixelSpacePointForSeed.clear();
    }

    data.v_PixelSiSpacePointForSeed.clear();
    data.v_PixelSiSpacePointForSeed.reserve(data.ns);
    std::unordered_map<std::size_t, std::size_t> bridge_idx_sispacepoints;

    std::array<const Trk::SpacePoint*, 3> spacePoints {nullptr, nullptr, nullptr};
    std::array<ITk::SiSpacePointForSeed*, 3> pixelSpacePointsForSeeds {nullptr, nullptr, nullptr};

    for (const ActsTrk::Seed* seed : seedPtrs) {
      std::array<std::size_t, 3> indexes {
	seed->sp()[0]->index(),
	  seed->sp()[1]->index(),
	  seed->sp()[2]->index()
	  };

      const auto [bottom_idx, medium_idx, top_idx] = indexes;
      std::size_t max_index = std::max(bottom_idx, std::max(medium_idx, top_idx));
      if (data.v_PixelSpacePointForSeed.size() < max_index + 1) {
	data.v_PixelSpacePointForSeed.resize( max_index + 1 );
      }

      // Retrieve/make the space points!
      if (not m_useClusters) {
	// Get the Space Point from the element link
	if (not linkAcc.isAvailable(*seed->sp()[0])){
	  ATH_MSG_FATAL("no pixelSpacePointLink for bottom sp!");
	  return false;
	}
	if (not linkAcc.isAvailable(*seed->sp()[1])){
	  ATH_MSG_FATAL("no pixelSpacePointLink for middle sp!");
	  return false;
	}
	if (not linkAcc.isAvailable(*seed->sp()[2])){
	  ATH_MSG_FATAL("no pixelSpacePointLink for top sp!");
	  return false;
	}

	spacePoints = {
	  *linkAcc(*seed->sp()[0]),
	  *linkAcc(*seed->sp()[1]),
	  *linkAcc(*seed->sp()[2])
	};      
      }
      else {
	// Get the clusters from the xAOD::Clusters and then make the space points
	const xAOD::PixelCluster* bottom_cluster = reinterpret_cast<const xAOD::PixelCluster*>(seed->sp()[0]->measurements()[0]);
	const xAOD::PixelCluster* medium_cluster = reinterpret_cast<const xAOD::PixelCluster*>(seed->sp()[1]->measurements()[0]);
	const xAOD::PixelCluster* top_cluster = reinterpret_cast<const xAOD::PixelCluster*>(seed->sp()[2]->measurements()[0]);

	if (not pixelLinkAcc.isAvailable(*bottom_cluster)) {
	  ATH_MSG_FATAL("no pixelClusterLink for cluster associated to bottom sp!");
	  return false;
	}
	if (not pixelLinkAcc.isAvailable(*medium_cluster)) {
	  ATH_MSG_FATAL("no pixelClusterLink for cluster associated to middle sp!");
	  return false;
	}
	if (not pixelLinkAcc.isAvailable(*top_cluster)) {
	  ATH_MSG_FATAL("no pixelClusterLink for cluster associated to top sp!");
	  return false;
	}

	if (not data.v_PixelSpacePointForSeed[bottom_idx]) 
	  data.v_PixelSpacePointForSeed[bottom_idx] = std::make_unique<InDet::PixelSpacePoint>(bottom_cluster->identifierHash(), *(pixelLinkAcc(*bottom_cluster)));
	if (not data.v_PixelSpacePointForSeed[medium_idx]) 
	  data.v_PixelSpacePointForSeed[medium_idx] = std::make_unique<InDet::PixelSpacePoint>(medium_cluster->identifierHash(), *(pixelLinkAcc(*medium_cluster)));
	if (not data.v_PixelSpacePointForSeed[top_idx]) 
	  data.v_PixelSpacePointForSeed[top_idx] = std::make_unique<InDet::PixelSpacePoint>(top_cluster->identifierHash(), *(pixelLinkAcc(*top_cluster)));
	
	spacePoints =  { 
	  data.v_PixelSpacePointForSeed[bottom_idx].get(),
	  data.v_PixelSpacePointForSeed[medium_idx].get(),
	  data.v_PixelSpacePointForSeed[top_idx].get()
	};

      }
           
      for (int index = 0; index<3; ++index) {
	std::size_t sp_idx = indexes[index];

	// Try add an element. 
	// If already there just return the old value
	// If not present, we add a new element to it
	auto [itr, outcome] = bridge_idx_sispacepoints.try_emplace(sp_idx, data.v_PixelSiSpacePointForSeed.size());
	std::size_t mapped_idx = itr->second;
	// We added a new element
	if (outcome) { 
	  float r[15];
	  r[0] = seed->sp()[index]->x();
	  r[1] = seed->sp()[index]->y();
	  r[2] = seed->sp()[index]->z();
	  pixInform(spacePoints[index], r);	 
	  data.v_PixelSiSpacePointForSeed.emplace_back( spacePoints[index], r );

	}
	data.v_PixelSiSpacePointForSeed[mapped_idx].setQuality(-seed->seedQuality());
	pixelSpacePointsForSeeds[index] = &data.v_PixelSiSpacePointForSeed[mapped_idx];
      }
      
      data.i_ITkSeeds.emplace_back(pixelSpacePointsForSeeds[0],
				   pixelSpacePointsForSeeds[1],
				   pixelSpacePointsForSeeds[2],
				   seed->z());
      data.i_ITkSeeds.back().setQuality(-seed->seedQuality());
    }    
    
    return true;
  }
  
}

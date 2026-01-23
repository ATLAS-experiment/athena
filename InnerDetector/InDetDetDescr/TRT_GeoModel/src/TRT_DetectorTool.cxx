/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TRT_DetectorTool.h"
#include "TRTDetectorFactory_Full.h"
#include "TRTDetectorFactory_Lite.h"

#include "GeoModelUtilities/GeoModelExperiment.h"

#include "GeoModelUtilities/DecodeVersionKey.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

#include "DetDescrConditions/AlignableTransformContainer.h"
#include "TRT_ConditionsData/StrawDxContainer.h"
#include "PathResolver/PathResolver.h"
#include "SGTools/DataProxy.h"

#include <memory>

/////////////////////////////////// Constructor //////////////////////////////////
//
TRT_DetectorTool::TRT_DetectorTool( const std::string& type, const std::string& name, const IInterface* parent )
  : GeoModelTool( type, name, parent )
{
}

//////////////  Create the Detector Node corresponding to this tool //////////////
//
StatusCode TRT_DetectorTool::create()
{
  // Get the detector configuration.
  ATH_CHECK( m_geoDbTagSvc.retrieve());

  ServiceHandle<IRDBAccessSvc> accessSvc(m_geoDbTagSvc->getParamSvcName(),name());
  ATH_CHECK( accessSvc.retrieve());

  // Locate the top level experiment node
  GeoModelExperiment* theExpt{nullptr};
  ATH_CHECK(detStore()->retrieve(theExpt,"ATLAS"));
  GeoPhysVol *world = theExpt->getPhysVol();

  // Retrieve the Geometry DB Interface
  ATH_CHECK( m_geometryDBSvc.retrieve() );

  // Pass athena services to factory, etc
  m_athenaComps.setDetStore(detStore().operator->());
  m_athenaComps.setGeoDbTagSvc(m_geoDbTagSvc.get());
  m_athenaComps.setRDBAccessSvc(accessSvc.get());
  m_athenaComps.setGeometryDBSvc(m_geometryDBSvc.get());

  std::unique_ptr<TRTStrawStatusAccessor> strawStatusAccessor;
  ATH_CHECK(m_sumTool.retrieve(DisableTool{ !m_dumpStrawStatus }));
  if(!m_dumpStrawStatus && (m_doArgonMixture || m_doKryptonMixture) ) {
    // Read Straw Statuses from the ASCII file
    strawStatusAccessor = std::make_unique<TRTStrawStatusAccessor>();
    const std::string strawStatusPath = PathResolverFindCalibFile(m_strawStatusFile);
    if (strawStatusPath.empty()) {
      ATH_MSG_ERROR("Failed to resolve path for StrawStatusFile: " << m_strawStatusFile << ", the job will fail now.");
      return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE("StrawStatusFile: " << m_strawStatusFile << ", resolved path: " << strawStatusPath);
    strawStatusAccessor->fill(strawStatusPath);
  }

  GeoModelIO::ReadGeoModel* sqliteReader  = m_geoDbTagSvc->getSqliteReader();
  //
  // If we are using the SQLite reader, then we are not building the raw geometry but
  // just locating it and attaching to readout geometry and various other actions
  // taken in this factory.
  //
  if (sqliteReader) {
    ATH_MSG_INFO( " Building TRT geometry from GeoModel factory TRTDetectorFactory_Lite" );
    TRTDetectorFactory_Lite theTRTFactory(sqliteReader,
					  &m_athenaComps,
					  std::move(strawStatusAccessor),
					  m_useOldActiveGasMixture,
					  m_DC2CompatibleBarrelCoordinates,
					  m_alignable,
					  m_useDynamicAlignFolders
					  );

    theTRTFactory.create(world);
    m_manager=theTRTFactory.getDetectorManager();
  }
  else {
    DecodeVersionKey versionKey(m_geoDbTagSvc.get(), "TRT");

    ATH_MSG_INFO( "Building TRT with Version Tag: "<< versionKey.tag() << " at Node: " << versionKey.node() );

    // Print the TRT version tag:
    std::string trtVersionTag = accessSvc->getChildTag("TRT", versionKey.tag(), versionKey.node());
    ATH_MSG_INFO("TRT Version: " << trtVersionTag );

    // Check if version is empty. If so, then the TRT cannot be built. This may or may not be intentional. We
    // just issue an INFO message.
    if (trtVersionTag.empty()) {
      ATH_MSG_INFO("No TRT Version. TRT will not be built." );
      return StatusCode::SUCCESS;
    }

    ATH_MSG_DEBUG( "Keys for TRT Switches are "  << versionKey.tag()  << "  " << versionKey.node() );
    IRDBRecordset_ptr switchSet =  accessSvc->getRecordsetPtr("TRTSwitches", versionKey.tag(), versionKey.node());
    const IRDBRecord    *switches   = (*switchSet)[0];

    if (switches->getInt("DC1COMPATIBLE")) {
      ATH_MSG_ERROR( "DC1COMPATIBLE flag set in database, but DC1 is no longer supported in the code!!");
      return StatusCode::FAILURE;
    }

    m_DC2CompatibleBarrelCoordinates = switches->getInt("DC2COMPATIBLE");
    m_useOldActiveGasMixture         = ( switches->getInt("GASVERSION") == 0 );
    m_initialLayout                  = switches->getInt("INITIALLAYOUT");

    // Check if the new switches exists:
    if (m_doArgonMixture || m_doKryptonMixture ){
      if      ( switches->getInt("DOARGONMIXTURE") == 0) { m_doArgonMixture = false; }
      else if ( switches->getInt("DOARGONMIXTURE") == 1) { m_doArgonMixture = true; }

      if      ( switches->getInt("DOKRYPTONMIXTURE") == 0) { m_doKryptonMixture = false; }
      else if ( switches->getInt("DOKRYPTONMIXTURE") == 1) { m_doKryptonMixture = true; }
    }

    ATH_MSG_INFO( "Creating the TRT" );
    ATH_MSG_INFO( "TRT Geometry Options:" << std::boolalpha );
    ATH_MSG_INFO( "  UseOldActiveGasMixture         = " << m_useOldActiveGasMixture );
    ATH_MSG_INFO( "  Do Argon    = " << m_doArgonMixture );
    ATH_MSG_INFO( "  Do Krypton  = " << m_doKryptonMixture );
    ATH_MSG_INFO( "  DC2CompatibleBarrelCoordinates = " << m_DC2CompatibleBarrelCoordinates );
    ATH_MSG_INFO( "  InitialLayout                  = " << m_initialLayout );
    ATH_MSG_INFO( "  Alignable                      = " << m_alignable );
    ATH_MSG_INFO( "  VersioName                     = " << switches->getString("VERSIONNAME") );

    ATH_MSG_INFO( " Building TRT geometry from GeoModel factory TRTDetectorFactory_Full" );

    TRTDetectorFactory_Full theTRTFactory(&m_athenaComps,
					  m_dumpStrawStatus ? m_sumTool.get() : nullptr,
					  std::move(strawStatusAccessor),
					  m_useOldActiveGasMixture,
					  m_DC2CompatibleBarrelCoordinates,
					  m_alignable,
					  m_doArgonMixture,
					  m_doKryptonMixture,
					  m_useDynamicAlignFolders
					  );
    theTRTFactory.create(world);
    m_manager=theTRTFactory.getDetectorManager();

  }

  // Register the TRTDetectorNode instance with the Transient Detector Store
  if (!m_manager) return StatusCode::FAILURE;

  theExpt->addManager(m_manager);
  ATH_CHECK(detStore()->record(m_manager,m_manager->getName()));
  return StatusCode::SUCCESS;
}

StatusCode TRT_DetectorTool::clear()
{
  SG::DataProxy* proxy = detStore()->proxy(ClassID_traits<InDetDD::TRT_DetectorManager>::ID(),m_manager->getName());
  if(proxy) {
    proxy->reset();
    m_manager = nullptr;
  }
  return StatusCode::SUCCESS;
}

StatusCode
TRT_DetectorTool::align()
//The manager align call invalidates all elements
{
  MsgStream log(msgSvc(), name());
  if (!m_manager) {
    msg(MSG::WARNING) << "Manager does not exist" << endmsg;
    return StatusCode::FAILURE;
  }
  if (m_alignable) {
    return const_cast<InDetDD::TRT_DetectorManager*>(m_manager)->align();
  } else {
    msg(MSG::DEBUG) << "Alignment disabled. No alignments applied" << endmsg;
    return StatusCode::SUCCESS;
  }
}

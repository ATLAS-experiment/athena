/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//
// Implementation file for TRT_CablingSvc class
//

#include "TRT_CablingSvc.h"

#include "eformat/SourceIdentifier.h"  // change to new eformat v3
#include "PathResolver/PathResolver.h"
  // Tool 
#include "GaudiKernel/IToolSvc.h"


using eformat::helper::SourceIdentifier; 

// Constructor
TRT_CablingSvc::TRT_CablingSvc( const std::string& name, 
			      ISvcLocator * pSvcLocator)
   : base_class( name, pSvcLocator )
{
}

// Initialisation
StatusCode TRT_CablingSvc::initialize( )
{
  StatusCode sc;
  ATH_MSG_INFO( "TRT_CablingSvc::initialize" );

  // Retrieve Detector Store
  ServiceHandle<StoreGateSvc> detStore("DetectorStore",name());
  ATH_CHECK(detStore.retrieve());
  ATH_CHECK(detStore->retrieve(m_manager,"TRT"));

  // Get ToolSvc
  SmartIF<IToolSvc> toolSvc{service("ToolSvc")};
  ATH_CHECK( toolSvc.isValid() );

  // Get tool for filling of cabling data
  std::string toolType;
  if (m_manager->getLayout()=="TestBeam")
  {
    m_TRTLayout = 1;
    toolType = "TRT_FillCablingData_TB04";
    ATH_MSG_INFO( "TRT TB04 Cabling" ); 
    if(StatusCode::SUCCESS !=toolSvc->retrieveTool(toolType,m_cablingTool_TB))
    {
      ATH_MSG_ERROR( " Can't get TRT_FillCablingData_TB04 tool " );
      return StatusCode::FAILURE;
    }
    m_cabling = m_cablingTool_TB->fillData();
  }
    // DC1
  else if ((m_manager->getLayout() == "Initial") || 
	   (m_manager->getLayout() == "Final") )
  {
    m_TRTLayout = 6;
    toolType = "TRT_FillCablingData_DC3";
    ATH_MSG_INFO( "TRT DC3 Cabling" ); 
    if( StatusCode::SUCCESS !=
	toolSvc->retrieveTool(toolType,m_cablingTool_DC3) )
    {
      ATH_MSG_ERROR( " Can't get TRT_FillCablingData_DC3 tool " );
      return StatusCode::FAILURE;
    }

    m_cabling = m_cablingTool_DC3->fillData();

  }
  else if ( m_manager->getLayout()=="SR1" )      // SR1 Barrel cosmics layout
  {
    m_TRTLayout = 4;
    toolType = "TRT_FillCablingData_SR1";
    ATH_MSG_INFO( "TRT SR1 Cabling" ); 
    if(StatusCode::SUCCESS !=toolSvc->retrieveTool(toolType,m_cablingTool_SR1))
    {
      ATH_MSG_ERROR( " Can't get TRT_FillCablingData_SR1 tool " );
      return StatusCode::FAILURE;
    }

    m_cabling = m_cablingTool_SR1->fillData();
  }
  else if ( m_manager->getLayout()=="SR1-EndcapC" )  // SR1 EC C cosmics layout
  {
    m_TRTLayout = 5;
    toolType = "TRT_FillCablingData_SR1_ECC";
    ATH_MSG_INFO( "TRT SR1 Cabling" ); 
    if( StatusCode::SUCCESS !=
	toolSvc->retrieveTool(toolType,m_cablingTool_SR1_ECC) )
    {
      ATH_MSG_ERROR( " Can't get TRT_FillCablingData_SR1_ECC tool " );
      return StatusCode::FAILURE;
    }

    m_cabling = m_cablingTool_SR1_ECC->fillData();
  }
  else
  {
     ATH_MSG_FATAL( " Layout is not defined for TRT_FillCablingData (" << m_manager->getLayout() << ")! " );
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO( "TRT_CablingSvc::initializiation finished" );
  
  return sc;
}


std::vector<uint32_t> TRT_CablingSvc::getRobID(Identifier& id) const
{
  // TB Case
  if (m_TRTLayout == 1)
  {
    return m_cablingTool_TB->getRobID(id);
  }
  // SR1 Barrel
  else if ( m_TRTLayout == 4 )
  {
     return m_cablingTool_SR1->getRobID( id );
  }
  // SR1 EC C
  else if ( m_TRTLayout == 5 )
  {
     return m_cablingTool_SR1_ECC->getRobID( id );
  }
    // DC3
  else if ( m_TRTLayout == 6 )
  {
     return m_cablingTool_DC3->getRobID(id);
  }
  else
  {
    std::vector<uint32_t> v;
    return v;
  }
}

Identifier TRT_CablingSvc::getIdentifier(const eformat::SubDetector&,
  const unsigned& rod, const int& bufferOffset, 
  IdentifierHash& hashId) const
{
  int intRod = (int) rod;

  // TB04, SR1 or DC3 Case 
  hashId = m_cabling->get_identifierHashForAllStraws(intRod, bufferOffset);
  return m_cabling->get_identifierForAllStraws(intRod, bufferOffset);
}

/*
 * getBufferOffset( strawId ) -
 * get Offset into ROD buffer given straw Identifier StrawId.
 *
 */
uint32_t TRT_CablingSvc::getBufferOffset( const Identifier &StrawId )
{
 
   // DC3 Case
   if ( m_TRTLayout == 6 )
   {
      return m_cabling->get_BufferOffset( StrawId );
   }
   else
   {
     ATH_MSG_FATAL( "TRT_CablingSvc::getBufferOffset called in invalid case !" );
     assert(0);
   }

   return 0;
}

const std::vector<uint32_t>& TRT_CablingSvc::getAllRods() const
{
  return m_cabling->get_allRods();
}

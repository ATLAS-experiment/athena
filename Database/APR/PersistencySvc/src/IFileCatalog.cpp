/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiUtils/IFileCatalog.h"
#include "GaudiUtils/IFileCatalogMgr.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"

#include <algorithm>
#include "POOLCore/SystemTools.h"
#include "PersistencySvc/IFileCatalog.h"

#include <exception>

using namespace pool;

pool::IFileCatalog::IFileCatalog()
  : AthMessaging( Gaudi::svcLocator()->service<IMessageSvc>("MessageSvc").get(), "APRFileCatalog" ),
    m_mgr (Gaudi::svcLocator()->service<Gaudi::IFileCatalogMgr>( "Gaudi::MultiFileCatalog" )),
    m_fc (m_mgr)
{
   // set the output level of this service
   setLevel( SystemTools::GetOutputLvl() );
   // set the output level of the XMLCatalog component - works only if the Gaudi AppMgr was initialized
   Gaudi::svcLocator()->service<IMessageSvc>("MessageSvc")
      ->setOutputLevel("XMLCatalog", SystemTools::GetOutputLvl() );
}


void pool::IFileCatalog::
getFirstPFN( const std::string& fid, std::string& pfn, std::string& tech ) const
{
   Files   pfns_techs;
   getPFNs( fid, pfns_techs );
   if( pfns_techs.empty() ) {
      pfn.clear();
      tech.clear();
   } else {
      pfn = pfns_techs[0].first;
      tech = pfns_techs[0].second;
   }
}


/// Get FID and filetype for a given PFN
void pool::IFileCatalog::
lookupFileByPFN( const std::string& pfn, std::string& fid, std::string& tech ) const
{
   fid = lookupPFN( pfn );
   if( !fid.empty() ) {
      Files   pfns_techs;
      getPFNs( fid, pfns_techs );
      tech.clear();
      for( const auto& attr_pair: pfns_techs ) {
         if( attr_pair.first == pfn )
            tech = attr_pair.second;
      }
   }
}
   

/// Register PFN, assign new FID if not given
void pool::IFileCatalog::
registerPFN( const std::string& pfn, const std::string& ftype, std::string& fid )
{
   if( existsPFN(pfn) ) {
      throw std::runtime_error("PFN '" + pfn + "' already registered (APR: \" registerPFN \" from \" FileCatalog \")");
   }
   if( fid.empty() ) fid = m_fc->createFID();
   ATH_MSG_DEBUG("Registering PFN=" << pfn << " of type=" << ftype << " GUID=" << fid);
   m_fc->registerPFN(fid, pfn, ftype);
}


// ------------------------- Catalog Manager interface

/// Add new catalog identified by name to the existing ones
void pool::IFileCatalog::addCatalog( const std::string& connect, bool forWriting )
{
   std::string url;
   if( connect.empty() ) {
      url = "file:PoolFileCatalog.xml";
   } else {
      auto pos = connect.find("file:");
      url = (pos==std::string::npos)? "file:" + connect : connect.substr(pos);
   }
   Catalogs& cats = m_mgr->catalogs();     
   auto i = std::find_if( cats.begin(), cats.end(),
                          [&]( const Gaudi::IFileCatalog* f )
                          { return url == f->connectInfo(); } );
   if( i==cats.end() ) {
      // add a new catalog
      const std::string fullconnectstr = "xmlcatalog_" + url;
      ATH_MSG_DEBUG("addCatalog(\"" << fullconnectstr << "\")" );
      m_mgr->addCatalog( fullconnectstr );
   }
   if( forWriting ) {
      m_mgr->setWriteCatalog( m_mgr->findCatalog( url, true ) );
   }
}
     


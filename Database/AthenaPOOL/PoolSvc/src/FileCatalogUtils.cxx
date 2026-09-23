/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PoolSvc/FileCatalogUtils.h"

#include <algorithm>
#include <exception>

void FileCatalogUtils::
getFirstPFN( const Gaudi::IFileCatalog& fc,
             const std::string& fid, std::string& pfn, std::string& tech )
{
   Gaudi::IFileCatalog::Files pfns_techs;
   fc.getPFN( fid, pfns_techs );
   if( pfns_techs.empty() ) {
      pfn.clear();
      tech.clear();
   } else {
      pfn = pfns_techs[0].first;
      tech = pfns_techs[0].second;
   }
}


void FileCatalogUtils::
lookupFileByPFN( const Gaudi::IFileCatalog& fc,
                  const std::string& pfn, std::string& fid, std::string& tech )
{
   fid = fc.lookupPFN( pfn );
   tech.clear();
   if( !fid.empty() ) {
      Gaudi::IFileCatalog::Files pfns_techs;
      fc.getPFN( fid, pfns_techs );
      for( const auto& attr_pair: pfns_techs ) {
         if( attr_pair.first == pfn )
            tech = attr_pair.second;
      }
   }
}


void FileCatalogUtils::
registerPFN( Gaudi::IFileCatalog& fc,
             const std::string& pfn, const std::string& ftype, std::string& fid )
{
   if( fc.existsPFN(pfn) ) {
      throw std::runtime_error("PFN '" + pfn + "' already registered (APR: \" registerPFN \" from \" FileCatalog \")");
   }
   if( fid.empty() ) fid = fc.createFID();
   fc.registerPFN(fid, pfn, ftype);
}


std::string FileCatalogUtils::
addCatalog( Gaudi::IFileCatalogMgr& mgr, const std::string& connect, bool forWriting )
{
   std::string url;
   if( connect.empty() ) {
      url = "file:PoolFileCatalog.xml";
   } else {
      auto pos = connect.find("file:");
      url = (pos==std::string::npos)? "file:" + connect : connect.substr(pos);
   }
   Gaudi::IFileCatalogMgr::Catalogs& cats = mgr.catalogs();
   auto i = std::find_if( cats.begin(), cats.end(),
                          [&]( const Gaudi::IFileCatalog* f )
                          { return url == f->connectInfo(); } );
   std::string addedConnectStr;
   if( i==cats.end() ) {
      // add a new catalog
      addedConnectStr = "xmlcatalog_" + url;
      mgr.addCatalog( addedConnectStr );
   }
   if( forWriting ) {
      mgr.setWriteCatalog( mgr.findCatalog( url, true ) );
   }
   return addedConnectStr;
}

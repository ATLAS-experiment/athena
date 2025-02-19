/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RNTCOLLECTION_H
#define RNTCOLLECTION_H

#include "CollectionBase/ICollection.h"
#include "CollectionBase/CollectionDescription.h"
#include "CollectionBase/CollectionRowBuffer.h"

#include "FileCatalog/IFileCatalog.h"
#include "CoralBase/MessageStream.h"

#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/SmartIF.h"
#include "Gaudi/PluginService.h"

#include <string>
#include <vector>

class TFile;
class IFileMgr;

// Import classes from experimental namespace for the time being
namespace ROOT::Experimental {
   class RNTupleReader;
}

namespace pool {

   class ISession; 

   namespace RootCollection {

      using RNTupleReader = ROOT::Experimental::RNTupleReader;
      class Attribute;
      class AttributeSpecification;
  
      /**
         @brief Collection (and CollectionProxy) implementation based on RNTuple
      */

      class RNTCollection :  public ICollection {
    
     public:
	typedef Gaudi::PluginService::Factory<ICollection*( const ICollectionDescription*, ICollection::OpenMode, ISession*)> Factory;
    
        /// Constructor
        /// @param session If you want to access the referenced objects you have to provide an ISession
        /// @param connection The location of the collection file is uniquely defined by the parameters name and connection
        /// @param name The location of the collection file is uniquely defined by the parameters name and connection
        /// @param mode The open mode of the collection
        ///
        /// - Without use of FileCatalog:
        ///   - The path to the collection file is simply created by the following concatenation:\n
        ///     connection+name+".root"
        ///   - name: Name of the collection file
        ///   - connection:
        ///     - It can be a relative or absolute path
        ///     - In case of an empty connection string it is assumed that the file is located in the current directory
        ///     - Remote access via rootd: e.g. "root://pcepsft02.cern.ch:9090//localdisk/ \n 
        ///       Further documentation can be found in the class description of TNetFile
        ///       (http://root.cern.ch/root/html/TNetFile.html)
        /// .
        /// .
        /// - Utilization of FileCatalog:
        ///   - This mode is triggered if the name parameter starts with one of the following prefixes
        ///     "PFN:", "FID:" or "LFN:". 
        ///   - According to the prefix the name is interpreted as 
        ///     Physical File Name, unique File ID or Logical File Name
        ///   - The connection string is interpreted as URI of the FileCatalog. 
        ///     The collection retrieves the FileCatalog defined by the given URI from FileCatalogMap.
        ///     A default file catalog (empty connection string) can be defined there.
        
    
        RNTCollection(  const pool::ICollectionDescription* description,
                         pool::ICollection::OpenMode mode,
                         pool::ISession* );

        /// Destructor
        ~RNTCollection();
    
        /// Return openMode
        virtual ICollection::OpenMode openMode() const; 

        /// Explicitly re-opens the collection after it has been closed.
        virtual void open();
    
        /// Checks if the collection is open.
        virtual bool isOpen() const;

        /// Adds a new row of data to the collection.
        virtual void insertRow( const pool::CollectionRowBuffer& inputRowBuffer );

        /// Commits the last changes made to the collection
        virtual void commit( bool restartTransaction = false );
    
        /// Explicitly closes the collection
        virtual void close();
    
        /// Returns an object used to describe the collection properties.
        virtual const ICollectionDescription& description() const;

        /// Returns an object used to query the collection.
        virtual ICollectionQuery*             newQuery();

     private:    
        /// copying unimplemented in this class.
        RNTCollection(const RNTCollection &) = delete;
        RNTCollection & operator = (const RNTCollection &) = delete;
    
        void delayedFileOpen( const std::string& method );
        std::unique_ptr< RNTupleReader > getCollectionRNTuple();

        bool fileCatalogRequired() const;
        std::string retrievePFN() const;
        std::string retrieveFID();
        std::string retrieveUniquePFN(const FileCatalog::FileID& fid);
        std::string retrieveBestPFN(const FileCatalog::FileID& fid)const;  
        void retrieveFileCatalog()const;

        void cleanup();

        std::unique_ptr< RNTupleReader >     m_reader;
        CollectionDescription                m_description;
        
        std::string                          m_name;
        std::string                          m_fileName;
        ICollection::OpenMode                m_mode;
        TFile*                               m_file;
        ISession*                            m_session;
        bool                                 m_open;
        bool                                 m_readOnly;
        
        std::unique_ptr<pool::IFileCatalog> m_fileCatalog;
        coral::MessageStream                m_poolOut;

        SmartIF<IFileMgr>                    m_fileMgr;
      };
   }
}
#endif


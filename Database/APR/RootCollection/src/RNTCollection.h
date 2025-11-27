/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RNTCOLLECTION_H
#define RNTCOLLECTION_H

#include "CollectionBase/ICollection.h"
#include "CollectionBase/CollectionDescription.h"
#include "CollectionBase/CollectionRowBuffer.h"

#include "POOLCore/DbPrint.h"

#include "GaudiKernel/IFileMgr.h"
#include "GaudiKernel/SmartIF.h"
#include "Gaudi/PluginService.h"


#include <string>
#include <memory>

#include "RVersion.h"

class TFile;
class IFileMgr;

namespace ROOT {
   class RNTupleModel;
   class RNTupleWriter;
   class RNTupleReader;
}

namespace pool {

   class ISession; 

   namespace RootCollection {
      class Attribute;
      class AttributeSpecification;

      static constexpr auto MODULE_NAME = "RootCollection";
  
      /**
         @brief Collection (and CollectionProxy) implementation based on RNTuple
      */

      class RNTCollection :  public ICollection, public APRMessaging {
    
     public:
	typedef Gaudi::PluginService::Factory<ICollection*( const ICollectionDescription*, ICollection::OpenMode, ISession*)> Factory;
    
        /// Constructor
        /// @param session If you want to access the referenced objects you have to provide an ISession
        /// @param connection The location of the collection file is uniquely defined by the parameters name and connection
        /// @param name The location of the collection file is uniquely defined by the parameters name and connection
        /// @param mode The open mode of the collection
        ///
        ///   - The path to the collection file is simply created by the following concatenation:\n
        ///     connection+name+".root"
        ///   - name: Name of the collection file
        ///   - connection:
        ///     - It can be a relative or absolute path
        ///     - In case of an empty connection string it is assumed that the file is located in the current directory
        
    
        RNTCollection(  const pool::ICollectionDescription* description,
                         pool::ICollection::OpenMode mode,
                         pool::ISession* );

        /// Destructor
        ~RNTCollection();
    
        /// Return openMode
        virtual ICollection::OpenMode openMode() const final override; 

        /// Explicitly re-opens the collection after it has been closed.
        virtual void open() final override;
    
        /// Checks if the collection is open.
        virtual bool isOpen() const final override;

        /// Adds a new row of data to the collection.
        virtual void insertRow( const pool::CollectionRowBuffer& inputRowBuffer ) final override;

        /// Commits the last changes made to the collection
        virtual void commit( bool restartTransaction = false ) final override;
    
        /// Explicitly closes the collection
        virtual void close() final override;
    
        /// Returns an object used to describe the collection properties.
        virtual const ICollectionDescription& description() const final override;

        /// Returns a cursor for the collection.
        virtual ICollectionCursor&         cursor() final override;

     private:    
        /// copying unimplemented in this class.
        RNTCollection(const RNTCollection &) = delete;
        RNTCollection & operator = (const RNTCollection &) = delete;
    
        void delayedFileOpen(const std::string& method);
        std::unique_ptr<ROOT::RNTupleReader> getCollectionRNTuple();
        void addField(ROOT::RNTupleModel* model, const std::string& field_name, const std::string& field_type);

        void cleanup();

        CollectionDescription                m_description;
        std::unique_ptr<ROOT::RNTupleReader> m_reader;
        std::unique_ptr<ROOT::RNTupleWriter> m_rntupleWriter;

        std::string                          m_name;
        std::string                          m_fileName;
        ICollection::OpenMode                m_mode;
        TFile*                               m_file;
        ISession*                            m_session;
        bool                                 m_open;
        bool                                 m_readOnly;
        
        SmartIF<IFileMgr>                    m_fileMgr;
      };
   }
}
#endif

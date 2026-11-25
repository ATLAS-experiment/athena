/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef STORAGESVC_ISTORAGETOOL_H
#define STORAGESVC_ISTORAGETOOL_H

// Framework include files
#include "StorageSvc/pool.h"
#include "DataModelRoot/RootType.h"
#include "AthenaBaseComps/AthAlgTool.h"

// STL include files
#include <string>

// Forward declarations
class Guid;
class Token;
class StatusCode;
class FileDescriptor;
class DbOption;
class DbTypeInfo;

/** @class IStorageTool IStorageTool.h StorageSvc/IStorageTool.h
    *
    * The IStorageTool interface is able to handle user request for
    *     - transient objects to become persistent and
    *     - persistent objects to be made trasient.
    *     What the storage manager however needs to perform this task 
    *     are object mappings, which translate between the two representations.
    *
    * The activity of the storage manager includes the Transaction handling and
    * hence the management of
    *     - Database connections: 
    *       For file based technologies, such as ROOT, this is involves the opening of the file.
    *     - Database Transactions: Start and end a Transaction.
    *
    * @author  Markus Frank
    */

class IStorageTool : virtual public AthAlgTool {
  public:
    /// Get container name for object
    /**
      * @param   refDB     [IN] Reference to Database descriptor 
      * @param   pToken    [IN] Token to the persistent object.
      * @return                 std::string container name.
      */
    virtual std::string getContName(FileDescriptor& refDB,
                                    Token&          pToken) const = 0;

    /// Register object for write
    /**
      * @param   refDB     [IN] Reference to Database descriptor 
      * @param   refCont   [IN] Reference to container name 
      * @param   technology[IN] Specialised sub-technology
      * @param   object    [IN] Pointer to persistent data object.
      * @param   type      [IN] Persistent type information
      * @param   refpTok  [OUT] Reference to location for storing the
      *                         pointer of the persistent object token.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode write(  FileDescriptor&       refDB,
                               const std::string&    refCont,
                               int                   technology,
                               const void*           object,
                               const RootType        type,
                               Token*&               refpTok) = 0;


    /// Read a persistent object from the medium.
    /** Reading an object does not create the object.
      *
      * @param   refDB     [IN] Reference to Database descriptor 
      * @param   pToken    [IN] Reference to persistent token information.
      * @param   object   [OUT] Pointer to persistent data pointer.
      * 
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode read( FileDescriptor& refDB,
                             const Token&    pToken,
                             void**          object) = 0;

    /** Retrieve persistent tinfo from Storage manager.
      * The persistent tinfo is saved at write time to a Database.
      * It should match the transient type tinfo when reading
      *
      * @param   refDB     [IN] Reference to Database descriptor 
      * @param   objType   [IN] Shape identifier.
      * @param   tinfo    [OUT] Persistent shpae of a given object type.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode getShape(  FileDescriptor&       refDB,
                                  const Guid&           objType,
                                  const DbTypeInfo*&    tinfo) = 0;

    /// Create a Shape representation based on a ShapeID
    /**
      * @param   typeID   [IN] Shape identifier.
      *
      * @return            Handle to persistent mapping of a given object type.
      */
    virtual const DbTypeInfo* createShape( const Guid& typeID ) = 0;

    /// Start a new Database Session.
    /** The Database session handles Databases of one given type. This
      * involves specific handling of a given domain represented 
      * by a technology type. All subsequent actions involving Database
      * actions will re-use this technology identifier.
      * The session is a purely logical concept, which typically cannot fail
      * unless the underlying technology requires global initialization calls.
      *
      * @param    mode     [IN] Flag to indicate the accessmode of the session.
      *                         READ, NEW/CREATE/WRITE, UPDATE, RECREATE
      * @param    tech     [IN] Flag indicating the technology type of the
      *                         Database  the user  wants to connect to.
      * @param    ageLimit [IN] Overwriting default age limit of the session.
      *
      * @return                 StatusCode code indicating success or failure.
      */
      virtual StatusCode startSession(Io::IoFlag mode, int tech, int ageLimit = -1) = 0;

    /// End the Database session.
    /** The  request to end a Database session requires, that all pending 
      * Transactions and connections are closed. Otherwise internally the
      * close will be forced and  potentially data on pending Transactions
      * will be lost. The token will be invalidated may not be used at any 
      * longer once the session ended.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode endSession() = 0;

    /// Connect to a logical Database unit.
    /** A connection is equivalent to the triple (OCISession, OCIServer, 
      * OCISvcCtx) in ORACLE, a login to a datasource using ODBC, or a 
      * single federation for Objectivity. For file based technologies, 
      * such as root, MS Access, ODBC/Text etc., this is involves the 
      * opening of the requested file.
      *
      * @param    mode     [IN] Flag to indicate the accessmode of the session.
      *                         READ, NEW/CREATE/WRITE, UPDATE, RECREATE.
      * @param    refDB   [I/O] Descriptor of the Database to be opened. 
      *                         On successful return the Database handle is
      *                         valid.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode connect(Io::IoFlag mode, FileDescriptor& refDB) = 0;

    /// Disconnect from a logical Database unit.
    /** The  request for disconnect requires, that all pending Transactions
      * are already commited. Otherwise data are lost. On disconnection the 
      * access to the Database is finalized, closed. The token will be 
      * invalidated may not be used at any longer after disconnection.
      *
      * @param    refDB    [IN] Descriptor of the Database access. 
      *                         This handle was retrieved when connecting 
      *                         to the logical Database.
      *                         On successful return the Database handle will
      *                         be invalidated.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode disconnect(FileDescriptor&     refDB) = 0;

    /// Query the access mode of a Database unit.
    /**
      * @param    refDB    [IN] Descriptor of the Database to be queried.
      * @param    mode    [OUT] Open mod to the database.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode openMode(FileDescriptor& refDB, Io::IoFlag& mode ) = 0;

    /// End/Finish an existing Transaction sequence.
    /** At  this  phase all  objects, which were marked for  write when 
      * the Transaction was started, are either going to be made persistent
      * or scratched. After the Transaction ended, the Transaction context
      * is invalidated and may no longer be used independent wether the 
      * Transaction was successful or not.
      *
      * @param    conn  [IN]    Database connection
      * @param    typ   [IN]    Enum indicating an action to be performed.
      *                         Valid arguments are COMMIT
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode endTransaction(FileDescriptor& refDB, Transaction::Action typ) = 0;

    /// Access options for a given database domain.
    /** Domain options are global options, which refer to the
      * database technology and not to a particular connection.
      *
      * Note: The options depend on the underlying implementation
      * and are not normalized.
      *
      *  @param   opt       [IN] Reference to option object.
      *
      *  @return StatusCode code indicating success or failure.
      */
    virtual StatusCode getDomainOption(DbOption& opt) = 0;

    /// Set options for a given database domain.
    /** Domain options are global options, which refer to the
      * database technology and not to a particular connection.
      *
      * Note: The options depend on the underlying implementation
      * and are not normalized.
      *
      *  @param   opt       [IN] Reference to option object.
      *
      *  @return StatusCode code indicating success or failure.
      */
    virtual StatusCode setDomainOption(const DbOption& opt) = 0;
};
  
#endif

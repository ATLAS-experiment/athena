/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOL_ISTORAGESVC_H
#define POOL_ISTORAGESVC_H

// Framework include files
#include "StorageSvc/Transaction.h"

// STL include files
#include <string>

// Forward declarations
class Guid;
class Token;
class StatusCode;

/*
 *     pool namespace declaration
 */
namespace pool  {

  // Forward declarations
  class FileDescriptor;
  class DbOption;

  typedef class Session            *SessionH;
  typedef class DatabaseConnection *ConnectionH;
  typedef const class Shape        *ShapeH;

  /** @class IStorageSvc IStorageSvc.h StorageSvc/IStorageSvc.h
    *
    * The IStorageSvc interface is able to handle user request for
    *     - transient objects to become persistent and
    *     - persistent objects to be made trasient.
    *     What the storage manager however needs to perform this task 
    *     are object mappings, which translate between the two representations.
    *
    * The activity of the storage manager includes the Transaction handling and
    * hence the management of
    *     - Database sessions:
    *       The Database session handles Databases of one given type. This
    *       involves specific handling of a given domain represented
    *       by a technology type. 
    *     - Database connections: A connection is equivalent to the triple
    *       (OCISession, OCIServer, OCISvcCtx) in ORACLE, a login to a 
    *       datasource using ODBC, or a single federation for Objectivity.
    *       For file based technologies, such as root, MS Access, 
    *       ODBC/Text etc., this is involves the opening of the file.
    *     - Database Transactions: Start and end a Transaction.
    *
    * @author  Markus Frank
    * @version 1.0
    */
  class IStorageSvc   {
  protected:
    /// Destructor (called only by sub-classes)
    virtual ~IStorageSvc()   {     }

  public:
    /// Retrieve interface ID
    static const Guid& interfaceID();

    /// Retrieve category name
    static const char* category()             { return "pool_IStorageSvc"; }

    /// IInterface implementation: Query interfaces of Interface
    virtual StatusCode queryInterface(const Guid& riid, void** ppvUnkn) = 0;

    /// IInterface implementation: Reference Interface instance               
    virtual unsigned int addRef() = 0;

    /// IInterface implementation: Release Interface instance                 
    virtual unsigned int release() = 0;

    /// Get container name for object
    /**
      * @param   refDB     [IN] Reference to Database descriptor 
      * @param   pToken    [IN] Token to the persistent object.
      * @return                 std::string container name.
      */
    virtual std::string getContName(FileDescriptor& refDB,
                                    Token&          pToken) = 0;

    /// Register object for write
    /**
      * @param   refDB     [IN] Reference to Database descriptor 
      * @param   refCont   [IN] Reference to container name 
      * @param   technology[IN] Specialised sub-technology
      * @param   object    [IN] Pointer to persistent data object.
      * @param   shapeH    [IN] Handle to persistent type information
      * @param   refpTok  [OUT] Reference to location for storing the
      *                         pointer of the persistent object token.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode allocate(  FileDescriptor&       refDB,
                                  const std::string&    refCont,
                                  int                   technology,
                                  const void*           object,
                                  ShapeH                shapeH,
                                  Token*&               refpTok) = 0;


    /// Read a persistent object from the medium.
    /** Reading an object does not create the object.
      *
      * @param   refDB     [IN] Reference to Database descriptor 
      * @param   pToken    [IN] Reference to persistent token information.
      * @param   object   [OUT] Pointer to persistent data pointer.
      * @param   shapeH    [IN] Desired object shape to be read
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode read(      const FileDescriptor& refDB,
                                  const Token&          pToken,
                                  ShapeH                shapeH,
                                  void**                object) = 0;

    /// Retrieve persistent shape from Storage manager.
    /** The persistent shape is saved at write time to a Database.
      * To match the transient shape of an object to the persistent shape of 
      * the data at the time the data were written should allow for schema
      * evolution and object transformation(s).
      *
      * @param   refDB     [IN] Reference to Database descriptor 
      * @param   objType   [IN] Shape identifier.
      * @param   shapeH   [OUT] Handle to persistent mapping of a given
      *                         object type.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode getShape(  FileDescriptor&       refDB,
                                  const Guid&           objType,
                                  ShapeH&               shapeH) = 0;

    /// Create a Shape representation based on a ShapeID
    /**
      * @param   shapeID   [IN] Shape identifier.
      *
      * @return            Handle to persistent mapping of a given object type.
      */
    virtual ShapeH     createShape( const Guid& shapeID ) = 0;

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
      * @param    session [OUT] Token or handle to the Database session.
      *                         This handle may later be used to open a
      *                         new Database connection.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode startSession(int                 mode,
                                    int                 tech,
                                    SessionH&           session) = 0;

    /// End the Database session.
    /** The  request to end a Database session requires, that all pending 
      * Transactions and connections are closed. Otherwise internally the
      * close will be forced and  potentially data on pending Transactions
      * will be lost. The token will be invalidated may not be used at any 
      * longer once the session ended.
      *
      * @param    session  [IN] Handle to the Database 
      *                         session. This handle was retrieved when 
      *                         starting the session. 
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode endSession(  const SessionH           session) = 0;

    /// Check the existence of a logical Database unit.
    /** 
      *
      * @param    sessionH [IN] Session context to be used to open the Database.
      * @param    mode     [IN] Flag to indicate the accessmode of the session.
      *                         READ, NEW/CREATE/WRITE, UPDATE, RECREATE.
      * @param    refDB   [I/O] Descriptor of the Database to be opened. 
      *                         On successful return the Database handle is
      *                         valid.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode existsConnection(const SessionH        sessionH,
                                        int                   mode,
                                        const FileDescriptor& refDB) = 0;

    /// Connect to a logical Database unit.
    /** A connection is equivalent to the triple (OCISession, OCIServer, 
      * OCISvcCtx) in ORACLE, a login to a datasource using ODBC, or a 
      * single federation for Objectivity. For file based technologies, 
      * such as root, MS Access, ODBC/Text etc., this is involves the 
      * opening of the requested file.
      *
      * @param    sessionH [IN] Session context to be used to open the Database.
      * @param    mode     [IN] Flag to indicate the accessmode of the session.
      *                         READ, NEW/CREATE/WRITE, UPDATE, RECREATE.
      * @param    refDB   [I/O] Descriptor of the Database to be opened. 
      *                         On successful return the Database handle is
      *                         valid.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode connect(   const SessionH      sessionH,
                                  int                 mode,
                                  FileDescriptor&     refDB) = 0;

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
    virtual StatusCode disconnect(  FileDescriptor&     refDB) = 0;

    /// Query the access mode of a Database unit.
    /**
      * @param    refDB    [IN] Descriptor of the Database to be queried.
      * @param    mode    [OUT] Open mod to the database.
      *
      * @return                 StatusCode code indicating success or failure.
      */
    virtual StatusCode openMode(  FileDescriptor&     refDB,
                                  int&                mode ) = 0;

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
    virtual StatusCode endTransaction( ConnectionH conn, Transaction::Action typ) = 0;

    /// Access options for a given database domain.
    /** Domain options are global options, which refer to the
      * database technology and not to a particular connection.
      *
      * Note: The options depend on the underlying implementation
      * and are not normalized.
      *
      *  @param   sessionH  [IN] Session context to be used to open the Database.
      *  @param   opt       [IN] Reference to option object.
      *
      *  @return StatusCode code indicating success or failure.
      */
    virtual StatusCode getDomainOption(const SessionH  sessionH, DbOption& opt) = 0;

    /// Set options for a given database domain.
    /** Domain options are global options, which refer to the
      * database technology and not to a particular connection.
      *
      * Note: The options depend on the underlying implementation
      * and are not normalized.
      *
      *  @param   sessionH  [IN] Session context to be used to open the Database.
      *  @param   opt       [IN] Reference to option object.
      *
      *  @return StatusCode code indicating success or failure.
      */
    virtual StatusCode setDomainOption(const SessionH  sessionH, const DbOption& opt) = 0;

  };

  // Factory function
  IStorageSvc* createStorageSvc(const std::string& componentName);
  
}       // End namespace pool
#endif  // POOL_ISTORAGESVC_H

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  DbDatabase handle implementation
//--------------------------------------------------------------------
//
//  Package    : StorageSvc (The POOL project)
//
//  Description: Generic data persistency
//
//  @author      M.Frank
//====================================================================

#include "StorageSvc/DbDomain.h"
#include "StorageSvc/DbDatabase.h"
#include "DbContainerObj.h"
#include "DbDatabaseObj.h"

using namespace std;
using namespace pool;

static const string& s_empty = "";

/// Add reference count to object if present
int DbDatabase::refCount() const  
{  return isValid() ? ptr()->refCount() : int(INVALID);                 }

/// Get hosting domain
const DbDomain& DbDatabase::containedIn() const   {
  static const DbDomain null_dom(m_type);
  return isValid() ? ptr()->domain() : null_dom;
}
DbDomain& DbDatabase::containedIn()  {
  if (!isValid()) std::abort();
  return ptr()->domain();
}

/// Release handle/switch to new pointer
void DbDatabase::switchPtr(DbDatabaseObj* obj) {
  if (   obj ) obj->addRef();
  if ( isValid() ) {
    if (ptr()->release() == 0) {
      setPtr(0);
      setType(DbType(0));
    }
  }
  if (   obj )  {
    setPtr(obj);
    setType(obj->type());
  } else {
    setPtr(0);
    setType(DbType(0));
  }
}

/// Check if Database exisst within scope of domain
bool DbDatabase::exist(DbDomain& dH, const string& n) const 
{  return dH.isValid() ? dH.existsDbase(n) : false;                     }

/// Open Database
StatusCode DbDatabase::open(DbDomain&       domH,
                            const string&   pfn, 
                            const string&   fid, 
                            DbAccessMode    mode)
{
  if ( domH.isValid() )   {
    DbDatabaseObj* db = domH.find(fid);
    if ( !db )   {
      db = new DbDatabaseObj(domH, pfn, fid, mode);
      if ( !db->open().isSuccess() ) {
        switchPtr(0);
        domH.remove(db).ignore();
        return StatusCode::FAILURE;
      }
    }
    switchPtr(db);
    m_type = domH.type();
    return StatusCode::SUCCESS;
  }
  return StatusCode::FAILURE;
}

/// Close Database
StatusCode DbDatabase::close() {
  StatusCode ret = StatusCode::SUCCESS;
  if ( isValid() ) ret = ptr()->close();
  switchPtr(0);  
  return ret;
}

/// Access the size of the database: May be undefined for some technologies
long long int DbDatabase::size()
{  return isValid() ? ptr()->size() : -1;                               }

/// Re-open database with changing access permissions
StatusCode DbDatabase::reopen(DbAccessMode mod)
{  return isValid() ? ptr()->reopen(mod) : StatusCode::FAILURE;                       }

/// End database access, but still leave database accessible
StatusCode DbDatabase::retire()
{  return isValid() ? ptr()->retire() : StatusCode::FAILURE;                          }

/// Add domain to session
StatusCode DbDatabase::add(const string& nam, DbContainerObj* cnt)
{  return (isValid() && cnt) ? ptr()->add(nam, cnt) : StatusCode::FAILURE;            }

/// Remove domain from session
StatusCode DbDatabase::remove(const DbContainerObj* cnt)
{  return (isValid() && cnt) ? ptr()->remove(cnt) : StatusCode::FAILURE;              }

/// Access to access mode member
DbAccessMode DbDatabase::openMode() const 
{  return isValid() ? ptr()->mode() : DbAccessMode(pool::NOT_OPEN);     }

/// Access to db name (FID)
const string& DbDatabase::name() const 
{  return isValid() ? ptr()->name() : s_empty;                          }

/// Set db name (FID) after opening in case opening happened by PFN
void DbDatabase::setName(const string& nam)
{  if ( isValid() ) ptr()->setName(nam);                                }

/// Access to db name
const string& DbDatabase::logon() const 
{  return isValid() ? ptr()->logon() : s_empty;                         }

/// Select container object in Database
const DbContainerObj* DbDatabase::find(const string& nam) const 
{  return isValid() ? ptr()->find(nam) : 0;                             }

/// Select container object in Database
DbContainerObj* DbDatabase::find(const string& nam)
{  return isValid() ? ptr()->find(nam) : 0;                             }

/// Check if the database was opened
bool DbDatabase::isOpen() const  
{  return isValid();                                                    }

/// Add association link to link container
StatusCode DbDatabase::makeLink(Token* pToken, Token::OID_t& refLink)
{  return isValid() ? ptr()->makeLink(pToken, refLink) : StatusCode::FAILURE;         }

/// read an object referenced by the token
StatusCode DbDatabase::read(const Token& token, ShapeH shape, void** object)
{  return isValid() ? ptr()->read(token, shape, object) : StatusCode::FAILURE;  }

/// Expand OID into a full Token, based on the Links table.
StatusCode DbDatabase::getLink(const Token::OID_t& oid, Token* pTok)
{  return isValid() ? ptr()->getLink(oid, pTok) : StatusCode::FAILURE;  }

/// Access local container token (if container exists)
std::string DbDatabase::cntName(Token& token)
{  return isValid() ? ptr()->cntName(token) : "";                       }

/// Execute Database Transaction action
StatusCode DbDatabase::transAct(Transaction::Action action) {
  return isValid()? ptr()->transAct(action) : StatusCode::FAILURE;
}

/// Pass options to the implementation
StatusCode DbDatabase::setOption(const DbOption& refOpt) 
{  return isValid() ? ptr()->setOption(refOpt) : StatusCode::FAILURE;                 }

/// Pass options from the implementation to the user
StatusCode DbDatabase::getOption(DbOption& refOpt)
{  return isValid() ? ptr()->getOption(refOpt) : StatusCode::FAILURE;                 }

/// Add persistent type to the Database
StatusCode DbDatabase::addShape(const DbTypeInfo* refType)
{  return isValid() ? ptr()->addShape(refType) : StatusCode::FAILURE;                 }

/// Retrieve persistent type information
const DbTypeInfo* DbDatabase::objectShape(const Guid& guid)
{  return isValid() ? ptr()->objectShape(guid) : 0;                     }

/// Retrieve persistent type information by class handle
const DbTypeInfo* DbDatabase::objectShape(const RootType& typeH)
{  return isValid() ? ptr()->objectShape(typeH) : 0;                    }

/// Retrieve persistent type information
const DbTypeInfo* DbDatabase::contShape(const string& nam)
{  return isValid() ? ptr()->contShape(nam) : 0;                        }

/// Access local container token (if container exists)
const Token* DbDatabase::cntToken(const string& cntName)
{  return isValid() ? ptr()->cntToken(cntName) : 0;                     }

/// Allow access to all known containers
StatusCode DbDatabase::containers(vector<const Token*>& conts,bool with_internal)
{  return isValid() ? ptr()->containers(conts,with_internal) : StatusCode::FAILURE;   }

/// Allow access to all known containers
StatusCode DbDatabase::containers(vector<IDbContainer*>& conts,bool with_internal)
{  return isValid() ? ptr()->containers(conts,with_internal) : StatusCode::FAILURE;   }

/// Allow access to all known associations between containers
StatusCode DbDatabase::associations(vector<const Token*>& assocs)
{  return isValid() ? ptr()->associations(assocs) : StatusCode::FAILURE;              }

/// Allow access to all known shapes used by the database
StatusCode DbDatabase::shapes(vector<const DbTypeInfo*>& shaps)
{  return isValid() ? ptr()->shapes(shaps) : StatusCode::FAILURE;                     }

/// Retrieve the number of user parameters
int DbDatabase::nParam()
{  return isValid() ? ptr()->nParam() : -1;                             }

/// Add a persistent parameter to the file
StatusCode DbDatabase::addParam(const string& nam, const string& val)
{  return isValid() ? ptr()->addParam(nam, val) : StatusCode::FAILURE;                }

/// Retrieve existing parameter by name
StatusCode DbDatabase::param(const string& nam, string& val)
{  return isValid() ? ptr()->param(nam, val) : StatusCode::FAILURE;                   }

/// Retrieve all parameters
StatusCode DbDatabase::params(Parameters& vals)
{  return isValid() ? ptr()->params(vals) : StatusCode::FAILURE;                      }

/// Access the token of the database object
const Token* DbDatabase::token() const
{  return isValid() ? ptr()->token() : 0;                               }

/// Access implementation internals
IDbDatabase* DbDatabase::info()
{  return isValid() ? ptr()->info() : 0;                                }

const IDbDatabase* DbDatabase::info()   const
{  return isValid() ? ptr()->info() : 0;                                }

/// Allow access to the Database implementation
IOODatabase* DbDatabase::db()
{  return isValid() ? ptr()->db() : 0;                                  }

const IOODatabase* DbDatabase::db()    const
{  return isValid() ? ptr()->db() : 0;                                  }

/// Update database age
void DbDatabase::setAge(int value)
{  if ( isValid() ) ptr()->setAge(value);                               }

/// Access age value
int DbDatabase::age()  const
{  return isValid() ? ptr()->age() : 0;                                 }


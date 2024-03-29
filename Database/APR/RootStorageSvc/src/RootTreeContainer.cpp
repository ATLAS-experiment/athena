/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//    Root Database container implementation
//--------------------------------------------------------------------
//
//    Package    : Athena APR RootStorageSvc (Former POOL project)
//
//    Author     : M.Frank, M.Nowak
//====================================================================

// Framework include files
#include "StorageSvc/DbObject.h"
#include "StorageSvc/DbOption.h"
#include "StorageSvc/DbSelect.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbColumn.h"
#include "StorageSvc/DbTypeInfo.h"
#include "StorageSvc/DbArray.h"
#include "POOLCore/DbPrint.h"
#include "StorageSvc/Transaction.h"
#include "StorageSvc/DbReflex.h"
#include "CxxUtils/checker_macros.h"

// Local implementation files
#include "RootTreeContainer.h"
#include "RootDataPtr.h"
#include "RootDatabase.h"

// Root include files
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TLeaf.h"
#include "TBranch.h"
#include "RootUtils/TBranchElementClang.h"
#include "TTreeFormula.h"

#include "RootAuxDynIO/RootAuxDynIO.h"

using namespace pool;
using namespace std;

namespace {

class TBranchAccess : public TBranch
{
public:
  static int doNotUseBufferMap() { return kDoNotUseBufferMap; }
};


/**
 * @brief Fix up branch status bits after possible schema evolution
 *        from std::vector to SG::PackedContainer.
 * @param br The branch to process.
 *
 * Some background is needed here.
 *
 * ROOT can keep a map associated with a buffer giving the location of objects
 * in the root file.  This is used to handle C pointer resolution on I/O.
 * The use of this is controlled by two flags in fBits.
 * If kDoNotUseBufferMap is _clear_, then the map get cleared on each
 * event (when reading); otherwise, it is not.  This bit is set by default
 * when a branch is created.  When Fill() is called, then it is cleared
 * if the map is empty.  The bit is then saved and restored along
 * with the TBranch.
 *
 * But a branch also has the kBranchAny bit.  If this is set, then ROOT
 * will actually try to make entries in the map.  This is set when
 * a branch is initialized and also when SetAddress is called.  The bit
 * is set if the class _might_ contain pointers.  Specifically, the bit
 * is clear for branches with fundamental types, set::string, and std::vector
 * of any of these.  It is turned on for anything else, in particular
 * for any user types.
 *
 * This logic causes a problem when we schema evolve from std::vector
 * to SG::PackedContainer.  When the branch is written, the map is not
 * used.  So kDoNotUseBufferMap is set, and kBranchAny is clear.
 * The branch is saved, and read back in with the same flag settings.
 * But then when we go to do the SetAddress, ROOT sees that the type
 * we have is SG::PackedContainer --- a user type.  So kBranchAny
 * is turned on.  This means that the map gets filled when we read.
 * But because kDoNotUseBufferMap is set, the map is not cleared
 * when we go to a new event.  This results in errors from TExMap
 * about duplicate keys.
 *
 * The easiest thing to do to resolve this seems to be to turn off
 * the kDoNotUseBufferMap so that the map will in fact get cleared.
 * (The arguably more consistent scheme of turning off kBranchAny
 * is not preferred since we'd have to do that every time after
 * SetAddress is called.)  Ideally, we'd like to be able to tell
 * ROOT that a given class does not use pointers, but there's no way
 * to do that at the moment.
 */
void fixupPackedConversion (TBranch* br)
{
  TIter next( br->GetListOfBranches() );
  while ( TBranch* b = (TBranch*)next() ) {
    fixupPackedConversion (b);
  }

  TBranchElement* belt = dynamic_cast<TBranchElement*> (br);
  if (belt) {
    TClass* cl = belt->GetCurrentClass();
    if (cl && strncmp (cl->GetName(), "SG::PackedContainer<", 20) == 0) {
      br->ResetBit (TBranchAccess::doNotUseBufferMap());
    }
  }
}


} // anonymous namespace

// I/O buffers (protected by mutex where used)
static UCharDbArrayAthena  s_char_Blob ATLAS_THREAD_SAFE;
static IntDbArray   s_int_Blob ATLAS_THREAD_SAFE;


// required out-of-line for unique_ptr compilation
RootTreeContainer::BranchDesc::BranchDesc( TClass* cl, TBranch* b, TLeaf* l, void* o, const DbColumn* c)
   : clazz(cl), branch(b), leaf(l), object(nullptr), buffer(o), column(c)
{}

SG::IAuxStoreIO*
RootTreeContainer::BranchDesc::getIOStorePtr() {
   return ( aux_iostore_IFoffset >= 0 ?
            reinterpret_cast<SG::IAuxStoreIO*>( (char*)object + aux_iostore_IFoffset) : nullptr );
}


RootTreeContainer::RootTreeContainer()
: m_tree(nullptr), m_type(0), m_dbH(POOL_StorageType),
  m_rootDb(nullptr), m_branchName(), m_ioBytes(0), m_treeFillMode(false),
  m_isDirty(false)
{
}

/// Standard destructor
RootTreeContainer::~RootTreeContainer()   {
   RootTreeContainer::close();
}

/// Ask if a given shape is supported
DbStatus RootTreeContainer::isShapeSupported(const DbTypeInfo* typ) const  {
  return typ == m_type;
}

uint64_t RootTreeContainer::size()    {
  if (m_tree == nullptr) return 0;
  auto s = DbContainerImp::size();
  if( isBranchContainer() ) {
     TBranch * pBranch = m_tree->GetBranch(m_branchName.c_str());
     if (pBranch == nullptr) return 0;
     s += pBranch->GetEntries();
  } else {
     s += m_tree->GetEntries();
  }
  return s;
}

/// Access branch by name
TBranch* RootTreeContainer::branch(const std::string& nam)  const  {
  Branches::const_iterator k;
  for(k=m_branches.begin(); k !=m_branches.end(); ++k) {
    if ( (*k).branch->GetName() == nam )  {
      return (*k).branch;
    }
  }
  return nullptr;
}

DbStatus RootTreeContainer::writeObject( ActionList::value_type& action )
{
   int icol;
   int num_bytes = 0;
   bool aux_needs_fill = false;
   Branches::iterator k;
   for(k=m_branches.begin(), icol=0; k !=m_branches.end(); ++k, ++icol) {
      BranchDesc& dsc( *k );
      RootDataPtr p( nullptr );
      p.cptr = action.dataAtOffset( dsc.column->offset() );
      switch( dsc.column->typeID() ) {
       case DbColumn::ANY:
       case DbColumn::POINTER:
          dsc.object = p.ptr;
          p.ptr      = &dsc.object;
          try {
             if( dsc.auxdyn_writer ) {
                num_bytes += dsc.auxdyn_writer->writeAuxAttributes
                   ( dsc.branch->GetName(), dsc.getIOStorePtr(), dsc.rows_written );
                aux_needs_fill = aux_needs_fill || dsc.auxdyn_writer->needsCommit();
             }
          } catch(const std::exception& exc) {
             DbPrint err(m_name);
             err << DbPrintLvl::Error << "Dynamic attributes writing error: " << exc.what()
                 << DbPrint::endmsg;
             p.ptr = nullptr;  // signal error
             break;
          }
          dsc.rows_written++;
          break;
       case DbColumn::BLOB:
          // MN: not sure if we ever use this case
          s_char_Blob.m_size    = p.blobSize();
          s_char_Blob.m_buffer  = (unsigned char*)p.blobData();
          dsc.object            = &s_char_Blob;
          p.ptr                 = &dsc.object;
          break;
       case DbColumn::STRING:
       case DbColumn::LONG_STRING:
          // handling "std::string*" - e.g. ##Links and ##Params
          // pass the char* from c_str() to ROOT
          p.cptr =  p.string()->c_str();
          break;
       case DbColumn::NTCHAR:
       case DbColumn::LONG_NTCHAR:
          // MN: not sure if we ever use this case
          p.ptr   = p.deref();
          break;
       case DbColumn::TOKEN:
          // p.ptr is "char*" already so just pass it on
          break;
       default:
          break;
      }
      if ( nullptr == p.ptr )   {
         DbPrint err( m_name);
         err << DbPrintLvl::Error
             << "[RootTreeContainer] Could not write an object"
             << DbPrint::endmsg;
         return Error;
      }
      //if (p.ptr != dsc.branch->GetAddress()) {
      dsc.branch->SetAddress(p.ptr);
      //}
      if( isBranchContainer() && !m_treeFillMode ) {
         num_bytes += dsc.branch->Fill();
      }
   }

   if( !isBranchContainer() ) {
      // Single Container per TTree - just Fill it now
      num_bytes = m_tree->Fill();
   } else
      if( m_treeFillMode ) {
      // Multiple containers per TTree - mark TTree for later Fill at commit
      if( m_isDirty ) {
         DbPrint log(m_name);
         log << DbPrintLvl::Error << "Attempt to write to a Branch Container twice in the same transaction! "
             << "This conflicts with TTree AUTO_FLUSH option. "
             << DbPrint::endmsg;
         m_ioBytes = -1;
         return Error;
      }
      m_isDirty = true;
      m_ioBytes = 0;  // no information per container available!
      return Success;
   }
   // else (branch container NOT in tree fill mode)
   // do nothing, the branch was filled in the previous block already

   for(k=m_branches.begin(); k !=m_branches.end(); ++k) {
      switch ( (*k).column->typeID() )    {
       case DbColumn::BLOB:
          s_char_Blob.release(false);
          break;
       default:
          break;
      }
   }
   if ( num_bytes > 0 )  {
      m_ioBytes = num_bytes;
      m_rootDb->addByteCount(RootDatabase::WRITE_COUNTER, num_bytes);
      return Success;
   }
   DbPrint err( m_name);
   err << DbPrintLvl::Error
       << "[RootTreeContainer] Could not write an object"
       << DbPrint::endmsg;
   m_ioBytes = -1;
   return Error;
}


/// Fetch refined object address. Default implementation returns identity
DbStatus RootTreeContainer::fetch(const Token::OID_t& linkH, Token::OID_t& stmt)  {
  return DbContainerImp::fetch(linkH, stmt);
}

// Fetch next object address of the selection to set token
DbStatus RootTreeContainer::fetch(DbSelect& sel)  {
  if ( sel.criteria().length() == 0 || sel.criteria() == "*" )  {
    sel.link().second++;
    return DbContainerImp::fetch(sel.link(), sel.link());
  }
  DbSelect::Ptr<TTreeFormula>* stmt =
    dynamic_cast<DbSelect::Ptr<TTreeFormula>* >(sel.statement());
  if ( stmt ) {
    TTreeFormula* selStmt = stmt->m_ptr;
    if ( selStmt )  {
      std::lock_guard<std::recursive_mutex>   lock( m_rootDb->ioMutex() );
      Branches::iterator k;
      long long cur  = sel.link().second;
      Long64_t last = m_tree->GetEntries();
      for(k=m_branches.begin(); k != m_branches.end(); ++k)  {
        BranchDesc& dsc = (*k);
        int typ = dsc.column->typeID();
        switch ( typ )    {
        case DbColumn::POINTER:
        case DbColumn::BLOB:
          dsc.branch->SetAddress(&dsc.buffer);
        default:
          break;
        }
      }
      selStmt->SetTree(m_tree);
      // loop on all selected entries
      for( ++cur; cur < last; ++cur) {
        m_tree->LoadTree(cur);
        selStmt->GetNdata();
        if ( selStmt->EvalInstance(0) != 0 ) {
          sel.link().second = cur;
          return Success;
        }
      }
    }
  }
  return Error;
}


DbStatus
RootTreeContainer::loadObject(void** obj_p, ShapeH /*shape*/, Token::OID_t& oid)
{
  auto evt_id = oid.second;
  if( evt_id < 0 || (uint64_t)evt_id >= size() ) {
     // -1 may be from a failed index lookup
     *obj_p = nullptr;
     // do not return Error to avoid error printouts in case someone just tries to iterate over all OIDs
     return Success;
  }
  // lock access to this DB for MT safety
  std::lock_guard<std::recursive_mutex>     lock( m_rootDb->ioMutex() );
  try {
     int numBytesBranch, numBytes = 0;
     bool hasRead(false);
     for( auto& dsc : m_branches ) {
        RootDataPtr p(nullptr), q(nullptr);
        int typ = dsc.column->typeID();
        // cout << "LOAD object, typ=" << typ << ",  col offset=" << dsc.column->offset() << endl;
        // associate branch with an object
        switch ( typ )    {
         case DbColumn::STRING:
         case DbColumn::LONG_STRING:
         case DbColumn::NTCHAR:
         case DbColumn::LONG_NTCHAR:
         case DbColumn::TOKEN:
            // set data pointer to the object data member for this branch
            p = *obj_p;
            p.c_str += dsc.column->offset();
            break;
         case DbColumn::BLOB:
            dsc.object = &s_char_Blob;
            p.ptr      = &dsc.object;
            dsc.branch->SetAddress(p.ptr);
            break;
         case DbColumn::ANY:
         case DbColumn::POINTER:
         default:
            //dsc.branch->SetAddress( &p.ptr );
            dsc.branch->SetAddress( obj_p );
            break;
        }
        // read the object
        numBytesBranch = dsc.branch->GetEntry(evt_id);
        TTree::TClusterIterator clusterIterator = dsc.branch->GetTree()->GetClusterIterator(evt_id);
        clusterIterator.Next();
        if (evt_id == clusterIterator.GetStartEntry() && dsc.branch->GetTree()->GetMaxVirtualSize() != 0) {
           for (int i = dsc.branch->GetReadBasket(); i < dsc.branch->GetMaxBaskets()
                   && dsc.branch->GetBasketEntry()[i] < clusterIterator.GetNextEntry(); i++) {
              dsc.branch->GetBasket(i);
           }
        }
        numBytes += numBytesBranch;
        if ( numBytesBranch >= 0 )     {
           hasRead=true;
           switch ( typ )    {
            case DbColumn::STRING:
            case DbColumn::LONG_STRING:
               // assign to std::string
               *p.str = (char*) dsc.leaf->GetValuePointer();
               break;
            case DbColumn::NTCHAR:
            case DbColumn::LONG_NTCHAR:
            case DbColumn::TOKEN:
               p = *obj_p;
               q = dsc.leaf->GetValuePointer();
               ::strcpy(q.c_str, p.c_str);
               break;
            case DbColumn::BLOB:
               p.blob->adopt((char*)s_char_Blob.m_buffer,s_char_Blob.m_size);
               s_char_Blob.release(false);
               break;
            case DbColumn::POINTER:
               // for AUX store objects with the right interface supply a special store object
               // that will read branches on demand
               if( dsc.auxdyn_reader ) {
                  dsc.auxdyn_reader->addReaderToObject( *obj_p, evt_id, &m_rootDb->ioMutex() );
               }
               break;
           }
        } else {
           DbPrint log(m_name);
           log << DbPrintLvl::Error << "Cannot load branch " << dsc.branch->GetName()
               << " for entry No." << evt_id << DbPrint::endmsg;
           m_ioBytes = -1;
           return Error;
        }
     }
     if ( hasRead )   {
        /// Update statistics
        m_ioBytes = numBytes;
        m_rootDb->addByteCount(RootDatabase::READ_COUNTER, numBytes);
        return Success;
     }
  }
  catch( const std::exception& e )    {
     DbPrint err(m_name);
     err << DbPrintLvl::Fatal << "[RootTreeContainer] "
         << "STL C++ Exception: " << e.what() << DbPrint::endmsg;
  }
  catch (...)   {
     DbPrint err(m_name);
     err << DbPrintLvl::Fatal << "[RootTreeContainer] "
         << "Unknown exception occurred. Cannot give more details."
         << DbPrint::endmsg;
  }
  DbPrint log(m_name);
  log << DbPrintLvl::Info << "Cannot load entry No." << evt_id << "..."
      << (m_branchName.empty() ? " Tree has " : " Branch has " )
      << size() << " Entries in total." << DbPrint::endmsg;
  m_ioBytes = -1;
  return Error;
}



DbStatus RootTreeContainer::close()   {
  m_dbH = DbDatabase(POOL_StorageType);
  for( BranchDesc& dsc : m_branches ) {
    if ( dsc.buffer && dsc.clazz )  {
      // This somehow fails for templates.
      dsc.clazz->Destructor(dsc.buffer);
    }
  }
  m_branches.clear();
  m_rootDb = nullptr;
  m_tree = nullptr;
  return DbContainerImp::close();
}


DbStatus RootTreeContainer::open( DbDatabase& dbH, 
                                  const std::string& nam, 
                                  const DbTypeInfo* info, 
                                  DbAccessMode mode)  
{
   DbPrint log(nam);
   m_branches.clear();
   m_name = nam;
   log << DbPrintLvl::Debug << "Opening"  << DbPrint::endmsg;
   if ( dbH.isValid() && info )    {
      const DbTypeInfo::Columns& cols = info->columns();
      DbTypeInfo::Columns::const_iterator i;
      std::string treeName(nam);
      for(std::string::iterator j = treeName.begin(); j != treeName.end(); ++j )    {
         if ( *j == '/' ) *j = '_';
      }
      log << DbPrintLvl::Debug << "   attributes# = " << cols.size() << DbPrint::endmsg;
      if (cols.size() == 1) {
         // extract tree and branch name for branch containers, notation: "tree(branch)"
         std::string::size_type inx = nam.find('(');
         if (inx != std::string::npos) {
            std::string::size_type inx2 = nam.find(')');
            if (inx2 == std::string::npos || inx2 != nam.size()-1) {
               log << DbPrintLvl::Error << "Misspecified branch name in " << m_name << "."
                   << DbPrint::endmsg;
               return Error;
            }
            m_branchName = treeName.substr(inx+1, inx2-inx-1);
            treeName.resize(inx);
            log << DbPrintLvl::Debug << "Branch container '" << m_branchName << "'" << DbPrint::endmsg;
         }
      }
      IDbDatabase* idb = dbH.info();
      m_rootDb = dynamic_cast<RootDatabase*>(idb);
      if (m_rootDb)
         m_tree = (TTree*)m_rootDb->file()->Get(treeName.c_str());

      bool hasBeenCreated = (m_branchName.empty()
                             ? m_tree != nullptr
                             : (m_tree && m_tree->GetBranch(m_branchName.c_str()) != nullptr));
      if ( hasBeenCreated && (mode&pool::READ || mode&pool::UPDATE) )   {
         if (treeName.substr(0, 2) == "##") {
            m_tree->SetCacheSize(0);
         }
         int count;
         if ( !m_tree->InheritsFrom(TTree::Class()) )   {
            log << DbPrintLvl::Error << "Cannot open the container " << m_name << " of type "
                << ROOTTREE_StorageType.storageName() << "." << DbPrint::endmsg
                << "The specified container is not a ROOT " << (m_branchName.empty() ? "Tree" : "Branch")
                << ", but rather of class " << m_tree->IsA()->GetName() << "."
                << DbPrint::endmsg;
            return Error;
         }
         m_branches.resize(cols.size());
         for(i = cols.begin(), count = 0; i != cols.end(); ++i, ++count )   {
            std::string colnam  = (m_branchName.empty() ? (*i)->name() : m_branchName);
            TBranch* pBranch = m_tree->GetBranch(colnam.c_str());
            if( !pBranch && m_branchName.empty() ) {
               for( char& c : colnam )
                  if( !::isalnum(c) ) c = '_';
               pBranch = m_tree->GetBranch(colnam.c_str());
            }
            if ( pBranch )    {
               fixupPackedConversion (pBranch);

               const DbColumn* c = *i;
               BranchDesc& dsc = m_branches[count];
               TClass* cl = nullptr;
               TLeaf* leaf = pBranch->GetLeaf( (*i)->name().c_str() );
               // cout << "GetLeaf for "<<  (*i)->name().c_str()  << " = " << leaf << endl;
               switch ( (*i)->typeID() )    {
                case DbColumn::ANY:
                case DbColumn::BLOB:
                case DbColumn::POINTER:
                   cl = TClass::GetClass(pBranch->GetClassName());
                   if ( nullptr == cl )  {
                      log << DbPrintLvl::Debug << "Cannot open the container " << m_name << " of type "
                          << ROOTTREE_StorageType.storageName()
                          << " Class " << pBranch->GetClassName() << " is unknown."
                          << DbPrint::endmsg;
                      return Error;
                   }
                   dsc = BranchDesc(cl, pBranch, leaf, cl->New(), c);
                   if( RootAuxDynIO::isAuxDynBranch(pBranch) ) {
                      dsc.auxdyn_reader = RootAuxDynIO::getBranchAuxDynReader( m_tree, pBranch );
                      if( !dsc.auxdyn_reader ) {
                         log << DbPrintLvl::Error << "Failed to locate dynamic attribute storage for container "
                             << m_name << " of type " << ROOTTREE_StorageType.storageName()
                             << " Class " << pBranch->GetClassName() << " is unknown."
                             << DbPrint::endmsg;
                         return Error;
                      }
                      if (dsc.auxdyn_reader) {
                         // If we set up a reader, then disable aging
                         // for this file.  That will prevent POOL from
                         // deleting the file while we still have
                         // references to its branches.
                         dbH.setAge (-10);
                      }
                   }
                   break;
                case DbColumn::CHAR:
                case DbColumn::UCHAR:
                case DbColumn::BOOL:
                case DbColumn::SHORT:
                case DbColumn::USHORT:
                case DbColumn::INT:
                case DbColumn::LONG:
                case DbColumn::UINT:
                case DbColumn::ULONG:
                case DbColumn::FLOAT:
                case DbColumn::DOUBLE:
                case DbColumn::LONGLONG:
                case DbColumn::ULONGLONG:
                case DbColumn::STRING:
                case DbColumn::LONG_STRING:
                case DbColumn::NTCHAR:
                case DbColumn::LONG_NTCHAR:
                case DbColumn::TOKEN:
                   dsc.leaf = leaf;
                   dsc.branch = pBranch;
                   dsc.column = *i;
                   break;
                default:
                   return Error;
               }
            }
            else  {
               log << DbPrintLvl::Warning << "Branch with name:" << colnam
                   << " not present in container:" << m_name << " of type "
                   << ROOTTREE_StorageType.storageName()
                   << DbPrint::endmsg;
            }
         }
         log << DbPrintLvl::Debug << "Opened container " << m_name << " of type "
             << ROOTTREE_StorageType.storageName()
             << DbPrint::endmsg;
         m_dbH = dbH;
         m_type = info;
         if( mode&pool::UPDATE ) {
            m_rootDb->registerBranchContainer(this);
         }
         return Success;
      }
      else if ( !hasBeenCreated && mode&pool::CREATE )    {
         int count, defSplitLevel=99,
            defAutoSave=16*1024*1024, defBufferSize=16*1024,
            branchOffsetTabLen=0, containerSplitLevel=defSplitLevel, auxSplitLevel=defSplitLevel;
         DbStatus res = Success;
         try   {
            DbOption opt1("DEFAULT_SPLITLEVEL","");
            DbOption opt2("DEFAULT_AUTOSAVE","");
            DbOption opt3("DEFAULT_BUFFERSIZE","");
            DbOption opt4("TREE_BRANCH_OFFSETTAB_LEN","");
            DbOption opt5("CONTAINER_SPLITLEVEL", m_name);
            DbOption opt6("CONTAINER_SPLITLEVEL", RootAuxDynIO::AUX_POSTFIX);
            dbH.getOption(opt1);
            dbH.getOption(opt2);
            dbH.getOption(opt3);
            dbH.getOption(opt4);
            dbH.getOption(opt5);
            dbH.getOption(opt6);
            opt1._getValue(defSplitLevel);
            opt2._getValue(defAutoSave);
            opt3._getValue(defBufferSize);
            opt4._getValue(branchOffsetTabLen);
            opt5._getValue(containerSplitLevel);
            opt6._getValue(auxSplitLevel);
            if (containerSplitLevel == defSplitLevel) {
               if( RootAuxDynIO::hasAuxStore( string_view(m_name).substr(0, m_name.size()-1), info->clazz().Class() ) ) {
                  containerSplitLevel = auxSplitLevel;
               }
            }
            if (!m_tree) {
               m_tree = new TTree(treeName.c_str(), treeName.c_str(), (m_branchName.empty() ? containerSplitLevel : defSplitLevel));
               if (m_rootDb)
                  m_tree->SetDirectory(m_rootDb->file());
               // Autosave every mega byte...
               m_tree->SetAutoSave(defAutoSave);
            }
            //            - C : a character string terminated by the 0 character
            //            - B : an 8 bit signed integer (Char_t)
            //            - b : an 8 bit unsigned integer (UChar_t)
            //            - S : a 16 bit signed integer (Short_t)
            //            - s : a 16 bit unsigned integer (UShort_t)
            //            - I : a 32 bit signed integer (Int_t)
            //            - i : a 32 bit unsigned integer (UInt_t)
            //            - F : a 32 bit floating point (Float_t)
            //            - D : a 64 bit floating point (Double_t)
            m_branches.resize(cols.size());
            for (i = cols.begin(), count = 0; i != cols.end(); ++i, ++count )   {
               DbStatus iret = Success;
               BranchDesc& dsc = m_branches[count];
               switch ( (*i)->typeID() )    {
                case DbColumn::CHAR:       iret=addBranch(*i,dsc,"/B"); break;
                case DbColumn::UCHAR:      iret=addBranch(*i,dsc,"/b"); break;
                case DbColumn::BOOL:       iret=addBranch(*i,dsc,"/O"); break;
                case DbColumn::SHORT:      iret=addBranch(*i,dsc,"/S"); break;
                case DbColumn::USHORT:     iret=addBranch(*i,dsc,"/s"); break;
                case DbColumn::INT:        iret=addBranch(*i,dsc,"/I"); break;
                case DbColumn::LONG:       iret=addBranch(*i,dsc,"/L"); break;
                case DbColumn::UINT:       iret=addBranch(*i,dsc,"/i"); break;
                case DbColumn::ULONG:      iret=addBranch(*i,dsc,"/l"); break;
                case DbColumn::FLOAT:      iret=addBranch(*i,dsc,"/F"); break;
                case DbColumn::DOUBLE:     iret=addBranch(*i,dsc,"/D"); break;
                case DbColumn::LONGLONG:   iret=addBranch(*i,dsc,"/L"); break;
                case DbColumn::ULONGLONG:  iret=addBranch(*i,dsc,"/l"); break;
                case DbColumn::STRING:
                case DbColumn::LONG_STRING:
                case DbColumn::NTCHAR:
                case DbColumn::LONG_NTCHAR:
                case DbColumn::TOKEN:
                   iret=addBranch(*i,dsc,"/C");
                   break;
                case DbColumn::BLOB:
                   iret=addObject(dbH, *i, dsc, "UCharDbArrayAthena", defSplitLevel, defBufferSize, branchOffsetTabLen);
                   break;
                case DbColumn::ANY:
                case DbColumn::POINTER:
                   iret=addObject(dbH, *i, dsc, (*i)->typeName(), containerSplitLevel, defBufferSize, branchOffsetTabLen);
                   break;
                default:
                   return Error;
               }
               if( !iret.isSuccess() )  {
                  res = iret;
               }
            }
            if( res.isSuccess() )    {
               log << DbPrintLvl::Debug << "Opened container " << m_name << " of type "
                   << ROOTTREE_StorageType.storageName()
                   << DbPrint::endmsg;
               m_dbH  = dbH;
               m_type = info;
               m_rootDb->registerBranchContainer(this);
               return Success;
            }
            debugBreak(nam, "Cannot open ROOT container(Tree/Branch)", false);
            return res;
         }
         catch( const std::exception& e )    {
            debugBreak(nam, "Cannot open ROOT container(Tree/Branch)", e, false);
            res = Error;
         }
         catch (...)   {
            DbPrint err( m_name);
            err << DbPrintLvl::Fatal << "Unknown exception occurred. Cannot give more details."
                << DbPrint::endmsg;
            debugBreak(nam, "Cannot open ROOT container(Tree/Branch)");
            res = Error;
         }
      }
   }
   log << DbPrintLvl::Error << "Cannot open container '" << nam << "', invalid Database handle."
       << DbPrint::endmsg;
   return Error;
}


// Define selection criteria
DbStatus  RootTreeContainer::select(DbSelect& sel)    {
  if ( nullptr != m_tree )  {
    if( sel.criteria().length() == 0 || sel.criteria() == "*" )  {
      sel.link().second = -1;
      return Success;
    }
    else  {
      TTreeFormula* stmt = new TTreeFormula("RootSelect", sel.criteria().c_str(), m_tree);
      sel.setStatement(new DbSelect::Ptr<TTreeFormula>(stmt));
      sel.link().second = -1;
      return Success;
    }
  }
  return Error;
}


DbStatus  RootTreeContainer::addObject(DbDatabase& dbH,
                                       const DbColumn* col,
                                       BranchDesc& dsc,
                                       const std::string& typ,
                                       int splitLevel,
                                       int bufferSize,
                                       int branchOffsetTabLen)
{
   try {
      dsc.buffer  = nullptr;
      dsc.object  = nullptr;
      dsc.column  = col;
      dsc.clazz = TClass::GetClass(typ.c_str());
      if ( nullptr != dsc.clazz )  {
         if ( dsc.clazz->GetStreamerInfo() )  {
            std::string nam = m_branchName;
            if( nam.empty() ) {
               nam = col->name();
               for ( std::string::iterator j = nam.begin(); j != nam.end(); ++j )    {
                  if ( !::isalnum(*j) ) *j = '_';
               }
            }
            int split = dsc.clazz->CanSplit() ? splitLevel : 0; // Do not split classes that don't allow it.
            dsc.branch  = m_tree->Branch(nam.c_str(),           // Branch name
                                         dsc.clazz->GetName(),  // Object class
                                         (void*)&dsc.buffer,    // Object address
                                         bufferSize,            // Buffer size
                                         split);                // Split Mode (Levels)
            if ( dsc.branch )  {
               dsc.leaf = dsc.branch->GetLeaf(nam.c_str());
               dsc.branch->SetAutoDelete(kFALSE);
               // AUTO-DELETE is now OFF.
               // This ensures, that all objects can be deleted
               // by the framework. Keep the created object in the
               // branch descriptor to allow selections
               setBranchOffsetTabLen( dsc.branch, branchOffsetTabLen );

               // AUX STORE specifics
               if( RootAuxDynIO::hasAuxStore(nam, dsc.clazz) ) {
                  TClass *storeTClass = dsc.clazz->GetBaseClass("SG::IAuxStoreIO");
                  if( storeTClass ) {
                     // Default splitting for dynamic attributes, one level less than aux store (since attributes are already separated).
                     int dynSplitLevel = splitLevel ? splitLevel - 1 : 0;
                     DbOption opt1("CONTAINER_SPLITLEVEL", RootAuxDynIO::AUXDYN_POSTFIX);
                     dbH.getOption(opt1);
                     opt1._getValue(dynSplitLevel);
                     // Default buffer size for dynamic attributes, one quarter of other branches (since attrbutes hold less data).
                     int dynBufferSize = bufferSize / 4;
                     // This is a class implementing SG::IAuxStoreIO
                     // Provide writers for its dynamic attibutes
                     dsc.aux_iostore_IFoffset = dsc.clazz->GetBaseClassOffset( storeTClass );
                     // TBranch Writer
                     bool do_branch_fill = isBranchContainer() && !m_treeFillMode;
                     dsc.auxdyn_writer = RootAuxDynIO::getBranchAuxDynWriter(m_tree, dynBufferSize, dynSplitLevel, branchOffsetTabLen, do_branch_fill);
                  }
               }
               return Success;
            }
         }
      }
   }
   catch( const std::exception& e )    {
      debugBreak(m_name, "Cannot attach ROOT object branch.", e);
   }
   catch (...)   {
      DbPrint err( m_name);
      err << DbPrintLvl::Fatal << "Unknown exception occurred. Cannot give more details."
          << DbPrint::endmsg;
      debugBreak(m_name, "Cannot attach ROOT object branch.", true);
   }
   DbPrint log( m_name);
   log << DbPrintLvl::Error << "Failed to open the container " << m_name << " of type "
       << ROOTTREE_StorageType.storageName()
       << " Class " << typ << " is unknown."
       << DbPrint::endmsg;
   return Error;
}


DbStatus
RootTreeContainer::addBranch(const DbColumn* col,BranchDesc& dsc,const std::string& desc) {
  dsc.column = col;
  const char* nam  = (m_branchName.empty() ? col->name().c_str() : m_branchName.c_str());
  std::string  coldesc = col->name() + desc;
  char buff[32];
  dsc.branch = m_tree->Branch(nam, buff, coldesc.c_str(), 4096);
  if( dsc.branch )  {
    dsc.leaf = dsc.branch->GetLeaf(nam);
    return Success;
  }
  return Error;
}


void RootTreeContainer::setTreeFillMode(bool mode) {
   m_treeFillMode = mode;
   for( auto& desc : m_branches ) {
      if( desc.auxdyn_writer ) desc.auxdyn_writer->setBranchFillMode(isBranchContainer() && !mode);
   }
}


void RootTreeContainer::setBranchOffsetTabLen(TBranch* b, int offsettab_len)
{
   if( offsettab_len > 0 ) {
      if( b->GetEntryOffsetLen() > 0 )
         b->SetEntryOffsetLen( offsettab_len );
      TIter biter( b->GetListOfBranches() );
      TBranch* subbranch(nullptr);
      while( (subbranch = (TBranch*)biter.Next()) ) {
         setBranchOffsetTabLen( subbranch, offsettab_len );
      }
   }
}


/// Access options
DbStatus RootTreeContainer::getOption(DbOption& opt) {
  if ( m_tree )  {
    const char* n = opt.name().c_str();
    if ( !strcasecmp(n,"BYTES_IO") )  {
       for( auto& branch: m_branches ) {
          if( branch.auxdyn_reader ) {
             const_cast<RootTreeContainer*>(this)->m_ioBytes += branch.auxdyn_reader->getBytesRead();
             branch.auxdyn_reader->resetBytesRead();
          }
       }
       return opt._setValue((int)m_ioBytes);
    }
    else if ( !strcasecmp(n,"BRANCH") )  {
      TBranch* b = branch(opt.option());
      return opt._setValue((void*)b);
    }
    else if ( ::toupper(n[0])=='B' && opt.name().length() > 7 ) {
      TBranch* b = branch(opt.option());
      if ( b )  {
        switch(::toupper(n[7]))  {
        case 'B':
          if ( !strcasecmp(n+7,"BASKET_SIZE") )
            return opt._setValue(int(b->GetBasketSize()));
          break;
        case 'C':
          if ( !strcasecmp(n+7,"COMPRESSION_LEVEL") )
            return opt._setValue(int(b->GetCompressionLevel()));
          if ( !strcasecmp(n+7,"COMPRESSION_ALGORITHM") )
            return opt._setValue(int(b->GetCompressionAlgorithm()));
          break;
        case 'E':
          if ( !strcasecmp(n+7,"ENTRIES") )
            return opt._setValue(int(b->GetEntries()));
          break;
        case 'F':
          if ( !strcasecmp(n+7,"FILE_NAME") )
            return opt._setValue(b->GetFileName());
          else if ( !strcasecmp(n+7,"FILE") )
            return opt._setValue((void*)b->GetFile());
          break;
        case 'M':
          if ( !strcasecmp(n+7,"MAX_BASKETS") )
            return opt._setValue(int(b->GetMaxBaskets()));
          break;
        case 'N':
          if ( !strcasecmp(n+7,"NLEAVES") )
            return opt._setValue(int(b->GetNleaves()));
          break;
        case 'S':
          if ( !strcasecmp(n+7,"SPLIT_LEVEL") )
            return opt._setValue(int(b->GetSplitLevel()));
          break;
        case 'T':
          if ( !strcasecmp(n+7,"TOTAL_SIZE") )
            return opt._setValue(double(b->GetTotalSize()));
          else if ( !strcasecmp(n+7,"TOTAL_BYTES") )
            return opt._setValue(double(b->GetTotBytes()));
          break;
        case 'Z':
          if ( !strcasecmp(n+7,"ZIP_BYTES") )
            return opt._setValue(double(b->GetZipBytes()));
          break;
        }
      }
    }
    else if ( !strcasecmp(n,"TREE") )  {
      return opt._setValue((void*)m_tree);
    }
    else if ( ::toupper(n[0])=='T' && opt.name().length() > 6 ) {
      switch(::toupper(n[5]))   {
      case 'B':
        if ( !strcasecmp(n+5,"BRANCH_IDX") )  {
          int idx = 0;
          opt._getValue(idx);
          TTree* tree ATLAS_THREAD_SAFE = m_tree;  // GetListOfBranches should be const
          const TObjArray* arr = tree->GetListOfBranches();
          return opt._setValue((void*)arr->At(idx));
        }
        if ( !strcasecmp(n+5,"BRANCH_NAME") )  {
          const char* br_nam = nullptr;
          opt._getValue(br_nam);
          if ( br_nam )  {
            TTree* tree ATLAS_THREAD_SAFE = m_tree;  // GetBranch should be const
            return opt._setValue((void*)tree->GetBranch(br_nam));
          }
          opt._setValue((void*)nullptr);
        }
        if ( !strcasecmp(n+5,"BRANCH_IDX_NAME") )  {
          int idx = 0;
          opt._getValue(idx);
          TTree* tree ATLAS_THREAD_SAFE = m_tree;  // GetListOfBranches should be const
          const TObjArray* arr = tree->GetListOfBranches();
          TBranch* br = (TBranch*)arr->At(idx);
          if ( br )  {
            return opt._setValue(br->GetName());
          }
          opt._setValue((char*)nullptr);
        }
        break;
      case 'E':
        if ( !strcasecmp(n+5,"ENTRIES") )
          return opt._setValue(int(m_tree->GetEntries()));
        break;
      case 'F':
        if ( !strcasecmp(n+5,"FILE_NUMBER") )
          return opt._setValue(int(m_tree->GetFileNumber()));
        break;
      case 'M':
        if ( !strcasecmp(n+5,"MAX_SIZE") )
          return opt._setValue((long long int)m_tree->GetMaxTreeSize());
        else if ( !strcasecmp(n+5,"MAX_VIRTUAL_SIZE") )
          return opt._setValue(int(m_tree->GetMaxVirtualSize()));
        break;
      case 'N':
        if ( !strcasecmp(n+5,"NBRANCHES") ) {
          TTree* tree ATLAS_THREAD_SAFE = m_tree;  // GetNBranches should be const
          const int nBranches = tree->GetNbranches();
          return opt._setValue(nBranches);
        }
        break;
      case 'T':
        if ( !strcasecmp(n+5,"TOTAL_BYTES") )
          return opt._setValue(double(m_tree->GetTotBytes()));
        break;
      case 'Z':
        if ( !strcasecmp(n+5,"ZIP_BYTES") )
          return opt._setValue(double(m_tree->GetZipBytes()));
        break;
      }
    }
  }
  return Error;
}

/// Set options
DbStatus RootTreeContainer::setOption(const DbOption& opt)  {
  if ( m_tree )  {
    const char* n = opt.name().c_str();
    if ( ::toupper(n[0]) == 'B' )  {
      TBranch* b = branch(opt.option());
      if ( b )  {
        switch(::toupper(n[7]))   {
        case 'A':
          if ( !strcasecmp(n+7,"AUTODELETE") )  {
            int val=0;
            opt._getValue(val);
            b->SetAutoDelete(val!=0);
            return Success;
          }
          break;
        case 'C':
          if ( !strcasecmp(n+7,"COMPRESSION_LEVEL") )  {
            int val=1;
            opt._getValue(val);
            b->SetCompressionLevel(val);
            return Success;
          }
          if ( !strcasecmp(n+7,"COMPRESSION_ALGORITHM") )  {
            int val=1;
            opt._getValue(val);
            b->SetCompressionAlgorithm(val);
            return Success;
          }
          break;
        case 'P':
          if ( !strcasecmp(n+7,"PRINT") )  {
            b->Print();
            std::cout << std::endl;
            return Success;
          }
          break;
        case 'B':
          if ( !strcasecmp(n+7,"BASKET_SIZE") )  {
            int value = 16*1024;
            opt._getValue(value);
            b->SetBasketSize(value);
            return Success;
          }
          break;
        default:
          break;
        }
      }
    }
    else if ( ::toupper(n[0]) == 'T' )  {
      switch(::toupper(n[5]))   {
      case 'A':
        if ( !strcasecmp(n+5,"AUTO_SAVE") )  {
          int val=1;
          opt._getValue(val);
          m_tree->SetAutoSave(val);
          return Success;
        } else if ( !strcasecmp(n+5,"AUTO_FLUSH") )  {
          int val=1;
          opt._getValue(val);
          m_tree->SetAutoFlush(val);
          // cout << "----------- setting AUTO_FLUSH for " << m_tree->GetName() << endl;
          return Success;
        }
        break;
      case 'M':
        if ( !strcasecmp(n+5,"MAX_SIZE") )  {
          long long int val=1;
          opt._getValue(val);
          m_tree->SetMaxTreeSize(val);
          return Success;
        }
        break;
      case 'P':
        if ( !strcasecmp(n+5,"PRINT") )  {
          m_tree->Print();
          std::cout << std::endl;
          return Success;
        }
        break;
      }
    }
  }
  return Error;
}

/// Execute transaction action
DbStatus RootTreeContainer::transAct(Transaction::Action action)
{
   // execure action on the base class first
   DbStatus status = DbContainerImp::transAct(action);
   if( !status.isSuccess() ) return status;
   if( action != Transaction::TRANSACT_FLUSH ) return Success;
   if( !m_tree ) return Error;

   if( !isBranchContainer() ) {
      m_tree->AutoSave();
      return Success;
   }
   // check if all TTree branches were filled and write the TTree
   for( auto& desc : m_branches ) {
      Long64_t branchEntries = desc.branch->GetEntries();
      Long64_t treeEntries = m_tree->GetEntries();
      if (branchEntries > treeEntries) {
         TIter next(m_tree->GetListOfBranches());
         TBranch * b = nullptr;
         while( (b = (TBranch*)next()) ) {
            if (b->GetEntries() != branchEntries) {
               DbPrint log(m_name);
               log << DbPrintLvl::Error << "Every branch must have the same number of entries."
                   << "  branch " << b->GetName() << " " << b->GetEntries()
                   << DbPrint::endmsg;
               return Error;
            }
         }
         m_tree->SetEntries(branchEntries);
         m_tree->AutoSave();
      } else if (branchEntries < treeEntries) {
         DbPrint log(m_name);
         log << DbPrintLvl::Error << "Every branch must have the same number of entries."
             << " Tree entries=" << treeEntries << " but this branch shows " << branchEntries
             << " entries" << DbPrint::endmsg;
         return Error;
      }
      desc.rows_written = 0;
   }
   return Success;
}

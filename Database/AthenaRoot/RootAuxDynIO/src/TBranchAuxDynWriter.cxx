/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TBranchAuxDynWriter.h"

#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/normalizedTypeinfoName.h"

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TClass.h"


namespace RootAuxDynIO
{
   
   void*     AuxInfo::setDummyAddr()   {
      object = nullptr;
      if( tclass ) {
         if( !dummy ) {
            using std::placeholders::_1;
            void(TClass::*dxtor)(void*, Bool_t) = &TClass::Destructor;
            std::function<void(void*)> del = std::bind(dxtor, tclass, _1, false);
            dummyptr = dummy_ptr_t( tclass->New(), std::move(del) );
            dummy = dummyptr.get();
         }
         branch->SetAddress( &dummy );
         return &dummy;
      }
      return nullptr;
   }

   AuxInfo::~AuxInfo() {
      if( buffer && tclass ) {
         tclass->Destructor(buffer);
      }
   }


   TBranchAuxDynWriter::TBranchAuxDynWriter( TTree& tree, TClass& cls, int bufferSize, int splitLevel,
                                             int offsettab_len, bool branch_fill ) :
      AthMessaging ("TBranchAuxDynWriter"), AuxDynAttrAccess(cls),
      m_ttree( &tree ),
      m_bufferSize( bufferSize ),
      m_splitLevel( splitLevel ),
      m_branchOffsetTabLen( offsettab_len ),
      m_branchFillMode( branch_fill )
   { }


   /* Copied from RootTreeContainer::setBranchOffsetTabLen - probably should be kept in sych with it */
   void TBranchAuxDynWriter::setBranchOffsetTabLen(TBranch* b, int offsettab_len) {
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


   //  throws exceptions
   void TBranchAuxDynWriter::createAuxBranch( AuxInfo& info ) {
      std::string error_type("is unknown");
      const std::type_info& ti = *info.typeinfo;
      try {
         auto createBasicAuxBranch = [&](const char* typeopt) {
            info.is_basic_type = true;
            info.branch = m_ttree->Branch(info.branch_name.c_str(), info.buffer, (info.name+typeopt).c_str(), 2048);
            ATH_MSG_VERBOSE("Created branch with name=" << info.branch_name << "  type: " << info.name+typeopt);
         };
         if     ( ti == typeid(UInt_t) )    createBasicAuxBranch("/i");
         else if( ti == typeid(Int_t) )     createBasicAuxBranch("/I");
         else if( ti == typeid(Double_t) )  createBasicAuxBranch("/D");
         else if( ti == typeid(Float_t) )   createBasicAuxBranch("/F");
         else if( ti == typeid(Long64_t)  or ti == typeid(std::int64_t) )  createBasicAuxBranch("/L");
         else if( ti == typeid(ULong64_t) or ti == typeid(std::uint64_t) ) createBasicAuxBranch("/l");
         else if( ti == typeid(Short_t) )   createBasicAuxBranch("/S");
         else if( ti == typeid(UShort_t) )  createBasicAuxBranch("/s");
         else if( ti == typeid(Char_t) )    createBasicAuxBranch("/B");
         else if( ti == typeid(UChar_t) )   createBasicAuxBranch("/b");
         else if( ti == typeid(bool) )      createBasicAuxBranch("/O");
         else if( ti == typeid(char*) || ti == typeid(unsigned char*) ) createBasicAuxBranch("/C");
         else {
            TClass* cl = TClass::GetClass(info.type_name.c_str());
            if( !cl ) {
               error_type =" has no TClass";
            } else if( !cl->GetStreamerInfo() )  {
               error_type =" has no streamer";
            } else if( !cl->HasDictionary() ) {
               error_type =" has no dictionary";
            } else {
               info.tclass = cl;
               int split = cl->CanSplit() ? m_splitLevel : 0;
               info.branch = m_ttree->Branch( info.branch_name.c_str(),  // Branch name
                                              cl->GetName(),             // Object class
                                              (void*)&info.buffer,       // Object address
                                              m_bufferSize,                // Buffer size
                                              split);                    // Split Mode (Levels)
            ATH_MSG_VERBOSE("Created branch with name=" << info.branch_name << "  type: " << cl->GetName());
            }
         }
      } catch( const std::exception& e ) {
         error_type += std::string("  Exception msg: ") + e.what();
      }
      catch (...)   {
         error_type += "  Unknown exception.";
      }

      if( !info.branch ) {
         throw std::runtime_error( std::string("Failed to create Auxiliary branch '") + info.branch_name
                                   + "'. Class: " + info.type_name + error_type );
      }
      // Set AUTO-DELETE OFF.  ROOT will not delete objects created by the framework
      info.branch->SetAutoDelete(kFALSE);

      setBranchOffsetTabLen( info.branch, m_branchOffsetTabLen );
   }


   /// handle writing of dynamic xAOD attributes of an object - called from RootTreeContainer::writeObject()
   //  throws exceptions
   int TBranchAuxDynWriter::writeAuxAttributes( const std::string& base_branchname,
                                                void* object,
                                                size_t backfill_nrows ) 
   {
      SG::IAuxStoreIO *store = castIOStore(object);
      int bytes_written = 0;
      const SG::auxid_set_t selection = store->getSelectedAuxIDs();
      ATH_MSG_DEBUG("Writing " << base_branchname << " with " << selection.size() << " Dynamic attributes");

      // mark all attributes as not written yet
      for( auto& aux_info_entry : m_auxInfoMap )  aux_info_entry.second.written = false;
      m_needsFill = false;

      for(SG::auxid_t id : selection) {
         AuxInfo& attrInfo = m_auxInfoMap[id];
         if( !attrInfo.branch ) {
            // new attribute info, fill it
            attrInfo.typeinfo = store->getIOType(id);
            attrInfo.type_name = SG::normalizedTypeinfoName( *attrInfo.typeinfo );
            attrInfo.name = SG::AuxTypeRegistry::instance().getName(id);
            attrInfo.branch_name = RootAuxDynIO::auxBranchName(attrInfo.name, base_branchname);

            ATH_MSG_DEBUG("Creating branch for new dynamic attribute, Id=" << id << ": type=" << attrInfo.type_name
                          << ",  branch=" << attrInfo.branch_name );
            createAuxBranch( attrInfo );
            // backfill here
            if( backfill_nrows ) {
               // if this is not the first row, catch up with the rows written already to other branches
               ATH_MSG_DEBUG("  Backfilling " << backfill_nrows << " entries for " << attrInfo.name);
               // As of root 6.22, calling SetAddress with nullptr may not work as expected if the address had
               // previously been set to something non-null.
               // So we need to create the temp object ourselves.
               attrInfo.setDummyAddr();
               for( size_t r=0; r<backfill_nrows; ++r ) {
                  bytes_written += attrInfo.branch->BackFill();
                  ATH_MSG_VERBOSE("BACKFilled branch:" << m_ttree->GetName() << "::" << attrInfo.branch->GetName() <<
                                  " Tree size=" << m_ttree->GetEntries()
                                  << "  branch size:" << attrInfo.branch->GetEntries() );
               }
            }
         }
         void *obj ATLAS_THREAD_SAFE = const_cast<void*>( store->getIOData(id) );
         attrInfo.object = obj;
         attrInfo.branch->SetAddress( attrInfo.objectAddr() );
         attrInfo.written = true;
      }

      for( auto& aux_info_entry : m_auxInfoMap ) {
         AuxInfo& attrInfo = aux_info_entry.second;
         // if an attribute was not written create a default object for it
         if( !attrInfo.written ) {
            attrInfo.setDummyAddr();
            ATH_MSG_DEBUG("Default object added to branch: " << attrInfo.branch_name);
         }
         // if writing to branches independently, do it now
         if( m_branchFillMode ) {
            bytes_written += attrInfo.branch->Fill();
         } else {
            m_needsFill = true;
         }
      }
      return bytes_written;
   }


   int TBranchAuxDynWriter::commit() {
      ATH_MSG_WARNING("Commit for TBranchAuxDynWriter should be handled on the DB level");
      return 0;
   }
   
} // namespace

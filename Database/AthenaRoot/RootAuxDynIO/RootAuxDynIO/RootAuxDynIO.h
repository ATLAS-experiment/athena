/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ROOTAUXDYN_IO_H
#define ROOTAUXDYN_IO_H

#include "RVersion.h"

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <tuple>
#include "RootAuxDynIO/RootAuxDynDefs.h"

class TBranch;
class TTree;
class TFile;
class TClass;

#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 35, 0 )
   namespace ROOT { class RFieldBase; }
   namespace ROOT { class RNTupleReader; }
#else
   namespace ROOT::Experimental { class RFieldBase; }
   namespace ROOT::Experimental { class RNTupleReader; }
#endif

namespace SG { class IAuxStoreIO;  class auxid_set_t; }


namespace RootAuxDynIO
{
#if ROOT_VERSION_CODE >= ROOT_VERSION( 6, 35, 0 )
   using ROOT::RFieldBase;
   using ROOT::RNTupleReader;
#else
   using ROOT::Experimental::RFieldBase;
   using ROOT::Experimental::RNTupleReader;
#endif

   class IRootAuxDynReader;
   class IRootAuxDynWriter;
   class IRNTupleAuxDynWriter;
   class IRNTupleWriter;

   /// check if a field/branch with fieldname and type tc has IAuxStore interface
   bool hasAuxStore(std::string_view fieldname, TClass *tc);

  /**
   * @brief Check is a branch holds AuxStore objects
   * @param branch TBranch to check
   */
   bool isAuxDynBranch(TBranch *branch);

  /**
   * @brief Exctract the Aux object SG Key from the branch name
   * @param branch TBranch with Key in its name
   */
   std::string getKeyFromBranch(TBranch* branch);

   std::unique_ptr<IRootAuxDynReader> getBranchAuxDynReader(TTree*, TBranch*);
   std::unique_ptr<IRootAuxDynWriter> getBranchAuxDynWriter(TTree*, int bufferSize, int splitLevel,
                                                              int offsettab_len, bool do_branch_fill);
   
   std::unique_ptr<IRootAuxDynReader>    getNTupleAuxDynReader(const std::string& field_name, const std::string& field_type, RNTupleReader* reader);
   std::unique_ptr<IRNTupleAuxDynWriter> getNTupleAuxDynWriter();
   std::unique_ptr<IRNTupleWriter>       getNTupleWriter(TFile*,  const std::string& ntupleName, bool enableBufferedWrite, bool enableMetrics);

   // The convention for the tuple is <name, type, data>
   typedef std::tuple<std::string, std::string, void*> attrDataTuple;

   class IRootAuxDynReader
   {
   public :
      /**
       * @brief Attach specialized AuxStore for reading dynamic attributes
       * @param object object instance to which the store will be attached to - has to be an instance of the type the reader was created for
       * @param ttree_row

       Use this method to instrument an AuxStore object AFTER it was read (every time it is read)
       This will attach its dynamic attributes with read-on-demand capability
      */
      virtual void addReaderToObject(void* object, size_t row, std::recursive_mutex* iomtx = nullptr) = 0;

      virtual const SG::auxid_set_t& auxIDs() const = 0;

      virtual size_t getBytesRead() const = 0;

      virtual void resetBytesRead() = 0; 

      virtual ~IRootAuxDynReader() {}
   };


   /// Interface for an AuxDyn Writer - TTree based 
   class IRootAuxDynWriter {
   public:
      virtual ~IRootAuxDynWriter() {}

      /// handle writing of dynamic xAOD attributes of an AuxContainer - called from RootTreeContainer::writeObject()
      /// may report bytes written (see concrete implementation)
      //  may throw exceptions
      virtual int writeAuxAttributes(const std::string& base_branch, SG::IAuxStoreIO* store, size_t rows_written ) = 0;

      /// is there a need to call commit()?
      virtual bool needsCommit() = 0;

      /// Call Fill() on the ROOT object used by this writer
      virtual int commit() = 0;

      /// set per-branch independent commit/fill mode
      virtual void setBranchFillMode(bool) = 0;
   };

   /// Interface for a RNTuple-based Writer that handles AuxDyn attributes 
   /// Works in conjuction with the generic writer
   class IRNTupleAuxDynWriter {
   public:
      /// Default Destructor
      virtual ~IRNTupleAuxDynWriter() = default;

      /// Collect Aux data information to be writting out
      virtual std::vector<attrDataTuple> collectAuxAttributes( const std::string& base_branch, SG::IAuxStoreIO* store ) = 0;
   };

} // namespace

#endif

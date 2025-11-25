/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef IROOTAUXDYN_IO_H
#define IROOTAUXDYN_IO_H

#include "RootAuxDynIO/RootAuxDynDefs.h"

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <tuple>

#include "RVersion.h"

class TBranch;
class TTree;
class TFile;
class TClass;


// Forward declarations
namespace ROOT { class RNTupleReader; }

namespace RootAuxDynIO
{
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

      virtual size_t getBytesRead() const = 0;

      virtual void resetBytesRead() = 0; 

      virtual ~IRootAuxDynReader() {}
   };


   /// Interface for an AuxDyn Writer - TTree based 
   class IRootAuxDynWriter {
   public:
      virtual ~IRootAuxDynWriter() = default;

      /// handle writing of dynamic xAOD attributes of an AuxContainer - called from RootTreeContainer::writeObject()
      /// may report bytes written (see concrete implementation)
      //  may throw exceptions
      virtual int writeAuxAttributes(const std::string& base_branch, void* object, size_t rows_written ) = 0;

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
      virtual std::vector<attrDataTuple> collectAuxAttributes( const std::string& base_branch, void* object ) = 0;
   };



   class IFactoryTool
   {
   public:
      virtual ~IFactoryTool() = default;

      virtual std::unique_ptr<IRootAuxDynReader>
      getBranchAuxDynReader(TTree*, TBranch*) const = 0;

      virtual std::unique_ptr<IRootAuxDynWriter>
      getBranchAuxDynWriter(TTree&, TClass&, int bufferSize, int splitLevel,
                            int offsettab_len, bool do_branch_fill) const = 0;

      virtual std::unique_ptr<IRNTupleAuxDynWriter>
      getNTupleAuxDynWriter(TClass &tc) const = 0;

      virtual std::unique_ptr<IRootAuxDynReader>
      getNTupleAuxDynReader(const std::string& field_name, const std::string& field_type,
                            ROOT::RNTupleReader* reader) const = 0;


      /// check if a field/branch with fieldname and type tc has IAuxStore interface
      virtual bool hasAuxStore(std::string_view fieldname, TClass *tc) const = 0;

      /// check if the type tc has IAuxStoreIO interface
      virtual bool hasAuxStoreIO(TClass *tc) const = 0;

      /**
       * @brief Check is a branch holds AuxStore objects
       * @param branch TBranch to check
       */
      virtual bool isAuxDynBranch(TBranch *branch) const = 0;
   };

} // namespace

#endif


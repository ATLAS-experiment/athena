/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ROOTAUXDYN_IO_H
#define ROOTAUXDYN_IO_H

#include <string>
#include <mutex>

class TBranch;
class TTree;
class TFile;
class TClass;

namespace ROOT { namespace Experimental { namespace Detail {
   class RPageSource;
   class RFieldBase;
} } }
namespace SG { class IAuxStoreIO;  class auxid_set_t; }


namespace RootAuxDynIO
{
   class IRootAuxDynReader;
   class IRootAuxDynWriter;
   class IRNTupleWriter;

   /// Common post-fix for the names of auxiliary containers in StoreGate
   constexpr char   AUX_POSTFIX[] = "Aux.";
   constexpr size_t AUX_POSTFIX_LEN = sizeof(AUX_POSTFIX)-1;
   constexpr char   AUXDYN_POSTFIX[] = "Dyn.";
   constexpr size_t AUXDYN_POSTFIX_LEN = sizeof(AUXDYN_POSTFIX)-1;

   /// check if a string ends with AUX_POSTFIX
   inline bool endsWithAuxPostfix(std::string_view str) {
      return str.size() >= AUX_POSTFIX_LEN and
         str.compare(str.size()-AUX_POSTFIX_LEN, AUX_POSTFIX_LEN, AUX_POSTFIX) == 0;
   }

   /// check if a field/branch with fieldname and type tc has IAuxStore interface
   bool hasAuxStore(std::string_view fieldname, TClass *tc);

  /**
   * @brief Check is a branch holds AuxStore objects
   * @param branch TBranch to check
   */
   bool isAuxDynBranch(TBranch *branch);

   /**
   * @brief Construct branch name for a given dynamic attribute
   * @param attr_name the name of the attribute
   * @param baseBranchName branch name for the main AuxStore object
   */
   std::string auxBranchName(const std::string& attr_name, const std::string& baseBranchName);

   /**
   * @brief Construct field name for a given dynamic attribute
   * @param attr_name the name of the attribute
   * @param baseBranchName branch name for the main AuxStore object
   */
   std::string auxFieldName(const std::string& attr_name, const std::string& baseName);

  /**
   * @brief Exctract the Aux object SG Key from the branch name
   * @param branch TBranch with Key in its name
   */
   std::string getKeyFromBranch(TBranch* branch);

   std::unique_ptr<IRootAuxDynReader> getBranchAuxDynReader(TTree*, TBranch*);
   std::unique_ptr<IRootAuxDynWriter> getBranchAuxDynWriter(TTree*, int bufferSize, int splitLevel,
                                                              int offsettab_len, bool do_branch_fill);
   
   std::unique_ptr<IRootAuxDynReader> getNTupleAuxDynReader(ROOT::Experimental::Detail::RFieldBase* field,
                                                              ROOT::Experimental::Detail::RPageSource* source);
   std::unique_ptr<IRNTupleWriter>    getNTupleAuxDynWriter(TFile*,  const std::string& ntupleName, bool enableBufferedWrite, bool enableMetrics);


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

   
   /// Interface for a generic RNTuple-based Writer (can handle both normal objects and AuxDyn attributes
   class IRNTupleWriter {
   public:
      virtual ~IRNTupleWriter() {}

      virtual const std::string& getName() const = 0;

      virtual size_t size() const = 0;

      /// Add a new field to the RNTuple
      virtual void addField( const std::string& field_name, const std::string& attr_type ) = 0;

      /// Supply data address for a given field
      virtual void addFieldValue( const std::string& field_name, void* attr_data ) = 0;

      /// handle writing of dynamic xAOD attributes of an AuxContainer - called from RNTupleContainer::writeObject()
      /// should report bytes written  - it does not do than yet though
      //  may throw exceptions
      virtual int writeAuxAttributes(const std::string& base_branch, SG::IAuxStoreIO* store, size_t rows_written ) = 0;

      /// Add a APR container to this RNTuple - if there is more than one than do grouped DB commit
      virtual void increaseClientCount() = 0;
      /// Check if there is more than one container writing to this RNTuple
      virtual bool isGrouped() const = 0;
      
      /// is there a need to call commit()?
      virtual bool needsCommit() const = 0;

      /// Call Fill() on the ROOT object used by this writer
      virtual int commit() = 0;

      virtual void close() = 0;
   };

} // namespace

#endif

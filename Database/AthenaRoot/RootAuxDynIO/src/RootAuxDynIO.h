/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ROOTAUXDYN_IO_H
#define ROOTAUXDYN_IO_H

#include "Gaudi/PluginService.h"

#include "RootAuxDynIO/IRootAuxDynIO.h"

namespace SG { class IAuxStoreIO; }
class TClass;
class TBranch;

namespace RootAuxDynIO
{

 class AuxDynAttrAccess
   {
   public:
      AuxDynAttrAccess(TClass& tc);
      bool hasAuxDynStore() const;
      
   protected:
      int auxStoreOffset(TClass &tc);
      SG::IAuxStoreIO* castIOStore(void *object);

      /// TClass of the type containing the AuxStore with attributes
      TClass&   m_holderType;
      /// AuxStoreIO interface offset in the subclass type (for casting). negative means no inheritance
      int       m_ioStoreOffset;
   };


  /**
   * @brief Exctract the Aux object SG Key from the branch name
   * @param branch TBranch with Key in its name
   */
   std::string getKeyFromBranch(TBranch* branch);


   class FactoryTool : public IFactoryTool
   {
   public:
      using Factory = Gaudi::PluginService::Factory< IFactoryTool*() >;

      virtual std::unique_ptr<IRootAuxDynReader>
      getBranchAuxDynReader(TTree*, TBranch*) const override final;

      virtual std::unique_ptr<IRootAuxDynWriter>
      getBranchAuxDynWriter(TTree&, TClass&, int bufferSize, int splitLevel,
                            int offsettab_len, bool do_branch_fill) const override final;

      virtual std::unique_ptr<IRNTupleAuxDynWriter>
      getNTupleAuxDynWriter(TClass &tc) const override final;

      virtual std::unique_ptr<IRootAuxDynReader>
      getNTupleAuxDynReader(const std::string& field_name, const std::string& field_type,
                            ROOT::RNTupleReader* reader) const override final;

      /// check if a field/branch with fieldname and type tc has IAuxStore interface
      virtual bool hasAuxStore(std::string_view fieldname, TClass *tc) const override final;

      /// check if the type tc has IAuxStoreIO interface
      virtual bool hasAuxStoreIO(TClass *tc) const override final;

      /**
       * @brief Check is a branch holds AuxStore objects
       * @param branch TBranch to check
       */
      virtual bool isAuxDynBranch(TBranch *branch) const override final;
   };

}

#endif

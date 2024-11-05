/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ROOTAUXDYN_DEFS_H
#define ROOTAUXDYN_DEFS_H

#include <format>

namespace RootAuxDynIO
{
   /// Common post-fix for the names of auxiliary containers in StoreGate
   constexpr char   AUX_POSTFIX[] = "Aux.";
   constexpr size_t AUX_POSTFIX_LEN = sizeof(AUX_POSTFIX)-1;
   constexpr char   AUXDYN_POSTFIX[] = "Dyn.";
   constexpr size_t AUXDYN_POSTFIX_LEN = sizeof(AUXDYN_POSTFIX)-1;

   /**
   * @brief Check if a string ends with AUX_POSTFIX
   * @param str the string to check
   */
   inline bool endsWithAuxPostfix(std::string_view str) {
      return str.ends_with(AUX_POSTFIX);
   }

   /**
   * @brief if a string ends with AUX_POSTFIX then remove it
   * @param str the string to modify
   */
   bool removeAuxPostfix(std::string str);

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

} // namespace


inline bool
RootAuxDynIO::removeAuxPostfix(std::string str)
{
   if( endsWithAuxPostfix(str) ) {
      str.resize( str.size() - AUX_POSTFIX_LEN );
      return true;
   }
   return false;
}


inline std::string
RootAuxDynIO::auxBranchName(const std::string& attr_name, const std::string& baseBranchName)
{
   std::string branch_name = baseBranchName;
   if( !branch_name.empty() and branch_name.back() == '.' )  branch_name.pop_back();
   branch_name += RootAuxDynIO::AUXDYN_POSTFIX + attr_name;
   return branch_name;
}

inline std::string
RootAuxDynIO::auxFieldName(const std::string& attr_name, const std::string& baseName)
{
   // RNTuple field names cannot contain dot characters '.'
   // Therefore, for now, we're using a colon instead ':'
   // This can be changed in the future
   std::string field_name = baseName;
   if (field_name.ends_with(':')) {
     field_name.pop_back();
   }
   field_name = std::format("{}Dyn:{}", field_name, attr_name);
   return field_name;
}

#endif

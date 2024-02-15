/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ROOTAUXDYN_DEFS_H
#define ROOTAUXDYN_DEFS_H


namespace RootAuxDynIO
{
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
   std::string field_name = baseName;
   if( field_name.back() == '.' )  field_name.pop_back();
   field_name += ":" + attr_name;    // MN TODO <- find a good delimiter
   return field_name;
}




#endif

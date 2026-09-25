/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PersistentDataModel/AthenaAttributeList.h"
#include <print>

AthenaAttributeList::AthenaAttributeList() : coral::AttributeList()
{}

AthenaAttributeList::AthenaAttributeList(const coral::AttributeList& rhs) : coral::AttributeList(rhs)
{}

AthenaAttributeList::AthenaAttributeList(const coral::AttributeListSpecification& spec) : coral::AttributeList(spec)
{}

void AthenaAttributeList::print(std::ostream& os) const {
  std::print (os, "{{");
  for (coral::AttributeList::const_iterator itr=this->begin();
      itr!=this->end();++itr) {
    if (itr!=this->begin()) std::print (os, ",");
    itr->toOutputStream(os);
  }
  std::print (os, "}}");
}

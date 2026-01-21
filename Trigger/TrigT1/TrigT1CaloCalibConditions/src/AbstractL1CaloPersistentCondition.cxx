/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1CaloCalibConditions/AbstractL1CaloPersistentCondition.h"

#include "CoralBase/AttributeListSpecification.h"

#include <iostream>

DataObject* AbstractL1CaloPersistentCondition::makePersistent() const {
    std::cout << "ERROR - The makePersistent() method you are calling is not implemented by the current class !" << std::endl;
	return 0;
}

void AbstractL1CaloPersistentCondition::makeTransient(const std::map<std::string, const CondAttrListCollection*>& /* condAttrListCollectionMap */) {
    std::cout << "ERROR - The makeTransient(const map<std::string, const CondAttrListCollection*>) method you are calling is not implemented by the current class !" << std::endl;
	return;
}

void AbstractL1CaloPersistentCondition::makeTransient(const std::map<std::string, const AthenaAttributeList*>& /* athenaAttributeList */) {
    std::cout << "ERROR - The makeTransient(const map<std::string, const AthenaAttributeList*>) method you are calling is not implemented by the current class !" << std::endl;
	return;
}

void AbstractL1CaloPersistentCondition::addSpecification(int specId, const std::string& specName, const std::string& specType) {
	m_attrSpecificationNameMap[specId] = specName;
	m_attrSpecificationTypeMap[specId] = specType;
}

coral::AttributeListSpecification* AbstractL1CaloPersistentCondition::createAttributeListSpecification() const {

	coral::AttributeListSpecification* attrSpecification =  new coral::AttributeListSpecification();

	AttrSpecificationMap::const_iterator it_name = m_attrSpecificationNameMap.begin();
	AttrSpecificationMap::const_iterator it_type = m_attrSpecificationTypeMap.begin();

	for(;it_name!=m_attrSpecificationNameMap.end();++it_name,++it_type) {
		attrSpecification->extend(it_name->second, it_type->second);
	}
	return attrSpecification;
}

std::string AbstractL1CaloPersistentCondition::specificationName(int specId) const {
  auto p = m_attrSpecificationNameMap.find(specId);
  if (p == m_attrSpecificationNameMap.end()) return "";
	return p->second;
}

std::string AbstractL1CaloPersistentCondition::specificationType(int specId) const {
  auto p = m_attrSpecificationTypeMap.find(specId);
  if (p == m_attrSpecificationTypeMap.end()) return "";
	return p->second;
}

void AbstractL1CaloPersistentCondition::clear() {
	std::cout << "ERROR - The clear() method you are calling is not implemented by the current class !" << std::endl;
	return;
}

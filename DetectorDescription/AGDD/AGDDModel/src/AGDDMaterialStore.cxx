/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "AGDDModel/AGDDSimpleMaterial.h"
#include "AGDDModel/AGDDElement.h"
#include "AGDDModel/AGDDMaterialStore.h"

#include <iostream>

AGDDMaterialStore::AGDDMaterialStore():m_nrOfMaterials(0),m_nrOfElements(0)
{
	m_theMaterials.clear();
	m_theElements.clear();
}

void
AGDDMaterialStore::RegisterElement(AGDDElement* el){
  const auto &[it, inserted] = m_theElements.try_emplace(el->GetName(), el);
  if (!inserted) {
    std::cout << "Element " << el->GetName()<< " already defined: skipping\n";
    return;
  }
  ++m_nrOfElements;
}
void
AGDDMaterialStore::RegisterMaterial(AGDDSimpleMaterial* mat){
  const auto &[it, inserted] = m_theMaterials.try_emplace(mat->GetName(), mat);
  if (!inserted) {
    std::cout << "Material " << mat->GetName() << " already defined: skipping\n";
    return;
  }
  ++m_nrOfMaterials;
}

AGDDSimpleMaterial*
AGDDMaterialStore::GetMaterial(std::string_view mat){
  const auto it = m_theMaterials.find(mat);
  if (it == m_theMaterials.end()) {
    std::cout << " Material " << mat << " not found!\n";
    return nullptr;
  }
  return it->second;
}
AGDDElement* AGDDMaterialStore::GetElement(std::string_view el){ 
  const auto it = m_theElements.find(el);
	if (it == m_theElements.end()){
		std::cout<<" Element "<<el<<" not found!"<<std::endl;
		return nullptr;
	}
	return it->second;
}

void AGDDMaterialStore::PrintElementNames()
{
	std::cout<<"List of elements so far defined: "<<
			std::endl<<"-----> ";
	AGDDElementMap::const_iterator it;
	int i=0;
	for (it=m_theElements.begin();it!=m_theElements.end();++it)
	{
		i++;
		if (!(i%5)) std::cout<<std::endl<<"-----> ";
		std::cout<<(*it).first<<",";
	}
}
	
void AGDDMaterialStore::PrintMaterialNames()
{
	std::cout<<"List of materials so far defined: "<<
			std::endl<<"-----> ";
	AGDDMaterialMap::const_iterator it;
	int i=0;
	for (it=m_theMaterials.begin();it!=m_theMaterials.end();++it)
	{
		i++;
		if (!(i%5)) std::cout<<std::endl<<"-----> ";
		std::cout<<(*it).first<<",";
	}
}

void AGDDMaterialStore::PrintElement(const std::string& n)
{
	if (m_theElements.find(n)!=m_theElements.end())
		std::cout<<*(m_theElements[n]);
	else
		std::cout<<"Element "<<n<<" not found!"<<std::endl;
}

void AGDDMaterialStore::PrintMaterial(const std::string& n)
{
	if (m_theMaterials.find(n)!=m_theMaterials.end())
		std::cout<<*(m_theMaterials[n]);
	else
		std::cout<<"Material "<<n<<" not found!"<<std::endl;
}

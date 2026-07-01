/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AGDDMaterialStore_H
#define AGDDMaterialStore_H

#include <string>
#include <string_view>
#include <map>

class AGDDSimpleMaterial;
class AGDDElement;



typedef std::map<std::string,AGDDSimpleMaterial*, std::less<> > AGDDMaterialMap;
typedef std::map<std::string,AGDDElement*, std::less<> > AGDDElementMap;

typedef AGDDMaterialMap::const_iterator MaterialIterator;
typedef AGDDElementMap::const_iterator ElementIterator;

class AGDDMaterialStore {
public:
	AGDDMaterialStore();
	void RegisterElement(AGDDElement *);
	void RegisterMaterial(AGDDSimpleMaterial *);
	AGDDSimpleMaterial* GetMaterial(std::string_view);
	AGDDElement* GetElement(std::string_view);
		
	int NumberOfMaterials() {return m_nrOfMaterials;}
	int NumberOfElements()  {return m_nrOfElements;}
	
	MaterialIterator MaterialBegin() {return m_theMaterials.begin();}
	MaterialIterator MaterialEnd() {return m_theMaterials.end();}
	ElementIterator ElementBegin() {return m_theElements.begin();}
	ElementIterator ElementEnd() {return m_theElements.end();}
	
	
	void PrintElementNames();
	void PrintMaterialNames();
	void PrintElement(const std::string& n);
	void PrintMaterial(const std::string& n);
	
private:
	AGDDMaterialMap m_theMaterials;
	AGDDElementMap m_theElements;
	
	int m_nrOfMaterials;
	int m_nrOfElements;

};

#endif

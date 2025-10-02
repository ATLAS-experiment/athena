/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "Identifier/IdentifierDict.h"
#include "Identifier/IdHelper.h"
#include "AthContainers/DataVector.h"
#include <map>


struct DetDescrDictionaryDict
{
    DataVector<Identifier>                    idvec;
    std::vector<Identifier>                   idvec1;
    std::vector<HWIdentifier>                 idvec2;
    std::vector<HWIdentifier>::iterator       it1;
    std::vector<HWIdentifier>::const_iterator cit1;
    std::map<Identifier,int>                  idmapInt;
    std::map<HWIdentifier,float>              idmapFloat;
    std::pair<Identifier,int>                 idpairInt;
    std::pair<HWIdentifier,float>             idpairFloat;
    std::pair < IdentifierHash, int >         idhpairInt;
    DataVector<std::pair<IdentifierHash, int> >    dvidhpairInt;
    std::vector<std::pair<IdentifierHash, int>* >  vidhpairInt;
};

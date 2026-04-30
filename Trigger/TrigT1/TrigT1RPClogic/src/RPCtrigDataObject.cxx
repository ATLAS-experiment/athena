/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <iomanip>
#include "TrigT1RPClogic/RPCtrigDataObject.h"

using namespace std;

RPCtrigDataObject::RPCtrigDataObject(int num,const std::string& name) : 
    BaseObject(Data,name),m_number(num) {}

RPCtrigDataObject::RPCtrigDataObject(int num,const char* name) : 
    BaseObject(Data,name),m_number(num) {}


void 
RPCtrigDataObject::Print(ostream& stream,bool detail) const
{
    detail = true;
    if(detail)
    {
        stream << name() << " number " << setw(3) << number();
    }
    stream << endl;
}

void
RPCtrigDataObject::set_number(int number)
{
    m_number = number;
}

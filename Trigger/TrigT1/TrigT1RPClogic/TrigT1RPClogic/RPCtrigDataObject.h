/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DATAOBJECT_H
#define DATAOBJECT_H

#include <sstream>
#include <iosfwd>
#include <string>

#include "MuonCablingTools/BaseObject.h"


class RPCtrigDataObject : public BaseObject
{
    private:
    int m_number{};

    public:
    RPCtrigDataObject (int,const std::string&);
    RPCtrigDataObject (int,const char*);
    RPCtrigDataObject(const RPCtrigDataObject&) = default;
    RPCtrigDataObject& operator=(const RPCtrigDataObject&) = default;
    RPCtrigDataObject (RPCtrigDataObject&&) noexcept = default;
    ~RPCtrigDataObject() = default;

    void set_number (int);

    int number()         const {return m_number;}

    void PrintElement(std::ostream&,std::string,bool) const {}

    void Print(std::ostream&,bool) const;
};


template <class X> X& operator<<(X& stream,const RPCtrigDataObject& data)
{
    std::ostringstream display;
    data.Print(display,false);
    stream << display.str();
    return stream;
}

#endif

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///@author r.d.schaffer

#ifndef IDENTIFIER_IDENTIFIABLE_H
# define IDENTIFIER_IDENTIFIABLE_H

class Identifier;
class IdentifierHash;
class IdHelper;

/**
 * @class Identifiable 
 * @brief This class provides an abstract interface to an Identifiable object.
 * 
 * It is "identifiable" in the sense that each object must have
 * an identify method returning an Identifier.
 
 * The interface also is extended to also provide access to a "hash"
 * form of an identifier. And there is the possiblity to add a
 * conversion strategy to allow conversion from Identifier <->
 * IdentifierHash.
 **/

class Identifiable {
public:
    
    virtual ~Identifiable() = default;
    
    virtual Identifier identify() const = 0;

    virtual IdentifierHash identifyHash() const;

    virtual const IdHelper* getHelper() const;
};

#endif // IDENTIFIER_IDENTIFIABLE_H

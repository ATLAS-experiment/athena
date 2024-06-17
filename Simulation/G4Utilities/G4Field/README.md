# G4Field
Author Andrea Dell'Acqua (dellacqu@mail.cern.ch)
Converted from packagedoc.h

## Introduction

This package provides a simple interface to G4 Fields and for modifying the magnetic field to be used during simulation.

## Class Overview

The package includes the following classes:
 - AtlasField , H8Field , H8FieldSym , ConstantField : Various base classes for fields which can be used during simulation (including, of course, test beam simulation)
 - ConstantFieldHandler : An interface for constructing constant fields
 - ConstantFieldXMLHandler : An interface for constructing constant fields from an XML file
 - FieldMapMgr : The manager that handles the fields which will be used during the simulation

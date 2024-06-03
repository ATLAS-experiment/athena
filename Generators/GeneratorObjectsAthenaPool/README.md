# GeneratorObjectsAthenaPool Package

The package which defines POOL converters for general Monte-Carlo classes.

Authors: Sebastien Binet, Georgios Stavropoulos, Davide Costanzo

## Introduction

The MonteCarlo classes (e.g. @c HepMC::GenEvent ) are stored on disk through the so-called POOL persistification technology which insulates client code (Athena) from technology details and provides transparent switch of different storage backends (ROOT, MySQL,...)

This package provides the needed boilerplate code to use the T/P converters provided by the GeneratorObjectsTPCnv package (see twiki page about Transient/Persistent separation: https://twiki.cern.ch/twiki/bin/view/Atlas/TransientPersistentSeparation) and make them available to the GAUDI framework.

## See also

You may also be interested in having a look at the GeneratorObjectsTPCnv package which defines and provides various T/P converters for the MonteCarlo classes.

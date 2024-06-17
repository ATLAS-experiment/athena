# TruthJiveXML
Author Nikos.Konstantinidis -at- cern.ch, Juergen.Thomas -at- cern.ch, Sebastian.Boeser -at- cern.ch
Converted from packagedoc.h

## Introduction

This package contains the  AthAlgTools that retrieve truth information ( McEventCollection and  TrackRecordCollection ) from  StoreGate and 
forward them to the formatting tool. Each tool implements the IDataRetriever interface, through which it is called from the @c AlgoJiveXML.

## Retrievers

- JiveXML::TruthTrackRetriever @copydoc JiveXML::TruthTrackRetriever

- JiveXML::TruthMuonTrackRetriever @copydoc JiveXML::TruthMuonTrackRetriever

- JiveXML::TruthCosmicsTrackRetriever @copydoc JiveXML::TruthCosmicsTrackRetriever


# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

__author__ = "Marcin Nowak"
__doc__ = """
Find out what type of ROOT APR Collections is stored in a given file.
Return a string "RNTCollection" or "RootCollection" or None
"""

def getCollectionType( fileName ):
   """Returns APR Collection type (Root/RNT) that is stored in the given file (or None)"""
   
   import ROOT
   with ROOT.TFile.Open( fileName ) as file:
       ttree = file.Get( ROOT.APRDefaults.TTreeNames.EventTag )
       if ttree:
           return "RootCollection"

       # MN: once the namespaces are stable, maybe switch to direct RNTuple open
       # rntuple = ROOT.RNTupleReader.Open( ROOT.APRDefaults.RNTupleNames.EventTag, fileName )
       rntuple = file.Get( ROOT.APRDefaults.RNTupleNames.EventTag )
       if( rntuple ):
           return "RNTCollection"

   return None

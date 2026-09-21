#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# take a chain string configuration and decode it into the constituent parts,  
# so that it can be reconstructed without the tags. 

# This is an approximate python implementation of the equivalent c++ 
# class and wouldn't be needed if the  setPath() method on the GenericMonitoringTool
# worked correctly, since the decoding is done in the c++, and the path could then 
# be set correctly at that point. sadly, this doesn;t not seem to work, so we need 
# to decode consistently in the c++ and the python to make sure that things 
# are consistent

class chainString: 

  def __init__(self, input ):
    self.head    = ""
    self.tail    = ""
    self.roi     = ""
    self.vtx     = ""
    self.element = ""
    self.extra   = ""
    self.passed  = ""

    if ":" in input : parts = input.split( ":" )
    else            : parts = [ input ]

    if len(parts) == 1 : self.tail = parts[0]
    else:
        
      self.head  = parts[0]
        
      for part in parts[1:]: 
        if part.endswith(";DTE"):
          self.passed = True
          part = part[0:-4]
        if part.startswith("key=")   : self.tail = part[4:]
        if part.startswith("roi=")   : self.roi  = part[4:]
        if part.startswith("vtx=")   : self.vtx  = part[4:]
        if part.startswith("te=")    : self.element = part[3:]
        if part.startswith("extra=") : self.extra   = part[6:]

      if len(parts)>1 and self.tail==""    : self.tail = parts[1]
      if len(parts)>2 and self.roi==""     : self.roi  = parts[2]
      if len(parts)>3 and self.vtx==""     : self.vtx  = parts[3]
      if len(parts)>4 and self.element=="" : self.element = parts[4]
      if len(parts)>5 and self.extra==""   : self.extra   = parts[5]
            
    stuff = [ self.roi, self.vtx, self.element, self.extra ]

    sum = self.head

    if  self.tail != "" :
      if  sum == "" : sum = self.tail
      else          : sum += "_" + self.tail 

    for part in stuff:
        if part != "":
            sum += "_"+part

    if self.passed: sum += "_DTE"

    self.sum = sum

  def __str__(self) :
    return self.summary()
    
  # provide the summary    
  def summary( self ):
      return self.sum

  # printout if needed
  def printchain( self ):
      print( "  head:   ", self.head   )
      print( "  tail:   ", self.tail   )
      print( "  vtx:    ", self.vtx    )
      print( "  roi:    ", self.roi    )
      print( "  te:     ", self.element)
      print( "  extra:  ", self.extra  )
      print( "  passed: ", self.passed )
      print( "  sum:    ", self.sum    )


# provide the summary without needing the 
# intermediate class instance

def summarise( input ) :
    return chainString( input ).summary()


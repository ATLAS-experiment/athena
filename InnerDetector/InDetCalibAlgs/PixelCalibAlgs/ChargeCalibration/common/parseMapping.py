#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys
import re

def parsePosition(header_line):
  #negative signs can appear on the second and last numbers
  coords=re.match(r'^\s*{\s*(\d+),\s*(-?\d+),\s*(\d+),\s*(\d+),\s*(-?\d+)\s*},?', header_line)
  if coords is None:
    return None
  return [int(x) for x in coords.groups()]

#new format is a list with hash and position, [2027, 2, 2, 39, 0]
def convertToNewFormat(oldFormat):
  hash_positionStrings=oldFormat.split()[1:]
  theHash = int(hash_positionStrings[0])
  thePositions = [int(x) for x in hash_positionStrings[1].split(',')]
  result = [theHash]
  return result + thePositions


#old format is a single string, ' : 2027 2,2,39,0' 
#note: no comma between hash and position coordinates, the hash is r-justified
def convertToOldFormat(newFormat):
  newFormatPosition = str(newFormat[1:])
  joinedPosition = "".join(newFormatPosition)[1:-1]
  spaceStripped = re.sub(r'[\s+]', '', joinedPosition)
  newFormatHash = str(newFormat[0])
  return " : "+newFormatHash.rjust(4)+" "+spaceStripped

  
def create_mapping0(fname): 
  golden_modules = []
  module_file = open(fname, 'r')
  module_data = module_file.read()
  module_lines = module_data.splitlines()

  for modules in module_lines:
    modulename = ""
    modules = modules.split()
    if len(modules) != 0:
      if modules[0] == "else" and modules[1] == "if":
        geographicalID = modules[2].split('"')
        modulename = geographicalID[1]
      elif modules[0] == "if":
        geographicalID = modules[1].split('"')
        modulename = geographicalID[1]
    if modulename:
      golden_modules.append(modulename)

  golden_hash = []
  for modules in module_lines:
    modulehash = ""
    goldens = modules.split('=')
    modules = modules.split()
    if len(modules) != 0:
      if modules[0] == "else" and modules[1] == "if":
        golden_hashID = " : " + goldens[3].split(';')[0]
        golden_bec = goldens[4].split(';')[0].strip()
        golden_layer = goldens[5].split(';')[0].strip()
        golden_phi = goldens[6].split(';')[0].strip()
        golden_eta = goldens[7].split(';')[0].strip()
        modulehash = golden_hashID + " " + golden_bec + "," + golden_layer + "," + golden_phi + "," + golden_eta
      elif modules[0] == "if":
        golden_hashIds = modules[4].split(';')[0]
        golden_bec = modules[5].split('=')[1].split(';')[0]
        golden_layer = modules[6].split('=')[1].split(';')[0]
        golden_phi = modules[8].split(';')[0]
        golden_eta = modules[10].split(';')[0].strip()
        if len(golden_hashIds) == 1:
          golden_hashID = " :    " + golden_hashIds
        elif len(golden_hashIds) == 2:
          golden_hashID = " :   " + golden_hashIds
        elif len(golden_hashIds) == 3:
          golden_hashID = " :  " +  golden_hashIds
        elif len(golden_hashIds) == 4:
          golden_hashID = " : " +  golden_hashIds
        modulehash = golden_hashID + " " + golden_bec + "," + golden_layer + "," + golden_phi + "," + golden_eta
    if modulehash:
      golden_hash.append(modulehash)
  return dict(zip(golden_modules, golden_hash))



#create new mapping from c++ header file of format March 2013
#Two arrays in the file, first one is array of strings
#Second one is array of the struct 'positions'
def create_mapping1(fname):
#The following is truly horrible; it parses the .h file to read module names.
  module_names=[]
  module_file = open(fname, 'r')
  module_data = module_file.read()
  module_lines = module_data.splitlines()
  found_names=False
  for modules in module_lines:
    modulename = ""
    modules = modules.split()
    if len(modules) != 0:
      if found_names:
        if "};" in modules: #end of list in code
          break
        geographicalID = modules[0].split('"')
        modulename = geographicalID[1]
        module_names.append(modulename)
      if "names" in modules: #beginning of list in code
        found_names=True
  #
  found_positions = False
  module_positions=[]
  for position_line in module_lines:
    if len(position_line) != 0:
      if found_positions:
        if "}};" in position_line: #end of list in code
          break
        position = parsePosition(position_line)
        if position is None:
          print('No match')
        module_positions.append(position)
      if "values" in position_line: #beginning of list in code
        found_positions=True
  return dict(zip(module_names,module_positions))

def create_mapping(fname):
  result=dict()
  module_file = open(fname, 'r')
  module_data = module_file.read()
  module_lines = module_data.splitlines()
  for module in module_lines:
    if '#' in module:
      continue
    components = module.split(',')
    thisModuleName = components[0]
    thisModulePosition = [int(x) for x in components[1:]]
    result[thisModuleName] = thisModulePosition
  return result
        
        
if __name__ == "__main__":
  #pre-2023 format of the header, which was in 'pixels' and 'lbl' directories
  #name_position_dict0 = create_mapping0('oldPixelMapping.h')
  name_position_dict1 = create_mapping1('pixelMapping.h')
  name_position_dict =  create_mapping('mapping.csv')
  #print(name_position_dict)
  print ("#name, hash, bec, layer, phi, eta")
  for n in name_position_dict1:
    v =  convertToOldFormat(name_position_dict[n])
    #left as an example
    #v1 = convertToOldFormat(name_position_dict1[n])
    v0 = convertToOldFormat(name_position_dict1[n])
    if v!=v0:
      print (v0,v)
    
  '''
  Examples of extraction to lists
  ===============================
  myList = [*name_position_dict]
  print(myList)
  names=[]
  hashes=[]
  for (k,v) in name_position_dict.items():
    names.append(k)
    hashes.append(v[0])
  print(names)
  print(hashes)
  '''
  sys.exit(0)

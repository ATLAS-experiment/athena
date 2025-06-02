# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Author: Lukas Kretschmann (lukas.kretschmann@cern.ch)

# import modules
import os
import subprocess

# import Athena modules
from AthenaCommon import Logging
athMsgLog = Logging.logging.getLogger('Herwig7JOChecker')

# Helper function to get the current Herwig version
def herwig_version():
  herwig7_bin_path   = os.path.join(os.environ['HERWIG7_PATH'],'bin')
  herwig7_binary     = os.path.join(herwig7_bin_path,'Herwig')
  versions = subprocess.check_output([herwig7_binary,'--version']).splitlines()
  return(' '.join([x.decode('UTF8') for x in versions[0].split()[1:]]))

# Helper function to remove the command prefixes from the line
def clean_string(list,string,replace="",strip=False):
  for command in range(len(list)):
    list[command] = list[command].replace(string,replace)
    if strip:
      list[command] = list[command].strip()
  # return the cleaned list
  return list

# Helper functions to print the commands
def print_commands(list):
  for command in list:
    athMsgLog.debug(command)

# Helper function to split command lines into command and value
def split_commands(list):
  for command in range(len(list)):
    # Search for whitespaces, since the part before the whitespace gives the parameter which is set
    # the part after the whitespace gives the value which is set
    whitespace_idx = list[command].index(" ")
    # Now split the commands into the command and value set
    list[command] = [list[command][0:whitespace_idx],list[command][whitespace_idx+1:]]
  # return the new list
  return list

# Helper function to check for duplicate commands
def check_for_overwrites(list,file):
  found_duplicate = False
  existing_commands = {}
  for command in list:
    temp_com = command[0]
    temp_val = command[1]
    # Check if the same command is already present with another value
    if temp_com in existing_commands and existing_commands[temp_com] != temp_val:
      found_duplicate = True
      str1 = "\nFound option that is used twice"
      str2 = "The option %s is first set to %s" % (temp_com,existing_commands[temp_com])
      str3 = "And then the option %s ist set to %s" % (command[0],command[1])
      athMsgLog.info(str1+"\n"+str2+"\n"+str3)
      file.write(str1+"\n"+str2+"\n"+str3)
    else:
      existing_commands[temp_com] = temp_val
  # return the status
  return found_duplicate

# Helper function to write commands to file
def write_to_file(list,file):
  for command in list:
    file.write(command[0]+" "+command[1]+"\n")

# Helper function to sort the commands into the lists
def sort_commands(line,cd_in_line,cd_command,set_commands,read_commands,create_commands,insert_commands):
  # If the snippets cd's into a dir, we need to append the path to the other commands
  # To be able to compare to the settings from the master .in file
  if "cd" in line:
    cd_command = line.replace("cd ","")
    cd_command = cd_command.strip()
    cd_in_line = True
  # Get all the "set" commands and ignore lines that are commented out
  elif "set" in line:
    if cd_in_line and not line.startswith("/Herwig/"):
      set_commands.append(cd_command+"/"+line)
    else:
      set_commands.append(line)
  # Get all the "create" commands where an object is created
  elif "create" in line:
    if cd_in_line and not line.startswith("/Herwig/"):
      create_commands.append(cd_command+"/"+line)
    else:
      create_commands.append(line)
  # Get all the "insert" commands where an object is inserted
  elif "insert" in line:
    if cd_in_line and not line.startswith("/Herwig/"):
      insert_commands.append(cd_command+"/"+line)
    else:
      insert_commands.append(line)
  # Get all the "read" commands where a config snippet is read in
  elif "read" in line:
    read_commands.append(line)

  # return the cd status
  return cd_in_line,cd_command,set_commands,read_commands,create_commands,insert_commands

# Main function to check the Herwig config file 
def check_file():
  # Hello from the config checker
  athMsgLog.info("Hello from the config-checker!")
  # Check the Herwig version
  h_version = herwig_version()
  version = h_version[0]
  subversion = h_version[2]
  subsubversion = h_version[4]
  athMsgLog.info("The current Herwig version is "+version+"."+subversion+"."+subsubversion)

  # Get the name of ther Herwig7 in-file
  files = [f for f in os.listdir('.') if os.path.isfile(f)]
  HerwigINFile = []
  for file in files:
    if ".in" in file:
      HerwigINFile.append(file)

  # Check if there is only one files, as it should be
  run_name = HerwigINFile[0]
  athMsgLog.info("The Herwig7 *.in file is "+run_name)

  # Now get the settings from the .in file
  set_commands    = [] # all commands where a setting is set
  read_commands   = [] # all commands where a file is read in
  create_commands = [] # all commands where an object is created
  insert_commands = [] # all commands where an object is inserted

  f = open(run_name, "r")
  cd_in_line = False
  cd_command = ""
  for line in f:
    if not line.startswith("#"):
      cd,cd_cmd,set_commands,read_commands,create_commands,insert_commands = sort_commands(line,
         cd_in_line,cd_command,set_commands,
         read_commands,create_commands,insert_commands)
      cd_in_line,cd_command = cd,cd_cmd
    else:
      continue

  # remove the "set", "create" and "read" parts as well as the snippet prefix
  set_commands    = clean_string(set_commands,"set ","")
  create_commands = clean_string(create_commands,"create ","")
  read_commands   = clean_string(read_commands,"read ","")
  read_commands   = clean_string(read_commands,"snippets/","")

  # Now recursiveley read in the config snippets from the read commands 
  include_path = os.environ['HERWIG7_PATH']+"/share/Herwig/"
  snippet_path = os.environ['HERWIG7_PATH']+"/share/Herwig/snippets/"

  include_files = [f for f in os.listdir(include_path)]
  snippet_files = [f for f in os.listdir(snippet_path)]

  # Files that we need to read in, since they are read in in the original config
  read_files = []
  for file in include_files:
    for command in read_commands:
      if file in command:
        read_files.append(include_path+file)
  for file in snippet_files:
    for command in read_commands:
      if file in command:
        read_files.append(snippet_path+file)

  # Now loop over the snippets
  for file in read_files:
    if ".in" not in file:
      athMsgLog.info("Skipping file " + file + " since it does not contain any in-file")
      athMsgLog.info("It is just a directory, can be ignored.")
      continue
    else:
      f = open(file, "r")
      cd_in_line = False
      cd_command = ""
      for line in f:
        if not line.startswith("#"):
          cd,cd_cmd,set_commands,read_commands,create_commands,insert_commands = sort_commands(line,cd_in_line,cd_command,set_commands,read_commands,create_commands,insert_commands)
          cd_in_line,cd_command = cd,cd_cmd
        else:
          continue
      f.close()

  # remove the "set", "create" and "read" parts as well as the snippet prefix
  set_commands    = clean_string(set_commands,"set ","")
  create_commands = clean_string(create_commands,"create ","")
  read_commands   = clean_string(read_commands,"read ","")
  read_commands   = clean_string(read_commands,"snippets/","")

  # Remove "//" and remove line breaks
  clean_string(set_commands,"//","/",True)
  clean_string(create_commands,"//","/",True)
  clean_string(insert_commands,"//","/",True)

  # Print the commands
  athMsgLog.debug("\n These are the commands found by the Herwig7JOChecker:\n")
  print_commands(set_commands)
  print_commands(create_commands)
  print_commands(insert_commands)
  athMsgLog.debug("\n")

  # Now check if there are commands executed twice 
  # It can happen that there are commands set by the user but some config snippet from Herwig sets these commands again 
  # In this case a warning needs to be displayed, this should be checked by the user
  set_commands    = split_commands(set_commands)
  create_commands = split_commands(create_commands)
  insert_commands = split_commands(insert_commands)
  
  # Now we need to check of commands are executed twice 
  with open("HerwigCommandDuplicates.txt","w") as file:
    found_duplicate1 = check_for_overwrites(set_commands,file)
    found_duplicate2 = check_for_overwrites(create_commands,file)
    found_duplicate3 = check_for_overwrites(insert_commands,file)

  # Now save all the commands to an file
  # This file will be used by JEM/PAVER for the technical validation of generator changes
  # The files from refrence sample will be checked against the new one to spot config changes
  with open("HerwigCommands.txt","w") as file:
    write_to_file(set_commands,file)
    write_to_file(create_commands,file)
    write_to_file(insert_commands,file)

  # If there was an error, raise an exception
  if found_duplicate1 or found_duplicate2 or found_duplicate3:
    athMsgLog.warn("There were some settings which are overwritten, please check the log-file HerwigCommandDuplicates.txt")

  # Godbye from the config checker
  athMsgLog.info("Commands have been written to HerwigCommands.txt")
  athMsgLog.info("Duplicates have been written to HerwigCommandsDuplicates.txt")
  athMsgLog.info("Godybe from the config-checker!")


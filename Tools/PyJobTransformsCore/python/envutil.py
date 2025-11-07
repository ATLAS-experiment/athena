# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import os, re, glob
from PyJobTransformsCore import fileutil

__doc__ = """Environment variables utilities"""

#some names of environment variables to avoid typing mistakes
LD_LIBRARY_PATH = 'LD_LIBRARY_PATH'

# possible wildcards used in unix shell filename completion (and in module glob)
filenameWildCards = r'\?|\*|\[.*\]'
filenameWildCardsCompiled = re.compile(filenameWildCards)

# split paths according to different separators
defaultPathSeps = os.pathsep + '|,'

# regular expression with allowed endings for shared library names
_libraryNameRE = re.compile(r'\.so(\.[0-9]+)*$')


def has_wildcards(filename):
    """Return boolean indicating if the filename contains any unix shell filename wildcards"""
    if filenameWildCardsCompiled.search(filename):
        return True
    else:
        return False


def find_file_split( filename,
                     dirlist = [ os.getcwd() ],  # noqa: B008 (getcwd always returns the same)
                     access = os.R_OK,
                     depth = 0 ):
    """Search for file <filename> with access rights <access> (see os.access()) in directory list <dirlist>.
    Search into directory tree of each directory in <dirlist> up to depth <depth>. The default directory
    list is a list containing only the current working directory.
    No wildcards are allowed in <filename>.
    <depth> = 0 : only search in directories given in list.
    <depth> > 0 : descend deeper into the directory tree up to max <depth> levels.
    <depth> < 0 : ascend upwards into the directory tree up to max -<depth> levels.
    It returns 2-tuple (dir,file) where:
       file=<filename> if no wildcards, or the actual (local) match to <filename> if wildcarded.
       dir=the directory where <file> was found (from <dirlist>, or from a subdir if depth > 0)
    If no file is found, it returns None."""
    if not dirlist: return None
    for dir in dirlist:
        dir = os.path.abspath( os.path.expandvars( os.path.expanduser(dir) ) )
        if not os.path.isdir(dir): continue
        fullfile = os.path.join( dir, filename )
        #        print ("Checking file %s..." % fullfile)
        if fileutil.access( fullfile, access ):
            return (dir,filename)

    if depth == 0:
        # not found at all
        return None
    elif depth > 0:
        # not found at this level. Go one level down in directory structure
        for dir in dirlist:
            dir = os.path.abspath( os.path.expandvars( os.path.expanduser(dir) ) )
            if not os.path.isdir(dir): continue
            subdirlist = []
            for d in fileutil.listdir(dir):
                fulldir = os.path.join(dir,d)
                if os.path.isdir(fulldir):
                    subdirlist.append( fulldir )
            if subdirlist:
                found = find_file_split( filename, subdirlist, access, depth - 1 )
                if found: return found
    elif depth < 0:
        # not found at this level. Go one level up in directory structure
        for dir in dirlist:
            dir = os.path.abspath( os.path.expandvars( os.path.expanduser(dir) ) )
            if not os.path.isdir(dir): continue
            updir = os.path.dirname(dir)
            if updir != dir:
                found = find_file_split( filename, [ updir ], access, depth + 1 )
                if found: return found
                
        
    # not found at all
    return None


def find_file( filename,
               dirlist = [ os.getcwd() ],  # noqa: B008 (getcwd always returns the same)
               access = os.R_OK,
               depth = 0 ):
    """Search for file <filename> with access rights <access> (see os.access()) in directory list <dirlist>,
    Search into directory tree of each directory up to depth <depth>. The default directory list is
    a list containing only the current working directory.
    <depth> = 0 : only search in directories given in list.
    <depth> > 0 : descend deeper into the directory tree up to max <depth> levels.
    <depth> < 0 : ascend upwards into the directory tree up to max -<depth> levels."""
    found = find_file_split( filename, dirlist, access, depth )
    return found and os.path.join( found[0], found[1] )


def find_files_split( filename, dirlist, access, depth ):
    """Search for all (regular) files that match <filename> with access rights <access> (see os.access())
    in directory list <dirlist>.
    Search is done into subdirectories each directory up to depth <depth>.
    The default value for <dirlist> is the current working directory.
    If the same file (without the directory name) is found in more than one places, only the first match is kept.
    <filename> : can contain wildcards as used on the unix command line.
    <depth> = 0 : only search in directories given in list.
    <depth> < 0 : treated as = 0
    <depth> > 0 : descend deeper into the directory tree up to max <depth> levels.
    It returns a list of 2-tuples (dir,file) where
       file=<filename> if no wildcards, or the actual (local) match to <filename> if wildcarded.
       dir=the directory where <file> was found (from <dirlist>, or from a subdir if depth > 0)
    If none is found, an empty list is returned."""
#    if dirlist is None:
#        return []
    if depth < 0: depth = 0
    # to speed up search, do a single file search if the filename does not contain wildcards
    if not has_wildcards(filename):
        singleFile = find_file_split( filename, dirlist, access, depth )
        if singleFile:
            return [ singleFile ]
        else:
            return []
    # filename has wildcards. Find all (first) files that match.
    filenameList = [] # the list of files to return
    dirnameList = [] # keep track of files already found
    for dir in dirlist:
        dir = os.path.abspath( os.path.expandvars( os.path.expanduser(dir) ) )
        # protect against non-existing entries
        if not os.path.isdir(dir): continue
        olddir = os.getcwd()
        os.chdir(dir)
##        print ("Checking files %s..." % fullfile)
        filelist = glob.glob(filename)
        for f in filelist:
##            print ("Trying %s..." % f)
            if not os.path.isfile(f) or not fileutil.access(f, access): continue
            if f not in filenameList:
##                print ("==> Adding %s to list from %s" % (f,dir))
                dirnameList.append(dir)
                filenameList.append(f)
            else:
                pass
##                print ("==> Already have %s in list" % (base))
        os.chdir(olddir)
    if depth > 0:
        # Go one level down in directory structure
        for dir in dirlist:
            dir = os.path.abspath( os.path.expandvars( os.path.expanduser(dir) ) )
            # protect against non-existing entries
            if not os.path.isdir(dir): continue
            subdirlist = []
            for d in fileutil.listdir(dir):
                fulldir = os.path.join(dir,d)
                if os.path.isdir(fulldir):
                    subdirlist.append( fulldir )
            if subdirlist:
                filelist = find_files_split( filename, subdirlist, access, depth - 1 )
                for f in filelist:
                    if not f[1] in filenameList:
                        dirnameList.append(f[0])
                        filenameList.append(f[1])
    return map( lambda arg1,arg2 : (arg1,arg2) , dirnameList, filenameList )


def find_files( filename, dirlist, access, depth ):
    """Search for all (regular) files that match <filename> with access rights <access> (see os.access()) in directory list <dirlist>.
    Search is done into subdirectories each directory up to depth <depth>.
    The default value for <dirlist> is the current working directory.
    If the same file (without the directory name) is found in more than one places, only the first match is kept.
    <filename> : can contain wildcards as used on the unix command line.
    <depth> = 0 : only search in directories given in list.
    <depth> < 0 : treated as = 0
    <depth> > 0 : descend deeper into the directory tree up to max <depth> levels.
    It returns a list of filenames with full pathnames. If none is found, an empty list is returned."""
    return list (map( lambda arg : os.path.join( arg[0], arg[1] ), find_files_split( filename, dirlist, access, depth ) ))


def find_file_env( filename, env_var_name, access = os.R_OK, sep = defaultPathSeps, depth = 0 ):
    """Search for file <filename> with access rights <access> (see os.access()) in directory list
    given as a <sep> separated list of paths in environment variable <env_var_name>.
    Search into directory tree of each directory up to depth <depth> (0=don't descend at all)."""
    env = os.environ.get( env_var_name )
    envList = [ os.getcwd() ]
    if not env:
        return envList
    envList.extend( re.split( sep, env ) )
    return find_file( filename, envList, access, depth )


def find_files_env( filename, env_var_name, access = os.R_OK, sep = defaultPathSeps, depth = 0 ):
    """Search for all files that match <filename> with access rights <access> (see os.access()) in directory list
    given as a <sep> (a regular expression) separated list of paths in environment variable <env_var_name>.
    <filename> can contain wildcards as used on the unix command line.
    Search into directory tree of each directory up to depth <depth> (0=don't descend at all)."""
    env = os.environ.get( env_var_name )
    envList = [ os.getcwd() ]
    if not env:
        return envList
    envList.extend( re.split( sep, env ) )
    return find_files( filename, envList, access, depth )


def find_libraries( lib ):
    """Search for libraries in LD_LIBRARY_PATH. Return list of full paths of libraries if found.
    <lib> can contain wildcards, in which case all files matching the wildcard will be returned.
    If the same file appears in several paths, the first one found will be taken."""
    # require extension .so (or .so with version numbers)
    libsfull = []
    libname = lib
    if _libraryNameRE.search(lib): # fully specified ending
        libsfull = find_files_env( libname, LD_LIBRARY_PATH )
    else:
        libsfull = find_files_env( libname, LD_LIBRARY_PATH )
        # filter results for valid shared library ending (basically to get rid of *.cmtref)
        libsfull = [ l for l in libsfull if _libraryNameRE.search(l) ]
        if not libsfull:
            # add generic ending
            libname = lib + '.so*'
            libsfull = find_files_env( libname, LD_LIBRARY_PATH )
        # filter results for valid shared library ending (basically to get rid of *.cmtref)
        libsfull = [ l for l in libsfull if _libraryNameRE.search(l) ]
        
    # if still not found anything, try with prefix 'lib'
    if not libsfull and not lib.startswith('lib'):
        libsfull = find_libraries( 'lib' + lib )

    return libsfull

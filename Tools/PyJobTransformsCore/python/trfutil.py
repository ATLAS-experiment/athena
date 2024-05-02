# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import os, shutil

from .envutil import find_files_env


def linkPresent( filename, findBrokenLink = True ):
    """Check if the given filename path contains a symlink. This function also checks all it's parent directories. 
    If findBrokenLink is True, the first path component that is a symlink and is broken is returned. 
    If findBrokenLink is False, the first symlink in the path in returned."""
    if os.path.isabs( filename ):
        initial_p = ""
        p = os.path.commonprefix( [ os.getcwd(), filename ] )
        filename = filename[ len( p ) + 1: ] # name filename a relative path wrt p.
    else:
        p = os.getcwd()
        initial_p = p + os.sep
    intermediateList = filename.split( os.sep )
    while intermediateList:
        p = os.sep.join( [ p, intermediateList.pop(0) ] )
        if os.path.islink( p ):
            if findBrokenLink and os.path.exists( p ):
                continue
            return p.lstrip( initial_p )
    return ''

def isOnLocalFileSystem( filename ):
    """Check in the filename at every directory level to determine if any of its parent directories have different mount points."""
    filename = os.path.realpath( filename )
    cPrefix = os.path.commonprefix( [ os.getcwd(), filename ] )
    intermediateDirList = os.path.dirname( filename[ len( cPrefix ): ] ).split( os.sep )
    cPrefix = cPrefix.rstrip( os.sep )
    while intermediateDirList:
        cPrefix = os.sep.join( [ cPrefix, intermediateDirList.pop(0) ] )
        if os.path.ismount( cPrefix ):
            return False
    return True

def get_files( listOfFiles, fromWhere='data', doCopy='ifNotLocal', errorIfNotFound=True, keepDir=True, depth=0, sep=os.pathsep ):
    """Copy or symlink a list of files given from a search path given in an environment variable.
    <listOfFiles> is either as a python list of strings, or as a comma separated list in one string.
                  Each entry in the list can contain wildcards (a la unix commands)
    <fromWhere>: name of the environment variable containing the search paths. Available shortcuts: 'data' -> 'DATAPATH'.
    <doCopy>='Always': copy the file
            ='Never' : symlink the file
            ='ifNotLocal': symlink the file if the original is in a subdirectory of the run directory, otherwise copy.
    <errorIfNotFound>=True: An EnvironmentError exception is raised if any filename in the list is not found.
                            If the filename contains wildcards, it is considered not found if no files are found at all.
                     =False: If a file is not found, only a warning is printed
    <keepDir>=True : copy the file into a local subdirectory if the requested filename includes its directory part
              False: copy the file in the current directory with the requested filename while ignoring its directory part
              If a filename in <listOfFiles> contains an absolute path, the directory part is ignored in the copied filename.
    """
    if doCopy not in [ 'Always', 'Never', 'ifNotLocal' ]:
        print ("doCopy value of %s not recognised. Resetting it to 'ifNotLocal'" % doCopy)
        doCopy = 'ifNotLocal'
    fromWhereShorts = { 'data' : 'DATAPATH' }
    # split a comma separated list of files given in a string into a python list
    if isinstance( listOfFiles, str ):
        listOfFiles = listOfFiles.split(',')
    # origin of files
    fromPath = fromWhereShorts.get( fromWhere, fromWhere )
    # copy or symlink
    if doCopy == 'Always':
        copy_func = shutil.copyfile
    elif doCopy == 'Never':
        copy_func = os.symlink
    else: # doCopy == ifNotLocal
        copy_func = None
    fileDict = {}
    # Check if srcFileList contains entries from different directories
    realDirList = []
    symLinkList = []
    # First iteration of listOfFiles to create realDirList.
    # Store created lists for subsequent iteration without having to recreate the lists again.
    for filename in listOfFiles:
        filename = os.path.expanduser( os.path.expandvars( os.path.normpath( filename ) ) )
        srcFileList = find_files_env( filename, fromPath, sep=sep, depth=depth )
        fileDict[ filename ] = srcFileList
        if os.path.isabs( filename ):
            continue
        srcFilePrefix = os.path.dirname( os.path.commonprefix( srcFileList ) ) #.rstrip( os.sep )
        dirName = os.path.dirname( filename )
        # Test if dirName is present at the right end of srcFilePrefix. 
        # If this is not the case, this dirName should not be symlinked but a physical dirName s
        # hould be created instead (later).
        try:
            realDirList.append( symLinkList.pop( symLinkList.index( dirName ) ) )
        except ValueError:
            pass
        else:
            continue
        print ("srcFilePrefix = %s, dirName = %s" % ( srcFilePrefix, dirName ))
        try:
            srcFilePrefix.rindex( dirName, len( srcFilePrefix ) - len( dirName ) )
        except ValueError:
            realDirList.append( dirName )
        else:
            symLinkList.append( dirName )
    del symLinkList
    # Main iteration
    for filename, srcFileList in fileDict.items():
        if not srcFileList:
            if errorIfNotFound:
                raise EnvironmentError('Auxiliary file %s not found in %s' % (filename,fromPath) )
            else:
                print ("WARNING: auxiliary file %s not found in %s" % (filename,fromPath))
        parentDirLinked = None
        for srcFile in srcFileList:
            # Test if the parent directory of the current entry in srcFileList has been symlinked. 
            # If so, skip this entry. Assumes that the find_files_env() returns a sorted list of files.
            if os.path.dirname( srcFile ) == parentDirLinked:
                print ("%s has been symlinked." % parentDirLinked)
                continue
            # determine name of destination file
            if keepDir and not os.path.isabs( filename ):
                subdir = os.path.dirname( filename )      # e.g. subdir = geomDB
                targetFile = os.path.abspath( os.path.join( subdir, os.path.basename( srcFile ) ) ) # e.g. targetFile = $CWD/geomDB/actualfile.db
                if subdir == os.getcwd(): 
                    subdir = ''
            else: # keepDir == False or given filename is an absolute path. 
                targetFile = os.path.abspath( os.path.basename( srcFile ) ) # e.g. targetFile = $CWD/actualfile.db
                subdir = ''
            # associate copy_func with either shutil.copyfile or os.symlink
            if doCopy == 'ifNotLocal':
                if isOnLocalFileSystem( srcFile ):
                    copy_func = os.symlink
                else: # remote file
                    print ("%s is on a different mount point as $CWD." % srcFile)
                    copy_func = shutil.copyfile
            realDirRequired = False
            if copy_func is os.symlink:
                # redefine targetFile and srcFile to link subdirectory instead. 
                # However, if subdir needs to be a real directory due to multiple distinct contributing 
                # source directories, the reassignment is not done.
                if subdir:
                    if subdir in realDirList: 
                        print ("%s found in realDirList: %s" % ( subdir, realDirList ))
                        realDirRequired = True
                    else:
#                        print ("Reassigning targetFile from %s to %s." % ( targetFile, subdir ))
                        targetFile = subdir
                        srcFile = os.path.dirname( srcFile ).rstrip( os.sep )
                        # set flag to Stop iterating over srcFileList since parent dir has (will be) symlinked.
                        parentDirLinked = srcFile
            # create the directory to contain the required srcFiles
            if copy_func is shutil.copyfile or realDirRequired:
                if subdir and not os.path.isdir( subdir ):
                    brokenLink = linkPresent( subdir, findBrokenLink = True )
                    if brokenLink:
                        try:
                            print ("Attempting to remove broken symlink %s" % brokenLink)
                            os.remove( brokenLink )
                        except OSError as x:
                            raise EnvironmentError( 'Unable to create the directory %s as the broken symlink %s cannot be removed: %s' % ( subdir, brokenLink, x ) )
#                        targetFile = 
                    os.makedirs( subdir )
            # Check existence of targetFile (side effect of an exception when running os.path.samefile).
            try:
                isSameFile = os.path.samefile( srcFile, targetFile )
            except OSError: # good. targetFile does not exist.
#                print ("%s does not exist. %s" % ( targetFile, x ))
                if os.path.islink( targetFile ): # broken symlink
                    try:
                        print ("*Attempting to remove %s" % targetFile)
                        os.remove( targetFile )
                    except OSError as x:
                        raise EnvironmentError( 'Unable to remove broken symlink %s: %s' % ( targetFile, x ) )
                try:
                    copy_func( srcFile, targetFile )
                except OSError:
                    # try copying instead.
                    shutil.copyfile( srcFile, targetFile )
                continue
            # Test if srcFile and targetFile are one and the same
            # NB: If the requested file is already in $CWD, the boolean flag isSameFile is always going 
            # to be True since a valid, existing file/symlink is given priority. This is so even if the 
            # existing file is a symlink and resides on a different mount point. The responsibility lies 
            # on user to be aware of the potential problems of accessing files over different mount points.
            if isSameFile:                
                print ("%s is the same as %s. No further action." % ( srcFile, targetFile ))
                continue # do nothing
            print ("%s is not the same as %s" % ( srcFile, targetFile ))
            # This point is reached in only two cases:
            # 1) An absolute filename was used.
            #    targetFile is not what we want as since the function was called to get a file with an absolute filename. 
            #    Attempt to remove the existing file before performing the copy.
            try:
                print ("**Attempting to remove %s" % targetFile)
                os.remove( targetFile ) # remove files and symlinks
            except Exception: # dst file is a directory
                for _root, _dirs, _files in os.walk( targetFile , topdown = False ):
                    for name in _files:
                        os.remove( os.path.join( _root, name ) )
                    for name in _dirs:
                        os.rmdir( os.path.join( _root, name ) )
                os.rmdir( targetFile )
            try:
                copy_func( srcFile, targetFile )
            except OSError:
                # try copying instead.
                shutil.copyfile( srcFile, targetFile )

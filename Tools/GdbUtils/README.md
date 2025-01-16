This is a package of helpers to make gdb more useful in Atlas.

If you have an ATLAS release set up, you can enable these helpers
by adding this line to your .gdbinit (you can also enter it interactively):

python import GdbUtils

This will do the following:

 - Remove all directories from sys.path except for system paths.
   This is needed because gdb is built against the system python;
   in general, this will not work with the libraries for the version
   of python being used by atlas.

 - Add the GdbUtils python directory to sys.path and the gdb source search path.
   This means that the gdb `source' command and the python `import' statement
   will both search the gdbhacks package.

 - gdb will be configured so that when symbols for an Atlas/LCG library
   are loaded, it will automatically try to add the proper directories
   to the source search path.

   Only tested for the built releases on lxplus.

 - New gdb command: findlib

   This takes an address as an argument.  findlib will find the
   shared library containing this address and attempt to load symbols
   from that library.  This is sometimes useful in the case where
   you've turned off automatic symbol loading.

 - New gdb command: btload

   This will load shared libraries needed to print the current backtrace.
   This may be useful if you've disabled automatic loading of shared libraries.
   By default, this will look at only the first 100 stack frames,
   but you can change this by giving the number of frames as an argument.

 - New gdb command: import

     import FOO

   is shorthand for

     python import FOO

 - New commands pahole and offsets-of.

     Taken from https://github.com/PhilArmstrong/pahole-gdb.
     Will print out the layout of class/struct types.

 - New command sheap

     sheap START-ADDR [END-ADDR]

    will dump out free and allocated malloc blocks within the given address
    range.  If not given, END-ADDR is taken to be 1024 bytes after START-ADDR.

    Caveats: It is not possible to reliably identify allocated memory blocks.
             Dumping will start at the first free block found within
             the given range, if any.

             This only works with the GNU libc malloc, not with any
             alternative allocators (tcmalloc, etc).



Optional commands available:

 - import stl

   Importing this module turns on formatted dumps of STL containers.

 - import pointerprint

   Importing this module changes the way pointers are printed.
   If a pointer points at an object with a virtual table,
   gdb will attempt to print the dynamic type of the object.

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

"""Utilities for the interactive athena prompt."""

import os
import sys
import re
import AthenaCommon.Utils.unixtools as unixtools
from AthenaCommon.Logging import log

_shellCommands = [ 'echo', 'emacs', 'env', 'ls', 'less', 'more', 'pico', 'vi' ]
_unacceptable = [ 'false', 'true', 'sys' ]
_NAME = 'name'
_NAMEREX = re.compile( r"named? '?(?P<%s>[\w\d]+)'?" % _NAME )


### LazyPython based hook for shell commands ====================================
class ShellEscapes:
   """Provide shell escapes from the prompt by catching name and syntax errors."""

   def __init__( self ):
      self._orig_ehook = sys.excepthook
      log.debug( 'shell short-cuts enabled' )

   def uninstall( self ):
      sys.excepthook = self._orig_ehook
      log.debug( 'shell short-cuts disabled' )

   def __call__( self, exctype, value, traceb ):
      cmd = None

    # catch name and syntax errors to perform shell escapes if known
      if isinstance( value, NameError ):
         res = _NAMEREX.search( str(value) )
         if res:
            cmd = res.group( _NAME )

          # disallow selected commands/executables
            if cmd in _unacceptable:
               cmd = None

      elif isinstance( value, SyntaxError ):
         cmd = value.text[:-1]

    # execute command, if any
      if cmd is not None:
         args = cmd.split()
         exe = args[0]

       # special cases
         if exe == 'cd' and len(args) == 2:
            os.chdir( args[1] )
            log.info( 'new directory: %s', os.getcwd() )
            return

         if exe == 'help' and len(args) == 2:
            import __main__
            exec ('help( %s )' % args[1] in __main__.__dict__, __main__.__dict__)
            return

       # cache shell command
         if exe not in _shellCommands:
            log.debug( 'accepting executable "%s"', exe )
            if unixtools.which( exe ):
               _shellCommands.append( exe )
            else:
               exe = None

         if exe is not None:
            log.debug( 'executing shell command "%s"', cmd )
            os.system( cmd )
            return

    # nothing recognizable: normal exception processing
      self._orig_ehook( exctype, value, traceb )


def configureInteractivePrompt(completionDict = None):
   """Configure interactive prompt. The optional completionDict is used to
   configure the readline completer."""

   # Athena-specific command history
   import atexit
   import readline
   import rlcompleter  # noqa: F401 (needed for completion)

   fhistory = os.path.expanduser( '~/.athena.history' )

   if completionDict is not None:
      readline.set_completer(rlcompleter.Completer(completionDict).complete)

   readline.parse_and_bind( 'tab: complete' )
   readline.parse_and_bind( 'set show-all-if-ambiguous On' )

   if os.path.exists( fhistory ):
      readline.read_history_file( fhistory )

   readline.set_history_length( 1024 )

   # save history on exit
   atexit.register( readline.write_history_file, fhistory )

   # enable shell commands in interactive prompt
   sys.excepthook = ShellEscapes()

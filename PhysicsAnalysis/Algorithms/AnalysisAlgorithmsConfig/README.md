Combining Configuration Files
-----------------------------

Configuration files contain with one special key: `include`. This key should be a *list of files*.
These will be iteratively merged into the current scope, such that the current file has priority, 
followed by the first item in the list, with the final file in the list having the lowest priority.
For each file in the list, the parser will  assume the key
gives a path _relative to the current file_. Any entries in the file
will be imported at the same scope where the `include` key appears.
If local keys conflict with imported ones they will be merged, giving
local keys precedence over those imported from the file.

Conflicting keys are merged as follows:

   - If the keys point objects (i.e. a `dict` or `map`), the union
     of all keys and values within the objects is taken, and
     conflicting keys are merged recursively.
   - If the keys point lists, the lists are concatenated.
   - If the keys point to anything else (i.e. numbers or strings),
     the local key overwrites the key from the file.

This gives a useful way to specify default values, since any values
imported via `file` will be overwritten by local ones.


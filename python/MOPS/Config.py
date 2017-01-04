"""
Loads MOPS configuration files from various places and presents them in a
JSON-like format.  The actual format of the files might be Perl's
Config::Scoped, or actual JSON.

For Config::Scoped files, we will support a very basic subset of Config::Scoped
functionality, basically JSON-like:

  key = value
  dict_key = {
    key = value
    ...
  }
  list_key = [
    key = value
    ...
  ]

with no nesting
"""

__author__ = 'Larry Denneau, Institute for Astronomy, University of Hawaii'
__date__ = '$Date$'
__version__ = '$Revision$'[11:-2]

import re
#import cjson       # for when we support JSON


_RE_START_DICT = r'\w+(\s*\{|\s*=\s*\{)$'
_RE_END_DICT = r'\}'

_RE_START_LIST = r'\w+(\s*\[|\s*=\s*\[)$'
_RE_END_LIST = r'\]'

_RE_KEY_VAL = r'\w+\s*=\s*.+'
_RE_TRAILING_STUFF = r'[,;]$'
_RE_QUOTES = r'(^[\'"]|[\'"]$)'

_RE_INT = r'^(\+|-)?[0-9]+$'
_RE_FLOAT = r'^(\+|-)?((\.[0-9]+)|([0-9]+(\.[0-9]*)?))$'
_RE_DOUBLE = r'^(\+|-)?((\.[0-9]+)|([0-9]+(\.[0-9]*)?))([eE](\+|-)?[0-9]+)?$'


def _clean_val(val_str):
    """
    Clean up val_str by:
    * removing trailing , or ;
    * converting to float if matches a float/double
    * converting to int if matches int
    Return None if resulting string is empty
    """
    val_str = val_str.strip()                           # clean up leading/trailing space
    val_str = re.sub(_RE_TRAILING_STUFF, '', val_str)   # clean up trailing comma and semicolon
    if re.match(_RE_INT, val_str):
        val = int(val_str)
    elif re.match(_RE_FLOAT, val_str) or re.match(_RE_DOUBLE, val_str):
        val = float(val_str)
    else:   
        # treat as string. but clean up quotes if any
        val_str = re.sub(_RE_QUOTES, '', val_str)
        val = val_str
    return val


def _parse_list_val(line):
    """ Convert a line into a value for a list config item """
    return _clean_val(line)


def _parse_key_val(line):
    """ Convert a line into a key, value pair for a dict config item """
    (key, val) = line.split('=', 1)
    key = key.strip()
    val = _clean_val(val)
    return key, val


def _process_list(lines):
    mr_line = []
    while (lines):
        # pop off zeroth item, clean it up
        line = lines.pop(0).strip().split('#', 1)[0]    
        if not line:
            # line is empty, next!
            continue       

        cooked = re.sub(r"'.*'", 'XXX', line)       # hide single-quoted
        cooked = re.sub(r'".*"', 'XXX', cooked)     # hide double-quoted

        if re.match(_RE_START_DICT, cooked):
            key = line.split()[0]                   # first token
            mr_line.append(_process_dict(lines))    # recurse
        elif re.match(_RE_START_LIST, cooked):
            key = line.split()[0]                   # first token
            mr_line.append(_process_list(lines))    # recurse
        elif re.match(_RE_END_LIST, cooked):
            # done with this list
            break
        elif re.match(_RE_END_DICT, cooked):
            raise RuntimeError('end of dict found while expecting end of list')
        else:
            mr_line.append(_parse_list_val(line))

    return mr_line


def _process_dict(lines):
    mr_dict = {}
    while (lines):
        # pop off zeroth item, clean it up
        line = lines.pop(0).strip().split('#', 1)[0]    
        if not line:
            # line is empty, next!
            continue       

        cooked = line
        cooked = re.sub(r"'.*'", 'XXX', line)       # hide single-quoted
        cooked = re.sub(r'".*"', 'XXX', cooked)     # hide double-quoted

        if re.match(_RE_START_DICT, cooked):
            key = line.split()[0]               # first token
            mr_dict[key] = _process_dict(lines) # recurse
        elif re.match(_RE_START_LIST, cooked):
            key = line.split()[0]   # first token
            mr_dict[key] = _process_list(lines) # recurse
        elif re.match(_RE_END_DICT, cooked):
            # done with this dict
            break
        elif re.match(_RE_END_LIST, cooked):
            raise RuntimeError('end of list found while expecting end of dict')
        else:
            key, val = _parse_key_val(line)
            mr_dict[key] = val

    return mr_dict


def LoadFile(filename):
    """
    Load MOPS configuration data from a single file into a JSON-like
    structure, essentially dictionaries and lists.  The root level of
    the configuration is always a dict, and each key can hold a value,
    list, or dict.  The caller is expected to know the structure of the
    config tree.

    Upon load, keys that look like ints are converted to ints, and keys
    that look like floats are converted to floats.

    Options for kind are: 'master', 'cluster', 'backend'.

    If filename is specified, then the exact file is read and parsed.
    """
    fh = file(filename)
    lines = fh.readlines()
    fh.close()
    return _process_dict(lines)


def LoadString(config_str):
    """
    Load MOPS configuration data from a string.
    """

    lines = config_str.split('\n')
    return _process_dict(lines)


def InsertDB(dbh, config_text):
    """Insert some config text into the MOPS database.
        
        @param dbh: Open connection to database where configuration is to be
                    stored.
    """
    cursor = dbh.cursor()
    sql = 'insert into config (config_test) values (%s)' 
    results = cursor.execute(sql, (config_text,))
    cursor.close()
    return results

def RetrieveDB(dbh):
    """Retrieve the most recent configuration from the MOPS database.
    
        @param dbh: Open connection to database from which configuration is to
                    retrieved. 
    """
    cursor = dbh.cursor()
    sql = 'select config_text from config order by config_id desc limit 1' 
    cursor.execute(sql)
    results = cursor.fetchall()
    cursor.close()
    if results:
        return results[0][0]
    else:
        return ''

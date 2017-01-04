#!/usr/bin/env python
"""
execute.py


Usage

    shell> execute.py command


Description

Execute the user specified command on any number of machines in parallel. The 
details of command execution are left to each individual node and are handled
by a custon socket server running on each availble machine (on port PORT).

execute.py connects to the server running on each available machine (whose list
is specified in the config file CFG_NAME) and issues the ecommand to execute.
Status code and standard output of each command is returned on the socket as a
pickled object.
"""
# Imports
import asyncore
import cPickle as pickle
import os
import re
import socket


# Constants
CFG_NAME = 'cluster.cf'
PORT = 6900
TERMINATOR = '\n'



# The code!
def usage():
    """
    Print the usage strng and return.
    """
    USAGE_STR = 'execute.py command'
    print(USAGE_STR)
    return

def readConfigFile():
    """
    Try and read the run-specific config file. If that fails, then try and read 
    the system-wide file. If also that fails, then bail out and throw an 
    exception of type IOError
    """
    # Either way, we need $MOPS_HOME. We do not necessarily need 
    # $MOPS_DBINSTANCE, but is that is not defined, it is a problem.
    try:
        mopsHome = os.environ['MOPS_HOME']
    except:
        raise(IOError('Unable to read %s: $MOPS_HOME not defined.' %(CFG_NAME)))
    try:
        instanceName = os.environ['MOPS_DBINSTANCE']
    except:
        raise(IOError('Unable to read %s: $MOPS_DBINSTANCE not defined.' \
                      %(CFG_NAME)))
    
    # The two possible config file locations.
    localCF = os.path.join(mopsHome, 'var', instanceName, 'config', CFG_NAME)
    globalCF = os.path.join(mopsHome, 'config', CFG_NAME)
    
    # If localCF is there, read that one; otherwise go for globalCF.
    if(not os.path.exists(localCF)):
        return(parseConfigFile(globalCF))
    return(parseConfigFile(localCF))

def parseConfigFile(configFile):
    """
    Entries have the form 
        hostname = [user, numCPU]
    They are usually one per line, but there can also be multiple entries on a
    single line. In that case, entries are separated with a comma (',').
    """
    PATTERN= re.compile('([a-zA-Z0-9_]+)')
    GUARDS = ('nodes', 'nodes=', 'nodes=[')
    DELIMITER = ']'
    
    data = file(configFile).readlines()
    nodes = []
    found = False
    for line in data:
        line = line.strip()
        if(not line):
            continue
        
        valid = line.split('#', 1)[0]
        if(not valid):
            continue
        
        rest = ''
        try:
            token, rest = valid.split(' ', 1)
        except:
            token = valid.split(' ', 1)[0]
        if(not found and token in GUARDS):
            # At this point we have found our section. See if the data is on the
            # same line.
            found = True
            entry = rest.split(DELIMITER)[0]
            if(not entry):
                continue
            parser = parseEntry(entry, PATTERN)
            try:
                while(True):
                    nodes.append(parser.next())
            except:
                continue
        elif(found):
            tokens = valid.split(DELIMITER, 1)
            entry = tokens[0]
            parser = parseEntry(entry, PATTERN)
            try:
                while(True):
                    nodes.append(parser.next())
            except:
                pass
            if(len(tokens) > 1):
                break
            # <-- end if
        # <-- end if
    # <-- end for
    return(nodes)

def parseEntry(rawEntry, pattern):
    DELIMITER = ','
    
    rawEntries = rawEntry.split(DELIMITER)
    for raw in rawEntries:
        yield pattern.search(raw).groups()[0]
    

class Client(asyncore.dispatcher):
    def __init__(self, node, port, command):
        asyncore.dispatcher.__init__(self)
        self._cmd = command + TERMINATOR
        self._rawData = ''
        self.data = None
        
        # Connect to the server.
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.connect((node, port))
        return
    
    def handle_connect(self):
        pass
    
    def handle_close(self):
        self.close()
        self.data = pickle.loads(self._rawData)
        print(self.data)
        
    
    def handle_read(self):
        self._rawData += self.recv(4096)
        return
    
    def readable(self):
        return(True)
    
    def writable(self):
        return(len(self._cmd) > 0)

    def handle_write(self):
        sent = self.send(self._cmd)
        self._cmd = self._cmd[sent:]
        return



def prepareDataFiles(input, hosts):
    """
    Create random names for input files and split the master input file into as 
    many chunks as the number of hosts. Copy the chunks to their host.
    
    Return a dictionary of the form {host: input file name}
    """
    import math
    import tempfile
    
    
    fileInfo = {}
    numHosts = len(hosts)
    
    if(numHosts > 1):
        # Split the input file into chunks and write them to randomly named files.
        inData = file(input).readlines()
        numLines = len(inData)
        numLinesPerHost = int(math.ceil(float(numLines) / float(numHosts)))
        
        cursor = 0
        for host in hosts:
            # Split the master input file.
            endPoint = cursor + numLinesPerHost
            tmpInHandle, tmpInName = tempfile.mkstemp(dir='/tmp')
            # FIXME: This is WAY slow.
            err = map(lambda x: os.write(tmpInHandle, x), 
                      inData[cursor:endPoint])
            os.close(tmpInHandle)
            
            # Copy the chunk on the remote node.
            remoteCopy(tmpInName, host)
            
            # Update fileInfo and the file cursor.
            fileInfo[host] = tmpInName
            cursor = endPoint
        # <-- end for
    elif(numHosts == 1):
        # Copy the chunk on the remote node.
        remoteCopy(input, hosts[0])
        
        # Simply reuse the input file.
        fileInfo[hosts[0]] = input
    return(fileInfo)
    
def remoteCopy(fileName, hostName):
     """
     Copy the fileName onto the remote hostName. We preserve the file path the
     same way scp does.
     """
     err = os.system('scp -CBq %s %s:%s' %(fileName, hostName, fileName))
     if(err):
         raise(IOError('Remote copy of %s to %s failed.' %(fileName, hostName)))
     return   

def convertCMD(cmd, masterInputName, masterOutputName, clientInputName):
    """
    Given a command in the form command [options] [<] _input_=<input file> \
    [> | flag | flag=]_output_=<output file> (the order is not important),
    and the names of the master input and output files, remove the output file
    name alltogether and replace the name of the master input with that of the 
    client input file name. The resulting command will have the form
    command [options] [<] _input_=<client input file> [> | flag | flag=]_output_
    """
    # Remove the master output file name.
    outPat = re.compile("""_output_=["']?[a-zA-Z0-9_ \.%s]+[a-zA-Z0-9_%s]["']?"""\
                        %(os.sep, os.sep))
    newCMD = outPat.sub('_output_', cmd)
    
    # Replace the master input file name with the client file name.
    newCMD = newCMD.replace(masterInputName, clientInputName)
    return(newCMD)




if(__name__ == '__main__'):
    import sys
    
    
    # Parse Commans line input.
    try:
        cmd = sys.argv[1]
    except:
        usage()
        sys.exit(1)
    if(not cmd):
        usage()
        sys.exit(1)
    
    # cmd has some special variables to indocate inpput and output file names.
    # The format is the following: command [options] [<] _input_=<input file> \
    # [> | flag | flag=]_output_=<output file>. The order is not important.
    iPat = re.compile("""_input_=["']?([a-zA-Z0-9_ \.%s]+[a-zA-Z0-9_%s])["']?""" \
                       %(os.sep, os.sep))
    oPat = re.compile("""_output_=["']?([a-zA-Z0-9_ \.%s]+[a-zA-Z0-9_%s])["']?"""\
                      %(os.sep, os.sep))
    
    # Extract the names of input/output files.
    try:
        inputFile = iPat.search(cmd).groups()[0]
    except:
        sys.stderr.write('Fatal error: unable to parse command string to ' +
                         'extract input file name.\n')
        sys.exit(2)
    try:
        outputFile = oPat.search(cmd).groups()[0]
    except:
        sys.stderr.write('Fatal error: unable to parse command string to ' +
                         'extract output file name.\n')
        sys.exit(2)
    
    # Read the config file and extract the list of nodes.
    nodes = readConfigFile()
    
    # Prepare input files on the remote nodes.
    clientFileInfo = prepareDataFiles(inputFile, nodes)
    
    # Init the client threads.
    clients = [Client(node, 
                      PORT, 
                      convertCMD(cmd, 
                                 inputFile,
                                 outputFile, 
                                 clientFileInfo[node])) \
               for node in nodes]
    asyncore.loop(timeout=1, use_poll=False, map=None)
    
    # Well, at this point all the wark has been done. Exit.
    # TODO: the exit code should reflect the exit codes from the various workers
    sys.exit(0)







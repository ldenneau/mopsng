#!/usr/bin/env python
"""
executed.py

Socket server that waits for connections containing commands to execute on the 
localhost. These commands are excuted and the results sent back to the caller.

Each server knows how many CPUs are available on the localhost.
"""
# Imports
import asyncore
import asynchat
import cPickle as pickle
import os
import popen2
import re
import socket
import time



# Constants
TERMINATOR = '\n'                      # The end-of-message string
NCONNECTIONS = 1000                    # The number of concurrent connections
PORT = 6900
TIMEOUT = 30
MAX_PROCS = 4
POLLING_TIME = 0.5


class Worker(object):
    def __init__(self, command, outputFile, verbose=True):
        self._cmd = command
        self._outputFile = outputFile
        self.verbose = verbose
        
        self._activeProcess = None
        self._activeProcessGID = None
        self.output = ''
        self.status = None
        return
    
    def run(self):
        """
        Generator. Yiled None until the external process (self._cmd) has done
        executing. Then stop the iteration.
        """
        # Start the process.
        if(self.verbose):
            print('Executing %s' %(self._cmd))
        self._activeProcess = popen2.Popen4(self._cmd)
        
        # Put it into its own GID (for easier SIGTERMing it if needed).
        try:
            err = os.setpgid(self._activeProcess.pid,
                             self._activeProcess.pid)
            self._activeProcessGID = self._activeProcess.pid
        except:
            self._activeProcessGID = None
        
        # Give control back to the calling process.
        yield(None)
        
        # Start a loop until the external process has completed.
        status = self._activeProcess.poll()
        while(status == -1):
            # The external command has not returned yet. Sleep for a bit and 
            # check back later. Give control back to the caller.
            yield(None)
            status = self._activeProcess.poll()
        # Decode the status code.
        self.status = os.WEXITSTATUS(status)
        
        # Read the output of the command (from the output file).
        self.output = file(self._outputFile).read()
        
        # cleanup
        self._activeProcess.fromchild.close()
        self._activeProcess.tochild.close()
        self._activeProcess = None
        
        # Yield back the final status code
        if(self.verbose):
            print('Command (%s) exited with status %d.' % (self._cmd, status))
        yield(status)



class Server(asyncore.dispatcher):
    """
    SocketServer.TCPServer look alike but using asyncore/asynchat for 
    performance reasons.
    """
    address_family = socket.AF_INET
    socket_type = socket.SOCK_STREAM
    request_queue_size = NCONNECTIONS
    allow_reuse_address = True
    
    def __init__(self, server_address, RequestHandlerClass):
        """
        Standard constructor: setup instance variables and bind to the socket.
        """
        asyncore.dispatcher.__init__(self)
        
        self.server_address = server_address
        self.RequestHandlerClass = RequestHandlerClass
        
        self.handler = None
        self._timeout = 30.                        # Timeout in seconds.
        
        self.create_socket(self.address_family, self.socket_type)
        self.server_bind()
        self.server_activate()
        return

    def server_bind(self):
        """
        Called by constructor to bind the socket.
        
        May be overridden.
        """
        if self.allow_reuse_address:
            self.set_reuse_addr()
        self.bind(self.server_address)
        return
    
    
    def server_activate(self):
        """
        Called by constructor to activate the server.
        
        May be overridden.
        """
        self.listen(self.request_queue_size)
        return
    
    
    def handle_accept(self):
        """
        Called on listening channels (passive openers) when a connection can 
        be established with a new remote endpoint that has issued a connect()
        call for the local endpoint (from Python's manual).
        
        This implementation delegates the connection handling to a newly 
        created self.RequestHandlerClass instance.
        """
        request, client_address = self.accept()
        self.handler = self.RequestHandlerClass(request, client_address, self)
        return

    def handle_request (self):
        """ Start handling requests from to the server """
        asyncore.loop(timeout=self._timeout, 
                      use_poll=False, 
                      map=None)
        return

    def serve_forever (self): 
        asyncore.loop(timeout=self._timeout, 
                      use_poll=False, 
                      map=None)
        return



class ExecServer(Server):
    """
    ExecServer
    
    Socket server that waits for commands on the socket, executes them on the 
    localhost and sends back their output and exit code.
    
    For security reasons, only few commands are accepted:
        coarseEphem
        genEphem
    Absolute paths are stripped and only the command is retained.
    
    
    The server is initialized with 
        maxProcs:     the maximum number of consurrent processes.
        timeout:      the number of seconds before a hanging process is 
                      considered dead.
        
    """
    def __init__(self, server_address, RequestHandlerClass, timeout, maxProcs):
        Server.__init__(self, server_address, RequestHandlerClass)
        self.timeout = timeout
        self.maxProcs = maxProcs
        self._queue = []
        self.availableCommands = ['coarseEphem', 'genEphem']
        return



class RequestHandler(asynchat.async_chat):
    """
    Stream request handler.
    """
    def __init__(self, request, client_address, server, terminator=TERMINATOR):
        """
        Standard constructor: setup instance variables.
        """
        asynchat.async_chat.__init__(self, request)
        
        self.request = request
        self.client_address = client_address
        self.server = server
        
        self.set_terminator(terminator)
        self.raw_requestlines = []
        return
    
    def collect_incoming_data(self, data):
        """
        Buffer the data coming from the socket
        """
        self.raw_requestlines.append(data)
        return
    
    def handle_close(self):
        """
        Called when the channel is closed.
        """
        self.raw_requestlines = []
        asynchat.async_chat.handle_close(self)
        return
    
    def found_terminator(self):
        """
        Called when the "end-of-message" string (TERMINATOR) is found. It 
        replaces the handle() method in SocketServer.BaseRequestHandler class.
        """
        # Do something with the data collected so far.
        self.handle()
        
        # Cleanup and close the connection.
        self.raw_requestlines = []
        self.close_when_done()
        return
    
    def handle(self):
        """
        Fetch the command to execute from the socket, do minimal parsing, split
        the input file into as many processes as we are allowed to start, create
        individual commands and take care of creating output files.
        
        The command string has the form
        command [options] [<] _input_=<client input file> \
                [> | flag | flag=]_output_
        """
        avail = range(self.server.maxProcs)
        
        # Ignore any further data from now on.
        self.set_terminator(None)
        
        # Extract the command from the data we have read so far.
        cmd = self.raw_requestlines[0].strip()
        
        # Extract the name of the master input file.
        masterInputFile = extractInputFileName(cmd)
        
        # Prepare input files given the name of the master input file and the
        # maximun mumber of processes to start.
        procFileInfo = prepareDataFiles(masterInputFile, 
                                        ['proc%04d' %(i) for i in avail])
        
        
        # Start as many worker processes as self.server.maxProcs
        workers = [Worker(convertCMD(cmd,
                                     masterInputFile,
                                     procFileInfo['proc%04d' %(i)][0],
                                     procFileInfo['proc%04d' %(i)][1]),
                          procFileInfo['proc%04d' %(i)][1]) \
                   for i in avail]
        statusCodes = []
        outputData = ''
        
        # Activate the workers.
        n = self.server.maxProcs
        procs = [w.run() for w in workers]
        while(workers):
            i = 0
            while(i < n):
                try:
                    procs[i].next()
                    time.sleep(POLLING_TIME)
                    i += 1
                except StopIteration:
                    # cmd is done executing. Save the output and the status 
                    # code. Remove the corresponding worker from the pool.
                    statusCodes.append(workers[i].status)
                    outputData += workers[i].output
                    del(workers[i])
                    del(procs[i])
                    n -= 1
            # <-- end while
        # <-- end while
        
        # Return the exit codes and STDOUT to the client. The format is
        # [(exit code, STDOUT), ...]
        data = (statusCodes, outputData)
        self.push(pickle.dumps(data, protocol=-1))
        return


def extractInputFileName(cmd):
    # cmd has some special variables to indocate inpput and output file names.
    # The format is the following: command [options] [<] _input_=<input file> \
    # [> | flag | flag=]_output_. The order is not important.
    iPat = re.compile("""_input_=["']?([a-zA-Z0-9_ \.%s]+[a-zA-Z0-9_%s])["']?""" \
                       %(os.sep, os.sep))
    
    # Extract the names of input/output files.
    inputFile = iPat.search(cmd).groups()[0]
    return(inputFile)

def convertCMD(cmd, masterInputName, procInputName, procOutputName):
    """
    Given a command in the form command [options] [<] _input_=<input file> \
    [> | flag | flag=]_output_ (the order is not important), the name of the 
    master input and the names of the desired input and output files per process
    replace the name of the master input with that of the process specific input
    file and '_output_' with the process output file name. The resulting command
    will have the form
    command [options] [<] <input file> [> | flag | flag=]<output file>
    """
    # Add the output file name.
    newCMD = cmd.replace('_output_', '"%s"' %(procOutputName))
    
    # Replace the master input file name with the client file name.
    newCMD = newCMD.replace(masterInputName, procInputName)
    
    # Remove the _input_= delimiter.
    newCMD = newCMD.replace('_input_=', '')
    return(newCMD)

def prepareDataFiles(inputFile, procs):
    """
    Create random names for input and output files and split the master input 
    file into as many chunks as the number of hosts.
    
    Return a dictionary of the form {process number: [input name, output name]}
    """
    import math
    import tempfile
    
    
    fileInfo = {}
    
    # Split the input file into chunks and write them to randomly named files.
    inData = file(inputFile).readlines()
    numLines = len(inData)
    numProcs = len(procs)
    numLinesPerHost = int(math.ceil(float(numLines) / float(numProcs)))
    
    cursor = 0
    for proc in procs:
        # Split the master input file.
        endPoint = cursor + numLinesPerHost
        tmpInHandle, tmpInName = tempfile.mkstemp(dir='/tmp')
        err = map(lambda x: os.write(tmpInHandle, x), 
                      inData[cursor:endPoint])
        os.close(tmpInHandle)
        
        # Create a temp file name for output.
        tmpOutHandle, tmpOutName = tempfile.mkstemp(dir='/tmp')
        os.close(tmpOutHandle)
        
        # Update fileInfo and the file cursor.
        fileInfo[proc] = [tmpInName, tmpOutName]
        cursor = endPoint
    # <-- end for
    return(fileInfo)


if(__name__ == '__main__'):
    import optparse
    import sys
    
    
    USAGE = \
"""usage: executed.py [OPTIONS]

Options
 -p, --port=PORT          specify the PORT to bind to.
 -t, --timeout=TIMEOUT    specify the TIMEOUT in seconds.
 -n, --max_procs=N        specify the max number N of concurrent processes.
"""
    
    # Parse command line arguments.
    parser = optparse.OptionParser(USAGE)
    parser.add_option('-p', '--port',
                      action='store', 
                      type='int', 
                      dest='port',
                      help='pecify the PORT to bind to (default %d).' %(PORT))
    parser.add_option('-t', '--timeout',
                      action='store', 
                      type='int', 
                      dest='timeout',
                      help='specify the TIMEOUT in seconds (default %d).' \
                           %(TIMEOUT))
    parser.add_option('-n', '--num_procs',
                      action='store', 
                      type='int', 
                      dest='maxProcs',
                      help='specify the max number N of concurrent processes.')
    
    port = PORT
    timeout = TIMEOUT
    maxProcs = MAX_PROCS
    (options, args) = parser.parse_args()
    if(options.port):
        port = options.port
    if(options.timeout):
        timeout = options.timeout
    if(options.maxProcs):
        maxProcs = options.maxProcs
    
    # Start the server.
    srv = ExecServer(server_address=('', port),
                     RequestHandlerClass=RequestHandler,
                     timeout=timeout,
                     maxProcs=maxProcs)
    try:
        print('ExecServer up and running on port %d.' % (port))
        srv.serve_forever()
    except:
        print('ExecServer terminated.')
        sys.exit(0)















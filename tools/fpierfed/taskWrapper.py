#!/usr/bin/env python
"""
Wrapper for simple UNIX executables. Provides calling code with return code
and standard stream output from the called executable. Provides logging
facilities and timeout support.
"""
import logging
import os
import popen2
import time



class TaskWrapper(object):
    """
    Create a wrapper for the system command @param executable providing
    logging facilities (to @param logFile), timeout (after @param timeout
    seconds) and recovery (try again @param ntries times).
    """
    def __init__(self, executable, pollInterval, timeout, ntries, logFile,
                 verbosity=logging.INFO):
        self._executable = executable
        self._pollInterval = pollInterval
        self._timeout = timeout
        self._ntries = ntries
        self._logFile = logFile
        self._verbosity = verbosity

        # Setup loggers.
        logging.basicConfig(level=self._verbosity,
                            filename=self._logFile,
                            filemode='a',
                            format='%(levelname)s %(asctime)s:\t%(message)s')

        # Other basic initializations.
        self._activeProcess = None
        self._activeProcessGID = None
        return

    def execute(self):
        """
        Execute self._executable, monitor its execution, redirect its standard
        streams to self._logFile and, if appropriate (depending on
        self._ntries and self.timeout) provide a recovery strategy.
        """
        # FIXME: Implement timeout
        # FIXME: Implement error recovery

        # Start the executable and keep polling to see if it has quit.
        self._run()

        poller = self._poll()
        try:
            status = poller.next()
            while(status == None):
                time.sleep(self._pollInterval)
                status = poller.next()
        except StopIteration:
            pass

        # The process has finished executing/was terminated.
        # Log its output, cleanup and return its status code/signal.
        self._logOutput()
        self._cleanup()

        if(os.WIFEXITED(status)):
            # The process exited cleanly.
            return(os.WEXITSTATUS(status))
        else:
            # The process was terminated.
            return(os.WTERMSIG(status))
        return


    def _run(self):
        """
        Execute self._executable and yield its status code until it quits,
        then yield its return code.
        """
        self._activeProcess = popen2.Popen4(self._executable)

        # Put activeProcess in its own process group (just in case it spawns
        # other processes).
        try:
            err = os.setpgid(self._activeProcess.pid, self._activeProcess.pid)
            self._activeProcessGID = self._activeProcess.pid
        except:
            self._activeProcessGID = None
        return


    def _poll(self):
        """
        Check whether the process finished already. If not, keep waiting. Keep
        waiting also if the process was stopped. If still running/stopped
        yield None, otherwise its exit code/crash signal.
        """
        while(True):
            status = self._activeProcess.poll()
            if(status == -1):
                # self._activeProcess has not returned yet. Yield None
                yield(None)
            elif(os.WIFSTOPPED(status)):
                # Process was only stopped; yield None.
                yield(None)
            else:
                yield(status)
        yield(None)

    def _logOutput(self):
        """
        Fetch the statndard stream output from self._activeProcess and
        redirect it to self._logFile.
        """
        try:
            logging.info(self._activeProcess.fromchild.read())
            logging.debug(self._activeProcess.childerr.read())
        except:
            pass
        return

    def _cleanup(self):
        self._activeProcess.tochild.close()
        self._activeProcess.fromchild.close()
        self._activeProcess = None
        return


    def killActiveProcess(self):
        """
        Kill the processes associated to self._activeProcess, if any. It
        should be called when shutting down.
        """
        if(self._activeProcess != None):
            import signal
            # Send a SIGKILL to the process group to which self._activeProcess
            # belongs. Remember that the PID of the group is set to the PID of
            # self._activeProcess in executeActions().
            if(self._activeProcess.pid != None):
                os.killpg(self._activeProcessGID, signal.SIGKILL)
            else:
                os.kill(self._activeProcess.pid, signal.SIGKILL)
        return










if(__name__ == '__main__'):
    import getopt
    import sys

    # Constants
    USAGE = \
"""
    taskWrapper.py [--log=logfile]
                   [--timeout=timeout] [--polling=polling_interval]
                   [--ntries=num_tries] "executable args"

    --log=logfile : name of the log file [default: executable.log]
    --timeout=timeout : timeout is seconds before abotring the system call [default 0=no timeout]
    --polling=polling_interval : interval in seconds before two polls on the system call [default 1].
    --ntries=num_tries : number of times to retry executing the system call after failure [default 0].
    executable : the command to execute.
    args : the executable arguments.

    Note the quotes enclosing executable and its args.
"""

    # Parse command line flags and args
    if(len(sys.argv) == 1):
        sys.stderr.write('Usage:\n')
        sys.stderr.write('%s\n' % (USAGE))
        sys.exit(1)
    try:
        opts, args = getopt.getopt(sys.argv[1:], '', ['log=',
                                                      'log_dir=',
                                                      'timeout=',
                                                      'polling=',
                                                      'ntries='])
    except:
        sys.stderr.write('%s\n' % (USAGE))
        sys.exit(1)

    # Make sure that we have the executable name.
    try:
        execString = args[0]
        execName = execString.split()[0]
    except:
        sys.stderr.write('Fatal Error: Cannot determine executable name.\n')
        sys.stderr.write('%s\n' % (USAGE))
        sys.exit(2)

    # Parse the rest of the flags, if any.
    logFile = '%s.log' % (execName)
    timeout = 0
    polling = 1
    ntries = 0
    for key, val in opts:
        if(key == '--log'):
            logFile = os.path.abspath(val)
        elif(key == '--timeout'):
            try:
                timeout = float(val)
            except:
                sys.stderr.write('Fatal Error: timeout must be a number.\n')
                sys.stderr.write('%s\n' % (USAGE))
                sys.exit(4)
        elif(key == '--polling'):
            try:
                polling = float(val)
            except:
                sys.stderr.write('Fatal Error: polling must be a number.\n')
                sys.stderr.write('%s\n' % (USAGE))
                sys.exit(5)
        elif(key == '--ntries'):
            try:
                ntries = int(val)
            except:
                sys.stderr.write('Fatal Error: ntries must be a number.\n')
                sys.stderr.write('%s\n' % (USAGE))
                sys.exit(6)
        else:
            sys.stderr.write('Warning: flag %s not supported.\n' % (key))
            continue
    # <-- end for

    # Minimal sanity check.
    if(not logFile or not execString or not polling):
        sys.stderr.write('Fatal Error: log, polling and executable need to be specified.\n')
        sys.stderr.write('%s\n' % (USAGE))
        sys.exit(7)

    # Invoke the executable via a TaskWrapper instance.
    wrapper = TaskWrapper(execString,
                          polling,
                          timeout,
                          ntries,
                          logFile,
                          verbosity=logging.DEBUG)
    sys.exit(wrapper.execute())




## Abstraction layer for Condor.
import sys
import os
import re
import subprocess
import tempfile
import time
import atexit       # for file cleanup handler

from job import Job



# Constants
FREE = 'Unclaimed'
BUSY = 'Owner'



# Globally-scoped auto-delete filehandle.
_TEMP_FILEHANDLES = []
_CLEANUP_FILENAMES = []

def cleanup():
    """
    atexit() handler to remove all files in _CLEANUP_FILENAMES.
    """
    for file in _CLEANUP_FILENAMES:
        try:
            os.remove(file)
        except:
            pass
    return
atexit.register(cleanup)



def submit(condorJob, removeLog=False, removeJobFile=True):
    """
    Submit the given job to the Condor scheduler (on the localhost).

    Submission is syncronous meaning that this routine blocks and will not
    return until the job has completed.

    In case of error, log and job file are never deleted.
    
    @param condorJob instance of the condor.Job class.
    @param removeLog boolean
    @param removeJobFile boolean
    @return err error code as returned by condor_submit (0 meaning OK).
    """
    if condorJob._Job__sub_file_path:
        name = condorJob._Job__sub_file_path
    else:
        # Create a temporary file holder for the job description file.
        (fd, name) = tempfile.mkstemp(suffix='.job', prefix='condor', dir='/tmp')
        os.close(fd)
    # <-- end if

    if(removeJobFile):
        _CLEANUP_FILENAMES.append(name)
    # <-- end if

    # If needed, specify the name of the log file.
    log = condorJob.get_log_file()
    if(not log):
        log = name.replace('.job', '.log')
        condorJob.set_log_file(log)
        removeLog = True
    # <-- end if
    
    # Write the job description file.
    condorJob.set_sub_file(name)
    condorJob.write_sub_file()

    # Submit the job and wait for its completion.
    job_id = 'unknown'
    f = os.popen('condor_submit %s' %(name), 'r')
    if not f:
        raise RuntimeError('condor_submit %s failed.' %(name))

    stuff = f.readlines()
    if not stuff:
        raise RuntimeError('condor_submit %s produced no output.' %(name))
    
    foo = re.search(r'submitted to cluster (\d+)', stuff[-1])
    job_id = ''
    if foo:
        job_id = foo.group(1)
    else:
        raise RuntimeError('did not get a job ID')

    # Sleep for a fraction of a second to give condor_submit the time to create
    # the log file. Is this necessary?
#    time.sleep(0.5)

#    sys.stderr.write("Waiting on job file %s (job ID %s).\n" %(log, job_id))
#    err = os.system('condor_wait -wait 30 %s %s' %(log, job_id))
#
#    # Condor returns 1 for both unrecoverable errors and timeout, so we will
#    # ignore the error code since we're always going to re-wait.
##    if(err):
##        raise(RuntimeError('Error in condor_wait (exit code=%d).' %(err)))
#
#    # Wait again, this time indefinitely!  We're doing this to see if this cures 
#    # the problem we see of condor_wait not returning for jobs that
#    # have little or no processing.  
    err = os.system('condor_wait %s %s' %(log, job_id)) 
    if(err):
        raise(RuntimeError('Error in condor_wait (exit code=%d).' %(err)))

    # Clean up.
    if(not err and removeLog):
        os.remove(log)
    if(not err and removeJobFile):
        os.remove(name)

    return(err)


def status():
    """
    Return the status of the Condor pool. The status is returned as a dictionary
    whose keys are node names and values are states. Condor states are
      Unclaimed
      Owner

    @return status dictionary {machine_name: state, }
    """
    # Call condor_status -format "%s " Name -format "%s\n" State
    # and fetch its output.
    proc = subprocess.Popen(args=['condor_status',
                                  '-format', '%s ', 'Name',
                                  '-format', '%s\n', 'State'],
                            stdout=subprocess.PIPE)
    err = proc.wait()
    if(err):
        raise(RuntimeError('Error in condor_status (exit code=%d).' %(err)))
    # <-- end if

    # Parse the output.
    data = proc.stdout.readlines()
    res = dict([tuple(line.strip().split()) for line in data if line.strip()])

    # Cleanup and exit.
    proc.stdout.close()
    del(proc)
    return(res)


def available():
    """
    Convenience function to return the list of available nodes.

    @return list of available node names.
    """
    stats = status()
    return([key for key in stats if stats[key] == FREE])
    

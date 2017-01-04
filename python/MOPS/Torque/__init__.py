## Abstraction layer for Torque/PBS/OpenPBS.
import copy
import sys
import os
import re
import subprocess
import tempfile
import time
import atexit       # for file cleanup handler

import pbs
import PBSQuery

from job import Job
from job import fromCondorJobFile
from job import TorqueSubmitError

# Fetch the name of the Torque scheduler from the config file.
try:
    import MOPS.Instance as Instance
    mops_instance = Instance.Instance(dbname=os.environ['MOPS_DBINSTANCE'])
    config = mops_instance.getConfig()
    TORQUE_SCHEDULER = config['middleware']['scheduler']
    TORQUE_GROUP = config['middleware']['group']
except:
    TORQUE_SCHEDULER = ''               # Use default scheduler.
    TORQUE_GROUP = ''                   # Use default group




# Constants
FREE = 'Unclaimed'
BUSY = 'Owner'
POLLING_INTERVAL = 0.5                  # seconds



# Globally-scoped auto-delete filehandle.
_TEMP_FILEHANDLES = []
_CLEANUP_FILENAMES = []

def cleanup():
    """
    atexit() handler to remove all files in _CLEANUP_FILENAMES.
    """
    for file in _CLEANUP_FILENAMES:
        os.remove(file)

atexit.register(cleanup)

# Exceptions.
class Error(Exception):
    pass


def _torque_arrayid_fix(job, index):
    """
    This is a hack. The main reason for thi shack is that torque is not able to
    properly support job clusters.
    """
    job.out_file = job.out_file.replace('$(PBS_ARRAYID)', str(index))
    job.err_file = job.err_file.replace('$(PBS_ARRAYID)', str(index))
    job.arguments = [arg.replace('$(PBS_ARRAYID)', str(index)) \
                     for arg in job.arguments]
    return


def submit(torqueJob, removeLog=False, removeJobFile=True, async=True,
           schedulerHost=TORQUE_SCHEDULER,
           jobGroup=TORQUE_GROUP):
    """
    Submit the given job to the Torque scheduler (on the user supplied scheduler
    machine, or the default scheduler if none is specified.).

    Submission is syncronous meaning that this routine blocks and will not
    return until the job has completed.

    In case of error, log and job file are never deleted.
    
    @param torqueJob instance of the Torque.Job class.
    @param removeLog boolean
    @param removeJobFile boolean
    @return err error code as returned by torque_submit (0 meaning OK).
    """
    # Create a temporary file holder for the job description file.
    (fd, name) = tempfile.mkstemp(prefix='torque', dir='/tmp')
    os.close(fd)
    if(removeJobFile):
        _CLEANUP_FILENAMES.append(name)
    # <-- end if

    # If needed, specify the name of the log file.
    log = torqueJob.get_log_file()
    if(not log):
        log = name + '.log'
        torqueJob.set_log_file(log)
        removeLog = True
    # <-- end if

    # Connect to the Torque scheduler.
    if(not schedulerHost):
        torqueScheduler = pbs.pbs_default()
        torqueConnection = pbs.pbs_connect(torqueScheduler)
    else:
        torqueConnection = pbs.pbs_connect(schedulerHost)
    # <-- end if

    # Set the name of the job and specify that the job is re-runnable.
    # We need to do this since the PBS C library SUCKS!!!!!
    attropl = pbs.new_attropl(7)
    attropl[0].name  = pbs.ATTR_N
    attropl[0].value = 'MOPS'
    attropl[1].name  = pbs.ATTR_r
    attropl[1].value = 'y'
    attropl[2].name  = pbs.ATTR_o
    attropl[2].value = 'stdout'
    attropl[3].name  = pbs.ATTR_e
    attropl[3].value = 'stderr'
    attropl[4].name = pbs.ATTR_g
    attropl[4].value = jobGroup
    attropl[5].name = pbs.ATTR_l
    attropl[5].resource = 'mem'
    attropl[5].value = '1gb'
    attropl[6].name  = pbs.ATTR_v
    attropl[6].value = ''
    for env in torqueJob.env:
        attropl[6].value += env + ','
    # <-- end for
    if(schedulerHost):
        attropl[6].value += 'PBS_O_HOST=' + schedulerHost
    else:
        attropl[6].value = attropl[6].value[:-1].strip()
    # <-- end if
    
    # Torque got the job array functionality only in 2.2. Since several sites
    # are still running 2.1, we fake it here.
    numSubJobs = max(torqueJob.queue, 1)
    
    subJobIds = []
    for n in range(numSubJobs):
        subJob = copy.deepcopy(torqueJob)
        # Hack!!!!!!
        _torque_arrayid_fix(subJob, n)  # needed for torque < 2.2!!!!!!!!!!!!

        
        # Create the subjob description file.
        subFileName = '%s_%d_of_%d.job' %(name, n, numSubJobs)

        # Update the job stdout and stderr.
        if(subJob.initial_dir):
            subJob.err_file = os.path.join(subJob.initial_dir, subJob.err_file)
            subJob.out_file = os.path.join(subJob.initial_dir, subJob.out_file)
        # <-- end if
        
        # Write the job description file.
        subJob.set_sub_file(subFileName)
        subJob.write_sub_file()

        # Submit the subjob
        attropl[0].value = '%s.%d' %(os.environ.get('MOPS_DBINSTANCE',
                                        '%s_MOPS' %(os.environ['USER'])),
                                     n)
        attropl[2].value = subJob.out_file
        attropl[3].value = subJob.err_file
        jobId = pbs.pbs_submit(torqueConnection,
                               attropl,
                               subFileName,
                               'default',
                               'NULL')
        # Check for any error.
        (err, errorMessage) = pbs.error()
        if(err):
            raise(TorqueSubmitError('Error %d: %s' %(err, errorMessage)))
        # <-- end if

        # Clean up.
        if(not err and removeJobFile):
            os.remove(subFileName)
        # <-- end if
        
        # Add the subjob id to the list of subjob ids.
        subJobIds.append(jobId)
    # <-- enf for
    
    # Wait for all the sub-jobs to finish, unless async=False.
    if(not async):
        runningJobs = subJobIds
        while(runningJobs):
            time.sleep(POLLING_INTERVAL)
            runningJobs = [job for job in runningJobs \
                           if not hasCompleted(job, log)]
        # <-- end while
    # <-- end if
    
    # Clean up.
    # if(not err and removeLog):
    #     os.remove(log)

    # Disconnect from the scheduler.
    pbs.pbs_disconnect(torqueConnection)
    return(subJobIds)


def hasCompleted(jobId=None, jobName=None,
                 schedulerHost=TORQUE_SCHEDULER):
    """
    Check to see if the specified job has completed.
    """
    # If we discover that jobs stay in queued status (i.e. 'Q' status) for
    # too long, we could implement some remediation strategy here.
    if(schedulerHost):
        q = PBSQuery.PBSQuery(schedulerHost)
    else:
        q = PBSQuery.PBSQuery()
    # <-- end if
    return(not jobId in q.getjobs().keys())


def getJobIds(name_root, owner, schedulerHost=TORQUE_SCHEDULER):
    if(schedulerHost):
        q = PBSQuery.PBSQuery(schedulerHost)
    else:
        q = PBSQuery.PBSQuery()
    # <-- end if
    jobs = q.getjobs()
    return([_id for (_id, job) in jobs.items() \
            if job['Job_Name'].startswith(name_root) and \
               job['Job_Owner'].startswith(owner + '@')])


def status(schedulerHost=TORQUE_SCHEDULER):
    """
    Return the status of the Torque pool. The status is returned as a dictionary
    whose keys are node names and values are states. Torque states are
      Unclaimed
      Owner

    @return status dictionary {machine_name: state, }
    """
    state = {1: 'Unclaimed', 0: 'Owner'}
    
    if(schedulerHost):
        q = PBSQuery.PBSQuery(schedulerHost)
    else:
        q = PBSQuery.PBSQuery()
    # <-- end if
    nodes = query.getnodes()
    return(dict([(name, state[n.is_free()]) for (name, n) in nodes.items()]))


def available(schedulerHost=TORQUE_SCHEDULER):
    """
    Convenience function to return the list of available nodes.

    @return list of available node names.
    """
    if(schedulerHost):
        q = PBSQuery.PBSQuery(schedulerHost)
    else:
        q = PBSQuery.PBSQuery()
    # <-- end if
    nodes = query.getnodes()
    return([name for (name, n) in nodes.items() if n.is_free()])
    

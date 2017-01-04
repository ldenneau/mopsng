
"""
This modules contains objects that make it simple for the user to 
create python scripts that build Torque/PBS/OpenPBS jobs description files.
"""

__author__ = 'Francesco Pierfederici <fpierfed@lsst.org>'
__date__ = '$Date: 2007/05/30 22:27:00 $'
__version__ = '$Revision: 0.1 $'[11:-2]

import os
import os.path

import MOPS.Condor as Condor




class TorqueSubmitError(Exception):
    pass


# Constants
TRANSLATION = {
    'getenv': 'getenv',
    'environment': 'env',
    'should_transfer_files': 'transfer_files',
    'when_to_transfer_output': None,
    'transfer_input_files': None,
    'initialdir': 'initial_dir',
    'error': 'err_file',
    'output': 'out_file',
    'input': 'in_file',
    'universe': None,
    'executable': 'executable',
    'log': 'log_file',
    'notification': 'notification',
    'queue': 'queue',
    }


def fromCondorJobFile(inst, fileName):
    """
    Parse the input Condor job file and return a Torque Job instance.

    Parsing
    Normally a line has the form attribute = value and the corresponding Job
    instance variable is called attribute. The only exceptions are
    - getenv => job.getenv
    - environment => job.env
    - should_transfer_files => job.transfer_files
    - when_to_transfer_output => None
    - transfer_input_files => None
    - initialdir => job.initial_dir
    - error => job.err_file
    - output => job.out_file
    - input => job.in_file
    - log => job.log_file
    - arguments => job.options, job.short_options, job.arguments

    From the list above, it is clear that the only tab that needs special
    handling is 'arguments'. That needs to be parsed into up to two
    dictionaries and a list:
    - job.options: all the options of the form --key val and --key
    - job.short_options: all the options of the form -key val and -key
    - job.arguments: a list of simple arguments.
    
    Remember that the key = val syntax is broken by the queue directive in
    that queue does not need the = sign: queue val.
    """
    # Create an empty Torque job.
    job = Job(inst, universe='', executable='', queue=0)
    
    # Read the file, line by line: Condor job description files do not have
    # multi-line entries. Comments start with a '#' and can be ignored.
    f = file(fileName)
    line = f.readline()
    while(line):
        job._parseCondorLineAndAgument(line)
        line = f.readline()
    # <-- end while
    f.close()
    
    # Make sure that job.queue is an int.
    if(job.queue):
        job.queue = int(job.queue)
    # <-- end if
    return(job)



class Job(object):
    """
    Generic Torque job class. Provides methods to set the options in the
    Torque submit file for a particular executable
    """
    def __init__(self, inst, universe=None, executable=None, queue=None):
        """
        @parpam inst: MOPS instance object
        @param universe: the Torque universe to run the job in.
               Only supported value is vanilla
        @param executable: the executable to run.
        @param queue: number of jobs to queue.
        """
        self.torque_resources = {}
        self.universe = universe
        self.executable = executable
        self.queue = queue

        self.transfer_files = False
        self.getenv = False
        self.initial_dir = None

        # These are set by methods in the class
        self.options = {}
        self.short_options = {}
        self.arguments = []
        self.env = []
        self.condor_cmds = {}
#        self.notification = None
        self.notification = 'Error'
        self.log_file = os.path.join(inst.getEnvironment('LOGDIR'), 'condor.log')
        self.err_file = '/dev/null'   # STDERR
        self.in_file = '/dev/null'    # STDIN
        self.out_file = '/dev/null'   # STDOUT
        self.sub_file_path = None
        self.output_files = []
        self.input_files = []

        # Since this is MOPS.Job, perform some MOPS setup.
        self._setupEnvironment(inst)
        return

    def _setupEnvironment(self, inst, hasMopsUserAccount=True):
        """
        Setup the job environment in order to be able to run MOPS jobs on the
        compute nodes.

        @parpam inst: MOPS instance object

        The environment is taken from the cluster.cf config file. If some env
        variables are missing in the cobnfig file, then they are taken from the
        current shell environment.
        """
        env = {'MOPS_DBINSTANCE': inst.dbname}

        # Extract as many of the relevant environment variables as possible from
        # cluster.cf
        config = inst.getConfig()['cluster']

        # MOPS_HOME
        env['MOPS_HOME'] = config.get('MOPS_HOME', os.environ['MOPS_HOME'])

        # PATH
        env['PATH'] = config.get('PATH', os.environ['PATH'])

        # PERL5LIB
        # Might not be defined in the shell: double fallback solution.
        env['PERL5LIB'] = config.get('PERL5LIB',
             os.environ.get('PERL5LIB',
                            os.path.join(env['MOPS_HOME'],
                                         'lib',
                                         'perl5')))
            
        # PYTHONPATH
        # Might not be defined in the shell: double fallback solution.
        env['PYTHONPATH'] = config.get('PYTHONPATH',
             os.environ.get('PYTHONPATH',
                            os.path.join(env['MOPS_HOME'],
                                         'lib',
                                         'python')))
            
        # LD_LIBRARY_PATH
        # Might not be defined in the shell: double fallback solution.
        env['LD_LIBRARY_PATH'] = config.get('LD_LIBRARY_PATH',
             os.environ.get('LD_LIBRARY_PATH',
                            os.path.join(env['MOPS_HOME'],
                                         'lib')))
        
        # CAET_DATA
        # Might not be defined in the shell: double fallback solution.
        env['CAET_DATA'] = config.get('CAET_DATA',
             os.environ.get('CAET_DATA', '/home/mops/MOPS_DATA/caet_data'))
        
        # OORB_DATA
        # Might not be defined in the shell: double fallback solution.
        env['OORB_DATA'] = config.get('OORB_DATA',
             os.environ.get('OORB_DATA', '/home/mops/MOPS_DATA/oorb'))
        
        # ORBFIT_DATA
        # Might not be defined in the shell: double fallback solution.
        env['ORBFIT_DATA'] = config.get('ORBFIT_DATA',
             os.environ.get('ORBFIT_DATA', '/home/mops/MOPS_DATA/orbfit'))

        # huh
        env['PYTHON_EGG_CACHE'] = '/tmp'
        
        # Putting everything together...
        for key in env.keys():
            self.add_env('%s=%s' %(key, env[key]))
        # <-- end for    
        return

    def get_executable(self):
        """
        Return the name of the executable for this job.
        """
        return self.executable

    def add_condor_cmd(self, cmd, value):
        """
        Add a Condor command to the submit file (e.g. a class add or
        evironment).
        @param cmd: Condor command directive.
        @param value: value for command.
        """
        self.condor_cmds[cmd] = value

    def add_input_file(self, filename):
        """
        Add filename as a necessary input file for this DAG node.

        @param filename: input filename to add
        """
        self.input_files.append(filename)

    def add_output_file(self, filename):
        """
        Add filename as a output file for this DAG node.

        @param filename: output filename to add
        """
        self.output_files.append(filename)

    def get_input_files(self):
        """
        Return list of input files for this DAG node.
        """
        return self.input_files

    def get_output_files(self):
        """
        Return list of output files for this DAG node.
        """
        return self.output_files

    def add_arg(self, arg):
        """
        Add an argument to the executable. Arguments are appended after any
        options and their order is guaranteed.
        @param arg: argument to add.
        """
        self.arguments.append(arg)

    def add_file_arg(self, file):
        """
        Add a file argument to the executable. Arguments are appended after any
        options and their order is guaranteed. Also adds the file name to the
        list of required input data for this job.
        @param file: file to add as argument.
        """
        self.arguments.append(file)
        self.input_files.append(file)

    def get_args(self):
        """
        Return the list of arguments that are to be passed to the executable.
        """
        return self.arguments

    def add_opt(self, opt, value):
        """
        Add a command line option to the executable. The order that the
        arguments will be appended to the command line is not guaranteed, but
        they will always be added before any command line arguments. The name of
        the option is prefixed with double hyphen and the program is expected to
        parse it with getopt_long().
        @param opt: command line option to add.
        @param value: value to pass to the option (None for no argument).
        """
        self.options[opt] = value

    def add_file_opt(self, opt, file):
        """
        Add a command line option to the executable. The order that the
        arguments will be appended to the command line is not guaranteed, but
        they will always be added before any command line arguments. The name of
        the option is prefixed with double hyphen and the program is expected to
        parse it with getopt_long().
        @param opt: command line option to add.
        @param value: value to pass to the option (None for no argument).
        """
        self.options[opt] = file
        self.input_files.append(file)

    def get_opts(self):
        """
        Return the dictionary of opts for the job.
        """
        return self.options

    def add_short_opt(self, opt, value):
        """
        Add a command line option to the executable. The order that the
        arguments will be appended to the command line is not guaranteed, but
        they will always be added before any command line arguments. The name of
        the option is prefixed with single hyphen and the program is expected to
        parse it with getopt() or getopt_long() (if a single character option),
        or getopt_long_only() (if multiple characters).  Long and
        (single-character) short options may be mixed if the executable permits
        this.
        @param opt: command line option to add.
        @param value: value to pass to the option (None for no argument).
        """
        self.short_options[opt] = value

    def get_short_opts(self):
        """
        Return the dictionary of short options for the job.
        """
        return self.short_options

    def add_ini_opts(self, cp, section):
        """
        Parse command line options from a given section in an ini file and
        pass to the executable.
        @param cp: ConfigParser object pointing to the ini file.
        @param section: section of the ini file to add to the options.
        """
        for opt in cp.options(section):
            arg = string.strip(cp.get(section,opt))
            self.options[opt] = arg

    def add_env(self, env):
        """
        Add an environment variable and its value to the job description.
        @param env: string: 'key=val'
        """
        self.env.append(env)
            
    def set_notification(self, value):
        """
        Set the email address to send notification to.
        @param value: email address or never for no notification.
        """
        self.notification = value

    def set_log_file(self, path):
        """
        Set the Condor log file.
        @param path: path to log file.
        """
        self.log_file = path

    def get_log_file(self):
        """
        Get the Condor log file.
        """
        return(self.log_file)

    def set_stderr_file(self, path):
        """
        Set the file to which Condor directs the stderr of the job.
        @param path: path to stderr file.
        """
        self.err_file = path

    def get_stderr_file(self):
        """
        Get the file to which Condor directs the stderr of the job.
        """
        return self.err_file

    def set_stdout_file(self, path):
        """
        Set the file to which Condor directs the stdout of the job.
        @param path: path to stdout file.
        """
        self.out_file = path

    def get_stdout_file(self):
        """
        Get the file to which Condor directs the stdout of the job.
        """
        return self.out_file

    def set_sub_file(self, path):
        """
        Set the name of the file to write the Condor submit file to when
        write_sub_file() is called.
        @param path: path to submit file.
        """
        self.sub_file_path = path

    def get_sub_file(self):
        """
        Get the name of the file which the Condor submit file will be
        written to when write_sub_file() is called.
        """
        return self.sub_file_path

    def add_torque_res(self, name, value):
        """
        Add a Torque command to the submit file (i.e. a resource list).
        @param name: resource name.
        @param value: value for resource.
        """
        self.torque_resources[res] = value

    def write_sub_file(self):
        """
        Write a submit file for this Torque job.
        """        
        if not self.log_file:
            raise TorqueSubmitError, "Log file not specified."
        if not self.err_file:
            raise TorqueSubmitError, "Error file not specified."
        if not self.out_file:
            raise TorqueSubmitError, "Output file not specified."

        if not self.sub_file_path:
            raise TorqueSubmitError, 'No path for submit file.'
        try:
            subfile = open(self.sub_file_path, 'w')
        except:
            raise TorqueSubmitError, "Cannot open file " + self.sub_file_path

        # First off write the PBS directives.
        if(self.getenv):
            subfile.write('#PBS -V\n')
        
        if(self.env):
            envStr= '#PBS -v '
            for env in self.env:
                envStr += env + ','
            envStr = envStr[:-1].strip() + '\n'
            subfile.write(envStr)
        # <-- end if

        if self.notification:
            subfile.write( '#PBS -m ae\n' )
        else:
            subfile.write( '#PBS -m n\n' )
        # <-- end if

        resStr = ''
        if(self.torque_resources):
            resStr += '#PBS -l '
        for res in self.torque_resources.keys():
            resStr += res + "=" + self.condor_cmds[cmd] + ','
        if(resStr):
            subfile.write( resStr[:-1] + '\n' )

        subfile.write( '#PBS -e ' + self.err_file + '\n' )
        subfile.write( '#PBS -o ' + self.out_file + '\n' )

        
        # Then the commands.
        if(self.initial_dir):
            subfile.write('cd %s\n' %(self.initial_dir))
        
        # subfile.write( 'universe = ' + self.universe + '\n' )
        exeStr = self.get_executable()

        if self.options.keys() or self.short_options.keys() or \
               self.arguments:
            for c in self.options.keys():
                if self.options[c]:
                    exeStr +=  ' --' + c + ' ' + self.options[c]
                else:
                    exeStr += ' --' + c
            for c in self.short_options.keys():
                if self.short_options[c]:
                    exeStr += ' -' + c + ' ' + self.short_options[c]
                else:
                    exeStr += ' -' + c
            for c in self.arguments:
                exeStr += ' ' + c
        # <-- end if
        if(self.in_file and self.in_file != '/dev/null'):
            exeStr += ' < ' + self.in_file
        # <-- end if
        subfile.write( exeStr + '\n' )

        # if(self.transfer_files):
        #     subfile.write('should_transfer_files = YES\n')
        #     subfile.write('when_to_transfer_output = ON_EXIT\n')
        #     if(self.input_files):
        #         fileStr = 'transfer_input_files = '
        #         for f in self.input_files:
        #             fileStr += f + ','
        #         fileStr = fileStr[:-1] + '\n'
        #         subfile.write(fileStr)
        #     # <-- end if
        # # <-- end if
        
        # subfile.write( 'log = ' + self.log_file + '\n' )
        subfile.close()
        return
    

    def _parseCondorLineAndAgument(self, line):
        """
        Parse the input line (coming from a Condor job description) and, if
        applicable set the relevant instance variable of the input job instance.
        """
        # Skip comments.
        line = line.strip()
        if(line.startswith('#') or not line):
            return
        # <-- end if

        # Replace any occurrence of $(Process) with $(PBS_ARRAYID).
        line = line.replace('$(Process)', '$(PBS_ARRAYID)')

        # Split the line.
        # The queue command does not have an '=' sign.
        if(line.startswith('queue')):
            attr, val = [x.strip() for x in line.split(' ', 1)]
        else:
            attr, val = [x.strip() for x in line.split('=', 1)]
        # <-- end if

        # We need to be careful with 'environment'.
        if(attr == 'environment'):
            val = self._parseEnvironment(val)
        
        # Translate it.
        if(attr in TRANSLATION.keys() and TRANSLATION[attr] != None):
            return(setattr(self, TRANSLATION[attr], val))
        # <-- end if

        # Now we are left with the fun part.
        if(attr == 'arguments'):
            self._parseArgumentsAndAgument(val)
        # <-- end if

        # All the rest can be ignored.
        print('ignored line "%s"' %(line))
        return

    
    def _parseEnvironment(self, line):
        # Remove quotes, if necessary.
        if(line.startswith('"') or line.startswith("'")):
            line = line[1:]
        # <-- end if
        if(line.endswith('"') or line.endswith("'")):
            line = line[:-1]
        # <-- end if

        # Split on spaces.
        return(line.split())

    def _parseArgumentsAndAgument(self, line):
        """
        Parse the value of the keyword 'arguments' and populate the two
        dictionaries self.options and self.short_options as well as the list
        self.arguments as appropriate.
        """
        # For now, simply drop the whole thing in self.arguments[0] ;-)
        self.arguments = [line, ]
        return

"""
This modules contains objects that make it simple for the user to 
create python scripts that build Condor DAGs to run code on the LSC
Data Grid.
"""

__author__ = 'Duncan Brown <duncan@gravity.phys.uwm.edu>'
__date__ = '$Date: 2007/05/30 22:27:00 $'
__version__ = '$Revision: 1.114 $'[11:-2]

import os
import os.path


class Job(object):
    """
    Generic condor job class. Provides methods to set the options in the
    condor submit file for a particular executable
    """
    def __init__(self, inst, universe, executable, queue):
        """
        @parpam inst: MOPS instance object
        @param universe: the condor universe to run the job in.
        @param executable: the executable to run.
        @param queue: number of jobs to queue.
        """
        self.__universe = universe
        self.__executable = executable
        self.__queue = queue

        self.transfer_files = False
        self.getenv = False
        self.initial_dir = None

        # These are set by methods in the class
        self.__options = {}
        self.__short_options = {}
        self.__arguments = []
        self.__env = []
        self.__condor_cmds = {}
#        self.__notification = None
        self.__notification = 'Error'
        self.__log_file = os.path.join(inst.getEnvironment('LOGDIR'), 'condor.log')
        self.__err_file = '/dev/null'   # STDERR
        self.__in_file = '/dev/null'    # STDIN
        self.__out_file = '/dev/null'   # STDOUT
        self.__sub_file_path = None
        self.__output_files = []
        self.__input_files = []

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
             os.environ.get('OORB_DATA', '/home/mops/MOPS_DATA/oorb_data'))
        
        # ORBFIT_DATA
        # Might not be defined in the shell: double fallback solution.
        env['ORBFIT_DATA'] = config.get('ORBFIT_DATA',
             os.environ.get('ORBFIT_DATA', '/home/mops/MOPS_DATA/orbfit_data'))

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
        return self.__executable

    def add_condor_cmd(self, cmd, value):
        """
        Add a Condor command to the submit file (e.g. a class add or
        evironment).
        @param cmd: Condor command directive.
        @param value: value for command.
        """
        self.__condor_cmds[cmd] = value

    def add_input_file(self, filename):
        """
        Add filename as a necessary input file for this DAG node.

        @param filename: input filename to add
        """
        self.__input_files.append(filename)

    def add_output_file(self, filename):
        """
        Add filename as a output file for this DAG node.

        @param filename: output filename to add
        """
        self.__output_files.append(filename)

    def get_input_files(self):
        """
        Return list of input files for this DAG node.
        """
        return self.__input_files

    def get_output_files(self):
        """
        Return list of output files for this DAG node.
        """
        return self.__output_files

    def add_arg(self, arg):
        """
        Add an argument to the executable. Arguments are appended after any
        options and their order is guaranteed.
        @param arg: argument to add.
        """
        self.__arguments.append(arg)

    def add_file_arg(self, file):
        """
        Add a file argument to the executable. Arguments are appended after any
        options and their order is guaranteed. Also adds the file name to the
        list of required input data for this job.
        @param file: file to add as argument.
        """
        self.__arguments.append(file)
        self.__input_files.append(file)

    def get_args(self):
        """
        Return the list of arguments that are to be passed to the executable.
        """
        return self.__arguments

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
        self.__options[opt] = value

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
        self.__options[opt] = file
        self.__input_files.append(file)

    def get_opts(self):
        """
        Return the dictionary of opts for the job.
        """
        return self.__options

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
        self.__short_options[opt] = value

    def get_short_opts(self):
        """
        Return the dictionary of short options for the job.
        """
        return self.__short_options

    def add_ini_opts(self, cp, section):
        """
        Parse command line options from a given section in an ini file and
        pass to the executable.
        @param cp: ConfigParser object pointing to the ini file.
        @param section: section of the ini file to add to the options.
        """
        for opt in cp.options(section):
            arg = string.strip(cp.get(section,opt))
            self.__options[opt] = arg

    def add_env(self, env):
        """
        Add an environment variable and its value to the job description.
        @param env: string: 'key=val'
        """
        self.__env.append(env)
            
    def set_notification(self, value):
        """
        Set the email address to send notification to.
        @param value: email address or never for no notification.
        """
        self.__notification = value

    def set_log_file(self, path):
        """
        Set the Condor log file.
        @param path: path to log file.
        """
        self.__log_file = path

    def get_log_file(self):
        """
        Get the Condor log file.
        """
        return(self.__log_file)

    def set_stderr_file(self, path):
        """
        Set the file to which Condor directs the stderr of the job.
        @param path: path to stderr file.
        """
        self.__err_file = path

    def get_stderr_file(self):
        """
        Get the file to which Condor directs the stderr of the job.
        """
        return self.__err_file

    def set_stdout_file(self, path):
        """
        Set the file to which Condor directs the stdout of the job.
        @param path: path to stdout file.
        """
        self.__out_file = path

    def get_stdout_file(self):
        """
        Get the file to which Condor directs the stdout of the job.
        """
        return self.__out_file

    def set_sub_file(self, path):
        """
        Set the name of the file to write the Condor submit file to when
        write_sub_file() is called.
        @param path: path to submit file.
        """
        self.__sub_file_path = path

    def get_sub_file(self):
        """
        Get the name of the file which the Condor submit file will be
        written to when write_sub_file() is called.
        """
        return self.__sub_file_path

    def write_sub_file(self):
        """
        Write a submit file for this Condor job.
        """
        if not self.__log_file:
            raise CondorSubmitError, "Log file not specified."
        if not self.__err_file:
            raise CondorSubmitError, "Error file not specified."
        if not self.__out_file:
            raise CondorSubmitError, "Output file not specified."

        if not self.__sub_file_path:
            raise CondorSubmitError, 'No path for submit file.'
        try:
            subfile = open(self.__sub_file_path, 'w')
        except:
            raise CondorSubmitError, "Cannot open file " + self.__sub_file_path

        subfile.write( 'universe = ' + self.__universe + '\n' )
        subfile.write( 'executable = ' + self.__executable + '\n' )

        if self.__options.keys() or self.__short_options.keys() or \
               self.__arguments:
            subfile.write( 'arguments =' )
            for c in self.__options.keys():
                if self.__options[c]:
                    subfile.write( ' --' + c + ' ' + str(self.__options[c]) )
                else:
                    subfile.write( ' --' + c )
            for c in self.__short_options.keys():
                if self.__short_options[c]:
                    subfile.write( ' -' + c + ' ' + str(self.__short_options[c]) )
                else:
                    subfile.write( ' -' + c )
            for c in self.__arguments:
                subfile.write( ' ' + c )
            subfile.write( '\n' )

        for cmd in self.__condor_cmds.keys():
            subfile.write( cmd + " = " + self.__condor_cmds[cmd] + '\n' )

        if(self.getenv):
            subfile.write('getenv = True\n')
        
        if(self.__env):
            envStr= 'environment = "'
            for env in self.__env:
                envStr += env + ' '
            envStr = envStr.strip() + '"\n'
            subfile.write(envStr)
        # <-- end if

        if(self.transfer_files):
            subfile.write('should_transfer_files = YES\n')
            subfile.write('when_to_transfer_output = ON_EXIT\n')
            if(self.__input_files):
                fileStr = 'transfer_input_files = '
                for f in self.__input_files:
                    fileStr += f + ','
                fileStr = fileStr[:-1] + '\n'
                subfile.write(fileStr)
            # <-- end if
        # <-- end if

        if(self.initial_dir):
            subfile.write('initialdir = %s\n' %(self.initial_dir))
        
        subfile.write( 'log = ' + self.__log_file + '\n' )
        subfile.write( 'error = ' + self.__err_file + '\n' )
        subfile.write( 'output = ' + self.__out_file + '\n' )
        subfile.write( 'input = ' + self.__in_file + '\n' )
        if self.__notification:
            subfile.write( 'notification = ' + self.__notification + '\n' )
        subfile.write( 'queue ' + str(self.__queue) + '\n' )

        subfile.close()

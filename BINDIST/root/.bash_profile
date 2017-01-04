# .bashrc for MOPS

# Set up MOPS ENV.
export MOPS_HOME=$HOME
export MOPS_DBINSTANCE=psmops_test
export PERL5LIB=$MOPS_HOME/lib/perl5
export PATH=$MOPS_HOME/bin:$PATH
export CAET_DATA=$MOPS_HOME/data/caet_data
export LD_LIBRARY_PATH=$MOPS_HOME/lib

# Optional.  Shows current database name in bash prompt.
export PS1="[\u@\h (DEV/$MOPS_DBINSTANCE)]$ "

#!/bin/bash

DEBUG=0

HASH_PREFIX="__HASH_"

project=$(basename "${PWD}")
if [ -z "$project" ]; then
    echo "ERROR"
    exit 1
fi


# Hash implementation, for a *single* global hash.

# These functions use the global namespace and indirect variable
# expansion to simulate one.

# keys should *not* have spaces, or else the list of hash keys will break.
declare -a _HASHKEYS

hash_valitems() {
    local key
    local key2
    for key in ${_HASHKEYS[@]}; do
	key2=${HASH_PREFIX}${key}
        echo ${!key2} $key
    done
}

set_hash() {
    local key=${HASH_PREFIX}${1}
    if [ -z ${!key} ]; then _HASHKEYS[${#_HASHKEYS[@]}]=$1; fi
    export $key=$2
}


# Debugging help.
myerr() {
    echo "ERROR"
    if [ "$DEBUG" != 0 ]; then echo $* > /tmp/get_recursive_sibs.err; fi
    echo $* 1>&2
}

mymesg() {
    echo $* 1>&2
}



# Function to check that project and project/Makefile can be found.
check_project() {
    local project="$1"
    if [ ! -f "$project"/Makefile ]; then
	# check directory too
	if [ ! -d "$project" ]; then
	    myerr "Unable to find project '$projectakefile'"
	    exit 1
	fi

        myerr "Unable to find project makefile '$project/Makefile'"
        exit 1
    fi
}

# Function to check out the project and check that project/Makefile
# can be found.
checkout_project() {
    local project="$1"

    if [ "$DEBUG" -ne "0" ]; then
        myerr "Checking for project directory ../'$project'"
    fi

    # Check directory.
    if [ ! -d ../"$project" ]; then
        # Checkout dirlib returns 0 on success.  OLDPWD appears correct
        # for the The Open Group Base Specifications Issue 6,
        # IEEE Std 1003.1, 2004 Edition.  The difference between
        # cd "$OLDPWD" and cd - is that the latter emits output even
        # on success.

        # Note: this is now the only place CVS is called. I have removed it 
        # from Make.common.  -- agoode
        mymesg "Checking out '$project'"
        cd .. && cvs co "$project" > /dev/null 2>&1 && cd "$OLDPWD"
        local result="$?"

        if [ "$result" -ne 0 ]; then
            myerr "Unable to localize project '$project'"
            exit 1
        fi
    fi

    # verify all
    check_project ../"$project"
}

# Recursive function.
grs() {
    # Prase and check project.
    local project="$1"
    local prank="$2"
    local parent="$3"

    if [ "$DEBUG" -ne "0" ]; then
        myerr "grs: called on project '$project' from parent '$parent'"
    fi

    # Check that project exists.
    checkout_project "$project"

    # If the project has not been seen, or the project has a smaller rank
    # thank its parents, we need to update the project and its children.
    local rank
    local key=${HASH_PREFIX}${project}
    if [ -z "${!key}" ]; then rank=-1
    else rank=${!key}
    fi

    if [ "$rank" -le "$prank" ]; then
        local newrank=$(($prank + 1))

        # Store new rank, unless parent is 'none' (project is toplevel).
        if [ "$parent" != "none" ]; then set_hash "$project" $newrank; fi

        # Recurse over sibs.  INSERT CHECK FOR CIRCULAR DEPS
        local sibs=$(get_simple_sibs ../"$project")
        local sib
        for sib in $sibs; do
            # Sanity check: parent and project are not the same.
            if [ "$parent" = "$sib" ]; then
                myerr "grs: Error: Project '$project' has itself as a sibling."
                exit 1
            fi

            grs "$sib" $newrank "$project"
        done
    fi
}

get_recursive_sibs() {
    local project="$1"
    grs "$project" 0 "none"
}

get_simple_sibs() {
    local project="$1"
    check_project $project

    # Get and print sibs on a single line.
    # NOTE: The call to "make", below, inherits options from any make
    #       ancestors.
    #       This is why we must add --no-print_directory, to defeat a -w passed
    #       from an ancestor (through the MAKEFLAGS environment variable).
    ( echo '# project is \"$project\"'; \
	fgrep -v Make.common "$project"/Makefile; \
	echo 'sibs:'; \
	echo '	@echo $(siblings)' ) \
	| "${MAKE:-make}"  --no-print-directory -s -f - sibs
}

link_order_sibs() {
    hash_valitems | sort -n | cut -f2 -d' '
}

# Run
get_recursive_sibs "$project"
if [ "$DEBUG" -ne 0 ]; then hash_valitems; fi
result=$(link_order_sibs)

# Done
echo $result
exit 0

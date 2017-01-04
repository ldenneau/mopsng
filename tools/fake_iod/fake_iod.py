#!/usr/bin/env python
"""
Fake IOD.

Usage
    fake_iod.py Tracks_file

    Input
        Tracks_file

    Output
        Orbits (written to STDOUT)

Description
    
"""
import os
import sys

import MySQLdb as dbi



# Constants
DB_USER = 'mops'
DB_PASSWD = 'mops'
DB_HOST = 'mopsdc.ifa.hawaii.edu'
RESIDUALS = 0.001



class Conection(object):
    _cursor = None
    _user = None
    _passwd = None
    _host = None
    _port = None
    _db = None

    @classmethod
    def connect(cls, user, passwd, db, host='localhost', port=None):
        """
        Class method: re-use the same cursor all the time. Return the cursor,
        *not* the connection obect.
        """
        if(not cls._cursor or
           not user == cls._user or
           not passwd == cls._passwd or
           not db == cls._db or
           not host == cls._host or
           not port == cls._port):
            # First connection of this type. Raise exception if this fails.
            if(port):
                connection = dbi.connect(user=user,
                                         passwd=passwd,
                                         host=host,
                                         port=port,
                                         db=db)
            else:
                connection = dbi.connect(user=user,
                                         passwd=passwd,
                                         host=host,
                                         db=db)
            cursor = connection.cursor()
            cls._user=user
            cls._passwd=passwd
            cls._host=host
            cls._port=port
            cls._db=db
            cls._cursor = cursor
        # <-- end if
        return(cls._cursor)


def doIOD(tracks, instance):
    """
    Given a tracks file and a MOPS DB instance name, compute the IODs.
    """
    # Get a cursor from the DB connection.
    cursor = Conection.connect(DB_USER, DB_PASSWD, instance, DB_HOST)
    
    # Read the tracks file.
    tracks = file(tracks).readlines()
    if(not tracks):
        return
    
    # Loop through the list of tracks and extract the names of the SSM objects 
    # we want IODs for. Associate SSM name to orbit ID as given in the input
    # file.
    mapping = {}                                            # SSM name: orbit ID
    referenceID = None
    referenceName = None
    cluster = []                           # set ot tracks with the same orbitID
    for track in tracks:
        tokens = track.strip().split()
        if(len(tokens) != 7):
            # Malformed line or missing SSM name.
            # print('fake_iod.py: malformed input line (%s)' % (track.strip()))
            continue
        orbitID = tokens[0]
        objName = tokens[-1]
        
        
        # Make sure that the track is not mixed. This is equivalent of asking 
        # that all the entries with the same orbitID also have the same objName.
        if(referenceID == None):
            referenceID = orbitID
            referenceName = objName
        
        if(orbitID == referenceID and objName != referenceName):
            # Mixed cluster: invalidate referenceName.
            referenceName = None
            continue
        elif(orbitID != referenceID and referenceName == None):
            # New cluster. It was a mixed (referenceName == None). Skip it.
            # Update referenceID and referenceName.
            referenceID = orbitID
            referenceName = objName
            continue
        elif(orbitID != referenceID and referenceName != None):
            # New cluster. If referenceName != None (meaning mixed cluster), 
            # save this cluster.
            mapping[referenceName] = referenceID
            
            # Update referenceID and referenceName.
            referenceID = orbitID
            referenceName = objName
        # <-- end if
    # <-- end for
    
    # Fetch the last cluster.
    if(referenceName != None):
        # New cluster. If referenceName != None (meaning mixed cluster), 
        # save this cluster.
        mapping[objName] = orbitID
    
    # Now we should have all the unique SSM object names and the corresponding
    # orbit IDs. Fetch the orbit information from the database and print them 
    # out to STDOUT.
    objNames = mapping.keys()
    if(not objNames):                                # This should never happen.
        # print('********** fake_iod.py: objNames = None should NEVER happen!!')
        return
    objNames.sort()
    sql =  'select object_name, q, e, i, node, arg_peri, time_peri, epoch, h_v '
    sql += 'from ssm ' 
    if(len(objNames) > 1):
        sql += 'where object_name in (%s) ' \
               %(','.join([str(objName) for objName in objNames]))
    else:
        sql += 'where object_name = "%s" ' % (objNames[0])
    sql += 'order by object_name'
    
    try:
        nRes = cursor.execute(sql)
    except:
        print('**********************************************************')
        print('**********************************************************')
        print('**********************************************************')
        print('                      FAKE IOD FAILED                     ')
        print(sql)
        print('**********************************************************')
        print('**********************************************************')
        print('**********************************************************')
        return
    if(nRes):
        for (objName, 
             q, 
             e, 
             i, 
             node, 
             arg_peri, 
             time_peri, 
             epoch, 
             h_v) in cursor.fetchall():
            print(' '.join(('MIF-O',
                            str(mapping[objName]),
                            str(q),
                            str(e),
                            str(i),
                            str(node),
                            str(arg_peri),
                            str(time_peri),
                            str(epoch),
                            str(h_v),
                            str(RESIDUALS),
                            'undef',
                            'undef',
                            'I')))
    return

if(__name__ == '__main__'):
    import optparse


    # Get user input (tracks file) and make sure that it exists.
    parser = optparse.OptionParser('usage: fake_iod.py tracks_file')
    (options, args) = parser.parse_args()
    if(len(args) != 1):
        parser.error('incorrect number of arguments')

    tracksFile = args[0]
    if(not os.path.isfile(tracksFile)):
        parser.error('%s: no such file or directory' % (tracksFile))
    
    # Determine the name of MOPS DB instance.
    try:
        instance = os.environ['MOPS_DBINSTANCE']
    except:
        parser.error('MOPS_DBINSTANCE has to be defined.')
        
    # Invoke the fake IOD code
    doIOD(tracksFile, instance)

    # Cleanup and exit.
# <-- end if




















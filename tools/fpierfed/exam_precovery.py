#!/usr/bin/env python
"""
exam_precovery.py

Examine the progress of the precovery pipeine.

Given the name of a MOPS run (i.e. $MOPS_DBINSTANCE), extract the list of all
known orbits associated to Solar System Model (SSM) objects. For each SSM object 
check and see if all the corresponding detections/tracklets have been associated
to its orbit.

Report the number of fully identified/precovered SSM objects as a fraction of 
the total number of SSM objects in that MOPS run.

For the non fully precovered SSM objects, list them with the percentage of 
tracklets that were identified/precovered.
"""
import MySQLdb as dbi



# Constants
DB_USER = 'mops'
DB_PASSWD = 'mops'
DB_HOST = 'schmopc01.ifa.hawaii.edu'

LIMITING_MAG = 24.




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


def objectTracklets(ssmObject, maxMJD, limitingMag=LIMITING_MAG):
    """
    Query the tracklets table for the total number of tracklets associated to 
    this SSM object with their status. The status tells whether the tracklet was
    not found ('U). We are only interested in clean tracklets whose detections 
    are brighter than LIMITING_MAG and that have been observerd before maxMJD.
    """
    # Get a cursor from the DB connection.
    cursor = Conection.connect(DB_USER, DB_PASSWD, instance, DB_HOST)
    
    # Select al the tracklets associated with ssmObject.
    # TODO: Is using ext_epoch right?
    sql = 'select tracklet_id, status from tracklets where '
    sql += 'ssm_id = %d and classification = "C" ' % (ssmObject)
    sql += 'and ext_epoch <= %f' % (maxMJD)
    nRes = cursor.execute(sql)
    if(not nRes):
        return
    tracklets = cursor.fetchall()
    
    # Now, for each tracklet make sure that all the corresponding detections 
    # are visible.
    visibleTracklets = []
    sql = 'select d.det_id from detections d, tracklet_attrib ta where '
    sql += 'ta.tracklet_id = %d and d.det_id = ta.det_id and '
    sql += 'd.mag > %f' %(limitingMag)
    for (tracklet_id, status) in tracklets:
        nRes = cursor.execute(sql %(tracklet_id))
        if(not nRes):
            visibleTracklets.append((tracklet_id, status))
        # <-- end if
    # <-- end for
    return(visibleTracklets)

def processObject(ssmObject, maxMJD):
    """
    Find the fraction of the total number of clean tracklets theoretically 
    associated with the given SSM object that MOPS actually associated with it.
    ssmObject is the ssm_id
    """
    percentFound = -1.
    
    # Get a cursor from the DB connection.
    cursor = Conection.connect(DB_USER, DB_PASSWD, instance, DB_HOST)
    
    # Fnd all the visible tracklets associated with the object. Each tracklet is
    # an array of the form [tracklet_id, status]. Status = 'U' means not found.
    tracklets = objectTracklets(ssmObject, maxMJD)
    if(not tracklets):
        return(100.)
    numTracklets = len(tracklets)
    numFound = 0
    for (tracklet_id, status) in tracklets:
        if(status != 'U'):
            numFound += 1
    percentFound = 100. * float(numFound) / float(numTracklets)
    return(percentFound)

def completedPrecoveryMaxDate(instance):
    """
    We need to fetch the list of non complete precovery job timestamps.
    There is a BIG ASSUMPTION here: namely the fact that the block of records 
    for which precovery_status = 'N' is CONTIGUOUS. We also assume that it
    expands to the maximum precovery_id. This is the default way MOPS operates,
    but there s no guarantee that a user might not have overridden the standard
    behaviour. Also return the corresponding precovery_date_mjd.
    """
    # Get a cursor from the DB connection.
    cursor = Conection.connect(DB_USER, DB_PASSWD, instance, DB_HOST)
    
    # Fetch the maximum MJD precovery has processed.
    sql = 'select max(epoch_mjd) from detections d, tracklet_attrib ta '
    sql += 'where d.det_id = ta.det_id and ta.tracklet_id in '
    sql += '(select tracklet_id from history_precoveries)'
    nRes = cursor.execute(sql)
    return(cursor.fetchone()[0])

def listObjects(instance):
    """
    Find the SSM orbits associated with the given instance.
    Also return the maximum MJD that the precovery pipeline is working on.
    """
    # Get a cursor from the DB connection.
    cursor = Conection.connect(DB_USER, DB_PASSWD, instance, DB_HOST)
    
    # Compose the SQL query to find all the orbits/SSM objects. We do this with 
    # a simle query to the derivedobjects table since we realy only need the
    # ssm_id values.
    maxMJD = completedPrecoveryMaxDate(instance)
    if(maxMJD == None):
        return([], None)
    
    sql = 'select distinct(ssm_id) from derivedobjects where ssm_id is not null'
    sql += ' and status = "I"'
    # sql += ' and updated >= "%s"' %(minModifiedDate)
    # <-- end if
    
    nRes = cursor.execute(sql)
    return([x[0] for x in cursor.fetchall()], float(maxMJD))






if(__name__ == '__main__'):
    import optparse
    import sys
    
    
    # Constants
    USAGE_STR = 'usage: exam_precovery.py instance'

    # Get user input (instance name).
    parser = optparse.OptionParser(USAGE_STR)
    (options, args) = parser.parse_args()
    if(len(args) != 1):
        parser.error('incorrect number of arguments')
    instance = args[0]
    
    # List orbits.
    ssmObjects, maxMJD = listObjects(instance)
    
    # Process orbits.
    effDict = {}                              # {percent precovered: [object, ]}
    i = 0
    for ssmObject in ssmObjects:
        efficiency = round(processObject(ssmObject, maxMJD), 2)
        if(effDict.has_key(efficiency)):
            effDict[efficiency].append(ssmObject)
        else:
            effDict[efficiency] = [ssmObject, ]
        # <-- end if
        # i += 1
        # if(i % 100 == 0):
        #     print('Processed %d objects.' %(i))
    # <-- end for
    
    # Print the summary.
    totObjects = len(ssmObjects)
    if(not totObjects):
        print('Precovery has not started yet. Please try again later.')
        sys.exit(0)
        
    print('Examined a total of %d SSM objects.' %(totObjects))
    efficiencies = effDict.keys()
    efficiencies.sort()
    totObjects = 100. / float(totObjects)
    n = 0
    for efficiency in efficiencies:
        percentObjects = round(float(len(effDict[efficiency])) * totObjects, 2)
        print('%.02f%% efficiency: %.02f%% SSM objects.' %(efficiency, 
                                                           percentObjects))
    # <-- end for
    
    # Print out the ssm_id of objects with less than 50% efficiency
    for efficiency in efficiencies:
        if(efficiency < 100.):
            print(efficiency, effDict[efficiency])
# <-- end if










#!/usr/bin/env python
import os

import MySQLdb as dbi




# Global                    # FIXME: OK, OK... one should never use globals...
SSM_KEY = {}                # SSM ID -> detection cluster mapping.


# Constants
DIR_PREFIX = 'jplorb'
DET_FILE = 'det'
OUT_FILE = 'out'
IOD_FILE = 'iod'
DB_USER = 'mops'
DB_PASSWD = 'mops'
DB_DB = 'psmops_simple_test5_noise'    # FIXME: grab this one from sys.args
DB_HOST = 'mopsdc'



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




def getFailedDetections(dirName):
    """
    Return an iterator yielding "clusters" of detections that failed the
    differential corrector but that should have passed it being associated to
    the same SSM object.

    The returned cluster has the corrsponding IOD as first element.
    """
    dataFiles = findFailedDataFiles(dirName)
    for detFile in dataFiles:
        detections = [line.strip().split() \
                      for line in file(detFile).readlines()]
        if(sameSSM(detections)):
            # detections are all associated with the same SSM.
            # Insert the IOD.
            iodFile = os.path.join(os.path.dirname(detFile), IOD_FILE)
            detections.insert(0, file(iodFile).readline().strip().split())
            yield(detections)
        # <-- end if
    # <-- end for
    # return (<-- just to be neat)

def findFailedDataFiles(dirName):
    """
    Data files of failed diffcor runs are easy to recognize: there a 0 length
    "out" file and a "det" file containing the individual detections. Then there
    is a "iod" with the initial orbit. These three files are in directories
    called "jplorb..."
    """
    files = []
    content = os.listdir(dirName)
    for entry in content:
        entryPath = os.path.join(dirName, entry)
        if(os.path.isdir(entryPath) and entry[:len(DIR_PREFIX)] == DIR_PREFIX):
            try:
                statInfo = os.lstat(os.path.join(entryPath, OUT_FILE))
                size = statInfo[6]
                if(size != 0):
                    continue
            except OSError:
                continue

            # If we are here 'out' should be zero-length.
            files.append(os.path.join(entryPath, DET_FILE))
        # <-- end if
    # <-- end for
    return(files)

def sameSSM(detections):
    """
    Return True of all the detections correspond to the same SSM object. False
    otherwise.
    """
    # Did we see this detection cluster already?
    if(detections in SSM_KEY.values()):
        return(False)

    # The first element of the list is the IOD; skip it.
    reference = None
    for detection in detections[1:]:
        ssm_id = fetchSsmID(detection)
        if(ssm_id == None):
            return(False)

        if(reference == None):
            reference = ssm_id
            continue
        if(ssm_id != reference):
            return(False)

    # Update the DET_SSM mapping.
    SSM_KEY[reference] = detections
    return(True)

def fetchSsmID(detection):
    """
    Lookup the detection in the DB and return its ssm_id via tracklet_attrib and
    tracklets.
    """
    cursor = Conection.connect(DB_USER, DB_PASSWD, DB_DB, DB_HOST)

    # detection := (object_name, epoch_mjd, ra_deg, dec_deg, mag, obscode)
    # and they are all strings since we read from a file.
    (object_name, epoch_mjd, ra_deg, dec_deg, mag, obscode) = detection

    # TODO: Should we use the object_name as it appears in the det file?
    sql = 'select t.ssm_id from tracklets t, tracklet_attrib ta, detections d '
    sql += 'where d.epoch_mjd = %s and '
    sql += '      d.ra_deg = %s and '
    sql += '      d.dec_deg = %s and '
    sql += '      d.mag = %s and '
    sql += '      d.obscode = %s and '
    sql += '      d.det_id = ta.det_id and '
    sql += '      ta.tracklet_id = t.tracklet_id'

    nRes = cursor.execute(sql % (epoch_mjd,
                                 ra_deg,
                                 dec_deg,
                                 mag,
                                 obscode))
    if(not nRes):
        # print('Warning: dirty detection list. SKIPPED')
        return(None)
    return(int(cursor.fetchone()[0]))






if(__name__ == '__main__'):
    import sys

    try:
        dirName = sys.argv[1]
    except:
        print('usage: mopsdiffcor_exam.py dirname')
        sys.exit(1)

    detCluster = getFailedDetections(dirName)
    # It is an iterator.
    # Each cluster has the corrsponding IOD as first element.
    try:
        while(True):
            cluster = detCluster.next()
            print('------')
            print('IOD: %s' % (' '.join(cluster[0])))
            for det in cluster[1:]:
                print(' '.join(det))
    except StopIteration:
        pass
    sys.exit(0)












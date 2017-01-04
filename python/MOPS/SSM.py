"""
$Id$

Procedural interface for query/manipulation of SSM tables in MOPS database.
"""

__author__ = 'LSST'
__date__ = '$Date$'
__version__ = '$Revision$'[11:-2]




def objectName2ssmId(dbh, name):
    """
    Fetch an SSM ID from the database given an object name.
    """

    cursor = dbh.cursor()
    n = cursor.execute('select ssm_id from ssm where object_name=%s', (name,))
    return cursor.fetchall()[0][0]


def ssmId2objectName(dbh, ssm_id):
    """
    Fetch an object name from the database given an SSM ID.
    """

    cursor = dbh.cursor()
    n = cursor.execute('select object_name from ssm where ssm_id=%s', (ssm_id,))
    return cursor.fetchall()[0][0]


"""
Minimal interface to fetch SSM objects from DB.
"""

class SSM(object):
    def __init__(self, q, e, i, node, argPeri, timePeri, h_v, epoch, 
                 objectName, ssmId=None, moid_1=-1):
        self._id = ssmId
        self.q = q
        self.e = e
        self.i = i
        self.node = node
        self.argPeri = argPeri
        self.timePeri = timePeri
        self.h_v = h_v
        self.epoch = epoch
        self.objectName = objectName
        self.moid_1 = moid_1
        return
    # <-- end def

    @staticmethod
    def retrieve(dbh, ssm_id):
        """
        Retrieve a single orbit into an Orbit object.
        """
        cursor = dbh.cursor()

        # Compose the SQL statement.
        sql = '''\
select 
s.q, s.e, s.i, s.node, s.arg_peri, s.time_peri, s.h_v,
s.epoch, s.ssm_id, s.moid_1
from ssm s where
s.ssm_id=%s
'''

        # Execute it!
        n = cursor.execute(sql, (ssm_id,))
        if not n:
            raise RuntimeError("got empty SSM object for ID %d" % ssm_id)
        # <-- end if
        
        row = list(cursor.fetchone())
        return SSM(*row)
    # <-- end def
    
    def update(self, dbh):
        """
        Update the SSM table using the provided SSM object.
        """
        cursor = dbh.cursor()
        # Compose the SQL statement.
        sql = '''\
update ssm set q = %s, e = %s, i = %s, node = %s, arg_peri = %s, time_peri = %s, 
h_v = %s, epoch = %s
where object_name = %s
'''

        # Execute it!
        n = cursor.execute(sql, (self.q, self.e, self.i, self.node, self.argPeri,
                                 self.timePeri, self.h_v, self.epoch, self.objectName))
        if not n:
            raise RuntimeError("Failed to update %s in SSM table" % self.objectName)
        # <-- end if
        dbh.commit()
    # <-- end def
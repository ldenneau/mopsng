"""
MOPS Python object-relational mapper: Orbit class.
"""

from __future__ import with_statement
from MOPS.Exceptions import *



def _serialize(var):
    """
    Return the string representation of var. None becomes undef.
    Only scalar values please!
    """
    if(var == None):
        return('undef')
    elif type(var) is float:
        stringy = "%.12g" % var
    else:
        stringy = str(var)

    return stringy



class Orbit(object):
    def __init__(self, orbitId, q, e, i, node, argPeri, timePeri, h_v, epoch,
                 src=None, residuals=None, chiSq=None, moid_1=None,
                 convCode=None, moid_2=None, arcLength_days=None):
        self._id = orbitId
        self.q = q
        self.e = e
        self.i = i
        self.node = node
        self.argPeri = argPeri
        self.timePeri = timePeri
        self.h_v = h_v
        self.epoch = epoch
        self.src = src
        self.residuals = residuals
        self.chiSq = chiSq
        self.moid_1 = moid_1
        self.moid_2 = moid_2
        self.convCode = convCode
        self.arcLength_days = arcLength_days

        # Double check sqrt cov matrix (SRC), in case somebody passed in a list of Nones.
        if self.src and self.src[0] is None:
            self.src = None
        return

    def __str__(self):
        showList = sorted(set(self.__dict__))
        return Utilities.toString(self, showList)
    # <-- end __str__

    def hasSrc(self):
        """
        Return True if and only if self.src is not null and is valid. False
        otherwise.
        """
        try:
            if(self.src and len(self.src) == 21 and None not in self.src):
                return(True)
        except:
            return(False)
        return(False)

    def save(self, dbh):
        """
        Write self to the database. If self._id=None, then after writing to the
        DB, fetch the newly assigned orbit id as well.
        """
        cursor = dbh.cursor()

        # Compose the sql.
        if self.src:
            sql = '''\
insert into orbits (
   q, e, i, node, arg_peri, time_peri, epoch, h_v, residual, chi_squared, 
   cov_01, cov_02, cov_03, cov_04, cov_05,
   cov_06, cov_07, cov_08, cov_09, cov_10, 
   cov_11, cov_12, cov_13, cov_14, cov_15, 
   cov_16, cov_17, cov_18, cov_19, cov_20, 
   cov_21, 
   conv_code, moid_1, moid_2, arc_length_days)
values (
    /* q, e, i, node, argp, tp, epoch, hv, resid, chisq */
    %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, 

    /* cov x 21 */
    %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, 
    %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, 
    %s,

    /* conv_code, moid1, moid2, arcLength_days */
    %s, %s, %s, %s
)
'''
#            + list(self.src)[1:-1] + \     # XXX LD: what up with [1:-1]?
            values = [
                self.q,
                self.e,
                self.i,
                self.node,
                self.argPeri,
                self.timePeri,
                self.epoch,
                self.h_v,
                self.residuals,
                self.chiSq,
            ] \
            + list(self.src) + \
            [
                self.convCode,
                self.moid_1,
                self.moid_2,
                self.arcLength_days
            ]
#            print sql, len(values), len(self.src)
        else:
            sql = '''\
insert into orbits (
   q, e, i, node, arg_peri, time_peri, epoch, h_v, residual, chi_squared, 
   conv_code, moid_1, moid_2, arc_length_days)
values (
    /* q, e, i, node, argp, tp, epoch, hv, resid, chisq */
    %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, 

    /* conv_code, moid1, moid2, arcLength_days */
    %s, %s, %s, %s
)
'''
            values = [
                self.q,
                self.e,
                self.i,
                self.node,
                self.argPeri,
                self.timePeri,
                self.epoch,
                self.h_v,
                self.residuals, 
                self.chiSq,
                self.convCode,
                self.moid_1,
                self.moid_2,
                self.arcLength_days
            ]
#            print sql, len(values)
        # <-- end if

        # Execute the query.
        n = cursor.execute(sql, values)
        if(n != 1):
            raise RuntimeError('insert orbit failed')
        # <-- if

        # What is the orbitId that autoincrement selected for us?
        if(self._id == None):
            # Try and be vendor-agnostic...
            self._id = int(dbh.insert_id())
        # <-- if

        return

    def __str__(self):
        """
        Simple serialization into MITI format.
        """
        serializableVars = [self._id,
                            self.q,
                            self.e,
                            self.i,
                            self.node,
                            self.argPeri,
                            self.timePeri,
                            self.epoch,
                            self.h_v,
                            self.residuals,
                            self.chiSq,
                            self.moid_1,
                            self.arcLength_days,
                            self.convCode]
        if(self.src):
            s = ' '.join(['MIF-OC', ] + map(_serialize,
                                            serializableVars + list(self.src)))
        else:
            s = ' '.join(['MIF-O', ] + map(_serialize,
                                           serializableVars))
        # <-- end if
        return(s)


    @staticmethod
    def retrieve(dbh, orbit_id):
        """
        Retrieve a single orbit into an Orbit object.
        """
        cursor = dbh.cursor()

        # Compose the SQL statement.
        sql = '''\
select o.orbit_id,
o.q, o.e, o.i, o.node, o.arg_peri, o.time_peri, o.h_v,
o.epoch,
o.cov_01, o.cov_02, o.cov_03, o.cov_04, o.cov_05, o.cov_06,
o.cov_07, o.cov_08, o.cov_09, o.cov_10, o.cov_11, o.cov_12,
o.cov_13, o.cov_14, o.cov_15, o.cov_16, o.cov_17, o.cov_18,
o.cov_19, o.cov_20, o.cov_21,
o.residual, o.chi_squared, o.moid_1, o.conv_code, o.moid_2, o.arc_length_days
from orbits o where
o.orbit_id=%s
'''

        # Execute it!
        n = cursor.execute(sql, (orbit_id,))
        if not n:
            raise RuntimeError("got empty orbit for ID %d" % orbit_id)
        
        row = list(cursor.fetchone())
        return Orbit(*(row[0:9] + [row[9:30]] + row[30:]))

    
    @staticmethod
    def retrieve_derivedobject(dbh, derivedobject_id):
        """
        Retrieve a single orbit by derivedobject_id into an Orbit object.
        """
        cursor = dbh.cursor()

        # Compose the SQL statement.
        sql = '''\
select o.orbit_id,
o.q, o.e, o.i, o.node, o.arg_peri, o.time_peri, o.h_v,
o.epoch,
o.cov_01, o.cov_02, o.cov_03, o.cov_04, o.cov_05, o.cov_06,
o.cov_07, o.cov_08, o.cov_09, o.cov_10, o.cov_11, o.cov_12,
o.cov_13, o.cov_14, o.cov_15, o.cov_16, o.cov_17, o.cov_18,
o.cov_19, o.cov_20, o.cov_21,
o.residual, o.chi_squared, o.moid_1, o.conv_code, o.moid_2, o.arc_length_days
from derivedobjects do, orbits o where
do.derivedobject_id=%s and
do.orbit_id=o.orbit_id
'''

        # Execute it!
        n = cursor.execute(sql, (derivedobject_id,))
        if not n:
            raise RuntimeError("got empty orbit for derived object ID %d" % derivedobject_id)

        row = list(cursor.fetchone())
        if row[9] is None:
            sqrt_cov = None
        else:
            sqrt_cov = row[9:30]
        return Orbit(*(row[0:9] + [sqrt_cov] + row[30:]))

#!/usr/bin/env python
"""
In a bitterly ironic twist of fate, all the orbits derived from MPC detections
so far have simple covarian matrices associated to them, instead of the more
useful square root covariance (SRC). We are going to fix this.
"""
import sys

import MySQLdb
import numpy

import ssd




# Constants
DB_HOST = 'localhost'                   # database machine
DB_USER = 'mops'
DB_PASS = 'mops'
DB_DB = 'psmops_real_ss_orbits'



# Connect and get a cursor.
connection = MySQLdb.connect(user=DB_USER,
                             passwd=DB_PASS,
                             host=DB_HOST,
                             db=DB_DB)
selectCursor = connection.cursor()
updateCursor = connection.cursor()


# Select teh covariance elements for all the orbits in the `orbits` table.
sql = """select orbit_id, cov_01, cov_02, cov_03, cov_04, cov_05, cov_06,
cov_07, cov_08, cov_09, cov_10, cov_11, cov_12, cov_13, cov_14, cov_15, cov_16,
cov_17, cov_18, cov_19, cov_20, cov_21 from orbits"""

n = selectCursor.execute(sql)
print('Preparing to update %d orbits.' %(n))
if(n == 0):
    sys.exit(0)


# Execute the updates.
sql = """update orbits set cov_01 = %e, cov_02 = %e, cov_03 = %e,
cov_04 = %e, cov_05 = %e, cov_06 = %e, cov_07 = %e, cov_08 = %e, cov_09 = %e,
cov_10 = %e, cov_11 = %e, cov_12 = %e, cov_13 = %e, cov_14 = %e, cov_15 = %e,
cov_16 = %e, cov_17 = %e, cov_18 = %e, cov_19 = %e, cov_20 = %e, cov_21 = %e
where orbit_id = %d"""

n = 0
sqlParams = []
orbit = selectCursor.fetchone()
while(orbit):
    orbitID = orbit[0]
    covar = numpy.array(orbit[1:])
    src = ssd.covar2src(covar).tolist()
    src.append(orbitID)
    sqlParams.append(tuple(src))

    # print('Updating orbit %d' %(orbitID))
    n += updateCursor.execute(sql %tuple(src))
    
    # Read the next line.
    orbit = selectCursor.fetchone()
# <-- end while
connection.commit()

# n = cursor.executemany(sql, sqlParams)
print('Updated %d orbits.' %(n))
sys.exit(0)

#!/usr/bin/env python
"""
PS-MOPS efficiency inspection tool

Purpose
Given a simulation identifier (i.e. $MOPS_DBINSTANCE) and optionally a metric
(see below) and a set of constraints (see below) return the efficiency of the
simulation.


Metrics
Supported metrics are
  * tracklet: tracklet building efficiency.
  * linking: tracklet linking efficiency.
  * precovery: precovery efficiency.
  * derivation: derivation efficiency.
  * attribution: attribution efficiency.
  * identification: identification efficiency.


Constraints
The efficiency anaylisis can be limited to:
  * Single field ID.
  * Fields in a cone.
  * Single night (i.e. single MJD).
  * Range of nights (i.e. range of MJDs).


Usage
    efficiency.py --instance=MOPSInstance [--metric=Metric]
        [--night=MJDConstraint] [--field=FiledConstraint]
        [--object=ObjConstraint] [--update_interval=UpdateFreq]
        [--baseline=BaselineMetric]

    MOPSInstance:    the name of a PS-MOPS simulation instance (typically the
                     value of $MOPS_DBINSTANCE).
    Metric:          one of tracklet, linking, precovery, derivation,
                     attribution or identification  or a list in the form
                     metric1,metric2,metric3... If omitted efficiency is
                     computed for all metrics.
    MJDConstraint:   either a single MJD value or a range in the form
                     mjd1-mjd2. If omitted, efficiency is computed for the
                     whole time span of the simulation.
    FiledConstraint: either a single fieldID or a center and search radius in
                     the form ra,dec,radius. RA, Dec and radius in decimal
                     degrees. If omitted, efficiency is computed for all
                     fields. FOR FUTURE VERSIONS: NOT CURRENTLY SUPPORTED.
    ObjConstraint:   either a single solar syste model object ID or a list of
                     IDs in the form objID1,ojID2,objID3...
    UpdateFreq:      Number of seconds between successive updates of the stats.
                     If omitted statistics are computed every 5 minutes.
    BaselineMetric:  If specified, the total numbr of solar systm objects is
                     computed from BaselineMetric, instead of from each
                     individual metric.


Output
Output in the form
    Metric: available found extra
is printed on STDOUT (one line per selected metric).

    Metric:    one of tracklet, linking, precovery, derivation, attribution or
               identification.
    available: number of available objects (from the solar system model) that
               were injected in the simulation and had to be "found".
    found:     number of available objects from the solar system model that
               were actually found.
    extra:     number of objects that were found but not part of the solar
               system model (i.e. nt present in available).
"""
# TODO: Implement field constraints.


import MySQLdb as dbi



# Constants
METRIC_LIST = ['detection',
               'tracklet',
               'linking',
               'precovery',
               'derivation',
               'attribution',
               'identification']
USAGE_STR = """efficiency.py --instance=MOPSInstance [--metric=Metric]
    [--night=MJDConstraint] [--field=FiledConstraint]
    [--object=ObjConstraint] [--update_interval=UpdateFreq]
    [--baseline=BaselineMetric]"""

DB_HOST = 'mopsdc.ifa.hawaii.edu'
# DB_HOST = 'localhost'
DB_USER = 'mops'
DB_PASSWD = 'mops'
DB_TYPE = 'mysql'



def parseCommandLineArgs(argv):
    """
    Given a list of command line args (@param argv), parse them and return
    (instanceName, metricList, mjdList, fieldList, objectList).

    Only the instance name is compulsory, all other flags are optional.
    """
    import getopt

    instanceName = None
    metricList = METRIC_LIST
    mjdList = []
    fieldList = []
    objectList = []
    updateInterval = 5 * 60                 # 5 minutes.
    baselineMetric = None
    opts, args = getopt.getopt(sys.argv[1:], '', ['instance=',
                                                  'metric=',
                                                  'night=',
                                                  'field=',
                                                  'object=',
                                                  'update_interval=',
                                                  'baseline='])
    for key, val in opts:
        if(key == '--instance'):
            instanceName = val
        elif(key == '--metric'):
            metricList = val.split(',')
        elif(key == '--night'):
            if(val.find('-') != -1):
                # It is a range: split it up and build the list of start and
                # end MJDs.
                start, end = mjd.split('-')
                mjdList = (float(start), float(end))
            else:
                mjdList.append(float(val))
        elif(key == '--field'):
            print('Warning: field constraints are not supported yet.',
                  'Send an email to fpierfed@lsst.org with a reminder.')
            continue
        elif(key == '--object'):
            objectList = val.split(',')
        elif(key == '--update_interval'):
            updateInterval = int(val)
        elif(key == '--baseline'):
            baselineMetric = val
        else:
            print('Warning: option %s is not supported (ignored).' % (key))
            continue

    # Quick sanity check.
    assert instanceName != None, 'Error: MOPSInstance is a required parameter.'
    return(instanceName, metricList, mjdList, fieldList,
           objectList, updateInterval, baselineMetric)


def efficiency(instance, metricList, mjdList, fieldList, objectList,
               baselineMetric):
    """
    Given a simulation instance (@param instance), a list of metrics
    (@param metric), of MJDs (@param mjdList), of fields (@param fieldList)
    and of solar system objects (@param objectList), compute the efficiency
    for each of those. If @param baselineMetric is not None, the totl number of
    SSM object is computed from that metric. Otherwise it is computed from
    each individual metric.

    This is a generator. Output in the form (metric, available, found, extra)
    is yielded at each iteration.
    """
    query = EfficiencyQuery(instance)

    # Do we know hich metric gives us the total number of given objects?
    if(baselineMetric):
        method = getattr(query, '%sEfficiency' % (baselineMetric))
        (totAvailable, found) = method(mjdList, fieldList, objectList)

        # Compute the other metrics figures of merit and express them as
        # percentage of totAvailable.
        # TODO: Slight optimization: do not recompute values for baselineMetric.
        for metric in metricList:
            method = getattr(query, '%sEfficiency' % (metric))
            (available, found) = method(mjdList, fieldList, objectList)
            yield (metric, totAvailable, found)
    else:
        for metric in metricList:
            method = getattr(query, '%sEfficiency' % (metric))
            (available, found) = method(mjdList, fieldList, objectList)
            yield (metric, available, found)
    # return


def usage():
    """
    Print usage string to STDOUT.
    """
    print('usage: %s' % (USAGE_STR))
    return


class EfficiencyQuery(object):
    def __init__(self,
                 db_db,
                 db_type=DB_TYPE,
                 db_host=DB_HOST,
                 db_user=DB_USER,
                 db_passwd=DB_PASSWD):
        # Connect to the DB
        self._connection = dbi.connect(host=db_host,
                                       user=db_user,
                                       passwd=db_passwd,
                                       db=db_db)
        self._cursor = self._connection.cursor()
        return

    def detectionEfficiency(self, mjdList, fieldList, objectList):
        sqlSelect = 'select count(distinct(object_name)) from detections'
        sqlWhere = ''
        constraints = []

        # mjdList can only be a singlr value or a range. We do not support
        # random lists of MJD values.
        if(mjdList):
            if(len(mjdList) == 1):
                # FIXME: Handle this better.
                constraints.append('epoch_mjd = %f' % (mjdList[0]))
            else:
                constraints.append('epoch_mjd between %f and %f' \
                                   % (mjdList[0], mjdList[1]))
        # <-- end if

        # We have all the constraints: build the SQL query.
        if(constraints):
            sqlWhere = ' where '
        for constraint in constraints:
            sqlWhere += '(%s) and ' % (constraint)
        sqlWhere = sqlWhere[:-5]

        if(sqlWhere):
            sqlAvail = sqlSelect + sqlWhere
            sqlFound = sqlAvail + ' and (is_synthetic = 1)'
        else:
            sqlAvail = sqlSelect
            sqlFound = sqlAvail + ' where (is_synthetic = 1)'

        self._cursor.execute(sqlAvail)
        avail = int(self._cursor.fetchone()[0])

        self._cursor.execute(sqlFound)
        found = int(self._cursor.fetchone()[0])
        return(avail, found)

    def trackletEfficiency(self, mjdList, fieldList, objectList):
        sqlSelect = 'select count(distinct(ssm_id)) from tracklets'
        sqlWhere = ''
        constraints = []

        # mjdList can only be a singlr value or a range. We do not support
        # random lists of MJD values.
        if(mjdList):
            if(len(mjdList) == 1):
                # FIXME: Handle this better.
                constraints.append('ext_epoch = %f' % (mjdList[0]))
            else:
                constraints.append('ext_epoch between %f and %f' \
                                   % (mjdList[0], mjdList[1]))
        # <-- end if

        # We have all the constraints: build the SQL query.
        if(constraints):
            sqlWhere = ' where '
        for constraint in constraints:
            sqlWhere += '(%s) and ' % (constraint)
        sqlWhere = sqlWhere[:-5]

        if(sqlWhere):
            sqlAvail = sqlSelect + sqlWhere
            sqlFound = sqlAvail + ' and (classification = "C")'
        else:
            sqlAvail = sqlSelect
            sqlFound = sqlAvail + ' where (classification = "C")'

        self._cursor.execute(sqlAvail)
        avail = int(self._cursor.fetchone()[0])

        self._cursor.execute(sqlFound)
        found = int(self._cursor.fetchone()[0])
        return(avail, found)

    def linkingEfficiency(self, mjdList, fieldList, objectList):
        sqlSelect = 'select count(distinct(ssm_id)) from history, `fields`'
        sqlWhere = ''
        constraints = ['event_type = "D"']

        # mjdList can only be a singlr value or a range. We do not support
        # random lists of MJD values.
        if(mjdList):
            if(len(mjdList) == 1):
                # FIXME: Handle this better.
                constraints.append('`fields`.epoch_mjd = %f' % (mjdList[0]))
            else:
                constraints.append('fields.epoch_mjd between %f and %f' \
                                   % (mjdList[0], mjdList[1]))
        # <-- end if

        # We have all the constraints: build the SQL query.
        if(constraints):
            sqlWhere = ' where history.field_id = `fields`.field_id and '
        for constraint in constraints:
            sqlWhere += '(%s) and ' % (constraint)
        sqlWhere = sqlWhere[:-5]

        if(sqlWhere):
            sqlAvail = sqlSelect + sqlWhere
            sqlFound = sqlAvail + ' and (classification = "C")'
        else:
            sqlAvail = sqlSelect
            sqlFound = sqlAvail + ' where (classification = "C")'

        self._cursor.execute(sqlAvail)
        avail = int(self._cursor.fetchone()[0])

        self._cursor.execute(sqlFound)
        found = int(self._cursor.fetchone()[0])
        return(avail, found)

    def precoveryEfficiency(self, mjdList, fieldList, objectList):
        """
        Precovery improves known orbits by ading more tracklets to them. This
        means that the old orbit is "retuired" and the derived object is given
        a new one.
        """
        # The strategy is as follows: using the history table (where event_type
        # is 'P'), fetch the names of SSM objects. For each one of those look
        # if we have a successful precovery event (classification='C'). The
        # number of SSM objects for which this happens is "found"; the total
        # number of SSM bjects is "avail".
        sql = 'select distinct(ssm_id) from history where event_type="P"'
        avail = self._cursor.execute(sql)
        res = self._cursor.fetchall()
        ssmObjects = [x[0] for x in res]

        found = 0
        for ssmObject in ssmObjects:
            sql = 'select count(*) from history where ' + \
                  'ssm_id = "%s" and event_type="P" and ' % (ssmObject) + \
                  'classification = "C"'
            self._cursor.execute(sql)
            n = int(self._cursor.fetchone()[0])
            if(n):
                found += 1
        return(avail, found)


    def derivationEfficiency(self, mjdList, fieldList, objectList):
        return(-1, -1)

    def attributionEfficiency(self, mjdList, fieldList, objectList):
        return(-1, -1)

    def identificationEfficiency(self, mjdList, fieldList, objectList):
        return(-1, -1)



if(__name__ == '__main__'):
    import sys
    import time


    try:
        (instanceName,
         metricList,
         mjdList,
         fieldList,
         objectList,
         updateInterval,
         baselineMetric) = parseCommandLineArgs(sys.argv[1:])
    except:
        usage()
        sys.exit(1)

    # Save the statistics for each available metric at the time they were
    # computed.
    stats = {}
    try:
        while(True):
            timeStamp = time.strftime('%a, %d %b %Y %H:%M:%S')
            for (metric, available, found) in efficiency(instanceName,
                                                         metricList,
                                                         mjdList,
                                                         fieldList,
                                                         objectList,
                                                         baselineMetric):
                if(available < 0 or found < 0):
                    continue
                try:
                    percentFound = 100. * float(found) / available
                except ZeroDivisionError:
                    percentFound = 100.

                print('%s %s:  %d  %d (%.01f%%)' % (timeStamp,
                                                    metric,
                                                    available,
                                                    found,
                                                    percentFound))
            time.sleep(updateInterval)
            # <-- end for
        # <-- end while
    except KeyboardInterrupt:
        # Quit
        pass
    sys.exit(0)

















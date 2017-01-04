#!/usr/bin/env python

#
# How to connect to the ipp database
# Some remarkable exposures:
#  * Chip:
#    - Has some (not all) chip quality concerns: o5483g0185o
#    - Has all 60 chips marked as invalid: o5086g0233o
#  * Cam:
#    - cam with bad quality: o5246g0064o, o5104g0242o
#  * Warp:
#    - warp with bad quality: 
#
# python ipp_query.py o5694g0324o o5483g0185o abcdefghijkl o5086g0233o o5104g0242o

DEBUG = False

connection = { #'hostname': 'localhost', 'port':6666,
               #'hostname': 'ipp0012', 'port': 3306,
               'hostname': 'ippdb01', 'port': 3306,
               'database': 'gpc1',
               'user': 'ipp',
               'password': 'ipp' }

#######################################################################################
import MySQLdb

def log(level, message):#TODO: Add datetime
    print "%8s | %s" % (level, message)
def debug(message):
    if DEBUG:
        log("DEBUG", message)
def info(message):
    log("INFO", message)
def warn(message):
    log("WARNING", message)
def error(message):
    log("ERROR", message)

queries = { 'GetExpId': "SELECT exp_id,state FROM rawExp where exp_name='%s'",
            'GetChipId': "SELECT chip_id,state FROM chipRun where exp_id=%d AND dist_group IS NOT NULL",
            'GetChipQuality': "SELECT quality FROM chipProcessedImfile WHERE chip_id=%d",
            'GetCamId': "SELECT cam_id,state FROM camRun where chip_id=%d",
            'GetCamQuality': "SELECT quality FROM camProcessedExp where cam_id=%d",
            'GetFakeId': "SELECT fake_id FROM fakeRun where cam_id=%d",
            'GetWarpId': "SELECT warp_id,state FROM warpRun where fake_id=%d",
            'GetWarpQuality': "SELECT quality FROM warpSkyfile where warp_id=%d AND data_state='full'",
            'GetDiff1Id': "SELECT diff_id FROM diffInputSkyfile where warp1=%d",
            'GetDiff2Id': "SELECT diff_id FROM diffInputSkyfile where warp2=%d",
            'GetDiffStatus': "SELECT state FROM diffRun where diff_id=%d",
            'GetPubId': "SELECT pub_id,state FROM publishRun where stage_id=%d AND client_id=%d",
            }

class QueryFailure:
    def __init__(self, _reason):
        self.reason = _reason

class IppInquisitor:
    def __init__(self, _connection = connection, _clientId = 5):
        self.connection = _connection
        self.clientId = _clientId
        self.connect()
        self.stage = 'Unregistered'

    def connect(self):
        debug("Connecting to IPP database")
        self.db = MySQLdb.connect( host=connection['hostname'],
                                   port=connection['port'],
                                   user=connection['user'],
                                   passwd=connection['password'],
                                   db=connection['database'])
        self.cursor = self.db.cursor()
        debug("Connected to database")

    def getExposureStatus(self, exposureName):
        debug('Checking status of [%s]' % exposureName)
        try:
            debug('          ExpId: %d (%s)' % self.getExpId(exposureName))
            self.getChipId()
            debug('         ChipId: %d (%s)' % (self.chipId, self.chipStatus))
            if self.getChipQuality():
                debug('   Chip Quality: PASS')
            else:
                raise QueryFailure('Bad Chip Quality for %s' % self.exposureName)
            debug('          CamId: %d (%s)' % self.getCamId())
            if self.getCamQuality():
                debug('    Cam Quality: PASS')
            else:
                raise QueryFailure('Bad Cam Quality for %s' % self.exposureName)
            debug('         FakeId: %d' % self.getFakeId())
            debug('         WarpId: %d (%s)' % self.getWarpId())
            if self.getWarpQuality():
                debug('   Warp Quality: PASS')
            else:
                raise QueryFailure('Bad Warp Quality for %s' % self.exposureName)
            self.getDiffId()
            debug('         DiffId: %d (%s)' % (self.diffId, self.diffStatus))
            debug('          PubId: %d (%s)' % self.getPubId())
            return ('%s: PASS' % self.exposureName)
        except QueryFailure, e:
            debug('Failure at some point for [%s]' % self.exposureName)
            debug('%s' % e.reason)
            return ('%s: FAIL (%s stage)' % (self.exposureName, self.stage))

    def getId(self, stageName, previousStep, warnIfMoreThanOneId=True):
        self.cursor.execute(queries['Get' + stageName + 'Id'] % previousStep)
        self.stage = stageName
        ids = self.cursor.fetchall()
        if ids == None:
            raise QueryFailure('No %s_id for exposure named [%s]'
                               % (stageName, self.exposureName) )
        if len(ids) == 0:
            raise QueryFailure('No %s_id for exposure named [%s]'
                               % (stageName, self.exposureName) )
        if warnIfMoreThanOneId:
            if len(ids) > 1:
                warn('WARNING: [%s] has more than one %s_id (Tell IPP team)' 
                     % (self.exposureName, stageName) )
                info('INFO: I will use the largest id: %d' % ids[-1][0])
                # raise QueryFailure('[%s] has more than one %s_id (Tell IPP team) [%d]'
                #                    % (self.exposureName, stageName, len(ids)))
        if len(ids[-1]) > 1:
            return (ids[-1][0],ids[-1][1])
        else:
            return ids[-1][0]

    def getExpId(self, exposureName):
        self.exposureName = exposureName
        self.stage = 'Registration / Burntool'
        self.cursor.execute(queries['GetExpId'] % exposureName)
        expId = self.cursor.fetchall()
        if len(expId) == 0:
            raise QueryFailure('No exp_id for exposure named [%s]' 
                               % self.exposureName)
        if len(expId) != 1:
            warn('[%s] has more than one exp_id (Tell IPP team)'
                 % self.exposureName)
        (self.expId, self.expStatus) = (expId[-1][0], expId[-1][1])
        return (self.expId, self.expStatus)
        
    def getChipId(self):
        (self.chipId, self.chipStatus) = self.getId('Chip', self.expId)
        return (self.chipId, self.chipStatus)

    def getChipQuality(self):
        self.cursor.execute(queries['GetChipQuality'] % self.chipId)
        self.stage = 'Chip (Bad Quality)'
        all60haveBadQuality = True
        for row in self.cursor.fetchall():
            all60haveBadQuality = all60haveBadQuality and (row[0] != 0)
            if not all60haveBadQuality:
                break
        return not all60haveBadQuality

    def getCamId(self):
        (self.camId, self.camStatus) =  self.getId('Cam', self.chipId)
        return (self.camId, self.camStatus)

    def getCamQuality(self):
        self.cursor.execute(queries['GetCamQuality'] % self.camId)
        self.stage = 'Cam (Bad Quality)'
        rows = self.cursor.fetchall()
        if len(rows) != 1:
            raise QueryFailure('Unexpected number of entries (%d instead of 1) for cam quality check' % len(rows))
        return (rows[0][0] == 0)

    def getFakeId(self):
        self.fakeId =  self.getId('Fake', self.camId)
        return self.fakeId

    def getWarpId(self):
        (self.warpId,self.warpStatus) =  self.getId('Warp', self.fakeId)
        return (self.warpId,self.warpStatus)

    def getWarpQuality(self):
        self.cursor.execute(queries['GetWarpQuality'] % self.warpId)
        self.stage = 'Warp (Bad Quality)'
        rows = self.cursor.fetchall()
        allRowsNon0 = True
        for row in rows:
            allRowsNon0 = allRowsNon0 and (row[0] != 0)
            if not allRowsNon0:
                break
        return not allRowsNon0

    def getDiffId(self):
        # print '---> ', queries['GetDiff1Id'] % self.warpId
        self.diffId = self.getId('Diff1', self.warpId, False)
        if self.diffId is None:
            debug('DiffId set from warp2')
            self.diffId = self.getId('Diff2', self.warpId, False)
        else:
            debug('DiffId set from warp1')
        self.cursor.execute(queries['GetDiffStatus'] % self.diffId)
        statuses = self.cursor.fetchall()
        if len(statuses) < 1:
            raise QueryFailure('No diff_id for exposure named [%s]' 
                               % self.exposureName)
        self.diffStatus = statuses[0][-1]
        self.stage = 'Diff'
        return (self.diffId, self.diffStatus)

    def getPubId(self):
        self.cursor.execute(queries['GetPubId'] % (self.diffId, self.clientId) )
        pubData = self.cursor.fetchall()
        if len(pubData) < 1:
            raise QueryFailure('No pub_id for exposure named [%s]' 
                               % self.exposureName)
        (self.pubId, self.pubState) = pubData[0]
        self.stage = 'Pub'
        return (self.pubId, self.pubState)

    def disconnect(self):
        debug('Disconnecting from IPP database')
        self.db.close()

#######################################################################
#
#
if __name__ == '__main__':
    import sys
    import os
#    environment = os.getenv('MOPS_DBINSTANCE')
#    if environment is None:
#        print 'Error: MOPS_DBINSTANCE environment variable has to be set'
#        sys.exit(2)
#    if environment == 'psmops_ps1_v6':
#        ippPublishClient = 1
#    elif environment == 'psmops_ps1_mdrm137':
#        ippPublishClient = 5
#    else:
#        print 'Error: MOPS_DBINSTANCE environment variable has to be either psmops_ps1_v6 or psmops_ps1_mdrm137'
#        sys.exit(2)
    ippPublishClient = 5
    if len(sys.argv) < 2:
        print 'I need at least one exposure name'
        print 'Usage: ipp_query.py [-v] [exposure name]...'
        print '  -v: Be verbose'
        sys.exit(1)

    firstExposure = 1
    if sys.argv[1] == '-v':
        DEBUG = True
        firstExposure += 1

    if '-' in sys.argv[firstExposure]:
        debug('Range query')
        (expFirst, expLast) = sys.argv[firstExposure].split('-')
        first = int(expFirst[6:10])
        last = int(expLast[6:10])
        format = expFirst[:6] + "%04do"
        exposuresNames = []
        for index in range(first, last+1, 1):
            exposuresNames.append(format % index)
    else:
        exposuresNames = sys.argv[firstExposure:]

    inquisitor = IppInquisitor(connection, ippPublishClient)
    
    for exposureName in exposuresNames:
        try:
            print inquisitor.getExposureStatus(exposureName)
        except QueryFailure, e:
            print '  Failure: %s' % e.reason
            print '  Exposure failed at "%s" stage' % inquisitor.stage

    inquisitor.disconnect()

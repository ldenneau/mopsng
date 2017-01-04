"""
MOPS Python object-relational mapper: Submission class.
"""

class Submission:
    def __init__(self, seq_num, batch_num, fpa_id, epoch_mjd, ra_deg, dec_deg, 
                 filter_id, mag, obscode, desig, digest, spatial_idx, 
                 survey_mode, dbname, det_id, tracklet_id, derivedobject_id, 
                 submitter, submitted, disposition, discovery, mpc_desig):
        self._seqNum = seq_num
        self._batchNum = batch_num
        self._fpaId = fpa_id
        self._epochMJD = epoch_mjd
        self._raDeg = ra_deg
        self._decDeg = dec_deg
        self._filterId = filter_id
        self._mag = mag
        self._obscode = obscode
        self._desig = desig
        self._digest = digest
        self._spatialIdx = spatial_idx
        self._surveyMode = survey_mode
        self._dbname = dbname
        self._detId = det_id
        self._trackletId = tracklet_id
        self._derivedObjId = derivedobject_id
        self._submitter = submitter
        self._submitted = submitted
        self._disposition = disposition
        self._discovery = discovery
        self._mpcDesig = mpc_desig
        return
    # <--end def
    
    #--------------------------------------------------------------------------   
    # Getter methods.
    #--------------------------------------------------------------------------   

    @property
    def seqNum(self):
        """Global submission sequence number"""
        return long(self._seqNum)
    # <-- end def 

    @property
    def batchNum(self):
        """Batch in which tracklet was submitted"""
        return long(self._batchNum)
    # <-- end def
    
    @property
    def fpaId(self):
        """PS1 exposure name"""
        return self._fpaId
    # <-- end def 

    @property
    def epochMJD(self):
        """MJD of observation"""
        return long(self._epochMJD)
    # <-- end def

    @property
    def raDeg(self):
        """Right ascension in degrees"""
        return float(self._raDeg)
    # <-- end def 

    @property
    def decDeg(self):
        """Declination in degrees"""
        return float(self._decDeg)
    # <-- end def

    @property
    def filterId(self):
        """Reported PS1 filter"""
        return self._filterId
    # <-- end def 

    @property
    def mag(self):
        """Reported mag"""
        return float(self._mag)
    # <-- end def

    @property
    def obscode(self):
        """MPC observatory code"""
        return self._obscode
    # <-- end def 

    @property
    def desig(self):
        """Internal designation"""
        return self._desig
    # <-- end def

    @property
    def digest(self):
        """Digest 2 score."""
        return float(self._digest)
    # <-- end def 

    @property
    def spatialIdx(self):
        """Spatial index"""
        return long(self._spatialIdx)
    # <-- end def

    @property
    def surveyMode(self):
        """PS1 survey mode"""
        return self._surveyMode
    # <-- end def 

    @property
    def dbname(self):
        """Database instance that contains the tracklet/detection/derivedObj"""
        return self._dbname
    # <-- end def

    @property
    def detId(self):
        """MOPS detection ID"""
        return long(self._detId)
    # <-- end def 

    @property
    def trackletId(self):
        """MOPS tracklet ID"""
        return long(self._trackletId)
    # <-- end def

    @property
    def derivedObjId(self):
        """MOPS derived object ID"""
        return long(self._derivedObjId)
    # <-- end def 

    @property
    def submitter(self):
        """Submitter initials"""
        return self._submitter
    # <-- end def

    @property
    def submitted(self):
        """Date submitted or updated (UT)"""
        return self._submitted
    # <-- end def 

    @property
    def disposition(self):
        """1-character status (S/C/D/N)"""
        return self._disposition
    # <-- end def

    @property
    def discovery(self):
        """Discovery observation"""
        return self._discovery
    # <-- end def 

    @property
    def mpcDesig(self):
        """MPC-assigned designation"""
        return self._mpcDesig
    # <-- end def
# <-- end class
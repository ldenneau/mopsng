"""
MOPS Python object-relational mapper: Detection class.
"""
from MOPS.Submissions import Submission
from MOPS.DetectionAttr import DetectionAttr

class Detection(object):
    def __init__(self, detId, ra, dec, mjd, mag, refMag, s2n, raErr,
                 decErr, magSigma, orient, length, status, fieldId, filt,
                 obscode,
                 objectName=None, detNum=None):
        self._id = detId
        self.ra = ra
        self.dec = dec
        self.mjd = mjd
        self.mag = mag
        self.refMag = refMag
        self.s2n = s2n
        self.raErr = raErr
        self.decErr = decErr
        self.magSigma = magSigma
        self.orient = orient
        self.length = length
        self.status = status
        self.fieldId = fieldId
        self.filt = filt
        self.obscode = obscode
        self.objectName = objectName
        self.detNum = detNum                # IPP detection number (for LSD dets)
        self.ssmId = None
        self.submissions = []
        self.attributes = None              # Raw attributes of detection
        return
    # <-- end def
    
    def fetchSubmissons(self, dbh=None):
        """
        Fetch any submissions that this detection is a part of.
        """
        if not self.submissions:
            if not dbh:
                raise RuntimeError('fetchSubmissions() called with no dbh')
            # <-- end if

            sql = '''\
select seq_num, batch_num, fpa_id, epoch_mjd, ra_deg, dec_deg, filter_id, mag,
obscode, desig, digest, spatial_idx, survey_mode, dbname, det_id, tracklet_id,
derivedobject_id, submitter, submitted, disposition, discovery, mpc_desig
from export.mpc_sub
where det_id = %d
order by epoch_mjd
''' %(self._id)

            # Execute the SQL statement.
            cursor = dbh.cursor()
            n = cursor.execute(sql)
            if(not n):
                # No submissions, then return
                return
            # <-- end if

            row = cursor.fetchone()
            while(row):
                row = list(row)
                self.submissions.append(Submission(*row))

                # Next line.
                row = cursor.fetchone()
            # <-- while
        # <-- end if
        return self.submissions
    # <-- end def

    def fetchAttributes(self, dbh=None):
        """
        Fetch the raw attributes of the detection.
        """
        if not self.attributes:
            if not dbh :
                raise RuntimeError('fetchAttributes() called with no dbh')
            # <-- end if
        
            sql = """\
SELECT det_id, psf_chi2, psf_dof, cr_sig, ext_sig, psf_major, psf_minor, 
psf_theta, psf_quality, psf_npix, moments_xx, moments_xy, moments_yy, 
n_pos, f_pos, ratio_bad, ratio_mask, ratio_all, flags, ipp_idet, psf_inst_flux, 
psf_inst_flux_sig, ap_mag, ap_mag_raw, ap_mag_radius, ap_flux, ap_flux_sig, 
peak_flux_as_mag, cal_psf_mag, cal_psf_mag_sig, sky, sky_sigma, psf_qf_perfect, 
moments_r1, moments_rh, kron_flux, kron_flux_err, kron_flux_inner, 
kron_flux_outer, diff_r_p, diff_sn_p, diff_r_m, diff_sn_m, flags2, n_frames
FROM det_rawattr_v2
WHERE det_id = %d
""" % (self._id)

            # Execute the SQL statement.
            cursor = dbh.cursor()
            n = cursor.execute(sql)
            if(not n):
                # No attributes return
                return
            # <--  end if

            row = cursor.fetchone()
            row = list(row)
            self.attributes = DetectionAttr(*row)
        # <-- end if
        return self.attributes
    # <-- end def    

    def upgrade(self, dbh):
        """
        Write ourselves into the database.  Normally this will only be called as
        part of a tracklet upgrade.  Also note that we should never be upgrading
        synthetic detections -- they are pre-inserted into the DB.
        """
        if self._id is not None:
            return                  # already upgraded

        # Do DB stuff here.
        is_synthetic = '0'      # we should only be upgrading nonsynth dets
        cursor = dbh.cursor()
        sql = '''
insert into detections (
ra_deg, dec_deg, epoch_mjd, mag, ref_mag, filter_id, is_synthetic, det_num, status,
s2n, ra_sigma_deg, dec_sigma_deg, mag_sigma, orient_deg, length_deg, object_name, field_id, obscode)
values (
%s, %s, %s, %s, %s, %s, %s, %s, %s,
%s, %s, %s, %s, %s, %s, %s, %s, %s
)
'''
        res = cursor.execute(sql, (
            self.ra,
            self.dec,
            self.mjd,
            self.mag,
            self.refMag,
            self.filt,
            is_synthetic,
            self.detNum,
            'F',            # found
            self.s2n,
            self.raErr,
            self.decErr,
            self.magSigma,
            self.orient,
            self.length,
            None,           # object name
            self.fieldId,
            self.obscode
        ))
        if not res:
            raise RuntimeError('upgrade of detection failed')
        self._id = dbh.insert_id()
    # <-- end def
# <-- end class
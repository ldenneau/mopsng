"""
MOPS Python object-relational mapper: DetectionAttr class.
"""
from MOPS.Submissions import Submission

class DetectionAttr(object):
    def __init__(self, det_id, psf_chi2, psf_dof, cr_sig, ext_sig, psf_major, 
                 psf_minor, psf_theta, psf_quality, psf_npix, moments_xx, 
                 moments_xy, moments_yy, n_pos, f_pos, ratio_bad, ratio_mask, 
                 ratio_all, flags, ipp_idet, psf_inst_flux, psf_inst_flux_sig,
                 ap_mag, ap_mag_raw, ap_mag_radius, ap_flux, ap_flux_sig, 
                 peak_flux_as_mag, cal_psf_mag, cal_psf_mag_sig, sky, sky_sigma, 
                 psf_qf_perfect, moments_r1, moments_rh, kron_flux, 
                 kron_flux_err, kron_flux_inner, kron_flux_outer, diff_r_p, 
                 diff_sn_p, diff_r_m, diff_sn_m, flags2, n_frames):
        
        self._det_id = det_id
        self._psf_chi2 = psf_chi2
        self._psf_dof = psf_dof
        self._cr_sig = cr_sig
        self._ext_sig = ext_sig
        self._psf_major = psf_major
        self._psf_minor = psf_minor
        self._psf_theta = psf_theta
        self._psf_quality = psf_quality
        self._psf_npix = psf_npix
        self._moments_xx = moments_xx
        self._moments_xy = moments_xy
        self._moments_yy = moments_yy
        self._n_pos = n_pos
        self._f_pos = f_pos
        self._ratio_bad = ratio_bad
        self._ratio_mask = ratio_mask
        self._ratio_all = ratio_all               
        self._flags = flags
        self._ipp_idet = ipp_idet
        self._psf_inst_flux = psf_inst_flux
        self._psf_inst_flux_sig = psf_inst_flux_sig
        self._ap_mag = ap_mag
        self._ap_mag_raw = ap_mag_raw
        self._ap_mag_radius = ap_mag_radius
        self._ap_flux = ap_flux
        self._ap_flux_sig = ap_flux_sig
        self._peak_flux_as_mag = peak_flux_as_mag
        self._cal_psf_mag = cal_psf_mag
        self._cal_psf_mag_sig = cal_psf_mag_sig
        self._sky = sky
        self._sky_sigma = sky_sigma
        self._psf_qf_perfect = psf_qf_perfect
        self._moments_r1 = moments_r1
        self._moments_rh = moments_rh
        self._kron_flux = kron_flux
        self._kron_flux_err = kron_flux_err
        self._kron_flux_inner = kron_flux_inner
        self._kron_flux_outer = kron_flux_outer
        self._diff_r_p = diff_r_p
        self._diff_sn_p = diff_sn_p
        self._diff_r_m = diff_r_m
        self._diff_sn_m = diff_sn_m
        self._flags2 = flags2
        self._n_frames = n_frames
        return
    # <-- end def
    
    #--------------------------------------------------------------------------   
    # Getter methods.
    #--------------------------------------------------------------------------   

    @property
    def det_id(self):
        """Detection Id"""
        return long(self._det_id)
    # <-- end def 

    @property
    def psf_chi2(self):
        """Chi square probability distribution of PSF"""
        return float(self._psf_chi2)
    # <-- end def
    
    @property
    def psf_dof(self):
        """
        Number of degrees of freedom for the fit -- for the diff detections, 
        the only free parameter is the flux, so PSF_DOF = N_PIX - 1; in other 
        cases, there may be more parameters (eg, position, size, etc).
        """
        return int(self._psf_dof)
    # <-- end def 

    @property
    def cr_sig(self):
        """Cosmic ray """
        return float(self._cr_sig)
    # <-- end def

    @property
    def ext_sig(self):
        """Extended source"""
        return float(self._ext_sig)
    # <-- end def 

    @property
    def psf_major(self):
        """PSF width along major axis (pixels)"""
        return float(self._psf_major)
    # <-- end def

    @property
    def psf_minor(self):
        """PSF width along minor axis (pixels)"""
        return float(self._psf_minor)
    # <-- end def 

    @property
    def psf_theta(self):
        """PSF orientation angle (degrees)"""
        return float(self._psf_theta)
    # <-- end def

    @property
    def psf_quality(self):
        """
        The psf-weighted fraction of unmasked pixels:
        Sum_i (psf_i * !masked) / Sum_i (psf_i)
        """
        return float(self._psf_quality)
    # <-- end def 

    @property
    def psf_npix(self):
        """Number of pixels for fit"""
        return int(self._psf_npix)
    # <-- end def

    @property
    def moments_xx(self):
        """
        Second moment in x:
        Sum((x_i - x_o)^2 f_i) / Sum(f_i) where x_o is the centroid Sum(x_i f_i) / Sum(f_i)
        """
        return float(self._moments_xx)
    # <-- end def 

    @property
    def moments_xy(self):
        """
        Second moment in xy:
        Sum((x_i - x_o)(y_i - y_o)^2 f_i) / Sum(f_i)
        """
        return float(self._moments_xy)
    # <-- end def

    @property
    def moments_yy(self):
        """
        Second moment in yy:
        Sum((y_i - y_o)^2 f_i) / Sum(f_i)
        """
        return float(self._moments_yy)
    # <-- end def 

    @property
    def n_pos(self):
        """
        nPos(npix > 0.0 flux) 
        """
        return int(self._n_pos)
    # <-- end def

    @property
    def f_pos(self):
        """
        fPos/(fPos + fNeg) where fPos = Sum(f_i) if f_i > 0.0, 
        and fNeg = Sum (f_i) if f_i < 0.0 
        """
        return float(self._f_pos)
    # <-- end def 

    @property
    def ratio_bad(self):
        """
        nPos/(nPos + nNeg) where nPos = number of pixels with f_i > 0.0, 
        and nNeg = number of pixels with f_i < 0.0
        """
        return float(self._ratio_bad)
    # <-- end def

    @property
    def ratio_mask(self):
        """
        nPos/(nPos + nMask) where nMask = number of masked pixels 
        """
        return float(self._ratio_mask)
    # <-- end def 

    @property
    def ratio_all(self):
        """
        nPos / (nGood + nMask + nNeg)
        """
        return float(self._ratio_all)
    # <-- end def

    @property
    def flags(self):
        """
        DEFAULT        0x0000.0000     Initial value: resets all bits
        PSFMODEL       0x0000.0001     Source fitted with a psf model (linear or non-linear)
        EXTMODEL       0x0000.0002     Source fitted with an extended-source model
        FITTED         0x0000.0004     Source fitted with non-linear model (PSF or EXT; good or bad)
        FITFAIL        0x0000.0008     Fit (non-linear) failed (non-converge, off-edge, run to zero)
        POORFIT        0x0000.0010     Fit succeeds, but low-SN, high-Chisq, or large (for PSF -- drop?)
        PAIR           0x0000.0020     Source fitted with a double psf
        PSFSTAR        0x0000.0040     Source used to define PSF model
        SATSTAR        0x0000.0080     Source model peak is above saturation
        BLEND          0x0000.0100     Source is a blend with other sourcers
        EXTERNALPOS    0x0000.0200     Source based on supplied input position
        BADPSF         0x0000.0400     Failed to get good estimate of object's PSF
        DEFECT         0x0000.0800     Source is thought to be a defect
        SATURATED      0x0000.1000     Source is thought to be saturated pixels (bleed trail)
        CR_LIMIT       0x0000.2000     Source has crNsigma above limit
        EXT_LIMIT      0x0000.4000     Source has extNsigma above limit
        MOMENTS_FAILURE 0x0000.8000     could not measure the moments
        SKY_FAILURE    0x0001.0000     could not measure the local sky
        SKYVAR_FAILURE 0x0002.0000     could not measure the local sky variance
        MOMENTS_SN     0x0004.0000     moments not measured due to low S/N
        BIG_RADIUS     0x0008.0000     poor moments for small radius, try large radius
        AP_MAGS        0x0010.0000     source has an aperture magnitude
        BLEND_FIT      0x0020.0000     source was fitted as a blend
        EXTENDED_FIT   0x0040.0000     full extended fit was used
        EXTENDED_STATS 0x0080.0000     extended aperture stats calculated
        LINEAR_FIT     0x0100.0000     source fitted with the linear fit
        NONLINEAR_FIT  0x0200.0000     source fitted with the non-linear fit
        RADIAL_FLUX    0x0400.0000     radial flux measurements calculated
        SIZE_SKIPPED   0x0800.0000     Warning:: if set, size could be determined 
        ON_SPIKE       0x1000.0000     peak lands on diffraction spike
        ON_GHOST       0x2000.0000     peak lands on ghost or glint
        OFF_CHIP       0x4000.0000     peak lands off edge of chip 
        """
        return int(self._flags)
    # <-- end def 

    @property
    def ipp_idet(self):
        """IPP detection id (increment from 0 per chip - gaps are from magic) """
        return int(self._ipp_idet)
    # <-- end def

    @property
    def psf_inst_flux(self):
        """PSF instrumental flux"""
        return float(self._psf_inst_flux)
    # <-- end def 

    @property
    def psf_inst_flux_sig(self):
        """Estimated PSF instrumental flux error"""
        return float(self._psf_inst_flux_sig)
    # <-- end def
    
    @property
    def ap_mag(self):
        """Aperture mag"""
        return float(self._ap_mag)
    # <-- end def 

    @property
    def ap_mag_raw(self):
        """
        """
        return float(self._ap_mag_raw)
    # <-- end def

    @property
    def ap_mag_radius(self):
        """Aperture mag radius (pixels) """
        return float(self._ap_mag_radius)
    # <-- end def 

    @property
    def ap_flux(self):
        """
        """
        return float(self._ap_flux)
    # <-- end def

    @property
    def ap_flux_sig(self):
        """
        """
        return float(self._ap_flux_sig)
    # <-- end def 

    @property
    def peak_flux_as_mag(self):
        """
        flux (counts/pixel) for the peak pixel expressed as an instrumental 
        magnitude (-2.5*log(flux))
        """
        return float(self._peak_flux_as_mag)
    # <-- end def

    @property
    def cal_psf_mag(self):
        """Calibrated PSF magnitude"""
        return float(self._cal_psf_mag)
    # <-- end def 

    @property
    def cal_psf_mag_sig(self):
        """Calibrated PSF magnitude error"""
        return float(self._cal_psf_mag_sig)
    # <-- end def 
    
    @property
    def sky(self):
        """Background in counts (DN, not electrons)"""
        return float(self._sky)
    # <-- end def 

    @property
    def sky_sigma(self):
        """Error in sky counts"""
        return float(self._sky_sigma)
    # <-- end def

    @property
    def psf_qf_perfect(self):
        """
        The psf-weighted fraction of unmasked pixels (including suspect pixels): 
        Sum_i(psf_i * !masked) / Sum_i(psf_i)
        """
        return float(self._psf_qf_perfect)
    # <-- end def 

    @property
    def moments_r1(self):
        """
        First radial moment (used to define the Kron radius = 2.5 R1)
        Sum r(f_i)/Sum(f_i) where r is radius from the centroid 
        """
        return float(self._moments_r1)
    # <-- end def

    @property
    def moments_rh(self):
        """
        Half radial moment (an indicator of concentration)
        Sum sqrt(r) (f_i) / Sum(f_i) where r is radius from the centroid 
        """
        return float(self._moments_rh)
    # <-- end def 

    @property
    def kron_flux(self):
        """Flux within Kron Radius (2.5 * first radial moment)"""
        return float(self._kron_flux)
    # <-- end def

    @property
    def kron_flux_err(self):
        """Kron flux error"""
        return float(self._kron_flux_err)
    # <-- end def 

    @property
    def kron_flux_inner(self):
        """Flux within annulus (1.0 R1 < R < 2.5 R1)"""
        return float(self._kron_flux_inner)
    # <-- end def

    @property
    def kron_flux_outer(self):
        """Flux within annulus (2.5 R1 < R < 4.0 R1) """
        return float(self._kron_flux_outer)
    # <-- end def 

    @property
    def diff_r_p(self):
        """Distance to matched source in positive image """
        return float(self._diff_r_p)
    # <-- end def

    @property
    def diff_sn_p(self):
        """Signal-to-noise of matched source in positive image """
        return float(self._diff_sn_p)
    # <-- end def 

    @property
    def diff_r_m(self):
        """Distance to matched source in negative image"""
        return float(self._diff_r_m)
    # <-- end def
    
    @property
    def diff_sn_m(self):
        """Signal-to-noise of matched source in negative image"""
        return float(self._diff_sn_m)
    # <-- end def 

    @property
    def flags2(self):
        """
        DIFF_WITH_SINGLE 0x0000.0001     diff source matched to a single positive detection
        DIFF_WITH_DOUBLE 0x0000.0002     diff source matched to positive detections in both images
        MATCHED          0x0000.0004     source was supplied at this location from somewhere else (eg, another image, forced photometry location, etc)
        ON_SPIKE         0x0000.0008     > 25% of (PSF-weighted) pixels land on diffraction spike
        ON_STARCORE      0x0000.0010     > 25% of (PSF-weighted) pixels land on star core
        ON_BURNTOOL      0x0000.0020     > 25% of (PSF-weighted) pixels land on burntool subtraction region
        ON_CONVPOOR      0x0000.0040     > 25% of (PSF-weighted) pixels land on region where convolution had substantial masked fraction contribution
        PASS1_SRC        0x0000.0080     source was detected in the first pass of psphot (bright detection stage) 
        """
        return int(self._flags2)
    # <-- end def

    @property
    def n_frames(self):
        """
        This is not currently set, but it is supposed to have the value of the 
        number of input images overlapping the center of the source (for 
        warp-warp diff, it would be 2, for a single image it would be 1, for a 
        stack it may be something else).
        """
        return int(self._n_frames)
    # <-- end def 
# <-- end class
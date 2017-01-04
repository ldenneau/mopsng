
/* IPP ubercal mags.
ra_mops,dec_mops,obsID_mops,detID_mops,ra,dec,mag,mag_err,mag_inst,mag_aper,time,exptime,airmass,expname,psf_qf,photflags,photcode,detID,
. */
drop table if exists calmag;
create table calmag(
    det_id bigint not null auto_increment       comment 'MOPS internal detection ID',
    mag float not null                          comment 'Apparent magnitude of detection',
    mag_sigma float                             comment 'Uncertainty in magnitude',
    mag_inst float                              comment 'Instrumental magnitude',
    mag_aper float                              comment 'Aperture magnitude',
    psf_quality float                           comment 'IPP PSF quality',
    photflags int                               comment 'IPP photometry flags',
    photcode int                                comment 'IPP photometry code',

    FOREIGN KEY (det_id) REFERENCES detections(det_id) ON DELETE CASCADE
) engine=InnoDB;

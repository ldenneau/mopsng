/* Per-detection post-fit residuals.  These are current or "instantaneous"
post-fit residuals for derived objects. */
drop table if exists residuals;
create table residuals(
    det_id bigint not null              comment 'ID of detection for this post-fit residual',
    tracklet_id bigint not null         comment 'ID of tracklet for this post-fit residual',

    ra_resid_deg float                  comment 'Residual in RA direction (multiplied by cos(dec))',
    dec_resid_deg float                 comment 'Residual in DEC',
    ra_error_deg float                  comment 'RA error, deg',
    dec_error_deg float                 comment 'DEC error, deg',

    ast_reject char(1)                  comment 'Set when astrometry reject flag raised for this det',
    
    PRIMARY KEY (det_id, tracklet_id),
    FOREIGN KEY (det_id) REFERENCES `detections`(det_id) ON DELETE CASCADE,
    FOREIGN KEY (tracklet_id) REFERENCES `tracklets`(tracklet_id) ON DELETE CASCADE
) engine=InnoDB;

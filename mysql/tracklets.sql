drop table if exists tracklets;
create table tracklets(
    tracklet_id bigint not null auto_increment  comment 'Auto-generated internal MOPS tracklet ID',
    field_id bigint not null                    comment 'Terminating field ID',
    v_ra float                                  comment 'Average RA velocity deg/day, cos(dec) applied',
    v_dec float                                 comment 'Average Dec velocity, deg/day)',
    v_tot float                                 comment 'Average total velocity, deg/day',
    v_ra_sigma float                            comment 'Uncertainty in RA velocity',
    v_dec_sigma float                           comment 'Uncertainty in Dec velocity',
    pos_ang_deg float                           comment 'Position angle, degrees',
    gcr_arcsec float                            comment 'great-circle residual, arcsec',

    ext_epoch real                              comment 'Extrapolated (central) epoch, MJD',
    ext_ra real                                 comment 'Extrapolated (central) RA, deg',
    ext_dec real                                comment 'Extrapolated (central) Dec, deg',
    ext_mag real                                comment 'Extrapolated (central) magnitude',

    probability float                           comment 'Likelihood tracklet is real (unused currently)',
    digest float                                comment 'NEO digest',
    status char(1)                              comment 'processing status (unfound X, unattributed U, attributed A)',
    classification char(1)                      comment 'MOPS efficiency classification',
    ssm_id bigint null                          comment 'Matching SSM ID for clean classifications',
    known_id bigint null                        comment 'Optional externally-assigned known object ID',

    primary key(tracklet_id),
    FOREIGN KEY (field_id) REFERENCES `fields`(field_id) ON DELETE CASCADE,
    FOREIGN KEY (ssm_id) REFERENCES ssm(ssm_id) ON DELETE CASCADE,
    FOREIGN KEY (known_id) REFERENCES known(known_id) ON DELETE CASCADE,
    index(field_id),
    index(ssm_id),
    index(known_id),
    index(classification),
    index(ext_epoch)
) engine=InnoDB;

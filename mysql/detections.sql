/* Detections. */
drop table if exists detections;
create table detections(
    det_id bigint not null auto_increment       comment 'Auto-defined MOPS internal detection ID',
    field_id bigint                             comment 'Source field ID containing this detection',
    ra_deg real not null                        comment 'RA, deg',
    dec_deg real not null                       comment 'Dec, deg',
    epoch_mjd real not null                     comment 'Exact time of detection acquistion, not necessarily constant across field',
    mag float not null                          comment 'Apparent magnitude of detection',
    ref_mag float                               comment 'Reference magnitude before shape modulation applied (for synth dets)',
    filter_id char(2) not null                  comment 'Filter used for this detection',
    s2n float                                   comment 'Detection signal-to-noise',
    /*rep_ra_sigma_deg float                      comment 'Reported 1-sigma uncertainty in RA, deg',*/
    /*rep_dec_sigma_deg float                     comment 'Reported 1-sigma uncertainty in Dec, deg',*/
    ra_sigma_deg float                          comment 'Working 1-sigma uncertainty in RA, deg',
    dec_sigma_deg float                         comment 'Working 1-sigma uncertainty in Dec, deg',
    mag_sigma float                             comment 'Uncertainty in magnitude',
    orient_deg float                            comment 'Orientation of detection, deg; 0 => N, + toward E',
    length_deg float                            comment 'Length of detection, deg',
    object_name varbinary(32)                   comment 'Originating SSM object name, if synthetic',
    obscode char(3)                             comment 'MPC observatory code',

    is_synthetic char(1)                        comment 'efficiency marker; indicates detection is synthetic',
    det_num bigint                              comment 'zero-based detection number in source IPP FITS file',
    status char(1)                              comment 'efficiency marker; indicates detection was detected (not lost in chip gap, etc.)',
    xyidx int                                   comment 'XY spatial index number within field',
    proc_id bigint                              comment 'opaque IPP processing identifier (usually DIFFIM_ID)',

    dev_x float                                 comment 'device X coordinate',
    dev_y float                                 comment 'device Y coordinate',
    dev_id bigint                               comment 'device ID',

    PRIMARY KEY (det_id),
    FOREIGN KEY (filter_id) REFERENCES filter_info(filter_id) ON DELETE CASCADE,
    FOREIGN KEY (field_id) REFERENCES `fields`(field_id) ON DELETE CASCADE,
    index(field_id),
    index(object_name),
    index(epoch_mjd),
    index(field_id, xyidx)
) engine=InnoDB;

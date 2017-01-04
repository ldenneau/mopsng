/* Detection Region. */
drop table if exists detection_region;
create table detection_region(
    det_id bigint not null                      comment 'MOPS internal detection ID',
    ra_deg real not null                        comment 'Central RA of HEALPix region, deg',
    dec_deg real not null                       comment 'Central Dec of HEALPix region, deg',
    region_idx int not null                     comment 'HEALPix region index number',

    PRIMARY KEY (det_id),
    FOREIGN KEY (det_id) REFERENCES detections(det_id) ON DELETE CASCADE,
    index(region_idx),
    index(ra_deg, dec_deg)
) type=InnoDB;

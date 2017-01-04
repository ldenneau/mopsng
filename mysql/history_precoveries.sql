/* Associate tracklets with attribution events.  One tracklet per event. */
drop table if exists history_precoveries;
create table history_precoveries(
    event_id bigint not null            comment 'Parent event ID (from history table)',
    tracklet_id bigint not null         comment 'Precovered tracklet ID',
    ephemeris_distance float not null   comment 'Predicted minus actual position, arcsecs',
    ephemeris_uncertainty float	        comment 'Predicted error ellipse semi-major axis, arcsecs',

    PRIMARY KEY (event_id),
    FOREIGN KEY (tracklet_id) REFERENCES tracklets(tracklet_id) ON DELETE CASCADE,
    FOREIGN KEY (event_id) REFERENCES history(event_id) ON DELETE CASCADE
) engine=InnoDB;

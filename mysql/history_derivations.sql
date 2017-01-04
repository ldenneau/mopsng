/* Associate tracklets with derivation events.  Note
here there will be multiple rows per event. */
drop table if exists history_derivations;
create table history_derivations(
    event_id bigint not null            comment 'Parent event ID (from history table)',
    tracklet_id bigint not null         comment 'Associated tracklet ID (multiple rows per event)',

	PRIMARY KEY (event_id, tracklet_id),
	FOREIGN KEY (tracklet_id) REFERENCES tracklets(tracklet_id) ON DELETE CASCADE,
	FOREIGN KEY (event_id) REFERENCES history(event_id) ON DELETE CASCADE
) engine=InnoDB;

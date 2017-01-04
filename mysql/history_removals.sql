/* Associate tracklets and derived objects with removal events. */
drop table if exists history_removals;
create table history_removals(
    event_id bigint not null                comment 'Parent event ID (from history table)',
    object_id bigint not null               comment 'ID of the removed item',
    object_type varchar(128) not null       comment 'Type of object removed. T = Tracklet, D = Derived Object',

    PRIMARY KEY (event_id),
    FOREIGN KEY (event_id) REFERENCES history(event_id) ON DELETE CASCADE
) engine=InnoDB;

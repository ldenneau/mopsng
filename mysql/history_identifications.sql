/* Associate derived objects with identifcation events.  The original
orbit and tracklets for the child can be obtained from the events table
by looking up the child object. */
drop table if exists history_identifications;
create table history_identifications(
    event_id bigint not null            comment 'Parent event ID (from history table)',
    childobject_id bigint not null      comment 'Matching (child) derived object ID',
    parent_orbit_id bigint not null     comment 'Parent orbit ID',
    child_orbit_id bigint not null      comment 'Child orbit ID',

    PRIMARY KEY (event_id),
    FOREIGN KEY (childobject_id) REFERENCES derivedobjects(derivedobject_id) ON DELETE CASCADE,
    FOREIGN KEY (event_id) REFERENCES history(event_id) ON DELETE CASCADE,
    FOREIGN KEY (parent_orbit_id) REFERENCES orbits(orbit_id) ON DELETE CASCADE,
    FOREIGN KEY (child_orbit_id) REFERENCES orbits(orbit_id) ON DELETE CASCADE
) engine=InnoDB;

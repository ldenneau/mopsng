drop table if exists history;
create table history(
    event_id bigint not null auto_increment     comment 'Auto-generated internal event ID',
    event_type char(1) not null                 comment 'Type of event (A)ttribution/(P)recovery/(D)erivation/(I)dentification/(R)emoval',
    event_time timestamp                        comment 'Timestamp for event creation',

    derivedobject_id bigint                     comment 'Referring derived object ID',
    orbit_id bigint                             comment 'Pointer to resulting orbit',
    orbit_code char(1)                          comment 'Information about computed orbit',

    d3 float                                    comment 'Computed 3-parameter D-criterion',
    d4 float                                    comment 'Computed 4-parameter D-criterion',

    field_id bigint                             comment 'Referring field ID generating the event',
    classification char(1)                      comment 'MOPS efficiency classification for event',
    ssm_id bigint                               comment 'Matching SSM ID for clean classifications',
    is_lsd char(1) not null                     comment 'Y/N if event involves LSD detection',
    last_event_id bigint                        comment 'refers to last event that modified this object',

    PRIMARY KEY (event_id),
    FOREIGN KEY (derivedobject_id) REFERENCES derivedobjects(derivedobject_id) ON DELETE CASCADE,
    FOREIGN KEY (orbit_id) REFERENCES orbits(orbit_id) ON DELETE CASCADE,
    FOREIGN KEY (field_id) REFERENCES `fields`(field_id) ON DELETE CASCADE,
    FOREIGN KEY (ssm_id) REFERENCES ssm(ssm_id) ON DELETE CASCADE,
    FOREIGN KEY (last_event_id) REFERENCES history(event_id) ON DELETE CASCADE,
    index(derivedobject_id),
    index(field_id)
) engine=InnoDB;

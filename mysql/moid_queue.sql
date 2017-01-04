drop table if exists moid_queue;
create table moid_queue(
    orbit_id bigint not null                        comment 'Referring orbit',
    event_id bigint not null                        comment 'Referring history event causing insertion',
    insert_timestamp timestamp not null             comment 'Wall clock time object was queued',
    
    PRIMARY KEY (orbit_id),
    FOREIGN KEY (orbit_id) REFERENCES orbits(orbit_id) ON DELETE CASCADE,
    FOREIGN KEY (event_id) REFERENCES history(event_id) ON DELETE CASCADE
) engine=InnoDB;

drop table if exists eon_queue;
create table eon_queue(
    derivedobject_id bigint not null                comment 'Referring derived object',
    event_id bigint not null                        comment 'Referring history event causing insertion',
    insert_timestamp timestamp not null             comment 'Wall clock time object was queued',
    status char(1) default 'N'                      comment 'Processing status N => new, I => ID1 done, P => precov done, X => finished',
    
    PRIMARY KEY (derivedobject_id),
    FOREIGN KEY (derivedobject_id) REFERENCES derivedobjects(derivedobject_id) ON DELETE CASCADE,
    FOREIGN KEY (event_id) REFERENCES history(event_id) ON DELETE CASCADE
) engine=InnoDB;

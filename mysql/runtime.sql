drop table if exists runtime;
create table runtime (
    event_id bigint not null auto_increment,
    subsystem varchar(20) not null,
    message varchar(250) not null,
    data bigint,
    date_submitted timestamp not null default current_timestamp,
    status char(1),

    primary key(event_id)
) engine=InnoDB;

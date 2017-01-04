drop table if exists config;
create table config(
    config_id bigint not null auto_increment        comment 'Referring derived object',
    config_text text                                comment 'Config contents',
    created TIMESTAMP DEFAULT CURRENT_TIMESTAMP     comment 'Creation timestamp',

    PRIMARY KEY (config_id)
) engine=InnoDB;

drop table if exists ssm_desc;
create table ssm_desc(
    desc_id bigint not null auto_increment  comment 'Auto-generated row ID',
    prefix char(4)                          comment 'MOPS prefix code S0/S1/etc.',
    description varchar(100)                comment 'Long description',

    PRIMARY KEY (desc_id)
) engine=InnoDB;


insert into ssm_desc (prefix, description) values ( 'S0', 'MOPS synthetic NEO');
insert into ssm_desc (prefix, description) values ( 'S1', 'MOPS synthetic main-belt object');
insert into ssm_desc (prefix, description) values ( 'St', 'MOPS synthetic Trojan');
insert into ssm_desc (prefix, description) values ( 'SC', 'MOPS synthetic Centaur');
insert into ssm_desc (prefix, description) values ( 'ST', 'MOPS synthetic trans-Neptunian object');
insert into ssm_desc (prefix, description) values ( 'SS', 'MOPS synthetic scattered disk object');
insert into ssm_desc (prefix, description) values ( 'Sc', 'MOPS synthetic comet');
insert into ssm_desc (prefix, description) values ( 'SM', 'MOPS synthetic control object');

drop table if exists filter_info;
create table filter_info(
    filter_id char(2) not null      comment 'Single-character (usually) filter specification, g/r/i/z/y/w',
    filter_desc varchar(40)         comment 'Description',
    primary key(filter_id)
) engine=InnoDB;

insert into filter_info values ('g', 'g-band');
insert into filter_info values ('r', 'r-band');
insert into filter_info values ('i', 'i-band');
insert into filter_info values ('z', 'z-band');
insert into filter_info values ('y', 'y-band');
insert into filter_info values ('u', 'u-band');
insert into filter_info values ('w', 'w-band');
insert into filter_info values ('V', 'V-band');
insert into filter_info values ('o', 'orange');
insert into filter_info values ('c', 'cyan');

drop table if exists locks;
create table locks(
       table_name varchar(30)          comment 'name of the table to lock/unlock',
       status char(1) default 'N'      comment 'Lock status N => non locked, L => locked'
) engine=InnoDB;

insert into locks values ('aqueue', 'N');

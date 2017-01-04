/* Need DB, grants

These tables live in the EXPERIMENTS database, a global DB for all MOPS processing.

create database experiments;
grant select, insert, update, delete on experiments.* to 'mopspipe'@'%' identified by 'epip';
grant select, insert, update, delete on experiments.* to 'mopspipe'@'localhost' identified by 'epip';
grant select, insert on experiments.* to 'mops'@'%' identified by 'mops';
grant select, insert on experiments.* to 'mops'@'localhost' identified by 'mops';
flush privileges;

Use cases:

1. Enter a tracklet for submission to MPC.  User/script must provide
  - DB/tracklet ID
  - username

*/

/*
Disposition codes:
P : pending NEO submission
Q : pending std submission
S : submitted NEO
T : submitted std (non-NEO)
C : confirmed NEO, discovery
A : confirmed NEO, discovery, PHA
R : recovered NEO
N : non-NEO discovery
L : lost
K : submitted as new NEO, but known
D : downgraded
O : submitted via outreach (IASC, etc.)
*/

/* Sequence for designations. */
drop table if exists desig_seq;
create table desig_seq(
    seq_num bigint not null auto_increment,
    PRIMARY KEY(seq_num)
) engine=InnoDB;
/* alter table desig_seq auto_increment=916136677 (P100100 = 62^5 + 62^2) = 916136677 */


/* Sequence for batches.  Every email submission will have its own batch number. */
drop table if exists batch_seq;
create table batch_seq(
    seq_num bigint not null auto_increment,
    PRIMARY KEY(seq_num)
) engine=InnoDB;


drop table if exists mpc_sub;
create table mpc_sub(
    seq_num bigint comment 'Global submission sequence number',
    batch_num bigint comment 'Batch number',
    fpa_id varbinary(20) not null comment 'PS1 exposure name',
    epoch_mjd double not null comment 'MJD of observation',
    ra_deg double not null comment 'Right ascension in degrees',
    dec_deg double not null comment 'Declination in degrees',
    filter_id char(1) not null comment 'Reported PS1 filter',
    mag float not null comment 'Reported mag',
    obscode char(3) not null comment 'MPC observatory code',
    desig varbinary(20) not null comment 'Internal designation',
    digest float not null comment 'MPC digest2 score',
    spatial_idx bigint comment 'Spatial index',

    survey_mode varbinary(40) comment 'PS1 survey mode',
    dbname varbinary(20) comment 'MOPS source DB name',
    det_id bigint comment 'MOPS detection ID (if available)',
    tracklet_id bigint comment 'MOPS tracklet ID (if available)',
    derivedobject_id bigint comment 'MOPS derived object ID (if available)',
    submitter varbinary(20) not null comment 'Submitter initials',
    submitted timestamp not null comment 'Date submitted or updated (UT)',
    disposition char(1) not null comment '1-character status (S/C/D/N)',
    discovery char(1) comment 'Discovery observation',
    mpc_desig varbinary(20) comment 'MPC-assigned designation',
    
    revised char(1) default NULL,

    index(desig),
    index(batch_num),
    index(epoch_mjd),
    index(fpa_id),
    index(submitter),
    index(survey_mode),
    index(mpc_desig),
    index(disposition),
    index(discovery),
    index(tracklet_id)
) engine=InnoDB;

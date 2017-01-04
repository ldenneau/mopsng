/* Master stamp table, across all MOPS DBs.  */

drop table if exists stamp_master;
create table stamp_master(
    stamp_id bigint not null auto_increment,
    stamp_type enum('DIFF', 'CHIP', 'WARP') comment 'IPP processing type (diff, chip, warp)', 
    status enum('R', 'X', 'S') comment 'requested, failed, succeeded', 
    fpa_id varbinary(20) not null comment 'PS1 exposure name', 
    dbname varbinary(40) not null comment 'PS1 database name', 
    det_id bigint,
    epoch_mjd double comment 'MJD UTC of exposure', 
    ra_deg double,
    dec_deg double,
    path varbinary(100) comment 'full filesystem path to stamp', 
   
    PRIMARY KEY(stamp_id),
    index(fpa_id),
    index(det_id),
    index(path)
) engine=InnoDB;

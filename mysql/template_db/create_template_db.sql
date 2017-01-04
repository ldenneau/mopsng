/* You must be MySQL root or equivalent to execute this. */

/* Create database. */
drop database if exists mops_template_db;
create database mops_template_db;

/* Read-write privileges for 'mops' user. */
grant all privileges on mops_template_db.* to 'mops'@'localhost' identified by 'mops';
grant all privileges on mops_template_db.* to 'mops'@'%' identified by 'mops';

/* Read-only privileges for 'mops' user. */
grant select, insert, update on mops_template_db.* to 'mopsuser'@'localhost' identified by 'mops';
grant select, insert, update on mops_template_db.* to 'mopsuser'@'%' identified by 'mops';

flush privileges;

/* Create table. */
use mops_template_db;

drop table if exists template_surveys;
create table template_surveys(
    survey_id bigint not null auto_increment    comment 'Auto-assigned private survey ID',
    survey_name varchar(50)                     comment 'Public survey name',
    description varchar(200)                    comment 'Friendly description',

    PRIMARY KEY(survey_id)
) type=InnoDB;


drop table if exists template_fields;
create table template_fields(
    survey_id bigint not null               comment 'Owning survey ID from template_surveys',
    epoch_mjd real not null                 comment 'Time (epoch) of field acquisition, MJD',
    ra_deg real not null                    comment 'RA, deg, of field center',
    dec_deg real not null                   comment 'Dec, deg, of field center',
    ra_sigma real                           comment 'Uncertainty in RA',
    dec_sigma real                          comment 'Uncertainty in Dec',
    filter_id char(2)                       comment 'Filter used for acquisition, g/r/i/z/y/w',
    limiting_mag real                       comment 'Field limiting magnitude',
    obscode char(3)                         comment 'MPC observatory code',
    time_start real                         comment 'Time of field start acquisition, MJD',
    time_stop real                          comment 'Time of field stop acquisition, MJD',
    de1 real                                comment 'IPP-defined detection efficiency 1',
    de2 real                                comment 'IPP-defined detection efficiency 2',
    de3 real                                comment 'IPP-defined detection efficiency 3',
    de4 real                                comment 'IPP-defined detection efficiency 4',
    de5 real                                comment 'IPP-defined detection efficiency 5',
    de6 real                                comment 'IPP-defined detection efficiency 6',
    de7 real                                comment 'IPP-defined detection efficiency 7',
    de8 real                                comment 'IPP-defined detection efficiency 8',
    de9 real                                comment 'IPP-defined detection efficiency 9',
    de10 real                               comment 'IPP-defined detection efficiency 10',
    survey_mode varchar(3)                  comment 'PS-defined survey mode, e.g. ESS, MSS, AP, 3PI, CAL',
    ocnum int                               comment 'pre-computed observing cycle number',
    nn int                                  comment 'pre-computed night number',

    FOREIGN KEY(survey_id) REFERENCES template_surveys(survey_id) ON DELETE CASCADE,
    PRIMARY KEY(survey_id, epoch_mjd, ra_deg, dec_deg),
    index(survey_id),
    index(epoch_mjd),
    index(ocnum),
    index(nn)
) type=InnoDB;

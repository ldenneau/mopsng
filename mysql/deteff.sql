drop table if exists deteff;
create table deteff(
    nn int comment 'MOPS night number',
    filter_id char(1) comment 'PS1 filter',
    eff_type enum('TRACKLET', 'DETECTION') comment 'type of efficiency, tracklet or detection',
    bin_start float comment 'Start of magnitude bin',
    bin_end float comment 'End of magnitude bin',
    known int comment 'Number of known objects in bin',
    found int comment 'Number of found objects in bin',

    index(nn)
) engine=InnoDB;


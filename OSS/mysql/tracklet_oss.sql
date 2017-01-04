/* Linkage table between OSS provided ID and MOPS generated tracklet ID. */
drop table if exists tracklet_oss;
create table tracklet_oss(
    tracklet_id bigint not null,
    oss_id char(22) NOT NULL,

    primary key(tracklet_id, oss_id),
    FOREIGN KEY (tracklet_id) REFERENCES tracklets(tracklet_id) ON DELETE CASCADE,
    index(tracklet_id),
    index(oss_id)
) type=InnoDB;

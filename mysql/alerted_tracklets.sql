drop table if exists alerted_tracklets;
create table alerted_tracklets(
    tracklet_id bigint not null                comment 'Referring tracklet',
    alert_timestamp timestamp not null        comment 'Wall clock time tracklet was alerted on',
    
    PRIMARY KEY (tracklet_id, alert_timestamp),
    FOREIGN KEY (tracklet_id) REFERENCES tracklets(tracklet_id) ON DELETE CASCADE
) engine=InnoDB;

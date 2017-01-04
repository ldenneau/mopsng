/* Tracklet membership.  Associate detections with tracklets. */
drop table if exists tracklet_attrib;
create table tracklet_attrib(
    tracklet_id bigint not null,
    det_id bigint not null,

    primary key(tracklet_id, det_id),
    FOREIGN KEY (tracklet_id) REFERENCES tracklets(tracklet_id) ON DELETE CASCADE,
    FOREIGN KEY (det_id) REFERENCES detections(det_id) ON DELETE CASCADE,
    index(tracklet_id),
    index(det_id)
) engine=InnoDB;

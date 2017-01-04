/* Current membership of tracklets and derived objects. */
drop table if exists derivedobject_attrib;
create table derivedobject_attrib(
    derivedobject_id bigint not null,
    tracklet_id bigint not null,

	FOREIGN KEY (derivedobject_id) REFERENCES derivedobjects(derivedobject_id) ON DELETE CASCADE,
	FOREIGN KEY (tracklet_id) REFERENCES tracklets(tracklet_id) ON DELETE CASCADE,
    index(derivedobject_id),
    index(tracklet_id)
) engine=InnoDB;

drop table if exists derivedobjects;
create table derivedobjects(
    derivedobject_id bigint not null auto_increment     comment 'Auto-assigned internal MOPS numeric ID',
    orbit_id bigint                                     comment 'Pointer to current orbit',
    object_name varbinary(32)                           comment 'MOPS base-64 object name',

    taxonomic_type char(2)                              comment 'Asteroid taxonomic type',
    rotation_period real                                comment 'Rotation period, days',
    amplitude real                                      comment 'Amplitude, magnitudes (dimensionless)',
    rotation_epoch real                                 comment 'Rotation time origin, MJD',
    g real                                              comment 'Slope parameter g',
    albedo real                                         comment 'Albedo, dimensionless',
    pole_lat real                                       comment 'Rotation pole latitude, deg',
    pole_long real                                      comment 'Rotation pole longitude, deg',

    /* Efficiency/history */
    classification char(1)                              comment 'MOPS efficiency classification (C/M/B/N/X)',
    ssm_id bigint                                       comment 'Source SSM obje t for C classification',

    /* Various other status/info */
    status char(1)                                      comment 'NULL, or set to M when DO is merged with parent',
    stable_pass char(1)                                 comment 'NULL, or set to Y when stable precovery pass completed',
    mops_classification char(3)                         comment '3-char MOPS asteroid classification',
    created timestamp                                   comment 'Timestamp for derived object creation',
    updated timestamp                                   comment 'Timestamp for derived object modification',

    PRIMARY KEY (derivedobject_id),
    FOREIGN KEY (orbit_id) REFERENCES orbits(orbit_id) ON DELETE CASCADE,
    FOREIGN KEY (ssm_id) REFERENCES ssm(ssm_id) ON DELETE CASCADE,
    index(object_name),
    index(ssm_id),
    index(status),
    index(mops_classification),
    index(updated)
) engine=InnoDB;


drop table if exists ssm;
create table ssm(
    ssm_id bigint not null auto_increment,
    q real not null             comment 'semi-major axis, AU',
    e real not null             comment 'eccentricity e (dimensionless)',
    i real not null             comment 'inclination, deg',
    node real not null          comment 'longitude of ascending node, deg',
    arg_peri real not null      comment 'argument of perihelion, deg',
    time_peri real not null     comment 'time of perihelion, MJD',
    epoch real not null         comment 'epoch of osculating elements, MJD',
    h_v real not null           comment 'Absolute magnitude',
    h_ss real                   comment '??',
    g real                      comment 'Slope parameter g, dimensionless',
    albedo real                 comment 'Albedo, dimensionless',

    moid_1 real                 comment 'Vestigial, deprecated',
    moid_long_1 real            comment 'Vestigial, deprecated',
    moid_2 real                 comment 'Vestigial, deprecated',
    moid_long_2 real            comment 'Vestigial, deprecated',

    object_name varbinary(32) unique not null   comment 'MOPS synthetic object name',
    taxonomic_type char(2)                  comment 'Asteroid taxonomic type (vestigial, deprecated)',
    desc_id bigint null                     comment 'Pointer to SSM description',

    PRIMARY KEY (ssm_id),
    FOREIGN KEY (desc_id) REFERENCES ssm_desc(desc_id) ON DELETE CASCADE,
    index(object_name),
    index(epoch)
) engine=InnoDB;

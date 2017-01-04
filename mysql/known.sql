drop table if exists known;
create table known(
    known_id bigint not null auto_increment     comment 'internal table/row ID',
    known_name varbinary(50)                    comment 'MPC/IAU name or designation',

    /* Orbit computed by KNOWN_SERVER, primarily for submission to MPC. */
    q real not null             comment 'semi-major axis, AU',
    e real not null             comment 'eccentricity e (dimensionless)',
    i real not null             comment 'inclination, deg',
    node real not null          comment 'longitude of ascending node, deg',
    arg_peri real not null      comment 'argument of perihelion, deg',
    time_peri real not null     comment 'time of perihelion, MJD',
    epoch real not null         comment 'epoch of osculating elements, MJD',
    h_v real not null           comment 'Absolute magnitude',

    PRIMARY KEY (known_id),
    index(known_name)
) engine=InnoDB;

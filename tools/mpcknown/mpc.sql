use mpc;
drop table if exists mpc_obs;
drop table if exists mpc_orb;


create table mpc_orb(
    mpc_name varbinary(50)      comment 'MPC/IAU name or designation',

    q real not null             comment 'semi-major axis, AU',
    e real not null             comment 'eccentricity e (dimensionless)',
    i real not null             comment 'inclination, deg',
    node real not null          comment 'longitude of ascending node, deg',
    arg_peri real not null      comment 'argument of perihelion, deg',
    time_peri real not null     comment 'time of perihelion, MJD',
    epoch real not null         comment 'epoch of osculating elements, MJD',
    h_v real not null           comment 'Absolute magnitude',

    PRIMARY KEY(mpc_name)
) type=InnoDB;

create table mpc_obs(
    mpc_name varbinary(20)          comment 'MPC-assigned designation',
    fpa_id varbinary(20) /*not null*/   comment 'PS1 exposure name',
    epoch_mjd double not null       comment 'MJD of observation',
    ra_deg double not null          comment 'Right ascension in degrees',
    dec_deg double not null         comment 'Declination in degrees',
    filter_id char(1) not null      comment 'Reported PS1 filter',
    mag float not null              comment 'Reported mag',
    obscode char(3) not null        comment 'MPC observatory code',

    dbname varbinary(20)            comment 'MOPS source DB name',
    det_id bigint                   comment 'MOPS detection ID (if available)',
    tracklet_id bigint              comment 'MOPS tracklet ID (if available)',
    discovery char(1)               comment 'Discovery observation',

    /* Useful detection parameters at time of observation. */
    r_au float                      comment 'Earth-object distance',
    delta_au float                  comment 'Sun-object distance',
    alpha_deg float                 comment 'Phase angle',
    true_anom_deg float             comment 'Orbital true anomaly',
    sol_elong_deg float             comment 'Solar elongation',
   
    FOREIGN KEY (mpc_name) REFERENCES mpc_orb(mpc_name) ON DELETE CASCADE,
    INDEX(epoch_mjd),
    INDEX(fpa_id),
    INDEX(mpc_name),
    INDEX(discovery),
    INDEX(tracklet_id)
) type=InnoDB;

grant select on mpc.* to 'mops'@'%' identified by 'mops';
grant select on mpc.* to 'mops'@'localhost' identified by 'mops';
grant all privileges on mpc.* to 'mopspipe'@'%' identified by 'epip';
grant all privileges on mpc.* to 'mopspipe'@'localhost' identified by 'epip';
flush privileges;


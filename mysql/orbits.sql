/* $Id: orbits.sql 5713 2015-07-17 02:30:24Z svnuser $

Orbit covariance matrices are upper-triangular matrices obtained from the JPL
differential corrector.  The values are passed as a linear array whose elements
are numbered as follows:

    EC    QR    TP    OM    W     IN
EC   1     2     4     7    11    16
QR         3     5     8    12    17
TP               6     9    13    18
OM                    10    14    19
W                           15    20
IN                                21

EC = eccentricity
QR = perihelion distance
TP = time perihelion
OM = node
W  = arg perihelion
IN = inclination
*/

drop table if exists orbits;
create table orbits(
    orbit_id bigint not null auto_increment,
   
    q real not null             comment 'semi-major axis, AU',
    e real not null             comment 'eccentricity e (dimensionless)',
    i real not null             comment 'inclination, deg',
    node real not null          comment 'longitude of ascending node, deg',
    arg_peri real not null      comment 'argument of perihelion, deg',
    time_peri real not null     comment 'time of perihelion, MJD',
    epoch real not null         comment 'epoch of osculating elements, MJD',
    h_v real not null           comment 'Absolute magnitude',

    residual real not null      comment 'RMS residual, arcsec',
    chi_squared real            comment 'chi-squared statistic',

    cov_01 real                 comment 'covariance EC EC (see SQL documentation)',
    cov_02 real                 comment 'covariance EC QR (see SQL documentation)',
    cov_03 real                 comment 'covariance QR QR (see SQL documentation)',
    cov_04 real                 comment 'covariance EC TP (see SQL documentation)',
    cov_05 real                 comment 'covariance QR TP (see SQL documentation)',
    cov_06 real                 comment 'covariance TP TP (see SQL documentation)',
    cov_07 real                 comment 'covariance EC OM (see SQL documentation)',
    cov_08 real                 comment 'covariance QR OM (see SQL documentation)',
    cov_09 real                 comment 'covariance TP OM (see SQL documentation)',
    cov_10 real                 comment 'covariance OM OM (see SQL documentation)',
    cov_11 real                 comment 'covariance EC W  (see SQL documentation)',
    cov_12 real                 comment 'covariance QR W  (see SQL documentation)',
    cov_13 real                 comment 'covariance TP W  (see SQL documentation)',
    cov_14 real                 comment 'covariance OM W  (see SQL documentation)',
    cov_15 real                 comment 'covariance W  W  (see SQL documentation)',
    cov_16 real                 comment 'covariance EC IN (see SQL documentation)',
    cov_17 real                 comment 'covariance QR IN (see SQL documentation)',
    cov_18 real                 comment 'covariance TP IN (see SQL documentation)',
    cov_19 real                 comment 'covariance OM IN (see SQL documentation)',
    cov_20 real                 comment 'covariance W  IN (see SQL documentation)',
    cov_21 real                 comment 'covariance IN IN (see SQL documentation)',

    conv_code varchar(8)        comment 'JPL convergence code',

    o_minus_c real              comment 'Vestigial observed-computed position, essentially RMS residual',
    moid_1 real                 comment 'Minimum orbital intersection distance (MOID) solution 1',
    moid_long_1 real            comment 'Longitude of MOID 1',
    moid_2 real                 comment 'Minimum orbital intersection distance (MOID) solution 2',
    moid_long_2 real            comment 'Longitude of MOID 2',
    arc_length_days real        comment 'Arc length of detections used to compute orbit',

    PRIMARY KEY (orbit_id)
) engine=InnoDB;

drop table if exists shapes;
create table shapes(
    shape_id bigint not null auto_increment     comment 'Auto-generated shape ID',
    ssm_id bigint not null                      comment 'SSM ID of described object',

    g real                                      comment 'Slope parameter g (dimensionless)',
    p real                                      comment 'Albedo (dimensionless)',
    period_d real                               comment 'Period, days',
    amp_mag real                                comment 'Maximum amplitude (dimensionless)',
    a_km real                                   comment 'Triaxial diameter a, km',
    b_km real                                   comment 'Triaxial diameter b, km',
    c_km real                                   comment 'Triaxial diameter c, km',
    beta_deg real                               comment 'Rotational pole orientation, deg',
    lambda_deg real                             comment 'Rotational pole orientation, deg',
    ph_a real                                   comment 'Phase parameter a (dimensionless)',
    ph_d real                                   comment 'Phase parameter d (dimensionless)',
    ph_k real                                   comment 'Phase parameter k (dimensionless)',

    PRIMARY KEY (shape_id),
    FOREIGN KEY (ssm_id) REFERENCES ssm(ssm_id) ON DELETE CASCADE,
    index(ssm_id)
) engine=InnoDB;

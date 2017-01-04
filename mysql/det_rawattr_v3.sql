/* Detections. ATLAS-specific detection attributes. */
drop table if exists det_rawattr_v3;
create table det_rawattr_v3(
    det_id bigint                               comment 'Source det_id',

    peak int,                                   /* peak pixel in PSF */
    sky float,                                  /* sky level */
    chin float,                                 /* normalized chi^2 */
    pstar float,                                /* prob star */
    preal float,                                /* prob real */
    star int,                                   /* nearest star flux */
    dstar int,                                  /* dist to star [px] */
    mstar float,                                /* mag of near star */
    dirty int,                                  /* dirtyness of detection (in vartest sense) */
    vtflags varchar(8),                         /* string of vartest codes, any of B, R, V, etc. */

    FOREIGN KEY (det_id) REFERENCES detections(det_id) ON DELETE CASCADE
) engine=InnoDB;

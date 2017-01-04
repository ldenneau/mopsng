drop table if exists `runs`;
CREATE  TABLE `runs` (
  `run_id` BIGINT NOT NULL   comment 'Auto-assigned internal MOPS run ID',
  `field_id` BIGINT NULL     comment 'ID of processed field in the fields table',
  `status` CHAR NULL         comment 'Status up to which field was processed during the run',
  `run_date` TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  `czared` TINYINT(1) NOT NULL DEFAULT 0,

  PRIMARY KEY (`run_id` ASC, `field_id` ASC),
  FOREIGN KEY (field_id) REFERENCES `fields`(field_id) ON DELETE CASCADE,
  index(field_id)
) engine=InnoDB comment='Tracks which fields are processed during a MOPS run';

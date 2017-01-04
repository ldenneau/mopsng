
CREATE TABLE manual_detections (
    det_id   BIGINT(20) NOT NULL COMMENT "Detection ID from this database"
  , related_det_id BIGINT(20) DEFAULT NULL COMMENT 'The detection (if any) which is related to this detection'  
  , mode CHAR(8) NOT NULL COMMENT 'How the detection was created'
  , creator VARCHAR(20) NOT NULL COMMENT 'Who created the detection'
  , creation_timestamp TIMESTAMP NOT NULL DEFAULT NOW() 
  , FOREIGN KEY(det_id) REFERENCES detections(det_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

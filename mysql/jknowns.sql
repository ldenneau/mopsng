# DROP TABLE jKnownsAliases;
# DROP TABLE jKnownsTracklets;
# DROP TABLE jKnowns;

CREATE TABLE jKnowns (
  jKnownId BIGINT NOT NULL AUTO_INCREMENT COMMENT 'Auto-generated internal MOPS known object ID'
, mpcDesignation CHAR(50) COMMENT 'Preferred MPC designation'
, PRIMARY KEY (jKnownId)
, KEY(mpcDesignation)
) Engine=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE jKnownsTracklets (
  jKnownId BIGINT NOT NULL COMMENT 'MOPS known object id'
, tracklet_id BIGINT NOT NULL UNIQUE COMMENT 'MOPS tracklet id'
, matchScore DOUBLE DEFAULT NULL COMMENT 'Score (Average GCR to expected position. In arcseconds)'
, FOREIGN KEY (jKnownId) REFERENCES jKnowns(jKnownId) ON DELETE CASCADE
, FOREIGN KEY (tracklet_id) REFERENCES tracklets(tracklet_id) ON DELETE CASCADE
) Engine=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE jKnownsAliases (
  jKnownId BIGINT NOT NULL COMMENT 'MOPS known object id'
, jKnownAlias CHAR(50) COMMENT 'Any designation that the MPC gave to the object'
, FOREIGN KEY (jKnownId) REFERENCES jKnowns(jKnownId) ON DELETE CASCADE
, KEY(jKnownAlias)
, PRIMARY KEY(jKnownId, jKnownAlias)
) Engine=InnoDB DEFAULT CHARSET=latin1;

#
# SELECT jKnownId, mpcDesignation, GROUP_CONCAT(jKnownAlias) 
# FROM jKnownsTracklets JOIN jKnowns USING(jKnownId) JOIN jKnownsAliases USING(jKnownId) GROUP BY jKnownId;
#
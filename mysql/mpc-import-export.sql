-- Tables for MPC Import/Export
-- export_dets is linked to table detections with enforcement of referential integrity

-- MySQL dump 10.11
--
-- Host: localhost    Database: psmops_drchang_export_eonqueue_st
-- ------------------------------------------------------
-- Server version	5.0.54-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `export`
--

DROP TABLE IF EXISTS `export`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `export` (
  `revision_id` bigint(20) NOT NULL auto_increment COMMENT 'id of revision record created for handling export',
  `rev_time` datetime default NULL COMMENT 'time of revision processing',
  `tracklet_export_id` bigint(20) default NULL COMMENT 'Tracklet ID of tracklet with known_id',
  `do_export_id` bigint(20) default NULL COMMENT 'ID of Derived Object',
  `export_time` datetime default NULL COMMENT 'time of actual export, status changes from P to E',
  `export_status` char(1) default NULL COMMENT 'P for pending, E for exported',
  PRIMARY KEY  (`revision_id`)
) ENGINE=InnoDB AUTO_INCREMENT=92 DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2009-07-15  8:35:18
-- MySQL dump 10.11
--
-- Host: localhost    Database: psmops_drchang_export_eonqueue_st
-- ------------------------------------------------------
-- Server version	5.0.54-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `export_dets`
--

DROP TABLE IF EXISTS `export_dets`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `export_dets` (
  `det_export_id` bigint(20) NOT NULL auto_increment COMMENT 'detection export id',
  `rev_id` bigint(20) default NULL COMMENT 'id of revision from table export',
  `det_id` bigint(20) default NULL COMMENT 'id of detection from table detections',
  `mpc_accepted` char(1) default NULL COMMENT '{Y|N} for accepted by MPC',
  `removal` char(1) default NULL COMMENT 'flag to indicate that the detection is to be removed',
  PRIMARY KEY  (`det_export_id`),
  KEY `rev_id` (`rev_id`),
  KEY `det_id` (`det_id`),
  CONSTRAINT `export_dets_ibfk_2` FOREIGN KEY (`det_id`) REFERENCES `detections` (`det_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `export_dets_ibfk_1` FOREIGN KEY (`rev_id`) REFERENCES `export` (`revision_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=251 DEFAULT CHARSET=utf8;
SET character_set_client = @saved_cs_client;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2009-07-15  8:35:55

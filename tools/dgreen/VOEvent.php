<?php

// Connect to MOPS export database
$db_host = "lsb02.psps.ifa.hawaii.edu";
$db_username = "mops";
$db_pass = "!m0PS!";
$db_name = "export";

$connection = mysql_connect($db_host, $db_username, $db_pass);
if (!$connection) {
	die("Could not connect to MySQL");
}
$db_select = mysql_select_db($db_name); 
if (!$db_select) {
	die("No database");
}

// Get passed in parameters.
$alert_id = $_GET["alertId"];
$rule = $_GET["rule"];

// Query database for VOEvent
$sql = "select vo_event from export.alerts where alert_id = $alert_id";
$result = mysql_query($sql) or dir("Could not execute the query: " . mysql_error());
$row = mysql_fetch_row($result);
$vo_event = $row[0];

// Close the database connection.
mysql_close($connection);

//Set the content-type header to xml and output the VOEvent
header("Content-type: text/xml");
echo $vo_event;
?>
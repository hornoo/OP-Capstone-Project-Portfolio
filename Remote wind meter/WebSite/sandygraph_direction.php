<?php



include 'connect.inc.php';

require_once ('jpgraph/jpgraph.php');
require_once ('jpgraph/jpgraph_scatter.php');

$averageWindArray = array();
$windPeakArray = array();
$timeStampArray = array();
$windDirArray = array();
$readingSize = array();
$directionArray = array('N','NNE','NE','ENE','E','ESE','SE','SSE','S','SSW','SW','WSW','W','WNW','NW','NNW');

$count = 0;


// Create the graph. These two calls are always required
$graph = new Graph(1000,600,'auto');    
$graph->SetScale("lin",0,15);
//$graph->SetY2Scale("int",0,15);

$selectString = "SELECT DATE_FORMAT(readDateTime, '%h:%i %p') as timeAM , ws, wa, wd from sandyWind where readDateTime >= DATE_SUB(NOW(), INTERVAL 1 DAY);";

$result = mysqli_query($connection,$selectString);

while($row= mysqli_fetch_assoc($result)){
	$averageWindArray[] = $row['wa'];
	$windPeakArray[] = $row['ws'];
	$timeStampArray[] = $row['timeAM'];
	$windDirArray[] = $row['wd'];
	$readingSize[] = $count;
	$count++;
}


$graph->xaxis->SetTickLabels($timeStampArray);
$graph->yaxis->SetTickLabels($directionArray);
$graph->xaxis->SetTextLabelInterval(2);
//$graph->yaxis->SetTextLabelInterval(1);

$graph->xaxis->SetLabelAngle(90);

$graph->yaxis->HideTicks(false,false);

$graph->legend->SetPos(0.5,0.98,'center','bottom');


$sp1 = new ScatterPlot($windDirArray,$readingSize);
$sp1->mark->SetType(MARK_FILLEDCIRCLE);
$sp1->mark->SetFillColor("orange");
$sp1->mark->SetWidth(4);



$graph->Add($sp1);




$graph->xaxis->SetTitle("Time","center");
$graph->xaxis->SetTitleMargin(60);
$graph->yaxis->title->Set("Direction");


$graph->Stroke();

/*
foreach($windPeakArray as $num)
 echo ("$num  <br>");
*/

?>
<?php
include 'connect.inc.php';

// content="text/plain; charset=utf-8"
require_once ('jpgraph/jpgraph.php');
require_once ('jpgraph/jpgraph_bar.php');
require_once ('jpgraph/jpgraph_scatter.php');

$averageWindArray = array();
$windPeakArray = array();
$timeStampArray = array();
$windDirArray = array();
$readingSize = array();

$count = 0;


// Create the graph. These two calls are always required
$graph = new Graph(1000,600,'auto');    
$graph->SetScale("textint",0,40);
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

$graph->xaxis->SetLabelAngle(90);
$graph->xaxis->SetTextLabelInterval(2);

$graph->yaxis->HideTicks(false,false);

$graph->legend->SetPos(0.5,0.98,'center','bottom');

$b1plot = new BarPlot($windPeakArray);
$b2plot = new BarPlot($averageWindArray);
//$sp1 = new ScatterPlot($windDirArray,$readingSize);
//$sp1->mark->SetType(MARK_FILLEDCIRCLE);
//$sp1->mark->SetFillColor("orange");
//$sp1->mark->SetWidth(4);


$b1plot->SetLegend('Peak Gust');
$b2plot->SetLegend('Average wind Speed');

//$gbplot = new GroupBarPlot(array($b1plot,$b2plot));
// ...and add it to the graPH
//$graph->Add($gbplot);
//$graph->AddY2($sp1);
$graph->Add($b1plot);
$graph->Add($b2plot);


$graph->xaxis->SetTitle("Time","center");
$graph->xaxis->SetTitleMargin(60);
$graph->yaxis->title->Set("Knots");

$b1plot->SetColor("white");
$b1plot->SetFillColor("#cc1111");

$b2plot->SetColor("white");
$b2plot->SetFillColor("#11cccc");

$graph->Stroke();

/*
foreach($windPeakArray as $num)
 echo ("$num  <br>");
*/
?>
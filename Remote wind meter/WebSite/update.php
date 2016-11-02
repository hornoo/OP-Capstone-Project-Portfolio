<?php

include 'connect.inc.php';

//wd = wind direction
//ws = wind Strength
//wa = wind average

if(isset($_POST['wd'])){

$wd = $_POST['wd'];
$ws = $_POST['ws'];
$wa = $_POST['wa'];

$inputString = "INSERT INTO sandyWind (wd,ws,wa) VALUES ('$wd',$ws,$wa)";

$result = mysqli_query($connection,$inputString);

echo($result);


}
?>
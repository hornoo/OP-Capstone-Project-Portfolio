<?php
        //Database connection settings

        $host = 'localhost';
        $user = '';
        $password = '';
        $dataBase = 'Motorway_DB';
        $connection = mysqli_connect($host,$user,$password,$dataBase) or die("DB connection error".mysqli_error());

?>


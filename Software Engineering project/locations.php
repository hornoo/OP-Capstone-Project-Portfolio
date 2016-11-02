<?php

include '/connection/connect.inc.php';

$timeFrame = 3;

//If post UpDate vaiable is set run following code otherwise return JASON of current markers.
if(isset($_POST['upDate'])){

	//Check what type of message is coming, new, update, edit etc... 0 = new, 1 = add message to existing hazard
    //3 = edit\delete message or marker status, validated as an integer before storing in variable.

	if(is_numeric($_POST['upDate'])){
        $messageOrUpdate = $_POST['upDate'];
    }

    if(is_numeric($_POST['m_ID'])){
        $message_ID = $_POST['m_ID'];
    }

    //setting long and lat coordinates- need to sanitise and validate
    //checks if coordinates match the expression before setting
    $coordRegex = "^[+-]?\d+\.\d+, ?[+-]?\d+\.\d+$";

    if(is_numeric($_POST['latT'])){
        //if(preg_match("/$coordRegex/", $_POST['latT']))
            $latT = $_POST['latT'];
    }
    if(is_numeric($_POST['longT'])){
        //if(preg_match("/$coordRegex/", $_POST['longT']))
            $longT = $_POST['longT'];
    }
	if(is_numeric($_POST['hazIcon'])){
		$hazIcon = $_POST['hazIcon'];
	}

    //setting messages var, need to further sanitise and validate
    //checks if the message matches the expression before setting
    $messageRegex= "^[a-zA-Z0-9_.,]{1,141}$";
    
    //if(preg_match("/$messageRegex/", $_POST['message']))
        $message = $_POST['message'];
    
    //$message =  strip_tags(trim($_POST['message']) );

    echo ("$message_ID");
    echo ("<br>$message");
    echo ("<br>$longT");
    echo ("<br>$latT");
    echo ("<br>$messageOrUpdate");
	echo ("<br>$hazIcon");
	

		if($messageOrUpdate == 0){
		$insertQuery ="INSERT INTO flag (hazard_Image, latT, LongT) VALUES ($hazIcon, $latT, $longT )";

		  $markerInsertResult = mysqli_query($connection,$insertQuery);

			echo("<br> marker insert result $markerInsertResult");

		  $markerInsertID = mysqli_insert_id($connection);

			echo ("<br> marker insert id $markerInsertID");
			//remove all characters except digits
			$markerInsertID = filter_var($markerInsertID, FILTER_SANITIZE_NUMBER_INT); 

		  $messageInsertQuery = "INSERT INTO message (flag_ID, message_Text) VALUES ( $markerInsertID, \"$message\" )";

			$messageInsertResult = mysqli_query($connection, $messageInsertQuery);

			echo("<br> message insert result $messageInsertResult");

		}else if($messageOrUpdate == 1){
		
			$messageInsertQuery = "INSERT INTO message (flag_ID, message_Text) VALUES ( $message_ID, \"$message\" )";

			$messageInsertResult = mysqli_query($connection, $messageInsertQuery);

			echo("<br> message insert result $messageInsertResult");
		
		}else if($messageOrUpdate == 2){
			
			$messageInsertQuery = "INSERT INTO message (flag_ID, message_Text) VALUES ( $message_ID, \"$message\" )";

			$messageInsertResult = mysqli_query($connection, $messageInsertQuery);
			
			$hazardResolveQuery = "UPDATE flag SET hazard_Image=5, resolved=1 WHERE flag_ID=$message_ID;";
			
			$hazardResolveResult = mysqli_query($connection, $hazardResolveQuery);

			echo("<br> message insert result $messageInsertResult $hazardResolveResult");
		}

}else{
	echo json_encode(getData($connection, $timeFrame));
}




//this function returns an array of marker objects and the associated messages.
function getData($connection, $timeFrame){

    //SQL query to return each marker from number for hours set in timeFrame Variable from current time.
    $flagQuery = "SELECT flag.flag_ID, hazard.hazard_Image, flag.longT, flag.latT, flag.resolved FROM flag
                LEFT JOIN hazard ON hazard.hazard_ID=flag.hazard_Image
                WHERE flag.flag_TimeStamp > SUBDATE(now(), INTERVAL $timeFrame HOUR)";


    $flagResult = mysqli_query($connection,$flagQuery);


    $marker = array();
    $arrayIndex= 0;

    //Build array of messages for each result returned from flag query, mesage 1, 2, 3 etc for each marker
    while($row = mysqli_fetch_assoc($flagResult)){


        $marker [$arrayIndex] = $row;

		$markerMessages = array();

        //Query returns messages associated with each marker. they are placed in array for each marker object
        $messageQuery= "SELECT message_ID, message_Text, DATE_FORMAT(message_TimeStamp, '%b %d %Y %h:%i %p') as mesDate FROM message WHERE flag_ID= '{$row['flag_ID']}'" ;

        $messageResult=mysqli_query($connection,$messageQuery);

        while($mRow = mysqli_fetch_assoc($messageResult)){
            $markerMessages[]= $mRow;
        }

        $marker[$arrayIndex]['message']=$markerMessages;



        $arrayIndex++;
    }
	

    return $marker;
}


?>




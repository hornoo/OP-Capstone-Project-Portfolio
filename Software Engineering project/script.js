
	
/*
 * Google Maps documentation: http://code.google.com/apis/maps/documentation/javascript/basics.html
 * Geolocation documentation: http://dev.w3.org/geo/api/spec-source.html
 */

//Map stored as a javascript object so other functions can access it properties.
var map;

// Map Markers sotred in an array so they can be accessed later but other functions.
var markerArray = [];

var markerIDforUpdate;

var voteMarker;

//Variable set with default gps coordinates, this is used to center map initially if a your current location cannot be found.
var defaultLatLng = new google.maps.LatLng(-45.7307748, 170.5857255);  // Default to Blueskin bay when no geolocation support

var refreshMarkers = false;


$(document).ready(function(){
	$('#mapresize').click(function(){
		setTimeout(function(){
			google.maps.event.trigger(map,'resize');
		}, 500);
		
	})


});


$( document ).on( "pageinit", "#two", function() {
//.ready(function(){
	
	if ( navigator.geolocation ) {
        function success(pos) {
            // Location found, show map with these coordinates
            drawMap(new google.maps.LatLng(pos.coords.latitude, pos.coords.longitude));
        }
        function fail(error) {
            drawMap(defaultLatLng);  // Failed to find location, show default map

        }
        // Find the users current position.  Cache the location for 5 minutes, timeout after 6 seconds
        navigator.geolocation.getCurrentPosition(success, fail, {maximumAge: 500000, enableHighAccuracy:true, timeout: 6000});
    } else {
        drawMap(defaultLatLng);  // No geolocation support, show default map
    }
	

	// This function is called to draw the map initially, is needs to be supplied with a lat and long (gps) location
	// eg var LatLng = new google.maps.LatLng(123456 , 123456); 
    function drawMap(latlng) {
        var myOptions = {
            zoom: 12,
            center: latlng,
            mapTypeId: google.maps.MapTypeId.ROADMAP
        };
		

		//drawing map on webpage @ id map-canvas
        map = new google.maps.Map(document.getElementById("map-canvas"), myOptions);
		
		var blueskinArea = new google.maps.Circle({
		    strokeColor: '#FF0000',
			strokeOpacity: 0.8,
			strokeWeight: 2,
			fillColor: '#FF0000',
			fillOpacity: 0.35,
			map: map,
			center:{ lat:-45.7307748, lng: 170.5857255},
			radius: 6000
		
		});
		
		//This waits for the map to be at idle after loading the web page. then Runs what ever is inside first set of brackets.
		google.maps.event.addListenerOnce( map, 'idle', function() {
				
				//print message to console, useful for debugging.
				console.log('map at idle');
				
					//This loads data from our webserver and draws markers from it on the map 

					drawAllMarkers();
				
				
				});

        
    }
	
	        function addMarker(location, map, iconPath, contentMessage) {
            // Add the marker at the clicked location
            var marker = new google.maps.Marker({
                position: location,
		
                icon: iconPath,
                map: map
            });
			
			marker.addListener('click', function() {
			
				markerBubble.open(map, marker);
				//$('#addMesPop').popup('open');
			
			});

			
			
			//add create speech bubble with text stored in contentMessage
          var markerBubble = new google.maps.InfoWindow({
                content: contentMessage
        });
			//add speech bubble to map and marker created just above.
        	//markerBubble.open(map, marker);
			//add marker and bubble to array
			markerArray.push([marker,markerBubble]);
		
		//print marker position to console - used for debug
        	//console.log(marker.getPosition());
		
        }


		
		function drawAllMarkers(){

			jQuery.ajax({
				//type of ajax call
				type: 'GET',
				//location of webpage that generates data in JSON format(FESRful)
				url: 'http://54.254.182.76/mapmob/locations.php',
				success: function(mPoints) {

					//on successfully getting data fdo the following
					var	datas = $.parseJSON(mPoints);
					//iterate through JSON array and convert data into map markers
					$.each(datas, function(i, data) {

						var messagebox = "<div class='bubbles'>";

						var LatLng = new google.maps.LatLng(data.latT , data.longT);
						//console.log()

						$.each(data.message, function(i, message){

							//console.log(message.message_Text);

							messagebox += "<H3>" + message.mesDate + "</H3>" + "<p>" + message.message_Text +"</p>";
						});
						
						if(data.resolved == 0){
						
						messagebox +=  "<a href='#addMesPop' data-rel='popup' data-position-to='window' data-transition='pop' class=' ui-btn ui-corner-all ui-shadow ui-btn-inline ui-icon-mail ui-btn-icon-left ui-btn-b' onclick='setMarkID("+ data.flag_ID +")' >Add Message</a> <a href='#resolveHazPop' data-rel='popup' data-position-to='window' data-transition='pop' class='ui-btn ui-corner-all ui-shadow ui-btn-inline ui-icon-check ui-btn-icon-left ui-btn-b' onclick='setMarkID("+ data.flag_ID +")' >Resolve Hazard</a></div>";
						} else{
							
							messagebox += "<H3> Hazard Resolved</H3></div>";
							
						}
						console.log(data.flag_ID);
						
						//$('#pp-12').click(function(){ window.fk =  });
						addMarker(LatLng, map, data.hazard_Image, messagebox);
					});

				}
			})

		}

	$('#voteButton').click(function(){
		//deleteMarkers();
		deleteOverlays();

		voteMarker = new google.maps.Marker({
			map: map,
			draggable: true,
			animation: google.maps.Animation.DROP,
			position: map.getCenter()
		});


		 markerBubble = new google.maps.InfoWindow({
			content: "<p class='bubbles'>Drag me to the location you want to report on.</p>"
		});
		//add speech bubble to map and marker created just above.
		markerBubble.open(map, voteMarker);
	});

	$('#confirmButton').click(function(){
	
		$('#confirmLocation').hide();
		$('#voteButton').show();
		$('#viewButton').hide();

		var iconSelected = $('input[name=radio-choice-b]:checked').val();
		console.log(iconSelected);
		var $formMessage = $('#messageText');

		//console.log(voteMarker.getPosition().lat());
		//console.log(voteMarker.getPosition().lng());
		var markerData = {
			upDate: 0,
			latT: voteMarker.getPosition().lat(),
			longT: voteMarker.getPosition().lng(),
			message: $formMessage.val(),
			hazIcon: iconSelected

		};
		
		console.log(markerData);

		$.ajax({
			type: 'POST',
			url: 'http://54.254.182.76/mapmob/locations.php',
			data: markerData,
			success: function(data) {
				console.log(data['success']);
				voteMarker.setMap(null);
				drawAllMarkers();
			},
			error: function(data){
				alert('error posting data');
				console.log(data['error']);
			}
		});



	});
	
	$('#confirmUpdate').click(function() {
	
		var formUpdateMessge = $('#updateText');
		
		
		var updateData ={
			upDate: 1,
			m_ID: markerIDforUpdate,
			message: formUpdateMessge.val()
		};
		
		console.log(updateData);
		
		$.ajax({
			type: 'POST',
			url: 'http://54.254.182.76/mapmob/locations.php',
			data: updateData,
			success: function(data) {
				console.log(data['success']);
				deleteOverlays();
				drawAllMarkers();
			},
			error: function(data){
				alert('error posting data');
				console.log(data['error']);
			}
		});
	
	});
	
		$('#resolveHazard').click(function() {
	
		var formResolveMessge = $('#ResolveText');
		
		var updateData ={
			upDate: 2,
			m_ID: markerIDforUpdate,
			message: formResolveMessge.val()
		};
		
		console.log(updateData);
		
		$.ajax({
			type: 'POST',
			url: 'http://54.254.182.76/mapmob/locations.php',
			data: updateData,
			success: function(data) {
				console.log(data['success']);
				deleteOverlays();
				drawAllMarkers();
			},
			error: function(data){
				alert('error posting data');
				console.log(data['error']);
			}
		});
	
	});

	function deleteMarkers() {
		clearMarkers();
		markerArray = [];
	}

	function clearMarkers() {
		setMapOnAll(null);
	}

	// Sets the map on all markers in the array.
	function setMapOnAll(map) {
		for (var i = 0; i < markerArray.length; i++) {
			markerArray[i][0].setMap(map);
		}
	}



	function deleteOverlays() {
		if (markerArray) {
			for (i in markerArray) {
			
			console.log(i);
				markerArray[i][0].setMap(null);
			}
			markerArray.length = 0;
		}
	}
	
	$('#voteButton').click(function() { 
	
		$(this).hide();
		$('#confirmLocation').show();
		$('#viewButton').show();
	
	})
	
	$('#confirmLocation').click(function() {
	
		
		//$('#voteButton').show();
		
		$('#popupDialog').popup('open');
		
	})
	
	$('#viewButton').click(function() {
	
		$(this).hide();
		$('#voteButton').show();
		$('#confirmLocation').hide();
		voteMarker.setMap(null);
		drawAllMarkers();
	})
	
	/*
	$("a").on('click',function () {
	
			var mesidtest = $('a').data();
			console.log(mesidtest);
			
			console.log('clicked');
		
	})
	*/
	
	setInterval(function() {
		
		if(refreshMarkers == true){
		console.log('timer tick');
		}
		
	}, 6000);
	
	$('#mapresize').click(function() {
	
		google.maps.event.trigger(map,'resize');
	
	})
	

});

function setMarkID(idnum){

	
	markerIDforUpdate = idnum;
		
	console.log('set message id to ' + markerIDforUpdate);

}
	

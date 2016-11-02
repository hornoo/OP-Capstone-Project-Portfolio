<!DOCTYPE html>
<html lang="en">
<head>

  <!-- Basic Page Needs
  –––––––––––––––––––––––––––––––––––––––––––––––––– -->
  <meta charset="utf-8">
  <title>SandyMount, ON?</title>
  <meta name="description" content="Weather station monitoring wind conditions of SandyMount flying site in Dunedin New Zealand">
  <meta name="author" content="Richard Horne">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <!-- Mobile Specific Metas
  –––––––––––––––––––––––––––––––––––––––––––––––––– -->
  
   
  <!-- FONT
  –––––––––––––––––––––––––––––––––––––––––––––––––– -->
  <link href='http://fonts.googleapis.com/css?family=Raleway:400,300,600' rel='stylesheet' type='text/css'>

  <!-- CSS
  –––––––––––––––––––––––––––––––––––––––––––––––––– -->
  <link rel="stylesheet" href="css/normalize.css">
  <link rel="stylesheet" href="css/skeleton.css">
  <link rel="stylesheet" href="css/custom.css">
  <link rel="stylesheet" href="css/lightbox.css">
  
  <script src="js/lightbox-plus-jquery.min.js"></script>

  <!-- Favicon
  –––––––––––––––––––––––––––––––––––––––––––––––––– -->
  <link rel="icon" type="image/png" href="images/hgicon.png">
  
  

</head>
<body>

  <!-- Primary Page Layout
  –––––––––––––––––––––––––––––––––––––––––––––––––– -->
  <div class="container">
    <div class="row">
      <div class="one-half column" style="margin-top: 25%">
        <h4>SandyMount, is it ON?</h4>
        <p>Weather station currently in development.</p>
      </div>
	  <div class="one-half column">
		<img class="u-max-full-width glider" src="images/hg-med.png" >
	  </div>
    </div>
  </div>
  
  <div class="section graphs">
	<div class="container">
	
	<h3 class="gtitle" >Wind conditions from last 24 hours</h3>
	
	<p> Graph will automatically refresh every minute - this does not function yet</p>
	
<div class="row">
		<div class="one-half column">
		<a href="sandygraph.php" data-lightbox="g1" data-title="My caption">
		<img class="u-max-full-width" src="sandygraph.php">
		</a>
		</div>
		
		<div class="one-half column">
		<a href="sandygraph_direction.php" data-lightbox="g1" data-title="My caption">
		<img class="u-max-full-width" src="sandygraph_direction.php">
		</a>
		</div>
	
		
</div>		
		
		<div>	
		<a class="button button-primary" style="margin-top:2rem" href="#">Refresh Graph - this does not function yet</a>
		</div>
		
		
	</div>	
  </div>
<!-- End Document
  –––––––––––––––––––––––––––––––––––––––––––––––––– -->
</body>
</html>

// Sampling-based binary decode for Richard's La Crosse anemometer
// Since the binary coding is just a string of high/low values, it's not self-clocking, so an interrupt-based approach won't work.  So, we'll just sample more often than necessary and average the results in suitably-sized windows.
// We'll use the Arduino's on-board LED to show activity.

// Hardware setup: connect the anemometer's TX to Arduino pin ??

// TODO: data structures and algorithms for aggregating the readings for periodic sending via SMS.  For wind speed, an average is reasonable. For wind direction, since there are only 16 categories, a histogram might make sense.  Do we need to fit each report into the 140-byte SMS limit?  Can we send binary?  TBH, text is probably easy enough to parse, and much easier to test.
// The periodic transmissions probably should be driven by number of readings rather than time, as we don't have a real-time clock (and the AVR timer counters wrap around), and the report period from the anemometer is quite variable.


// Constants:
// NOTE: sampling period (BIT_DURATION_MICROS / OVERSAMPLING_FACTOR) should be less than 16383 (microseconds limit for delayMicroseconds() function).
// NOTE: might need to tweak BIT_DURATION_MICROS to accommodate extra time spent processing in this program!  1200 us is a bit too long, 1000 us is a bit too short, 1100 is slightly too short, 1150 slightly too long, 1120 too long also, 1110 looks just about right!
const int	
	INPUT_PIN = 14, // Was 2 on the initial testing rig
	LED_PIN = 13,
	BITS_PER_MESSAGE = 41,
	OVERSAMPLING_FACTOR = 8,
	BIT_DURATION_MICROS = 1080, // 1110 us USB Droid, 1020 us for Richard's Funduino Mega2560.
	DEBUGGING = 0,
  readings_between_posts = 150 // 150 should give ~5 mins
;

#define HIGH 0
#define LOW 1

int reading = 0;
boolean recording = false;

// Buffer for storing the captured, oversampled waveform.  Store as bytes for relatively compact storage (could also pack 8 bits per byte but it would require more code).
const int SAMPLE_BUFFER_LENGTH = BITS_PER_MESSAGE * OVERSAMPLING_FACTOR;
unsigned char sample_buffer[SAMPLE_BUFFER_LENGTH];
int sample_buffer_index = 0;

// Second array for storing the decoded data, one element per bit:
unsigned char decoded_message[BITS_PER_MESSAGE];

// TODO: data structures for recording aggregate wind speed and direction readings for reporting via SMS:
// ...

int peak_gust = 0; // Keeps a record of the peak wind gust (in units of 0.1 m/s) since the last SMS report.

int reading_count = 0;
//const int readings_between_posts = 30; // 150 should give ~5 mins

int wind_speed_sum = 0; // For calculating the average


// Wind direction histogram. If we're reporting every 5 minutes approximately, and read every ~2 s, that's 150 readings. Even if they were all for the same direction, one unsigned char would be sufficient as the type for the array.
// The anemometer reports 16 directions, so 16 is the array size.

unsigned char wind_direction_histogram[16];

void clear_wind_direction_histogram() {	
	// Zero the wind direction histogram
	for (int i = 0; i < 16; i++) {
		wind_direction_histogram[i] = 0;
	}
}



//GPRS methods
int8_t sendATcommand(const char* ATcommand, const char* expected_answer1, unsigned int timeout){

    uint8_t x=0,  answer=0;
    char response[100];
    unsigned long previous;

    memset(response, '\0', 100);    // Initialize the string

    delay(100);

    while( Serial1.available() > 0) Serial1.read();    // Clean the input buffer

    Serial1.println(ATcommand);    // Send the AT command 


        x = 0;
    previous = millis();

    // this loop waits for the answer
    do{
        if(Serial1.available() != 0){    
            response[x] = Serial1.read();
            x++;
            // check if the desired answer is in the response of the module
            if (strstr(response, expected_answer1) != NULL)    
            {
                answer = 1;
            }
        }
        // Waits for the asnwer with time out
    }
    while((answer == 0) && ((millis() - previous) < timeout));    

    return answer;
}



int8_t sendATcommand2(const char* ATcommand, const char* expected_answer1, 
const char* expected_answer2, unsigned int timeout){

    uint8_t x=0,  answer=0;
    char response[100];
    unsigned long previous;

    memset(response, '\0', 100);    // Initialize the string

    delay(100);

    while( Serial1.available() > 0) Serial1.read();    // Clean the input buffer

    Serial1.println(ATcommand);    // Send the AT command 


        x = 0;
    previous = millis();

    Serial.print("previos var");
    Serial.println(previous);
    // this loop waits for the answer
    do{        
        if(Serial1.available() != 0){    
            response[x] = Serial1.read();
            x++;
            // check if the desired answer 1 is in the response of the module
            if (strstr(response, expected_answer1) != NULL)    
            {
                answer = 1;
            }
            // check if the desired answer 2 is in the response of the module
            if (strstr(response, expected_answer2) != NULL)    
            {
                answer = 2;
            }
        }
        // Waits for the asnwer with time out
    }while((answer == 0) && ((millis() - previous) < timeout)); 
 
     Serial.println(answer);   

    return answer;
}

void gprsInitialSetup() {

  delay(5000);
  
  Serial.println("starting");
  
  while (sendATcommand2("AT+CREG?", "+CREG: 0,1", "+CREG: 0,5", 2000) == 0);
  Serial.println("Registered");

  Serial.println("at+cipmux?");  
  Serial.println(sendATcommand("at+cipmux?","+CIPMUX: 0",500));

  Serial.println("at+cstt");  
  //Serial.println(sendATcommand("at+cstt","OK",500));

  do{
    Serial.println("Waiting on cstt command to complete");
  }while(sendATcommand("at+cstt","OK",5000) == 0);
  
  
  Serial.println("at+ciicr?");  
  //Serial.println(sendATcommand("at+ciicr","OK",2000));
  do{
    Serial.println("Waiting on ciicr command to complete");
  }while(sendATcommand("at+ciicr","OK",5000) == 0);
  
  
  Serial.println("AT+SAPBR=3,1,\"APN\",\"internet\"");  
  Serial.println(sendATcommand("AT+SAPBR=3,1,\"APN\",\"internet\"","OK",1000));
  
  Serial.println("AT+SAPBR=1,1");  
  Serial.println(sendATcommand("AT+SAPBR=1,1","OK",1000));
  
  Serial.println("at+httpinit");  
  Serial.println(sendATcommand("at+httpinit","OK",500));
  
  Serial.println("at+httppara=\"URL\",\"www.glide.duckdns.org/update.php\"");  
  Serial.println(sendATcommand("at+httppara=\"URL\",\"www.glide.duckdns.org/update.php\"","OK",500));
  
  Serial.println("at+httppara=\"CONTENT\",\"application/x-www-form-urlencoded\"");  
  Serial.println(sendATcommand("at+httppara=\"CONTENT\",\"application/x-www-form-urlencoded\"","OK",500));

  
}


void setup() {
	pinMode(INPUT_PIN, INPUT);
//	digitalWrite(INPUT_PIN, HIGH); // Enable the pull-up resistor to get the anemometer's TX high to be high enough..or use an external pull-up!
	digitalWrite(INPUT_PIN, LOW); // If using an external pull-up resistor or transistor circuit to level-shift.

	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, LOW);
	
	clear_wind_direction_histogram();
	peak_gust = 0;
	reading_count = 0;

	Serial.begin(19200);
  Serial1.begin(19200);


  gprsInitialSetup();
  
}


// Write out the sample buffer to the serial line:
// For initial testing, this function gets called every time we get a report from the anemometer, but for the real deployment we'd want to transmit via SMS only every 5 mins or so.  I guess we should rename this process_reading or something.  Perhaps every <n> readings we can run output_sms().
void process_reading() {
	// For debugging, we can output the raw sample values (really we want to decode groups of them into single bits):
	

	if (DEBUGGING) {
		Serial.println("Raw sample buffer: ");
		for (int i = 0; i < SAMPLE_BUFFER_LENGTH; i++) {
			Serial.print(sample_buffer[i]);
		}
		Serial.println();
	}
	
	// Decode the raw samples into one sample per bit.  The first bit should always be high.
	for (int i = 0; i < BITS_PER_MESSAGE; i++) {
		// Compute the average value of the relevant group of n bits from the source array (where n = OVERSAMPLING_FACTOR):
		int sum = 0;
		for (int s = 0; s < OVERSAMPLING_FACTOR; s++) {
			sum += sample_buffer[i * OVERSAMPLING_FACTOR + s];
		}
		decoded_message[i] = round((float)sum / OVERSAMPLING_FACTOR);
	}

	if (DEBUGGING) {
		// Print the decoded bit string:
		Serial.println("Decoded binary message:");
		for (int i = 0; i < BITS_PER_MESSAGE; i++) {
			Serial.print(decoded_message[i]);
		}
		Serial.println();
	}

	// Then break up the message into groups, invert, and apply endianness-swap.  The message format is as follows:
	// 5-bit preamble (always 00100; inverted, so HIGH-HIGH-LOW-HIGH-HIGH)
	// 4 bits for wind direction, inverted, little-endian
	// 12 bits for wind speed, inverted, little-endian
	// 4 bits for checksum
	// 4 bits for wind direction, noninverted, LE
	// 12 bits bits for wind speed, noninverted, LE
	

	// Wind direction is 4 bits, representing 16ths of a revolution clockwise from North (N = 0000).
	int wind_direction = 0;
	
	wind_direction = B1111 & ~(decoded_message[8] << 3 | decoded_message[7] << 2 | decoded_message[6] << 1 | decoded_message[5]);

	if (DEBUGGING) Serial.print("Raw wind direction = "); Serial.println(wind_direction);

	// Wind speed is 12 bits, inverted and little-endian, representing units of 0.1 m/s.
	// That's bits 9..20, but the first three are always 0, so ignore.
	// You can't use B1011-style literals for more than 8 bits, apparently, hence the hexadecimal 0x1FF below.
	int wind_speed = 0xFFF & ~(decoded_message[20] << 11 | decoded_message[19] << 10 | decoded_message[18] << 9 | decoded_message[17] << 8 | decoded_message[16] << 7 | decoded_message[15] << 6 | decoded_message[14] << 5 | decoded_message[13] << 4 | decoded_message[12] << 3 | decoded_message[11] << 2 | decoded_message[10] << 1 | decoded_message[9]);
	
		if (DEBUGGING) Serial.print("Raw wind speed = "); Serial.println(wind_speed, BIN);


	// The next four bits are the checksum (0b1111 = 0xF):
	int checksum = 0x000F & ~(decoded_message[24] << 3 | decoded_message[23] << 2 | decoded_message[22] << 1 | decoded_message[21]);

	// Debugging:
	if (DEBUGGING) {
		Serial.print("Wind speed nibble 1 = "); Serial.println(wind_speed & 0xF);
		Serial.print("Wind speed nibble 2 = "); Serial.println(wind_speed >> 4 & 0xF);
		Serial.print("Wind speed nibble 3 = "); Serial.println(wind_speed >> 8 & 0xF);
	}

	int computed_checksum = ((wind_speed & 0xF) + (wind_speed >> 4 & 0xF) + (wind_speed >> 8 & 0xF) + wind_direction) & 0xF;
	
	if (DEBUGGING) {
		Serial.print(" Message Checksum = "); Serial.println(checksum, BIN);
		Serial.print(" Raw Sum = "); Serial.println(((wind_speed + wind_direction)), BIN);
		Serial.print("Computed Checksum = "); Serial.print(computed_checksum); Serial.print(" "); Serial.println(computed_checksum, BIN);
	  
	}

	// The checksum should equal the least significant 4 bits of the sum of the wind direction and speed.  1 + 2 + 4 + 8 = 0xF
	if (computed_checksum != checksum) {
		Serial.println("Checksum error!");
	//	return;
	}
	
	reading_count++;

	// TODO: also check the non-inverted copies of direction and speed against the inverted ones.
	
	// If that all passed, print out the readings:
	
	Serial.print(wind_direction / 16.0 * 360);
	Serial.print(" degrees");
	Serial.println();

	wind_direction_histogram[wind_direction] = wind_direction_histogram[wind_direction] + 1;

	Serial.print(wind_speed * 0.1);
	Serial.println(" m/s");
	
	Serial.print(wind_speed * 0.1 * 3600 / 1852); // 1 nautical mile = 1852 m; 3600 seconds/minute
	Serial.println(" knots");
	
	if (wind_speed > peak_gust) peak_gust = wind_speed;
	wind_speed_sum += wind_speed;
	
	// TODO: store and aggregate and transmit via SMS.
	// First, check how many readings we've done since the last SMS transmission...
	if (reading_count >= readings_between_posts) {
		post();
	}
}

void post() {
	// Output wind direction histogram:

  int windDirection = 0,
      windDirectionCountMax = 0;
  float windDirectionAve = 0;
	Serial.println("Wind direction histogram:");
	for (int i = 0; i < 16; i++) {
  		Serial.print(i); Serial.print(": "); Serial.println(wind_direction_histogram[i]);

        windDirectionAve = i * wind_direction_histogram[i] + windDirectionAve;
        
      if(wind_direction_histogram[i] > windDirectionCountMax){
  
        windDirection = i;
        windDirectionCountMax = wind_direction_histogram[i];

         
      }
      
	}
	
	windDirectionAve = round(windDirectionAve / reading_count);

  String windSpeed = String(peak_gust * 0.1 * 3600 / 1852);
  String windSpeedAv = String(wind_speed_sum / (float)reading_count * 0.1 * 3600 / 1852);
  String postData = String("wd=" + String(windDirectionAve) + "&ws=" + windSpeed + "&wa=" + windSpeedAv);

  postData.trim();
  Serial.print("Data length: ");
  Serial.println(postData.length());
  Serial.println("Post Data: ");
  Serial.println(postData);

  String postCommand = String("at+httpdata=" + String(postData.length()) + "," + "1000" );
  Serial.println(postCommand);

  char finData[100];
 
    postCommand.toCharArray(finData,100);


   if (sendATcommand(finData,"DOWNLOAD",500))
 {

    Serial1.println(postData);    
    delay(150);
    Serial1.println("at+httpaction=1");
    
    Serial.println("made it sending data");

 }

  
  
  
  // ...and the wind speed stats:
	Serial.println("Average wind speed:");
	Serial.print(wind_speed_sum / (float)reading_count * 0.1 * 3600 / 1852);
	Serial.println(" knots");

	Serial.println("Peak wind gust:");
	Serial.print(peak_gust * 0.1 * 3600 / 1852);
	Serial.println(" knots");


  
	// Reset aggregate/running data:
	peak_gust = 0;
	clear_wind_direction_histogram();
	reading_count = 0;
	wind_speed_sum = 0;
}

void serialArSim () {
  //manually read and write bewteen arduino serial and sim 900 serial.
  if (Serial1.available()) {
  Serial.write(Serial1.read());
  }
  if (Serial.available()) {
    Serial1.write(Serial.read());
  }
}

void loop() {

  
	reading = digitalRead(INPUT_PIN);
	digitalWrite(LED_PIN, reading);	// Flicker the LED to indicate activity
	if (reading == HIGH) recording = true;
	if (recording) {
		// Store the reading:
	//	sample_buffer[sample_buffer_index] = (byte) reading; // For normal polarity
		sample_buffer[sample_buffer_index] = ((byte) reading) * -1 + 1; // For inverted (w transistor) polarity
		sample_buffer_index++;
		// Check if we reached the end of the buffer:
		if (sample_buffer_index >= SAMPLE_BUFFER_LENGTH) {
			process_reading();
			recording = false;
			sample_buffer_index = 0;
		}
	}
	delayMicroseconds(BIT_DURATION_MICROS / OVERSAMPLING_FACTOR); // Good for 3..16383 microseconds

  

}


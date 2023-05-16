#include <Wire.h>
#include <Adafruit_BME680.h>
#include <SD.h>
#include <ArduCAM.h>


//Ultrasonic sensor pins
#define trigPin 30
#define echoPin 31
File dataFile;

// Define hardware serial pins
#define SERIAL_PORT Serial2

// Define data structure
struct GPSData {
  float latitude;
  float longitude;
  float altitude;
  float speed;
};

#define SEALEVELPRESSURE_HPA (1013.25) // Define the standard sea level pressure in hPa
Adafruit_BME680 bme; // Create a BME680 object
const int SD_CS = 53;
const int CAM_CS = 49;
ArduCAM myCAM( OV2640, CAM_CS );

void setup() {
  Serial.begin(9600);

  // Start hardware serial for GPS  
  SERIAL_PORT.begin(9600);

  uint8_t vid, pid;
  uint8_t temp;
  Wire.begin();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
  if (!SD.begin(SD_CS)) {
    Serial.println("SD initialization failed!");
    return;
  }
  Serial.println("SD initialization successful.");
  dataFile = SD.open("data.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Temperature (C), Humidity (%), Pressure (hPa), Altitude (m), Distance (cm) ,Latitude ,Longitude ,Altitude , Picture");
    dataFile.close();
  }
  else {
    Serial.println("Could not open data.csv");
  }

  Serial.println(F("ArduCAM Start!"));
  //set the CS as an output:
  pinMode(CAM_CS,OUTPUT);
  digitalWrite(CAM_CS, HIGH);
  // initialize SPI:
  SPI.begin();
    
  //Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100);

  while(1){
  //Check if the ArduCAM SPI bus is OK
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  
  if (temp != 0x55){
    Serial.println(F("SPI interface Error!"));
    delay(1000);continue;
  } else {
    Serial.println(F("SPI interface OK."));
    break;
    }
  }

  while(1){
    //Check if the camera module type is OV2640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
      Serial.println(F("Can't find OV2640 module!"));
      delay(1000);continue;
    } 
    else{
      Serial.println(F("OV2640 detected."));break;
    } 
  }

  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_1600x1200);
  myCAM.OV2640_set_Light_Mode(Sunny);

  delay(1000);


}

String picname = "";

void loop() {
  float distance = getDistance();
  if (distance > 25 & distance < 35) {
    GPSData data = readGPSData();
    float temperature = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;
    float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.println(" *C");
    Serial.print("Humidity = ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("Pressure = ");
    Serial.print(pressure);
    Serial.println(" hPa");
    Serial.print("Altitude = ");
    Serial.print(altitude);
    Serial.println(" m");
    Serial.print("Distance = ");
    Serial.print(distance);
    Serial.println(" cm");
    delay(1000);
    if (SD.exists("data.csv")) {
      dataFile = SD.open("data.csv", FILE_WRITE);
      if (dataFile) {
        dataFile.print(temperature);
        dataFile.print(",");
        dataFile.print(humidity);
        dataFile.print(",");
        dataFile.print(pressure);
        dataFile.print(",");
        dataFile.print(altitude);
        dataFile.print(",");
        dataFile.print(distance);
        // Write GPS data to file
        dataFile.print(data.latitude, 6);
        dataFile.print(",");
        dataFile.print(data.longitude, 6);
        dataFile.print(",");
        dataFile.print(data.altitude, 2);
        dataFile.print(",");
        dataFile.println(picname);
        dataFile.close();

      }
      else {
        Serial.println("Could not open data.csv");
      }
    }
    else {
      Serial.println("data.csv does not exist.");
    }
    myCAMSaveToSDFile();
  } else {
    Serial.print("Incorrect Distance, distance is ");
    Serial.println((String)distance);
  }
}

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  float duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;
  return distance;
}

void myCAMSaveToSDFile(){
  char str[8];
  byte buf[256];
  static int i = 0;
  static int k = 0;
  uint8_t temp = 0,temp_last=0;
  uint32_t length = 0;
  bool is_header = false;
  File outFile;
  //Flush the FIFO
  myCAM.flush_fifo();
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  //Start capture
  myCAM.start_capture();
  Serial.println(F("start Capture"));
  while(!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
  Serial.println(F("Capture Done."));  
  length = myCAM.read_fifo_length();
  Serial.print(F("The fifo length is :"));
  Serial.println(length, DEC);
  if (length >= MAX_FIFO_SIZE) //384K
  {
    Serial.println(F("Over size."));
    return ;
  }
  if (length == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
    return ;
  }
  //Construct a file name
  k = k + 1;
  itoa(k, str, 10);
  strcat(str, ".jpg");
  picname = str;
  Serial.println(picname);
  //Open the new file
  outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
  if(!outFile){
    Serial.println(F("File open faild"));
    return;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  while ( length-- )
  {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
    {
      buf[i++] = temp;  //save the last  0XD9     
      //Write the remain bytes in the buffer
      myCAM.CS_HIGH();
      outFile.write(buf, i);    
      //Close the file
      outFile.close();
      Serial.println(F("Image save OK."));
      is_header = false;
      i = 0;
    }  
    if (is_header == true)
    { 
      //Write image data to buffer if not full
      if (i < 256)
      buf[i++] = temp;
      else
      {
        //Write 256 bytes image data to file
        myCAM.CS_HIGH();
        outFile.write(buf, 256);
        i = 0;
        buf[i++] = temp;
        myCAM.CS_LOW();
        myCAM.set_fifo_burst();
      }        
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      buf[i++] = temp_last;
      buf[i++] = temp;   
    } 
  } 
}

GPSData readGPSData() {
  GPSData data;
  
  // Wait until GPS data is available
  while (!SERIAL_PORT.available()) {
    delay(100);
  }
  
  // Parse GPS data
  String line = SERIAL_PORT.readStringUntil('\n');
  if (line.startsWith("$GPGGA")) {
    // Extract latitude, longitude, altitude, and speed from GPS data
    String parts[15];
    int index = 0;
    int startIndex = 0;
    int endIndex = 0;
    for (int i = 0; i < line.length(); i++) {
      if (line[i] == ',') {
        endIndex = i;
        parts[index] = line.substring(startIndex, endIndex);
        startIndex = endIndex + 1;
        index++;
      }
    }
    data.latitude = parts[2].toFloat();
    data.longitude = parts[4].toFloat();
    data.altitude = parts[9].toFloat();
    data.speed = parts[7].toFloat();
  }

  return data;
}



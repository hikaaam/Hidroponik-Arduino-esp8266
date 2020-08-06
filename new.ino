#include <DHT.h>
#include <ESP8266WiFi.h>
#include <SocketIoClient.h>

SocketIoClient socket;

//Setup Global Variable
String id = "1471984882";
bool IsConnect = false;



//Variable kalibrasi
float currWl = 0.0;
int nilaiToleransiWl = 12;

//Variable automatisasi
bool Mode_Prototype = false;
bool RelayStatus1 = false;
bool RelayStatus2 = false;
int NilaiWl = 28;
int NilaiTemp = 27;



//define your sensors here
#define DHTTYPE DHT11
#define trigPin 14 // D5
#define echoPin 12 // D6
#define dhtPin 4   // D2
#define relay1 (0) // D3
#define relay2 (2) // D4
#define TdsSensorPin A0
#define VREF 5.0  // analog reference voltage(Volt) of the ADC
#define SCOUNT 30 // sum of sample point

DHT dht(dhtPin, DHTTYPE);
int analogBuffer[SCOUNT]; // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25;

long duration;
float distance;
const int max_hight = 14;

// Setting WiFi
char ssid[] = "B";
char pass[] = "punyaucup11299";

//Setting Server
char SocketServer[] = "192.168.43.47";
int port = 4000;

void setup()
{

  //setup pins and sensor
  //  pinMode(relay1, OUTPUT);
  //  pinMode(relay1, LOW);
  pinMode(relay1, OUTPUT);
  pinMode(relay1, HIGH);
  pinMode(relay2, OUTPUT);
  pinMode(relay2, HIGH);

  pinMode(TdsSensorPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  dht.begin();

  //setup tombol realtime aplikasi
  SetupRelayAplikasi();

  Serial.begin(115200);

  //Setup WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  socket.begin(SocketServer, port);

  //listener for socket io start

  socket.on("connect", konek);
  socket.on("rwl", RelayWl);
  socket.on("rtds", RelayTds);
  socket.on("rtemp", RelayTemp);
  socket.on("rhum", RelayHum);
  socket.on("UbahMode",GantiMode);
  socket.on("disconnect", diskonek);

  //listener for socket io end
}



//function automatisasi
void automatisasi(float nilaiWl, float nilaiTemp)
{
  if (!Mode_Prototype)
  {
    if (nilaiWl <= NilaiWl)
    {
      if (!RelayStatus1)
      {
        JalankanRelay("true", "resWl", relay1);
        RelayStatus1 = true;
      }
    }
    else if (nilaiWl > NilaiWl)
    {
      if (RelayStatus1)
      {
        JalankanRelay("false", "resWl", relay1);
        RelayStatus1 = false;
      }
    }
    else if (nilaiTemp>=NilaiTemp)
    {
       if (!RelayStatus2)
      {
        JalankanRelay("true", "resTemp", relay2);
        RelayStatus2 = true;
      }
    }
      else if (nilaiTemp<NilaiTemp)
    {
       if (RelayStatus2)
      {
        JalankanRelay("false", "resTemp", relay2);
        RelayStatus2 = false;
      }
    }
  }
}

void loop()
{

 socket.loop();

 // sensor suhu dan humidity
 float t = dht.readTemperature();
 float h = dht.readHumidity();
 String temp = TangkapNilaiSensor(t);
 String hum = TangkapNilaiSensor(h);

 static unsigned long analogSampleTimepoint = millis();
 if (millis() - analogSampleTimepoint > 40U) //every 40 milliseconds,read the analog value from the ADC
 {
   analogSampleTimepoint = millis();
   analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin); //read the analog value and store into the buffer
   analogBufferIndex++;
   if (analogBufferIndex == SCOUNT)
     analogBufferIndex = 0;
 }
 static unsigned long printTimepoint = millis();
 if (millis() - printTimepoint > 800U)
 {
   printTimepoint = millis();
   for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
     analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
   averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;                                                                                                  // read the analog value more stable by the median filtering algorithm, and convert to voltage value
   float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);                                                                                                               //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
   float compensationVolatge = averageVoltage / compensationCoefficient;                                                                                                            //temperature compensation
   tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; //convert voltage value to tds value

   //Serial.print("voltage:");
   //Serial.print(averageVoltage,2);
   //Serial.print("V   ");
   // Serial.print("TDS Value:");
   // Serial.print(tdsValue,0);
   // Serial.println("ppm");
 }

 String tdsVal = TangkapNilaiSensor(tdsValue);

 // sensor ultrasound ketinggian air
 digitalWrite(trigPin, LOW); // Added this line
 delayMicroseconds(2);       // Added this line
 digitalWrite(trigPin, HIGH);
 //  delayMicroseconds(1000); - Removed this line
 delayMicroseconds(10); // Added this line
 digitalWrite(trigPin, LOW);
 duration = pulseIn(echoPin, HIGH);
 distance = (duration / 2) / 29.1;
 distance = max_hight - distance;
 distance = (distance / max_hight) * 100;

 String wlval = TangkapNilaiSensor(distance);
 
 automatisasi(30, 30); //contoh pemanggilan function otomatisasi nilai float WL, nilai Float Temp
 
 if (IsConnect)
 {
   //kirim Data Sensor Disini

//    KirimSocket("temp", temp);
//    KirimSocket("hum", hum);
//    KirimSocket("tds", tdsVal);
//    KirimSocket("wl", kalibrasiWl(float(wlval)));
   //
   //      KirimSocket("temp", "30");
   //      KirimSocket("hum", "70");
   //      KirimSocket("tds", "1000");
	//	 KirimSocket("wl", kalibrasiWl(float(random(1,100))));
   //
 }
 
  delay(1000);
}

int getMedianNum(int bArray[], int iFilterLen)
{
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++)
  {
    for (i = 0; i < iFilterLen - j - 1; i++)
    {
      if (bTab[i] > bTab[i + 1])
      {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}

//Function Function Penting Di Bawah

void konek(const char *payload, size_t length)
{
  socket.emit("new user", "\"P1471984882\"");
  Serial.println("Made Socket Connection");
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  IsConnect = true;
}

void JalankanRelay(const char *payload, String NamaSocket, uint8_t pin)
{
  String value = String(payload);
  if (value == "true")
  {
    digitalWrite(pin, LOW);
    KirimSocket(NamaSocket, "true");
    Serial.println("its true");
  }
  else
  {
    digitalWrite(pin, HIGH);
    KirimSocket(NamaSocket, "false");
    Serial.println("its false");
  }
}

void RelayWl(const char *payload, size_t length)
{
  Serial.println(payload);
  JalankanRelay(payload, "resWl", relay1);
}

void RelayTemp(const char *payload, size_t length)
{
  Serial.println(payload);
  JalankanRelay(payload, "resTemp", relay2);
}

void RelayHum(const char *payload, size_t length)
{
  Serial.println(payload);
  JalankanRelay(payload, "resHum", relay1);
}

void GantiMode(const char *payload, size_t length)
{
  String value = String(payload);
  String stat;
  bool statz;
    if(value  == "true"){
        stat = "false";
        statz = false;
     }
     else{
       stat = "true";
       statz = true;
     }
    KirimSocket("ModeProto",value);

  Mode_Prototype = statz;
  Serial.println("mode saat ini\n");
  Serial.println(stat);
}

void diskonek(const char *payload, size_t length)
{
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  IsConnect = false;
}

void RelayTds(const char *payload, size_t length)
{
  JalankanRelay(payload, "resTds", relay1);
}

void KirimSocket(String nama, String val)
{
  String Data = "{\"_id\":\"" + id + "\",\"_val\":\"" + val + "\"}";
  socket.emit(nama.c_str(), Data.c_str());
}

String TangkapNilaiSensor(float sensor)
{
  char Var[10];
  dtostrf(sensor, 1, 2, Var);
  String hasil = String(Var);
  return hasil;
}

void SetupRelayAplikasi()
{
  KirimSocket("resTemp", String(RelayStatus2));
  KirimSocket("resHum", "false");
  KirimSocket("resTds", "false");
  KirimSocket("resWl", String(RelayStatus1));
  KirimSocket("ProtoMode", String(Mode_Prototype));
}



//kalibrasi sensor Water Level
String kalibrasiWl(float wl){
  String hasil;
  if(currWl<1){
    if(wl<40){
      currWl = wl;
      hasil = String(wl);
    }
    else{
      currWl = 30;
      hasil = "30";
    }
  }
  else{
    if(abs(currWl-wl)>nilaiToleransiWl){
      hasil = String(currWl);
    }
    else if(wl < 1){
      hasil = String(currWl);
    }
    else if(wl > 99){
      hasil = String(currWl);
    }
    else{
      hasil = String(wl);
      currWl = wl;
    }
  }
  return hasil;
}

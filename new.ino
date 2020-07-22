#include <ESP8266WiFi.h>
#include <SocketIoClient.h>

SocketIoClient socket;

//Setup Global Variable
String id = "1471984882";
bool IsConnect = false;


//define your sensors here
#define relay1 (0)


// Setting WiFi
char ssid[] = "B";
char pass[] = "punyaucup11299";

//Setting Server
char SocketServer[] = "192.168.43.47";
int port = 4000;





void setup() {
  
  //setup pins and sensor
  pinMode(relay1, OUTPUT);
  pinMode(relay1, HIGH);

  
  Serial.begin(115200);

  //Setup WiFi
  WiFi.begin(ssid, pass); 
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
  }
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("");

  socket.begin(SocketServer, port);

  //listener for socket io start
  
    socket.on("connect",konek);
    socket.on("rwl",RelayWl);
    socket.on("rtds",RelayTds);
    socket.on("rtemp",RelayTemp);
    socket.on("rhum",RelayHum);
  
  //listener for socket io end
  

}

void loop() {
  
  socket.loop(); 
  
  if(IsConnect){
    //kirim Data Sensor Disini
      KirimSocket("wl","20");  
  }
  delay(1000);
}

//Function Function Penting Di Bawah

void konek(const char * payload, size_t length) {
    socket.emit("new user", "\"P1471984882\"");
    Serial.println("Made Socket Connection");
    IsConnect = true;
}

void JalankanRelay(const char * payload,String NamaSocket,uint8_t pin){
   String value = String(payload);
  if(value == "true"){
    digitalWrite(pin, HIGH);
    KirimSocket(NamaSocket,"true");
    Serial.println("its true");
  }
  else{
    digitalWrite(pin, LOW);
    KirimSocket(NamaSocket,"false");
    Serial.println("its false");
  }
}

void RelayWl(const char * payload, size_t length) {
  Serial.println(payload);
  JalankanRelay(payload,"resWl",relay1);
}

void RelayTemp(const char * payload, size_t length) {
  Serial.println(payload);
  JalankanRelay(payload,"resTemp",relay1);
}

void RelayHum(const char * payload, size_t length) {
  Serial.println(payload);
  JalankanRelay(payload,"resHum",relay1);
}

void RelayTds(const char * payload, size_t length) {
  JalankanRelay(payload,"resTds",relay1);
}

void KirimSocket(String nama,String val){
  String Data = "{\"_id\":\"" + id + "\",\"_val\":\"" + val + "\"}";
  socket.emit(nama.c_str(), Data.c_str());
}

String TangkapNilaiSensor(float sensor){
   char Var[10];
   dtostrf(sensor,1,2,Var);
   String hasil = String(Var);
   return hasil;
}

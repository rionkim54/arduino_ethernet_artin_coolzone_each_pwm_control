#include <SPI.h>
#include <Ethernet.h>
#include <EthernetClient.h>

#include <EthernetUdp.h>

#define USE_MEGA 1
#define USE_STATIC_IP 1

#define USE_TCP_SERVER 0
#define USE_UDP_SERVER 1

#define save_ip 0
#define save_ip2 220

#if USE_STATIC_IP

  // the dns server ip
  // the router's gateway address:
  // the subnet:
  IPAddress subnet(255, 255, 255, 0);
  //the IP address is dependent on your network
  IPAddress dnServer(192, 168, save_ip, 1);
  IPAddress gateway(192, 168, save_ip, 1);
  IPAddress ip(192, 168, save_ip, save_ip2);

#else

//   // the dns server ip
//   IPAddress dnServer(211, 9, 5, 1);
//   // the router's gateway address:
//   IPAddress gateway(211, 9, 5, 1);
//   // the subnet:
//   IPAddress subnet(255, 255, 255, 0);
//   //the IP address is dependent on your network
//   IPAddress ip(211, 9, 5, BOARD_IP);

// #endif

#endif

byte mac[] = {
  0x00, 0xAA, 0xBB, 0x76, 0x80 | save_ip2, 0xA0 
};

#define SS 10    //W5500 CS
#define RST 7    //W5500 RST
#define CS 4     //SD CS pin


#if USE_TCP_SERVER
  // telnet defaults to port 23
  EthernetServer server(8722);
#endif

#if USE_UDP_SERVER
  EthernetUDP udp;
#endif

unsigned int udpPort = 8723; // 사용할 로컬 포트 번호

boolean alreadyConnected = false; // whether or not the client was connected previously

#define PIN_NO 3
#define PIN2_NO 5
#define PIN3_NO 6

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(8, OUTPUT); // for test 
  digitalWrite(8, false); // 
  
  pinMode(PIN_NO, OUTPUT);
  pinMode(PIN2_NO, OUTPUT);
  pinMode(PIN3_NO, OUTPUT);

          analogWrite(PIN_NO, 0);
        analogWrite(PIN2_NO, 0);
        analogWrite(PIN3_NO, 0);

#if USE_MEGA
  pinMode(SS, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(CS, OUTPUT);
  digitalWrite(SS, HIGH);
  digitalWrite(CS, HIGH);
  /* If you want to control Reset function of W5500 Ethernet controller */
  digitalWrite(RST, HIGH);
#endif  

#if USE_STATIC_IP
   Ethernet.begin(mac, ip, dnServer, gateway, subnet);
#else

  if(0 < save_ip) {

    IPAddress dnServer(211, 9, 6, 1);
    // the router's gateway address:
    IPAddress gateway(211, 9, 6, 1);
    // the subnet:
    IPAddress subnet(255, 255, 255, 0);
    //the IP address is dependent on your network
    IPAddress ip(211, 9, save_ip, save_ip2);

    Ethernet.begin(mac, ip, dnServer, gateway, subnet);
  }
  else {

    if (Ethernet.begin(mac) == 0) {
      Serial.println("Failed to configure Ethernet using DHCP");
      // no point in carrying on, so do nothing forevermore:
      //for (;;)
      //  ;
    } 

    // max_timeout = 60;

  }

#endif

    // print your local IP address:
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();

  
#if USE_TCP_SERVER
  // start listening for clients
  server.begin();
#endif

  digitalWrite(RST, false); // 7 pin // GROUND

#if USE_UDP_SERVER
  udp.begin(udpPort);
#endif

}


#define MODE_DIMMING 

boolean bDimming = true;
int iDimming = 0;

unsigned long sendTimeStamp;

void listen(EthernetUDP thisUDP, unsigned int thisPort) {
  
  // 들어오는 패킷이 있는지 확인해서 헤더를 분석한다.
  int messageSize = thisUDP.parsePacket();
  // 패킷에 페이로드(payload)4부분이 있으면 모두 분석해서 저장한다.
  if (messageSize > 0) {
    Serial.print("message received from: ");
    // 송신 주소와 포트를 얻어낸다.
    IPAddress yourIp = thisUDP.remoteIP();
    unsigned int yourPort = thisUDP.remotePort();
    for (int thisByte = 0; thisByte < 4; thisByte++) {
      Serial.print(yourIp[thisByte], DEC);
      Serial.print(".");
    }
    Serial.println(" on port: " + String(thisPort));
    


    /////////////////
    char cTempData[4];
    memset(cTempData, 0x00, 4);
    int pos = 0;
    
    // 페이로드 부분을 시리얼 포트로 보낸다.
    while (thisUDP.available() > 0) {
      // packetBuffer로부터 패킷을 읽는다.
      int udpByte = thisUDP.read();
      cTempData[pos] = udpByte; pos += 1;
//      Serial.println(udpByte);
    }
    

    if(cTempData[0] == 'd') {
      bDimming = true;
      Serial.println("Dimming");
    }
    else {
      bDimming = false;
      
      
        //cTempData : 100
        int value = atoi(cTempData) % 1000;
        int no = atoi(cTempData) / 1000;
        if(no == 0) {
          analogWrite(PIN_NO, value);
          analogWrite(PIN2_NO, value);
          analogWrite(PIN3_NO, value);
        }
        if(no == 1) analogWrite(PIN_NO, value);
        if(no == 2) analogWrite(PIN2_NO, value);
        if(no == 3) analogWrite(PIN3_NO, value);
        Serial.print("LED ");
        Serial.println(value);
    }
    /////////////////
//    sendPacket(thisUDP, Ethernet.localIP(), yourIp, yourPort);
  }
}
void sendPacket(EthernetUDP thisUDP, IPAddress thisAddress,
  IPAddress destAddress, unsigned int destPort) {
  // 전송할 패킷을 만든다.
  thisUDP.beginPacket(destAddress, destPort);
  for (int thisByte = 0; thisByte < 4; thisByte++) {
    // 바이트 단위로 전송
    thisUDP.print(thisAddress[thisByte], DEC);
    thisUDP.print(".");
  }
  thisUDP.println("Hi there!");
  thisUDP.endPacket();
}

void loop() {

  if(0 < Serial.available()) {
    String str = Serial.readString();
    Serial.println(str);

    char cTempData[5];
    str.toCharArray(cTempData, 5);
    if(cTempData[0] == 'd') {
      bDimming = true;
      Serial.println("Dimming");
    }
    else {
      bDimming = false;
  
      
        //cTempData : 100
        int value = atoi(cTempData) % 1000;
        int no = atoi(cTempData) / 1000;
        if(no == 0) {
          analogWrite(PIN_NO, value);
          analogWrite(PIN2_NO, value);
          analogWrite(PIN3_NO, value);
        }
        if(no == 1) analogWrite(PIN_NO, value);
        if(no == 2) analogWrite(PIN2_NO, value);
        if(no == 3) analogWrite(PIN3_NO, value);
        Serial.print("LED ");
        Serial.println(value);
    }
  }

  // put your main code here, to run repeatedly:
//  digitalWrite(9, true);
//  delay(1000);
//  digitalWrite(9, false);
//  delay(1000);

  

//    for(int i = 0; i <= 512; i += 1) {
//      analogWrite(PIN_NO, i);
//      delay(i < 150 ? 20 : 10);
//    }
//
//    delay(1000);
//
//    for(int i = 255; 0 <= i; i -= 1) {
//      analogWrite(PIN_NO, i);
//      delay(i < 150 ? 20 : 10);
//    }
//
//    delay(1000);

    if(bDimming == true) {
      
      iDimming += 1;
      if(512 < iDimming) {
        iDimming = 0;
        bDimming = false;
      }

      if(iDimming == 256) {
        delay(iDimming < 150 ? 20 : 10);
      }
      else
      if(iDimming < 256) {
        analogWrite(PIN_NO, iDimming);
        analogWrite(PIN2_NO, iDimming);
        analogWrite(PIN3_NO, iDimming);
        delay(iDimming < 150 ? 10 : 5);
      }
      else {
        analogWrite(PIN_NO, 512 - iDimming);
        analogWrite(PIN2_NO, 512 - iDimming);
        analogWrite(PIN3_NO, 512 - iDimming);
        delay(512 - iDimming ? 10 : 5);
      }

//      Serial.println(iDimming);
    }

#if USE_UDP_SERVER
    listen(udp, udpPort);
#endif

    delay(5);

#if USE_TCP_SERVER

  EthernetClient connectedClient = server.available();

    // Do we have a client?
  if (!connectedClient) {
    return;
  }

  // when the client sends the first byte, say hello:
  if (connectedClient) {
    
    if (!alreadyConnected) {
      // clead out the input buffer:
      connectedClient.flush();
      Serial.println("We have a new client");
      Serial.print("client IP address: ");
      Serial.println(connectedClient.remoteIP());
      connectedClient.println("Hello, client!");
      alreadyConnected = true;
    }

    if (connectedClient.available() > 0) {
      
      // read the bytes incoming from the client:
      String str = connectedClient.readString();

      Serial.println(str);

      char cTempData[4];
      str.toCharArray(cTempData, 4);
      if(cTempData[0] == 'd') {
        bDimming = true;
        Serial.println("Dimming");
      }
      else {
        bDimming = false;
        
        
        //cTempData : 100
        int value = atoi(cTempData) % 1000;
        int no = atoi(cTempData) / 1000;
        if(no == 0) {
          analogWrite(PIN_NO, value);
          analogWrite(PIN2_NO, value);
          analogWrite(PIN3_NO, value);
        }
        if(no == 1) analogWrite(PIN_NO, value);
        if(no == 2) analogWrite(PIN2_NO, value);
        if(no == 3) analogWrite(PIN3_NO, value);
        Serial.print("LED ");
        Serial.println(value);
      }
      
      // echo the bytes back to the client:
      server.write(cTempData);
      // echo the bytes to the server as well:
//      Serial.write(thisChar);
    }
  }
#endif
    
}

 #include <Keypad.h>
 #include <LiquidCrystal.h>
 #include <Servo.h>
 #define Pecho 6
 #define Ptriger 7
 #define DEBUG true
 #define ESP8266 Serial3
 #define motor 52
 #define motorS 51
 #define buzzer 50
 #define BUFFER_SIZE 1024
String SSID = "mmm";
String PASSWORD = "hola1234";
int LED = 13;
boolean FAIL_8266 = false;

char buffer[BUFFER_SIZE];
Servo porton;
 const byte Filas=4;
 const byte Columnas=4;
 char Teclas[Filas][Columnas]={
   { '1','2','3', 'A' },
   { '4','5','6', 'B' },
   { '7','8','9', 'C' },
   { '*','0','#', 'D' }
 };
 String PassIngresado="";
 bool cambio=false;
 const byte rowPins[Filas] = { 22, 23, 24, 25 };
 const byte columnPins[Columnas] = {26,27,28,29};
 const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
 LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
 long duracion,distancia,llena,vacia;
 String pass="1234";
 int PIR = 8;
 int lectura=0;
 int entroC=A8;
 int estadoE,estadoS;
 int SalioC=A9;
 int prueba=10;
 int contadorCuarto,contadorPorton=0;
 int gas=A0;
 int si = A1; 
 int entroPorton=A7;
 int p=0;
 int estado;
 int limiteEntrada=500;
 bool NotificoLlena,NotificoVacia=false;
 bool lleno,vacio,apagaralarma,detenerM,detenerS=false;
 bool puertaBloqueada=true;
 bool abrirPuerta=false;
 bool activarPIR=false;
 int bloqSist=0;
 int  luz = 48;
 Keypad keypad = Keypad(makeKeymap(Teclas),rowPins,columnPins,Filas,Columnas);


 
void setup() {
  // put your setup code here, to run once:
  
  pinMode(LED, OUTPUT);
  
  digitalWrite(LED, LOW);
  delay(300);
  digitalWrite(LED, HIGH);
  delay(200);
  digitalWrite(LED, LOW);
  delay(300);
  digitalWrite(LED, HIGH);
  delay(200);
  digitalWrite(LED, LOW);
  do{
    //Serial.begin(9600);
    //ESP8266.begin(9600);
    Serial.begin(115200);
    ESP8266.begin(115200);
  
    //Wait Serial Monitor to start
    while(!Serial);
    Serial.println("--- Start ---");

    ESP8266.println("AT+RST");
  
    if(ESP8266.find("ready"))
    {
      Serial.println("Module is ready");
      sendData("AT+CWMODE=3\r\n",1000,DEBUG); // configuración punto de acceso
      sendData("AT+CIPMUX=1\r\n",1000,DEBUG); // multiples conexiones
      sendData("AT+CIPSERVER=1,80\r\n",1000,DEBUG); // encender servidor en puerto 80
      delay(10000);
      //Quit existing AP, for demo
      Serial.println("Quit AP");
      ESP8266.println("AT+CWQAP\r\n");
      delay(1000);    
      clearESP8266SerialBuffer();
      if(cwJoinAP())
      {
        Serial.println("CWJAP Success");
        FAIL_8266 = false;
        
        delay(3000);
        clearESP8266SerialBuffer();
        //Get and display my IP
        sendData("AT+CIFSR\r\n",1000,DEBUG); // se obtienen direcciones IP 
        delay(1000); 
        Serial.println("Setup finish");
      }else{
        Serial.println("CWJAP Fail");
        delay(500);
        FAIL_8266 = true;
      }
    }else{
      Serial.println("Module have no response.");
      delay(500);
      FAIL_8266 = true;
    }
  }while(FAIL_8266);
  
  // duration del tiempo fallido ESP8266.readBytesUntil
  ESP8266.setTimeout(1000);
  //Serial.begin(9600);
  pinMode(PIR, INPUT);
  pinMode(Pecho,INPUT);
  pinMode(Ptriger,OUTPUT);
  pinMode(prueba,INPUT);
  pinMode(entroC,INPUT);
  pinMode(SalioC,INPUT);
  pinMode(entroPorton,INPUT);
  pinMode(motor,OUTPUT);
  pinMode(motorS,OUTPUT);
  pinMode(buzzer,OUTPUT);
  pinMode(luz,OUTPUT);
  digitalWrite(13,LOW);
  digitalWrite(Ptriger,LOW);
  porton.attach(49);
  porton.write(35);
  lcd.begin(16, 2);
  lcd.print("PASS:");
  lcd.setCursor(7,0);
  lcd.print("E:B");
  llena=14;
  vacia=15;
} 
void loop() {
    if(activarPIR)
      MODULOPIR();
    //Serial.println(digitalRead(prueba)); 
    MODULOPISCINA();
    MODULOHUMO();
    ModuloLuz();
    espera(5000);
    //delay(300);
}
void ModuloLuz(){
  //-------------------------------------------------------------------------------------------------------
  //Se usa para detectar cuando entran personas al cuarto
  if(analogRead(entroC)<limiteEntrada){
      estadoE=1;
      //Serial.println(analogRead(entroC));
   }
  else{
      estadoE=0;
      //Serial.println(analogRead(entroC));
   }
  //Se usa para detectar cuando salen personas del cuarto
   if(analogRead(SalioC)<limiteEntrada){
      estadoS=1;
   }
   else{
      estadoS=0;
      //Serial.println(analogRead(SalioC));
   }
   
   //-----------------------------------------------------------------------------------------------------------
   //Se verifica si entra o sale
   if(estadoE==1 && estadoS==0){
      contadorCuarto=contadorCuarto+1;
      Serial.print("entro: ");
      Serial.println(contadorCuarto);
   }
   else if(estadoE==0 && estadoS==1){
      if(contadorCuarto>0){
        contadorCuarto=contadorCuarto-1;
        Serial.print("Salio: ");
        Serial.println(contadorCuarto);
      }
      else{
        contadorCuarto=0;
        Serial.print("Salio: ");
        Serial.println(contadorCuarto);
       }
   }
   //-------------------------------------------------------------------------------------------------------
   //SI NO HAY GENTE SE APAGA EL LED
   if(contadorCuarto==0)
      digitalWrite(luz,LOW);
   else
      digitalWrite(luz,HIGH);
   //--------------------------------------------------------------------------------------------------------
    
}
void ModuloTeclado(){
    char key = keypad.getKey();
    if(key){ 
        if(cambio){         
            if(key=='A'){
              lcd.setCursor(0,1);
              lcd.print("Guardado        ");
              delay(1000);
              lcd.setCursor(0, 0);
              lcd.print("PASS:");
              lcd.setCursor(7,0);
              lcd.print("E:B");
              lcd.setCursor(0,1);
              lcd.print("                ");
              pass=PassIngresado;
              cambio=false;           
              puertaBloqueada=true;
              p=0;
            }
            else if(key=='B'){
              cambio=true;
              lcd.setCursor(0,0);
              lcd.print("Nuevo Pass:");
              lcd.setCursor(0,1);
              PassIngresado="";
              lcd.print("        ");
              p=0;        
            }
            else if(key=='C'|| key=='D'){
              
            }
            else{
              lcd.setCursor(p, 1);
              lcd.print(key);
              p++;
              PassIngresado=PassIngresado+key;
            }
        } 
        else if(key=='B'){
            cambio=true;
            lcd.setCursor(0,0);
            lcd.print("Nuevo Pass:");
            lcd.setCursor(0,1);
            PassIngresado="";
            lcd.print("        ");
            p=0;
        }
        else if(key=='C'){
          if(puertaBloqueada){
              lcd.setCursor(0,1);
              lcd.print("Puerta Bloqueada");
              //Serial.println("Abriendo");
              String rq="GET /index.php/option/0 HTTP/1.1\r\n";
              String addr="apiphparqui1.azurewebsites.net";
              EnviarApi(addr,rq);
              PassIngresado="";
              p=0;
              delay(1000);
              lcd.setCursor(0,1);
              lcd.print("                ");
              lcd.setCursor(7,0);
              lcd.print("E:B");
          }else{
              lcd.setCursor(0,1);
              lcd.print("Abriendo        ");
              //Serial.println("Abriendo");
              String rq="GET /index.php/option/1 HTTP/1.1\r\n";
              String addr="apiphparqui1.azurewebsites.net";
              EnviarApi(addr,rq);
              PassIngresado="";
              p=0;
              //delay(1000);         
              for (int pos = 35; pos <= 125; pos += 1) {
                porton.write(pos);              // tell servo to go to position in variable 'pos'
                if(analogRead(entroPorton)<limiteEntrada)
                    contadorPorton++;               
                delay(15);                       // waits 15ms for the servo to reach the position
                lcd.setCursor(15,0);
                lcd.print(contadorPorton);
              }
              delay(10000);
              for (int pos = 125; pos >= 35; pos -= 1) {
                porton.write(pos);              // tell servo to go to position in variable 'pos'
                delay(15);                       // waits 15ms for the servo to reach the position
              }
              lcd.setCursor(0,1);
              lcd.print("                ");
              lcd.setCursor(7,0);
              lcd.print("E:A");
          }
        }
        else if(key=='A'){
            lcd.setCursor(0,1);
            if(PassIngresado==pass){
              lcd.print("Abriendo        ");
              //Serial.println("Abriendo");
              String rq="GET /index.php/option/1 HTTP/1.1\r\n";
              String addr="apiphparqui1.azurewebsites.net";
              EnviarApi(addr,rq);
              PassIngresado="";
              p=0;
              //delay(1000);
              
              puertaBloqueada=false;
              for (int pos = 35; pos <= 125; pos += 1) {
                porton.write(pos);              // tell servo to go to position in variable 'pos'
                if(analogRead(entroPorton)<limiteEntrada)
                    contadorPorton++;
                delay(15);                       // waits 15ms for the servo to reach the position
                lcd.setCursor(12,0);
                lcd.print(contadorPorton);
              }
              delay(10000);
              for (int pos = 125; pos >= 35; pos -= 1) {
                porton.write(pos);              // tell servo to go to position in variable 'pos'
                if(analogRead(entroPorton)<limiteEntrada){
                    contadorPorton++;
                    //Serial.println(analogRead(entroC));
                 }
                delay(15);                       // waits 15ms for the servo to reach the position
              }
              lcd.setCursor(0,1);
              lcd.print("                ");
              lcd.setCursor(7,0);
              lcd.print("E:A");
            }
            else{
              lcd.print("Incorrecto      ");
              //Serial.println("Incorrecto");
              String rq="GET /index.php/option/0 HTTP/1.1\r\n";
              String adr="apiphparqui1.azurewebsites.net";
              EnviarApi(adr,rq);
              PassIngresado="";
              delay(1000);
              lcd.setCursor(0,1);
              lcd.print("                ");
              lcd.setCursor(0,1);
              lcd.setCursor(7,0);
              lcd.print("E:B");
              p=0;
              puertaBloqueada=true;
            }
        }
        else{
            //Serial.println(key);
            lcd.setCursor(p, 1);
            lcd.print(key);
            p++;
            PassIngresado=PassIngresado+key;
        }
    }
}
void MODULOPISCINA(){
    digitalWrite(Ptriger,HIGH);
    espera(500);
    digitalWrite(Ptriger,LOW);
    duracion = pulseIn(Pecho,HIGH);
    distancia = (duracion/2)/29;
    if(distancia<=llena){
      if(!NotificoLlena){
        Serial.println("piscina llena");
        Serial.print(distancia);
        String rq="GET /1 HTTP/1.1\r\n";
        String adr="apijavap1a1g1.azurewebsites.net";
        EnviarApi(adr,rq);
        //Serial.println("cm");
        NotificoLlena=true;
        detenerM=true;
      }else{
        Serial.println("piscina llena");
        Serial.print(distancia);
        Serial.println("cm");
        detenerM=true;
      }
    }
    else if(distancia>=vacia){
      if(!NotificoVacia){
        Serial.println("Piscina vacia");
        Serial.print(distancia);
        Serial.println("cm");
        String rq="GET  /0 HTTP/1.1\r\n";
        String adr="apijavap1a1g1.azurewebsites.net";
        EnviarApi(adr,rq);
        NotificoVacia=true;
        detenerS=true;
      }else{
        Serial.println("piscina vacia");
        Serial.print(distancia);
        Serial.println("cm");
        detenerS=true;
      }
    }

}
void MODULOHUMO(){
  if(analogRead(gas)>300){
    Serial.println(analogRead(gas));
    String adr="apiarquigolang1.appspot.com";
    String rqs="GET / HTTP/1.1\r\n";
    EnviarApi(adr,rqs);
    digitalWrite(buzzer,HIGH);
    delay(100);
  }else{
    Serial.println(analogRead(gas));
    delay(100);
  }
}
void MODULOPIR(){
   // put your main code here, to run repeatedly:
  lectura = digitalRead(PIR);
  if(lectura==HIGH){
    digitalWrite(LED,HIGH);
    String adr="http://nodejsfull-env.4bd43u6mqr.us-east-1.elasticbeanstalk.com";
    String rqs="GET /1 HTTP/1.1\r\n";
    EnviarApi(adr,rqs);
    Serial.println("Se detecto movimiento");
  }else{
    digitalWrite(LED,LOW);
    Serial.println("No se detecto movimiento");
  }
}

void acciones(){
  if(lleno){
    digitalWrite(motorS,HIGH);
  }
  if(detenerM){
    digitalWrite(motorS,LOW);
    NotificoVacia=false;   
    detenerM=false;
    //lleno=false;
  }
  if(vacio){
    digitalWrite(motorS,HIGH);
  }
  if(detenerS){
    digitalWrite(motorS,LOW);
    NotificoLlena=false;
    vacio=false; 
    detenerS=false;
  }
  if(apagaralarma){
    digitalWrite(buzzer,LOW);
  }
  if(abrirPuerta){
    for (int pos = 35; pos <= 125; pos += 1) {
      porton.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    delay(10000);
    for (int pos = 125; pos >= 35; pos -= 1) {
      porton.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    abrirPuerta=false;
  }
  if(bloqSist==3){
    String adr="flask-env.zzivigmhhq.us-east-2.elasticbeanstalk.com";
    String rqs="GET /2 HTTP/1.1\r\n";
    EnviarApi(adr,rqs);
    bloqSist=0;
  }
}

boolean waitOKfromESP8266(int timeout){
  do{
    Serial.println("wait OK...");
    delay(1000);
    if(ESP8266.find("OK"))
    {
      return true;
    }

  }while((timeout--)>0);
  return false;
}

boolean cwJoinAP(){
  String cmd="AT+CWJAP=\"" + SSID + "\",\"" + PASSWORD + "\"";
  ESP8266.println(cmd);
  return waitOKfromESP8266(10);
}

void LeerWifi(){
  if(Serial3.available()) // Comprueba si el ESP esta enviando mensaje
  {
    if(Serial3.find("+IPD,")){
       delay(1000);    
     
       int connectionId = Serial3.read()-48; // Restar 48 porque la función read () devuelve 
       String webpage ="Correcto";
       String recibido=sendData("AT+CIPSEND=" + String(connectionId) + "," + webpage.length() + "\r\n", 500, true);
       sendData(webpage, 1000, true); // 
       sendData("AT+CIPCLOSE=" + String(connectionId) + "\r\n", 1000, true); 
       int posi= recibido.indexOf("GET /")+5;
       int posf=recibido.indexOf(" HTTP/1.1");
       recibido = recibido.substring(posi,posf);
       //Para llenar piscina
       if(recibido=="llenar")     {
          lleno=true;
          Serial.println("llenar");
       }
       //Para vaciar piscina
       else if(recibido=="vaciar"){
          vacio=true;
       }
       //para apagar alarma
       else if(recibido=="apagar"){
          apagaralarma=true;
       }
       //para abrir puerta
       else if(recibido=="abrir"){
          String adr="flask-env.zzivigmhhq.us-east-2.elasticbeanstalk.com";
          String rqs="GET /1 HTTP/1.1\r\n";
          EnviarApi(adr,rqs);
          abrirPuerta=true;
          bloqSist=0;
       }
       //para cerrar puerta
       else if(recibido=="fallo"){
          String adr="flask-env.zzivigmhhq.us-east-2.elasticbeanstalk.com";
          String rqs="GET /0 HTTP/1.1\r\n";
          EnviarApi(adr,rqs);
          bloqSist++;
       }
       //para activar modulo de seguridad
       else if(recibido=="seguridad"){
          activarPIR=!activarPIR;
       }
       //para apagar modulo de seguridad
       else if(recibido=="seguridadOff"){
          activarPIR=false;
       }else
        Serial.println(recibido);
     }
  }
}
void espera(const int timeout){
  int cont=0;
    while(timeout > cont)
    {
      ModuloTeclado();
      
      LeerWifi();
      acciones();
      cont++;
    }
}

String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    ESP8266.print(command); // Se envía el carácter de lectura a la esp8266
    long int time = millis();
    while( (time+timeout) > millis())
    {while(ESP8266.available())
      { 
        char c = ESP8266.read(); // Lee el siguiente carácter.
        response+=c;
      }  
    }
    
    if(debug)
    {
      Serial.print(response);
    }    
    return response;
}

void clearESP8266SerialBuffer(){
  Serial.println("= clearESP8266SerialBuffer() =");
  while (ESP8266.available() > 0) {
    char a = ESP8266.read();
    Serial.write(a);
  }
  Serial.println("==============================");
}

void EnviarApi(String TARGET_ADDR, String HTTP_RQS){
  String TARGET_ID="0";
  String TARGET_TYPE="TCP";
  String TARGET_PORT="80";

  String cmd="AT+CIPSTART=" + TARGET_ID;
  cmd += ",\"" + TARGET_TYPE + "\",\"" + TARGET_ADDR + "\"";
  cmd += ","+ TARGET_PORT;

  Serial.println(cmd);
  ESP8266.println(cmd);
  delay(1000);

  clearESP8266SerialBuffer();

  HTTP_RQS += "Host: " + TARGET_ADDR;
  HTTP_RQS += ":" + TARGET_PORT + "\r\n\r\n";
  
  String cmdSEND_length = "AT+CIPSEND=";
  cmdSEND_length += TARGET_ID + "," + HTTP_RQS.length() +"\r\n";
  
  ESP8266.print(cmdSEND_length);
  Serial.println(cmdSEND_length);
  
  Serial.println("waiting >");
  
  if(!ESP8266.available());
  
  if(ESP8266.find(">")){
    Serial.println("> received");
    ESP8266.println(HTTP_RQS);
    Serial.println(HTTP_RQS);
    
    boolean OK_FOUND = false;
    
    //program blocked untill "SEND OK" return
    while(!OK_FOUND){
      if(ESP8266.readBytesUntil('\n', buffer, BUFFER_SIZE)>0){
        Serial.println("...");
        Serial.println(buffer);
        
        if(strncmp(buffer, "SEND OK", 7)==0){
          OK_FOUND = true;
          Serial.println("SEND OK found");
        }else{
          Serial.println("Not SEND OK...");
        }
      }
    }

  }else{
    Serial.println("> NOT received, something wrong!");
  }
  
  //Close connection
  String cmdCIPCLOSE = "AT+CIPCLOSE=" + TARGET_ID; 
  ESP8266.println(cmdCIPCLOSE);
  Serial.println(cmdCIPCLOSE);
  
  delay(100);
}

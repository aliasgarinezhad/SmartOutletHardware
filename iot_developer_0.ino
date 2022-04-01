#include <WiFiClientSecureBearSSL.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

#define https_port 443
#define CHIP_ID 16531515
#define FLASH_ID 1335390
#define LED 0

void login_init();
void client_receive();
void server_process();
void control_init();
void eeprom_init();
void wifi_init();
void request_init();
void database_process();
void server_receive();

char ssid[32]="Smart_Sw4";
char password[32]="12345678";

char router_ssid[32]="SSID";
char router_password[32]="12345678";
String router_ssid_str;

String switch_server_request;
char serial_number[15]="6754-6303";

WiFiServer server(80);
WiFiClient client;
WiFiClientSecure switch_server;

//char url []="takmobile12.ir";
char url []="mamatirnoavar.ir";

String database_bytes;

String root;
String client_bytes;
char out=0;
String form1;
String form2;

char out_address=1;

char ssid_address=2;
char password_address=35;

char router_ssid_address=68;
char router_password_address=101;

char i;
int delay_counter;
char temp;
char router=0;

void setup() {

  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  
  if((ESP.getChipId() != CHIP_ID) || (ESP.getFlashChipId() != FLASH_ID))
  {
    ESP.restart();
  }
  
  eeprom_init();
  
  digitalWrite(LED, !out);
  
  wifi_init();
  
}

void loop() {
  
  if(router)
  {
    if (WiFi.status() == WL_CONNECTED) 
    {
      if(switch_server.connect(url, https_port))
      {
        request_init();
        switch_server.print(switch_server_request); 
        server_receive();
        database_process();   
        delay(3000);     
      }
    }  

    else
    {
      ESP.restart();
    }
  }

  else
  {
    client=server.available();
        
    if(client)
    {
      delay(100);
      client_receive();
      server_process();
    } 
    
    delay(1);
    delay_counter++;
    if(delay_counter>20000)
    {
      WiFi.scanNetworksAsync(scan_result_check, true);  
      delay_counter=0;
    }
           
  }
  
}

void client_receive()
{
  temp;
  client_bytes.remove(0);
  while(client.available())
  {
    temp=client.read();
    client_bytes += temp;
  }  
}

void server_process()
{
  client_bytes=client_bytes.substring(client_bytes.lastIndexOf("\r\n"));

  int f1=client_bytes.indexOf("turn+off");
  int f2=client_bytes.indexOf("turn+on");
  int f3=client_bytes.indexOf("setting");
  int f4=client_bytes.indexOf("save");
  
  if(f1==-1 && f2!=-1 && f3==-1 && f4==-1)
  {
    digitalWrite(LED, LOW);
    out=1;
    EEPROM.write(out_address,out);
    EEPROM.commit();
    
    control_init();
    client.print(root);
  }
  else if(f1!=-1 && f2==-1 && f3==-1 && f4==-1)
  {
    digitalWrite(LED, HIGH);
    out=0;
    EEPROM.write(out_address,out);
    EEPROM.commit();
    
    control_init();
    client.print(root);
  }
  else if(f1==-1 && f2==-1 && f3!=-1 && f4==-1)
  {
    login_init();
    client.print(root);
  }
  else if(f1==-1 && f2==-1 && f3==-1 && f4!=-1)
  {
    form1=client_bytes.substring(client_bytes.indexOf("local_name")+11,client_bytes.indexOf("&local_password"));
    form2=client_bytes.substring(client_bytes.indexOf("local_password")+15,client_bytes.indexOf("&router_name"));
    form1.toCharArray(ssid, 32);
    form2.toCharArray(password, 32);
    
    for(i=ssid_address;i<ssid_address+32;i++)
    {
      EEPROM.write(i, ssid[i-ssid_address]);
      EEPROM.commit();
    }
    
    for(i=password_address;i<password_address+32;i++)
    {
      EEPROM.write(i, password[i - password_address]);
      EEPROM.commit();   
    }

    form1=client_bytes.substring(client_bytes.indexOf("router_name")+12,client_bytes.indexOf("&router_password"));
    form2=client_bytes.substring(client_bytes.indexOf("router_password")+16,client_bytes.indexOf("&save"));
    form1.toCharArray(router_ssid, 32);
    form2.toCharArray(router_password, 32);
    
    for(i=router_ssid_address;i<router_ssid_address+32;i++)
    {
      EEPROM.write(i, router_ssid[i-router_ssid_address]);
      EEPROM.commit();
    }
    
    for(i=router_password_address;i<router_password_address+32;i++)
    {
      EEPROM.write(i, router_password[i - router_password_address]);
      EEPROM.commit();   
    }
        
    save_init();
    client.print(root);
    delay(1000);
    
    ESP.restart();   
  }

  else
  {
    control_init();
    client.print(root);  
  }
  
}

void control_init()
{
  root.remove(0);
  root += "HTTP/1.1 200 OK\r\n";
  root += "\r\n";
  root += "<!doctype html>\r\n";
  root += "<html>\r\n";
  root += "<head>\r\n";
  root += "<title>control panel</title>\r\n";
  root += "</head>\r\n";
  root += "<body>\r\n";
  root += "<form method=\"POST\">\r\n";
  if(out)
  {
    root += "<p style=\"font-size:30px; text-align:center; color:green;\"> وضعیت : روشن </p>\r\n";
    root += "<p style=\"text-align:center; \"><input style=\"width:250px; height:100px; \" type=\"submit\" name=\"turn off\" value=\"خاموش کردن\"><p>\r\n";  
  }
  else
  {
    root += "<p style=\"font-size:30px; text-align:center; color:red;\"> وضعیت : خاموش </p>\r\n";
    root += "<p style=\"text-align:center; \"><input style=\"width:250px; height:100px; \" type=\"submit\" name=\"turn on\" value=\"روشن کردن\"><p>\r\n";
  }   
  root += "<p> <br> <p>\r\n";
  
  root += "<p style=\"text-align:right; \"><input style=\"width:100px; height:25px; \" type=\"submit\" name=\"setting\" value=\"تنظیمات\"><p>\r\n";
  root += "</form>\r\n";
  root += "</body>\r\n";
  root += "</html>\r\n";
  root += "\r\n";
}

void login_init()
{
  root.remove(0);
  root += "HTTP/1.1 200 OK\r\n";
  root += "\r\n";
  root += "<!doctype html>\r\n";
  root += "<html>\r\n";
  root += "<head>\r\n";
  root += "<title>Login page</title>\r\n";
  root += "</head>\r\n";
  root += "<body>\r\n";
  root += "<form method=\"POST\">\r\n";
  
  root += "<p style=\"text-align:right;\"> نام <br> <input type=\"text\" name=\"local_name\" value=\""; 
  root += ssid;
  root += "\"></p>\r\n";
  
  root += "<p style=\"text-align:right;\"> رمز عبور <br> <input type=\"text\" name=\"local_password\" value=\"";
  root += password;
  root += "\"></p>\r\n";
  
  root += "<p> <br> <p>\r\n";
  
  root += "<p style=\"text-align:right;\"> نام مودم <br> <input type=\"text\" name=\"router_name\" value=\""; 
  root += router_ssid;
  root += "\"></p>\r\n";
  
  root += "<p style=\"text-align:right;\"> رمز عبور مودم <br> <input type=\"text\" name=\"router_password\" value=\"";
  root += router_password;
  root += "\"></p>\r\n";
  
  root += "<p> <br> <p>\r\n";

  root += "<p> <br> <p>\r\n";
  root += "<p style=\"text-align:right;\"> <input type=\"submit\" name=\"cancel\" value=\"لغو\"> <input type=\"submit\" name=\"save\" value=\"ذخیره\"> <p>\r\n"; 

  root += "</form>\r\n";
  
  root += "</body>\r\n";
  root += "</html>\r\n";
  root += "\r\n";
}

void save_init()
{
  root.remove(0);
  root += "HTTP/1.1 200 OK\r\n";
  root += "\r\n";
  root += "<!doctype html>\r\n";
  root += "<html>\r\n";
  root += "<head>\r\n";
  root += "<title>save page</title>\r\n";
  root += "</head>\r\n";
  root += "<body>\r\n";
  
  root += "<p style=\"font-size:30px; text-align:center;\"> تنظیمات ذخیره شد </p>\r\n";
  root += "<p style=\"font-size:30px; text-align:center;\"> لطفا به صفحه اصلی برگردید </p>\r\n";

  
  root += "</body>\r\n";
  root += "</html>\r\n";
  root += "\r\n";
}

void eeprom_init(){

  EEPROM.begin(512);
  
  char save = EEPROM.read(0);
  if(save == '1')
  {
     out=EEPROM.read(out_address);
     
     for(i=ssid_address;i<ssid_address+32;i++)
     {
        ssid[i-ssid_address]=EEPROM.read(i);
     }

     for(i=password_address;i<password_address+32;i++)
     {
        password[i-password_address]=EEPROM.read(i);
     }
     
     for(i=router_ssid_address;i<router_ssid_address+32;i++)
     {
        router_ssid[i-router_ssid_address]=EEPROM.read(i);
     }

     for(i=router_password_address;i<router_password_address+32;i++)
     {
        router_password[i-router_password_address]=EEPROM.read(i);
     }
  }
  else
  {
    EEPROM.write(out_address,out);
    EEPROM.commit();
    
    for(i=ssid_address;i<ssid_address+32;i++)
    {
      EEPROM.write(i, ssid[i-ssid_address]);
      EEPROM.commit();
    }
    
    for(i=password_address;i<password_address+32;i++)
    {
      EEPROM.write(i, password[i-password_address]);
      EEPROM.commit();
    }
    
    for(i=router_ssid_address;i<router_ssid_address+32;i++)
    {
      EEPROM.write(i, router_ssid[i-router_ssid_address]);
      EEPROM.commit();
    }
    
    for(i=router_password_address;i<router_password_address+32;i++)
    {
      EEPROM.write(i, router_password[i-router_password_address]);
      EEPROM.commit();
    }
    
    EEPROM.write(0,'1');
    EEPROM.commit();
  }
  
  router_ssid_str += router_ssid;  
}

void wifi_init()
{
  switch_server.setInsecure();
  WiFi.mode(WIFI_STA);
  WiFi.begin(router_ssid, router_password);

  for(i=0;i<20;i++)
  {  
    if (WiFi.status() == WL_CONNECTED) 
    {
      router=1;
      break;
    }
    delay(500);
  }
     
  if(!router)
  {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid, password);
    server.begin();
  }  
}

void request_init()
{
  switch_server_request.remove(0);
  switch_server_request +="GET /switchs/switches_ma.php/?serial_number=" + String(serial_number) + " HTTP/1.1\r\n";

  switch_server_request += String("Host: ") + url + String("\r\n");

  switch_server_request += "\r\n";
}

void server_receive()
{ 
    while(!switch_server.available()){}
    database_bytes.remove(0);
    while(switch_server.available())
    {             
      temp=switch_server.read();
      database_bytes += temp;
      delay(1);                     
    } 
}

void database_process()
{
    int status_index=database_bytes.indexOf("status:");
    if(status_index!=-1)
    {
      int turn_on=database_bytes.indexOf("status:1");  
      int turn_off=database_bytes.indexOf("status:0");
       
      if(turn_on!=-1 && turn_off==-1)
      {
        digitalWrite(LED, LOW);
        out=1;  
      }

      else if(turn_on==-1 && turn_off!=-1)
      {
        digitalWrite(LED, HIGH);
        out=0;   
      }
      EEPROM.write(out_address,out);
      EEPROM.commit();  
    }  
}

void scan_result_check(int number_of_networks)
{
  for (int i = 0; i<number_of_networks; i++)
  {
    if(WiFi.SSID(i)==router_ssid_str)
    {
      ESP.restart();   
    }
  }
}

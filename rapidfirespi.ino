#define SPI_DATA_PIN 19
#define SPI_SS_PIN 5
#define SPI_CLOCK_PIN 18

#define delaytime 70

byte bitBangData(byte _send)  // This function transmit the data via bitbanging
{

// for(int i=0; i<8; i++)  // 8 bits in a byte
  for(int i=7; i>=0; i--)  // 8 bits in a byte
  {
    
    digitalWrite(SPI_DATA_PIN, bitRead(_send, i));   
    delayMicroseconds(delaytime);
    digitalWrite(SPI_CLOCK_PIN, HIGH);  
    //delayMicroseconds(delaytime);
    //digitalWrite(SPI_DATA_PIN, LOW);
    delayMicroseconds(delaytime);
    digitalWrite(SPI_CLOCK_PIN, LOW); 
    delayMicroseconds(delaytime);
      
  } 
 
}

void setup(void) {
  //INIT SPI 
  pinMode(SPI_SS_PIN, OUTPUT);
  pinMode(SPI_DATA_PIN, OUTPUT);
  pinMode(SPI_CLOCK_PIN, OUTPUT);

  
  digitalWrite(SPI_SS_PIN, HIGH);
  digitalWrite(SPI_CLOCK_PIN, HIGH);
  digitalWrite(SPI_DATA_PIN, HIGH);
  delay(200); //Let Rapidfire init SPI       
  
  digitalWrite(SPI_SS_PIN, LOW);
  digitalWrite(SPI_CLOCK_PIN, LOW);
  digitalWrite(SPI_DATA_PIN, LOW);
  delay(1000);
  digitalWrite(SPI_SS_PIN, HIGH);

  delay(1000); //Wait again 
}

void loop(void) {
digitalWrite(SPI_SS_PIN, LOW);
delayMicroseconds(delaytime);
bitBangData(83); // data transmission   Let Buzzer Beep every second
bitBangData(62); // data transmission
bitBangData(00); // data transmission
bitBangData(145); // data transmission
delayMicroseconds(delaytime);
digitalWrite(SPI_SS_PIN, HIGH);
delay(1000);            //bissal länger warten
}

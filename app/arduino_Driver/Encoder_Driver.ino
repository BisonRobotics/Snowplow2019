#define A_PIN1 2
#define B_PIN1 4
#define A_PIN2 3
#define B_PIN2 5
#define IDENTIFIER '1'
#define BUFFER_LENGTH 64
#define DISTANCE_COMMAND 'd'
#define IDENTIFY_COMMAND 'i'

typedef struct{
  char returnChar;
  int32_t Distance1;
  uint8_t Crc1;
  int32_t Distance2;
  uint8_t Crc2;
}Distance_S;

typedef struct{
  char returnChar;
  int32_t Length;
  uint8_t Crc;
}Length_S;


volatile int32_t distance1 = 0, distance2 = 0;
char input;

void A_High1()
{
  if (digitalRead(B_PIN1))
  {
    distance1++;
  }
  else
  {
    distance1--;
  }
}
void A_High2()
{
  if (digitalRead(B_PIN2))
  {
    distance2++;
  }
  else
  {
    distance2--;
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(A_PIN1, INPUT);
  pinMode(B_PIN1, INPUT);
  pinMode(A_PIN2, INPUT);
  pinMode(B_PIN2, INPUT);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(A_PIN1), A_High1, RISING);
  attachInterrupt(digitalPinToInterrupt(A_PIN2), A_High2, RISING);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0)
  {
    input = Serial.read();
    processCommand(input);
  }
}

uint8_t CRC8(uint8_t* bufferStart, uint8_t bufferSize)
{
  uint8_t returnVal = 0;
  for(uint8_t i = 0; i<bufferSize; i++)
  {
    returnVal ^= (*bufferStart);
    bufferStart++;
  }
  return returnVal;
}

void processCommand(char command)
{
  uint8_t SaveSREG;
  switch (command)
  {
    case IDENTIFY_COMMAND:
      Serial.print(IDENTIFIER);
      break;
    case DISTANCE_COMMAND:
      Distance_S toSendD;
      SaveSREG = SREG;   // save interrupt flag

      toSendD.returnChar = command;
      
      cli();   // disable interrupts
      toSendD.Distance1 = distance1;
      toSendD.Distance2 = distance2;
      distance1 = 0;
      distance2 = 0;
      SREG = SaveSREG;   // restore the interrupt flag

      toSendD.Crc1 = CRC8((uint8_t*)&toSendD, 5);
      toSendD.Crc2 = CRC8((uint8_t*)&toSendD.Distance2, 4);

      Serial.write((uint8_t*)&toSendD, sizeof(Distance_S));
      break;

    case 'l':
      Length_S toSendL;
      toSendL.returnChar = command;
      toSendL.Length = analogRead(A0);
      toSendL.Crc = CRC8((uint8_t*) &toSendL, 5);
      Serial.write((uint8_t*)&toSendL, sizeof(toSendL));
      break;
  }
}


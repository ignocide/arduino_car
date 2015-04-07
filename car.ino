#include <SoftwareSerial.h> //아두이노 우노에서 시리얼 통신은 기본으로 0번과 1번을 쓴다.
#define dht11_pin 54 //Analog port 0 on Arduino Mega2560
// 이 포트를 중복해서 사용하지 않고 다른 포트를 사용하기 위해서는
// 아두이노에서 제공하는 라이브러리 SoftwareSerial을 사용하면 된다.
#include <Wire.h>
//----------------------------------MPU6050 연결핀 및 설정값------
#define MPU6050_ACCEL_XOUT_H 0x3B // R
#define MPU6050_PWR_MGMT_1 0x6B // R/W
#define MPU6050_PWR_MGMT_2 0x6C // R/W
#define MPU6050_WHO_AM_I 0x75 // R
#define MPU6050_I2C_ADDRESS 0x68
//----------------------------------모터제어모듈 연결핀-------
#define LE 52
#define RE 53
#define LM 50
#define RM 51
#define LS 9
#define RS 12
//----------------------------------기타 핀----------------
#define dp 1 //display to BT and serial
#define shock 7
#define sonicPin 8
#define led 13

SoftwareSerial BTserial(10,11); // BTserial이라는 이름의 통신장치로 2(TX), 3(RX)포트로 시리얼 통신한는 설정

long sonicval = 0;
long cm, inch;
int a = 0;
int b = 0;
int s = 0;
static int n=4;

int pwm = 120;//모터의 속도
int turn = 80;//좌우회전시 추가해야할 속도

byte read_dht11_dat()
{
  byte i = 0;
  byte result=0;
  for(i=0; i< 8; i++)
  {
    while (!digitalRead(dht11_pin));
    delayMicroseconds(30);
    if (digitalRead(dht11_pin) != 0 )
      bitSet(result, 7-i);
    while (digitalRead(dht11_pin));
  }
  return result;
}
long cmtomicro(long a){
  return a/29/2;
}
long inchtomicro(long b){
  return b/74/2;
}

typedef union accel_t_gyro_union
{
  struct
  {
    uint8_t x_accel_h;
    uint8_t x_accel_l;
    uint8_t y_accel_h;
    uint8_t y_accel_l;
    uint8_t z_accel_h;
    uint8_t z_accel_l;
    uint8_t t_h;
    uint8_t t_l;
    uint8_t x_gyro_h;
    uint8_t x_gyro_l;
    uint8_t y_gyro_h;
    uint8_t y_gyro_l;
    uint8_t z_gyro_h;
    uint8_t z_gyro_l;
  }
  reg;
  struct
  {
    int x_accel;
    int y_accel;
    int z_accel;
    int temperature;
    int x_gyro;
    int y_gyro;
    int z_gyro;
  }
  value;
};


void setup() {
  // initialize the LED pin as an output:
  Serial.begin(9600);
  BTserial.begin(9600); // 통신모듈의 baudrate값을 써 통신을 시작
  pinMode(led, OUTPUT);
  pinMode(dht11_pin, OUTPUT);
  digitalWrite(dht11_pin, HIGH);
  pinMode(sonicPin, OUTPUT);
  digitalWrite(sonicPin, HIGH);
  pinMode(shock, OUTPUT);
  digitalWrite(shock, HIGH);
  pinMode(LE,OUTPUT);
  pinMode(RE,OUTPUT);
  pinMode(LM,OUTPUT);
  pinMode(RM,OUTPUT);
  pinMode(LS,OUTPUT);
  pinMode(RS,OUTPUT);
  digitalWrite(LE,LOW); // 왼쪽 모터 정지 정보 핀
  digitalWrite(RE,LOW); // 왼쪽 모터 정지 정보 핀

  int error;
  uint8_t c;
  Wire.begin();

  error = MPU6050_read (MPU6050_WHO_AM_I, &c, 1);
  Serial.print(F("WHO_AM_I : "));
  Serial.print(c,HEX);
  Serial.print(F(", error = "));
  Serial.println(error,DEC);
  error = MPU6050_read (MPU6050_PWR_MGMT_2, &c, 1);
  Serial.print(F("PWR_MGMT_2 : "));
  Serial.print(c,HEX);
  Serial.print(F(", error = "));
  Serial.println(error,DEC);
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);
}

void loop(){
  //---------------------------------led on off----------------------------
  /*
digitalWrite(led, HIGH); // turn the LED on (HIGH is the voltage level)
   delay(300); // wait for a second
   digitalWrite(led, LOW); // turn the LED off by making the voltage LOW
   delay(300); // wait for a second
   BTserial.println("check"); // 블루투스를 통해 다른 장치에 출력
   Serial.println(a);
   */
  if(a==500)
  {
    //--------------------------------dht11-------------------------------------
    byte dht11_dat[5];
    byte dht11_in;
    byte i;// start condition
    digitalWrite(dht11_pin, LOW);
    delay(10);
    digitalWrite(dht11_pin, HIGH);
    delayMicroseconds(1);
    pinMode(dht11_pin, INPUT);
    delayMicroseconds(40);
    if (digitalRead(dht11_pin))
    {
      Serial.println("dht11 start condition 1 not met"); // wait for DHT response signal: LOW
      delay(1000);
      return;
    }
    delayMicroseconds(80);
    if (!digitalRead(dht11_pin))
    {
      Serial.println("dht11 start condition 2 not met"); //wair for second response signal:HIGH
      return;
    }
    delayMicroseconds(80);// now ready for data reception
    for (i=0; i<5; i++)
    {
      dht11_dat[i] = read_dht11_dat();
    } //recieved 40 bits data. Details are described in datasheet
    pinMode(dht11_pin, OUTPUT);
    digitalWrite(dht11_pin, HIGH);
    byte dht11_check_sum = dht11_dat[0]+dht11_dat[2];// check check_sum
    if(dht11_dat[4]!= dht11_check_sum)
    {
      Serial.println("DHT11 checksum error");
    }
    //------------------------------------sonic----------------------------------
    digitalWrite(sonicPin,LOW);
    delayMicroseconds(2);
    digitalWrite(sonicPin,HIGH);
    delayMicroseconds(5);
    digitalWrite(sonicPin,LOW);
    pinMode(sonicPin, INPUT);

    sonicval = pulseIn(sonicPin,HIGH); // 초음파가 반사하여 되돌아온 시간을 저장

    pinMode(sonicPin, OUTPUT);

    cm = cmtomicro(sonicval); // cm거리계산
    inch = inchtomicro(sonicval); //inch거리계산
    //-------------------------------shock---------------------------------------
    digitalWrite(shock,HIGH);
    delay(5);
    digitalWrite(shock,LOW);
    delay(5);
    pinMode(shock,INPUT);
    if(digitalRead(shock))
    {
      Serial.println("충격이 감지되었습니다.");
    }
    pinMode(shock,OUTPUT);
    //-------------------------------gyro----------------------------------------
    int error;
    double dT;
    accel_t_gyro_union accel_t_gyro;
    Serial.println(F(""));
    Serial.println(F("MPU-6050"));
    // Read the raw values.
    // Read 14 bytes at once,
    // containing acceleration, temperature and gyro.
    // With the default settings of the MPU-6050,
    // there is no filter enabled, and the values
    // are not very stable.
    error = MPU6050_read (MPU6050_ACCEL_XOUT_H, (uint8_t *) &accel_t_gyro, sizeof(accel_t_gyro));
    Serial.print(F("Read accel, temp and gyro, error = "));
    Serial.println(error,DEC);
    // Swap all high and low bytes.
    // After this, the registers values are swapped,
    // so the structure name like x_accel_l does no
    // longer contain the lower byte.
    uint8_t swap;
#define SWAP(x,y) swap = x; x = y; y = swap
    SWAP (accel_t_gyro.reg.x_accel_h, accel_t_gyro.reg.x_accel_l);
    SWAP (accel_t_gyro.reg.y_accel_h, accel_t_gyro.reg.y_accel_l);
    SWAP (accel_t_gyro.reg.z_accel_h, accel_t_gyro.reg.z_accel_l);
    SWAP (accel_t_gyro.reg.t_h, accel_t_gyro.reg.t_l);
    SWAP (accel_t_gyro.reg.x_gyro_h, accel_t_gyro.reg.x_gyro_l);
    SWAP (accel_t_gyro.reg.y_gyro_h, accel_t_gyro.reg.y_gyro_l);
    SWAP (accel_t_gyro.reg.z_gyro_h, accel_t_gyro.reg.z_gyro_l);
    // Print the raw acceleration values

    //-------------------------------display-------------------------------------
    if(dp == 1)
    {
      Serial.print(cm);
      Serial.print("cm ");
      Serial.print(inch);
      Serial.println("inch");

      Serial.print("Current humdity = ");
      Serial.print(dht11_dat[0], DEC);
      Serial.print("% ");
      Serial.print("temperature = ");
      Serial.print(dht11_dat[2], DEC);
      Serial.println("C ");

      BTserial.print(cm);
      BTserial.print("cm ");
      BTserial.print(inch);
      BTserial.println("inch"); // 블루투스를 통해 다른 장치에 출력
      BTserial.println();
      BTserial.print("Current humdity = ");
      BTserial.print(dht11_dat[0], DEC);
      BTserial.print("% ");
      BTserial.print("temperature = ");
      BTserial.print(dht11_dat[2], DEC);
      BTserial.println("C ");
      BTserial.println();
      Serial.print(F("accel x,y,z: "));
      Serial.print(accel_t_gyro.value.x_accel, DEC);
      Serial.print(F(", "));
      Serial.print(accel_t_gyro.value.y_accel, DEC);
      Serial.print(F(", "));
      Serial.print(accel_t_gyro.value.z_accel, DEC);
      Serial.println(F(""));
      BTserial.print(F("accel x,y,z: "));
      BTserial.print(accel_t_gyro.value.x_accel, DEC);
      BTserial.print(F(", "));
      BTserial.print(accel_t_gyro.value.y_accel, DEC);
      BTserial.print(F(", "));
      BTserial.print(accel_t_gyro.value.z_accel, DEC);
      BTserial.println(F(""));
      BTserial.println();
      // The temperature sensor is -40 to +85 degrees Celsius.
      // It is a signed integer.
      // According to the datasheet:
      // 340 per degrees Celsius, -512 at 35 degrees.
      // At 0 degrees: -512 - (340 * 35) = -12412
      Serial.print(F("temperature: "));
      BTserial.print(F("temperature: "));
      dT = ( (double) accel_t_gyro.value.temperature + 12412.0) / 340.0;
      Serial.print(dT, 3);
      Serial.print(F(" degrees Celsius"));
      Serial.println(F(""));
      BTserial.print(dT, 3);
      BTserial.print(F(" degrees Celsius"));
      BTserial.println(F(""));
      BTserial.println();
      // Print the raw gyro values.
      Serial.print(F("gyro x,y,z : "));
      Serial.print(accel_t_gyro.value.x_gyro, DEC);
      Serial.print(F(", "));
      Serial.print(accel_t_gyro.value.y_gyro, DEC);
      Serial.print(F(", "));
      Serial.print(accel_t_gyro.value.z_gyro, DEC);
      Serial.print(F(", "));
      Serial.println(F(""));
      BTserial.print(F("gyro x,y,z : "));
      BTserial.print(accel_t_gyro.value.x_gyro, DEC);
      BTserial.print(F(", "));
      BTserial.print(accel_t_gyro.value.y_gyro, DEC);
      BTserial.print(F(", "));
      BTserial.print(accel_t_gyro.value.z_gyro, DEC);
      BTserial.print(F(", "));
      BTserial.println(F(""));
      BTserial.println();
      BTserial.println();
      BTserial.println();
    }
    if(a==300)
      a=0;
  }



  //s= Drive_Count();
  b = BTserial.read(); // 스마트폰에서 블루투스 모듈로 들어오는 신호를 a에 저장
  if(dp == 0){
    //BTserial.print("car control_num : ");
    //BTserial.println(s);
    BTserial.print("car bt_num : ");
    BTserial.println(b);
    //Serial.print("car control_num : ");
    //Serial.println(s);
    Serial.print("car bt_num : ");
    Serial.println(b);
  }
  switch (b){
  case 0: // 0이면 정지
    digitalWrite(LM,HIGH); // 왼쪽 모터 전후진 정보 핀
    analogWrite(LS,LOW);
    digitalWrite(RM,HIGH); // 오른쪽 모터 전후진 정보 핀
    analogWrite(RS,HIGH);
    delay(20);
    break;
  case 4: // 4면 제자리 좌회전
    digitalWrite(LM,HIGH); // 왼쪽 모터 전후진 정보 핀
    analogWrite(LS,pwm);
    digitalWrite(RM,HIGH); // 오른쪽 모터 전후진 정보 핀
    analogWrite(RS,pwm);
    delay(20);
    break;
  case 8: // 8이면 제자리 우회전
    digitalWrite(LM,LOW); // 왼쪽 모터 전후진 정보 핀
    analogWrite(LS,pwm);
    digitalWrite(RM,LOW); // 오른쪽 모터 전후진 정보 핀
    analogWrite(RS,pwm);
    delay(20);
    break;
  case 16: // 16이면 전진
    digitalWrite(LM,LOW); // 왼쪽 모터 전후진 정보 핀
    analogWrite(LS,pwm);
    digitalWrite(RM,HIGH); // 오른쪽 모터 전후진 정보 핀
    analogWrite(RS,pwm);
    delay(20);
    break;
  case 20: // 20이면 좌회전
    digitalWrite(LM,LOW); // 왼쪽 모터 전후진 정보 핀
    analogWrite(LS,pwm-turn);
    digitalWrite(RM,HIGH); // 오른쪽 모터 전후진 정보 핀
    analogWrite(RS,pwm+turn);
    delay(20);
    break;
  case 24: // 24이면 우회전
    digitalWrite(LM,LOW); // 왼쪽 모터 전후진 정보 핀
    analogWrite(LS,pwm+turn);
    digitalWrite(RM,HIGH); // 오른쪽 모터 전후진 정보 핀
    analogWrite(RS,pwm-turn);
    delay(20);
    break;
  case 32: // 32이면 후진
    digitalWrite(LM,HIGH); // 왼쪽 모터 전후진 정보 핀
    analogWrite(LS,pwm);
    digitalWrite(RM,LOW); // 오른쪽 모터 전후진 정보 핀
    analogWrite(RS,pwm);
    delay(20);
    break;
  case 36: // 36이면 좌후진
    digitalWrite(LM,HIGH); // 왼쪽 모터 전후진 정보 핀
    analogWrite(LS,pwm-turn);
    digitalWrite(RM,LOW); // 오른쪽 모터 전후진 정보 핀
    analogWrite(RS,pwm+turn);
    delay(20);
    break;
  case 40: // 40이면 우후진
    digitalWrite(LM,HIGH); // 왼쪽 모터 전후진 정보 핀
    analogWrite(LS,pwm+turn);
    digitalWrite(RM,LOW); // 오른쪽 모터 전후진 정보 핀
    analogWrite(RS,pwm-turn);
    delay(20);
    break;
  }
  a++;
  delay(10); //fresh time
}



int MPU6050_read(int start, uint8_t *buffer, int size)
{
  int i, n, error;
  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  n = Wire.write(start);
  if (n != 1)
    return (-10);
  n = Wire.endTransmission(false); // hold the I2C-bus
  if (n != 0)
    return (n);
  // Third parameter is true: relase I2C-bus after data is read.
  Wire.requestFrom(MPU6050_I2C_ADDRESS, size, true);
  i = 0;
  while(Wire.available() && i<size)
  {
    buffer[i++]=Wire.read();
  }
  if ( i != size)
    return (-11);
  return (0); // return : no error
}
int MPU6050_write(int start, const uint8_t *pData, int size)
{
  int n, error;
  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  n = Wire.write(start); // write the start address
  if (n != 1)
    return (-20);
  n = Wire.write(pData, size); // write data bytes
  if (n != size)
    return (-21);
  error = Wire.endTransmission(true); // release the I2C-bus
  if (error != 0)
    return (error);
  return (0); // return : no error
}
int MPU6050_write_reg(int reg, uint8_t data)
{
  int error;
  error = MPU6050_write(reg, &data, 1);
  return (error);
}
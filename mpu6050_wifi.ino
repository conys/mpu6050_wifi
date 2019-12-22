#include <Wire.h>
#include <WiFi.h>
#include <WiFiUDP.h>

// 接続先のSSIDとパスワード
const char ssid[] = "******"; //アクセスポイント名
const char pass[] = "******"; //パスワード

static WiFiUDP wifiUdp; 
static const char *kRemoteIpadr = "**.**.**.**"; //送信先のIP
static const int kRmoteUdpPort = 9000; //送信先のポート

#define MPU6050_ADDR         0x68 // MPU-6050 device address
#define MPU6050_SMPLRT_DIV   0x19 // MPU-6050 register address
#define MPU6050_CONFIG       0x1a
#define MPU6050_GYRO_CONFIG  0x1b
#define MPU6050_ACCEL_CONFIG 0x1c
#define MPU6050_WHO_AM_I     0x75
#define MPU6050_PWR_MGMT_1   0x6b

double offsetX = 0, offsetY = 0, offsetZ = 0;
double gyro_angle_x = 0, gyro_angle_y = 0, gyro_angle_z = 0;
float angleX, angleY, angleZ;
float interval, preInterval;
float acc_x, acc_y, acc_z, acc_angle_x, acc_angle_y;
float gx, gy, gz, dpsX, dpsY, dpsZ;
int16_t raw_acc_x, raw_acc_y, raw_acc_z;

unsigned long nowtime, pretime, nowtime_u;

void culcAccelaration();

//I2c書き込み
void writeMPU6050(byte reg, byte data) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

//i2C読み込み
byte readMPU6050(byte reg) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.endTransmission(true);
  Wire.requestFrom(MPU6050_ADDR, 1/*length*/); 
  byte data =  Wire.read();
  return data;
}

static void WiFi_setup()
{
  static const int kLocalPort = 7000;  //自身のポート

  WiFi.begin(ssid, pass);
  while( WiFi.status() != WL_CONNECTED) {
    delay(500);  
  }  
  wifiUdp.begin(kLocalPort);
}

void setup() {

  Wire.begin(26, 25);
  Wire.setClock(400000); // I2C clock speed set to 400kbit/s (High Speed Mode)
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  
  Serial.begin(9600);
  delay(100);
  
  //正常に接続されているかの確認
  if (readMPU6050(MPU6050_WHO_AM_I) != 0x68) {
    Serial.println("\nWHO_AM_I error.");
    while (true) ;
  }

  //設定を書き込む
  writeMPU6050(MPU6050_SMPLRT_DIV, 0x07);   // sample rate: 8kHz/(7+1) = 8kHz
  writeMPU6050(MPU6050_CONFIG, 0x00);       // disable DLPF, gyro output rate = 8kHz
  writeMPU6050(MPU6050_GYRO_CONFIG, 0x08);  // gyro range: ±500dps
  writeMPU6050(MPU6050_ACCEL_CONFIG, 0x00); // accel range: ±2g
  writeMPU6050(MPU6050_PWR_MGMT_1, 0x01);   // disable sleep mode, PLL with X gyro  

  // Connect WiFi
  WiFi_setup();
}

void loop() {

  nowtime = millis();
  nowtime_u = micros();
  calcAccelaration();

  char buf[50];
  //sprintf(buf, "accX : %d    accY : %d    accZ : %d    interval : %d %d", raw_acc_x, raw_acc_y, raw_acc_z, nowtime - pretime, nowtime);
  sprintf(buf, "%d, %d, %d, %d, %d", raw_acc_x, raw_acc_y, raw_acc_z, nowtime, nowtime_u);
  pretime = nowtime;
  //Serial.println(buf);

  wifiUdp.beginPacket(kRemoteIpadr, kRmoteUdpPort);
  wifiUdp.print(buf);
  wifiUdp.endPacket(); 
  
  while(millis() < nowtime + 1){
  } // 1us間隔を作成
  
}

//加速度、ジャイロから角度を計算
void calcAccelaration(){

  //int16_t raw_acc_x, raw_acc_y, raw_acc_z, raw_t ;
  
  //レジスタアドレス0x3Bから、計6バイト分のデータを出力するようMPU6050へ指示
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 6, true);

  
  //出力されたデータを読み込み、ビットシフト演算
  raw_acc_x = Wire.read() << 8 | Wire.read();
  raw_acc_y = Wire.read() << 8 | Wire.read();
  raw_acc_z = Wire.read() << 8 | Wire.read();
  //raw_t = Wire.read() << 8 | Wire.read();
  /*
  //単位Gへ変換
  acc_x = ((float)raw_acc_x) / 16384.0;
  acc_y = ((float)raw_acc_y) / 16384.0;
  acc_z = ((float)raw_acc_z) / 16384.0;
  */
}

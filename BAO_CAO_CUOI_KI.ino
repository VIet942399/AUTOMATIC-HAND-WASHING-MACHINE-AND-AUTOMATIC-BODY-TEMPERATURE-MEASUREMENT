#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <Adafruit_MLX90614.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);


Adafruit_MLX90614 mlx = Adafruit_MLX90614();

#define chanphat 24 //trig
#define chanthu 22
#define Kchn 26
#define coi 28

QueueHandle_t queue_1;
struct Covid {
  float ss1; // Cảm biến nhiệt độ, ĐỘ C
  float ss2; // Cảm biến nhiệt độ, ĐỘ F
  int   ss3;   // Cảm biến khoảng cách hồng ngoại
  float thoigian;
  float khoangcach;
};
void TaskCambien(void *pvParameter);
void TaskNhietDoChoPhep(void *pvParameter);
void TaskNhietDoCao(void *pvParameter);
void TaskRuaTay(void *pvParameter);
void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(chanphat, OUTPUT);
  pinMode(chanthu, INPUT);
  pinMode(30, OUTPUT); // den vang
  pinMode(32, OUTPUT); // den xanh
  pinMode(34, OUTPUT); // den do
  pinMode(36, OUTPUT); // motor
  pinMode(coi, OUTPUT);
  pinMode(Kchn, INPUT);
  mlx.begin();
  lcd.init();
  queue_1 = xQueueCreate(10, sizeof(struct Covid));
  if (queue_1 == NULL) {
    Serial.println("Queue can not be created");
  }
  else
  {
    xTaskCreate(TaskCambien, "CamBien", 128, NULL, 1, NULL);
    xTaskCreate(TaskNhietDoChoPhep, "NhietDoChoPhep", 128, NULL, 2, NULL);
    xTaskCreate(TaskRuaTay, "Rua Tay", 128, NULL, 2, NULL);
    xTaskCreate(TaskNhietDoCao, "NhietDoCao", 128, NULL, 2, NULL);
  }
}
void loop() {
}

void TaskCambien(void * pvParameters) {
  struct  Covid a;
  int xStatus;
  for ( ; ; ) {
    digitalWrite(chanphat, HIGH);
    delay(50);
    digitalWrite(chanphat, LOW);
    a.thoigian = pulseIn(chanthu, HIGH);
    a.khoangcach = 0.0344 * (a.thoigian / 2);
    a.ss1 = mlx.readObjectTempC();
    a.ss2 = mlx.readObjectTempF();
    a.ss3 = digitalRead(Kchn);
    xStatus = xQueueSend(queue_1, &a, 1000 / portTICK_PERIOD_MS);
    if (xStatus != pdPASS)
    {
      Serial.print("dont send");
    }
  }
}

void TaskNhietDoChoPhep(void * pvParameters) {
  struct Covid b;
  int xStatus;
  for (;;)
  {
    xStatus = xQueueReceive(queue_1, &b, 1000 / portTICK_PERIOD_MS);
    if (xStatus == pdPASS)
    {
      Serial.println(b.ss1);
      if ( b.ss3 == 0) {
        lcd.clear();
        lcd.backlight();
        digitalWrite(30, HIGH);
        if (b.ss1 < 37)
        {
          digitalWrite(32, HIGH);
          lcd.clear();
          lcd.backlight();
          digitalWrite(coi, HIGH);
          delay(100);
          digitalWrite(coi, LOW);
          lcd.print("TEMP:");
          lcd.setCursor(1, 0);
          lcd.setCursor(5, 0);
          lcd.print(b.ss1);
          lcd.print("C");
          lcd.setCursor(0, 1);
          lcd.print("TEMP:");
          lcd.setCursor(5, 1);
          lcd.print(b.ss2);
          lcd.print("F");
          delay(1000);
          lcd.clear();
          lcd.noBacklight();
        }
        digitalWrite(30, LOW);
        digitalWrite(32, LOW);
      }
    }
    delay(50);
  }
}
void TaskNhietDoCao(void * pvParameters) {
  struct Covid d;
  int xStatus;
  for (;;)
  {
    xStatus = xQueueReceive(queue_1, &d, 1000 / portTICK_PERIOD_MS);
    if (xStatus == pdPASS)
    {
      Serial.println(d.ss1);
      if ( d.ss3 == 0) {
        lcd.clear();
        lcd.backlight();
        digitalWrite(30, HIGH);
        if (d.ss1 >= 37)
        {
          digitalWrite(34, HIGH);
          lcd.setCursor(0, 0);
          lcd.print("NHIET DO QUA MUC");
          lcd.setCursor(4, 1);
          lcd.print("CHO PHEP");
          digitalWrite(coi, HIGH);
          delay(300);
          digitalWrite(coi, LOW);
          digitalWrite(coi, HIGH);
          delay(300);
          digitalWrite(coi, LOW);
          digitalWrite(coi, HIGH);
          delay(300);
          digitalWrite(coi, LOW);
          lcd.clear();
          lcd.print("TEMP:");
          lcd.setCursor(1, 0);
          lcd.setCursor(5, 0);
          lcd.print(d.ss1);
          lcd.print("C");
          lcd.setCursor(0, 1);
          lcd.print("TEMP:");
          lcd.setCursor(5, 1);
          lcd.print(d.ss2);
          lcd.print("F");
          delay(1000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("BAN NEN KHAI BAO");
          lcd.setCursor(6, 1);
          lcd.print("Y TE");
          delay(500);
          lcd.clear();
          lcd.noBacklight();
        }
        digitalWrite(30, LOW);
        digitalWrite(34, LOW);
      }
      delay(50);
    }
  }
}

void TaskRuaTay(void * pvParameters)
{
  struct Covid b;
  int xStatus;
  for (;;)
  {
    xStatus = xQueueReceive(queue_1, &b, 1000 / portTICK_PERIOD_MS);
    if (xStatus == pdPASS)
    {
      Serial.println(b.khoangcach);
      if (b.khoangcach < 8  ) {
        digitalWrite(coi, HIGH);
        delay(50);
        digitalWrite(coi, LOW);
        lcd.clear();
        lcd.backlight();
        lcd.setCursor(0, 0);
        lcd.print("DANG RUA TAY.....");
        digitalWrite(36, HIGH);
        int red = digitalRead(36);
        Serial.println(red);
        delay(1500);
        digitalWrite(36, LOW);
        lcd.clear();
        lcd.noBacklight();
      }
    }
    digitalWrite(36, LOW);
    delay(50);
  }
}

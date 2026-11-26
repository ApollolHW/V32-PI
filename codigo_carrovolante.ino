#include <WiFi.h>
#include <PubSubClient.h>   // biblioteca "PubSubClient" (Nick O'Leary)
#include <ArduinoJson.h>    // biblioteca "ArduinoJson" (versão 7)

// ================== CONFIGURE AQUI ==================
const char* WIFI_SSID  = "LITEC";
const char* WIFI_SENHA = "liteclitec";

// IP do computador que roda o broker MQTT
const char* BROKER_IP    = "192.168.168.101";
const int   BROKER_PORTA = 1883;
const char* TOPICO       = "dados_volante";

const int CAR_ID = 1;      // só obedece mensagens com este car_id
const int MARCHA_FIXA = 2; // 1 = baixa, 2 = média, 3 = alta (o volante não manda marcha)
// ====================================================

// ---------- Pinos do L298N ----------
const int IN1 = 14;
const int IN2 = 26;
const int ENA = 27;

const int IN3 = 32;
const int IN4 = 12;
const int ENB = 33;

// ---------- PWM ----------
const int PWM_RES          = 8;      // 0 a 255
const int PWM_FREQ_TRACAO  = 1000;
const int PWM_FREQ_DIRECAO = 10000;
const int PWM_MIN          = 110;    // força mínima para a tração partir
const int ZONA_MORTA       = 10;     // em escala -100..100 (= 0,10 em -1..1)
const int CANAL_A          = 0;
const int CANAL_B          = 2;

// ---------- Direção ----------
const int  PWM_DIRECAO_MIN = 120;    // força mínima para a direção sair do lugar
const int  PWM_DIRECAO     = 200;    // força máxima da direção
const bool INVERTER_DIRECAO = false; // troque para true se virar para o lado errado
const unsigned long TEMPO_MAX_DIRECAO = 5000; // corta a direção após 5 s segurando (0 = desativa)
int ladoAtual = 0;
unsigned long inicioLado = 0;

// ---------- Marchas ----------
const int PWM_MAX_MARCHA[3] = { 150, 200, 255 };
int marcha = MARCHA_FIXA;

// ---------- Segurança ----------
unsigned long ultimoComando = 0;
const unsigned long TEMPO_FAILSAFE = 1000; // para o carro se ficar 1 s sem mensagem

WiFiClient rede;
PubSubClient mqtt(rede);

// ---------- PWM compatível com ESP32 core 2.x e 3.x ----------
void pwmIniciar(int pino, int canal, int freq) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(pino, freq, PWM_RES);
#else
  ledcSetup(canal, freq, PWM_RES);
  ledcAttachPin(pino, canal);
#endif
}

void pwmEscrever(int pino, int canal, int valor) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(pino, valor);
#else
  ledcWrite(canal, valor);
#endif
}

// ---------- Tração (motor traseiro) ----------
// vel vai de -100 (ré total) a +100 (frente total)
void tracao(int vel) {
  vel = constrain(vel, -100, 100);
  if (abs(vel) < ZONA_MORTA) vel = 0;

  int pwmMax = PWM_MAX_MARCHA[marcha - 1];
  int pwm = (vel == 0) ? 0 : map(abs(vel), 1, 100, PWM_MIN, pwmMax);

  digitalWrite(IN1, vel > 0 ? HIGH : LOW);
  digitalWrite(IN2, vel < 0 ? HIGH : LOW);
  pwmEscrever(ENA, CANAL_A, pwm);
}

// ---------- Direção (motor dianteiro, a mola volta ao centro) ----------
// x vai de -100 (esquerda total) a +100 (direita total)
void direcao(int x) {
  x = constrain(x, -100, 100);
  if (abs(x) < ZONA_MORTA) x = 0;

  int lado = (x > 0) - (x < 0);
  if (lado != ladoAtual) {
    ladoAtual = lado;
    inicioLado = millis();
  }
  // proteção: não deixa o motor forçando contra o limite por muito tempo
  if (lado != 0 && TEMPO_MAX_DIRECAO > 0 && millis() - inicioLado > TEMPO_MAX_DIRECAO) {
    x = 0;
  }
  if (INVERTER_DIRECAO) x = -x;

  if (x == 0) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    pwmEscrever(ENB, CANAL_B, 0);
    return;
  }
  int pwm = map(abs(x), ZONA_MORTA, 100, PWM_DIRECAO_MIN, PWM_DIRECAO);
  digitalWrite(IN3, x < 0 ? HIGH : LOW);
  digitalWrite(IN4, x < 0 ? LOW : HIGH);
  pwmEscrever(ENB, CANAL_B, pwm);
}

void parar() {
  tracao(0);
  direcao(0);
}

// ---------- Mensagem MQTT: {"car_id":1,"volante":0.004,"acelerador":-0.498} ----------
void aoReceberMQTT(char* topico, byte* payload, unsigned int tamanho) {
  JsonDocument doc;
  if (deserializeJson(doc, payload, tamanho)) return;   // JSON inválido

  if (doc["car_id"].is<int>() && doc["car_id"].as<int>() != CAR_ID) return; // é de outro carro

  float volante    = constrain(doc["volante"]    | 0.0f, -1.0f, 1.0f);
  float acelerador = constrain(doc["acelerador"] | 0.0f, -1.0f, 1.0f);

  // conversão: -1..1 do volante/pedal  ->  -100..100 que o código do carro usa
  int x = lroundf(volante * 100);       // direção: negativo = esquerda
  int y = lroundf(acelerador * 100);    // andar: negativo = ré

  ultimoComando = millis();
  direcao(x);
  tracao(y);
}

// ---------- Conexões ----------
void conectarWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 10000) {
    delay(300);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(" ok, IP: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println(" falhou");
  }
}

unsigned long ultimaTentativaMQTT = 0;
void conectarMQTT() {
  if (mqtt.connected() || millis() - ultimaTentativaMQTT < 2000) return;
  ultimaTentativaMQTT = millis();

  String id = "carro-esp32-" + String(CAR_ID);
  Serial.print("Conectando ao broker... ");
  if (mqtt.connect(id.c_str())) {
    mqtt.subscribe(TOPICO);
    Serial.println("ok");
  } else {
    Serial.print("falhou, rc="); Serial.println(mqtt.state());
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pwmIniciar(ENA, CANAL_A, PWM_FREQ_TRACAO);
  pwmIniciar(ENB, CANAL_B, PWM_FREQ_DIRECAO);
  parar();

  WiFi.mode(WIFI_STA);
  conectarWiFi();

  mqtt.setServer(BROKER_IP, BROKER_PORTA);
  mqtt.setCallback(aoReceberMQTT);
}

void loop() {
  conectarWiFi();
  conectarMQTT();
  mqtt.loop();

  // segurança: sem mensagens (ou sem conexão), o carro para sozinho
  if (millis() - ultimoComando > TEMPO_FAILSAFE) {
    parar();
  }
}

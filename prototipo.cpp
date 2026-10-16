#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// ================== CONFIGURAÇÕES ==================
const char* WIFI_SSID = "Redmi Note 13";
const char* WIFI_PASS = "mcgarvey";

// ================== PINOS ==================
// ATENÇÃO: os pinos 34, 35, 36 e 39 do ESP32 só funcionam como ENTRADA.
// Por isso o IN2 e o ENB saíram do 35 e do 34.
#define PIN_ENA 33      // Habilita o motor direito (PWM de velocidade)
#define PIN_DIR_IN1 32  // Motor direito (IN1 e IN2 da ponte H)
#define PIN_DIR_IN2 25

#define PIN_ENB 26     // Habilita o motor esquerdo (PWM de velocidade)
#define PIN_ESQ_IN3 5  // Motor esquerdo (IN3 e IN4 da ponte H)
#define PIN_ESQ_IN4 18

#define PIN_LUZ 27  // LED / luz

#define PWM_FREQ 1000   // 1 kHz
#define PWM_BITS 8      // 0 a 255
#define VELOCIDADE 255  // 0 a 255

// ================== COMANDOS (1 letra) ==================
// F = frente | B = trás | L = esquerda | R = direita | S = parar
// A = acender luz | D = desligar luz

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// ================== MOTORES ==================
// sentido: 1 = frente, -1 = trás, 0 = parado
void motor(int in1, int in2, int sentido) {
  digitalWrite(in1, sentido == 1);
  digitalWrite(in2, sentido == -1);
}

void mover(int esquerda, int direita) {
  motor(PIN_ESQ_IN3, PIN_ESQ_IN4, esquerda);
  motor(PIN_DIR_IN1, PIN_DIR_IN2, direita);
}

void executar(char cmd) {
  switch (cmd) {
    case 'F': mover(1, -1); break;
    case 'B': mover(-1, 1); break;
    case 'L': mover(1, 1); break;  // gira para a esquerda
    case 'R': mover(-1, -1); break;  // gira para a direita
    case 'S': mover(0, 0); break;//alterar para outro comando
    case 'A': digitalWrite(PIN_LUZ, HIGH); break;
    case 'D': digitalWrite(PIN_LUZ, LOW); break;
  }
}

// ================== WEBSOCKET ==================
void onWsEvent(AsyncWebSocket*, AsyncWebSocketClient* client,
               AwsEventType tipo, void* arg, uint8_t* dados, size_t tam) {
  switch (tipo) {
    case WS_EVT_CONNECT:
      Serial.printf("Cliente #%u conectado\n", client->id());
      break;

    case WS_EVT_DISCONNECT:
      Serial.printf("Cliente #%u desconectado\n", client->id());
      executar('S');  // segurança: perdeu conexão, para o carro
      break;

    case WS_EVT_DATA:
      {
        auto* info = (AwsFrameInfo*)arg;
        bool mensagemCompleta = info->final && info->index == 0 && info->len == tam;
        if (mensagemCompleta && info->opcode == WS_TEXT && tam > 0) {
          executar((char)dados[0]);
        }
        break;
      }

    default:
      break;
  }
}

// ================== PÁGINA WEB ==================
const char PAGINA[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
  <title>Controle ESP32</title>
  <style>
    body { font-family: Arial; text-align: center; background: #111; color: #eee;
           user-select: none; -webkit-user-select: none; }
    .pad { display: grid; grid-template-columns: repeat(3, 90px);
           grid-template-rows: repeat(3, 90px); gap: 8px;
           justify-content: center; margin: 20px auto; }
    button { font-size: 26px; border: none; border-radius: 14px;
             background: #333; color: #fff; touch-action: none; }
    button:active { background: #0a84ff; }
    .luz button { width: 140px; height: 50px; font-size: 16px; margin: 0 6px; }
  </style>
</head>
<body>
  <h1>Controle do Carrinho</h1>

  <!-- Segure o botão para andar, solte para parar -->
  <div class="pad">
    <span></span>
    <button class="mover" data-cmd="F">▲</button>
    <span></span>

    <button class="mover" data-cmd="L">◀</button>
    <button class="mover" data-cmd="S">■</button>
    <button class="mover" data-cmd="R">▶</button>

    <span></span>
    <button class="mover" data-cmd="B">▼</button>
    <span></span>
  </div>

  <div class="luz">
    <button onclick="enviar('A')"> Luz ON</button>
    <button onclick="enviar('D')"> Luz OFF</button>
  </div>

  <script>
    let ws;

    function conectar() {
      ws = new WebSocket('ws://' + location.host + '/ws');
      ws.onclose = () => setTimeout(conectar, 1000);  // reconecta sozinho
    }

    function enviar(cmd) {
      if (ws.readyState === WebSocket.OPEN) ws.send(cmd);
    }

    document.querySelectorAll('.mover').forEach(botao => {
      botao.addEventListener('pointerdown', () => enviar(botao.dataset.cmd));
      ['pointerup', 'pointerleave', 'pointercancel'].forEach(evento =>
        botao.addEventListener(evento, () => enviar('S')));
    });

    conectar();
  </script>
</body>
</html>
)rawliteral";

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);

  // Pinos
  pinMode(PIN_DIR_IN1, OUTPUT);
  pinMode(PIN_DIR_IN2, OUTPUT);
  pinMode(PIN_ESQ_IN3, OUTPUT);
  pinMode(PIN_ESQ_IN4, OUTPUT);
  pinMode(PIN_LUZ, OUTPUT);

  // PWM de velocidade (Arduino-ESP32 Core 3.x)
  ledcAttach(PIN_ENA, PWM_FREQ, PWM_BITS);
  ledcAttach(PIN_ENB, PWM_FREQ, PWM_BITS);
  ledcWrite(PIN_ENA, VELOCIDADE);
  ledcWrite(PIN_ENB, VELOCIDADE);

  executar('S');  // começa parado
  executar('D');  // começa com a luz apagada

  // Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  WiFi.setDNS(IPAddress(8, 8, 8, 8), IPAddress(8, 8, 4, 4));
  Serial.print("\nConectado! IP: ");
  Serial.println(WiFi.localIP());

  // Servidor
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
    req->send_P(200, "text/html", PAGINA);
  });
  server.begin();
  Serial.println("Servidor iniciado!");
}

// ================== LOOP ==================
void loop() {
  ws.cleanupClients();
}

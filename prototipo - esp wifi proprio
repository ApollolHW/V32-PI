#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

// ---------- Rede Wi-Fi criada pelo ESP32 ----------
const char* ssid  = "CarroESP32";
const char* senha = "12345678";   // mínimo 8 caracteres

// ---------- Pinos do L298N ----------
const int ENA = 33, IN1 = 32, IN2 = 25;   // motor esquerdo
const int ENB = 26, IN3 = 5,  IN4 = 18;   // motor direito
const int PINO_LED = 27;

// ---------- PWM ----------
const int PWM_FREQ   = 5000;
const int PWM_RES    = 8;      // 0 a 255
const int PWM_MIN    = 110;    // força mínima para o motor partir
const int ZONA_MORTA = 10;
const int CANAL_A    = 0;
const int CANAL_B    = 1;

// ---------- Marchas: PWM máximo de cada uma ----------
//                                   baixa média alta
const int PWM_MAX_MARCHA[3] = {       150,  200,  255 };
int marcha = 2;                       // 1 = baixa, 2 = média, 3 = alta

WebServer servidor(80);
WebSocketsServer ws(81);

unsigned long ultimoComando = 0;
const unsigned long TEMPO_FAILSAFE = 500;  // para o carro se ficar 500 ms sem comando
bool ledLigado = false;

// ---------- Página HTML (tela deitada) ----------
const char PAGINA[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no, viewport-fit=cover">
<title>Carro</title>
<style>
*{box-sizing:border-box}
html,body{margin:0;height:100%;background:#111;color:#fff;font-family:sans-serif;overflow:hidden;
  touch-action:none;user-select:none;-webkit-user-select:none;-webkit-touch-callout:none}

/* Tela deitada: com o celular em pé, o app gira sozinho 90 graus */
#app{position:fixed;top:0;left:0;width:100vw;height:100vh;width:100dvw;height:100dvh;
  display:flex;align-items:center;justify-content:space-between;padding:3vmin}
@media (orientation:portrait){
  #app{top:0;left:100vw;left:100dvw;width:100vh;height:100vw;width:100dvh;height:100dvw;
    transform-origin:top left;transform:rotate(90deg)}
}

.zona{display:flex;gap:3vmin;align-items:center;justify-content:center}
#zonaEsq{flex-direction:row}      /* ← → lado a lado, polegar esquerdo */
#zonaDir{flex-direction:column}   /* ↑ ↓ empilhados, polegar direito */

.seta{width:30vmin;height:30vmin;font-size:11vmin;border:0;border-radius:5vmin;
  background:#333;color:#fff;touch-action:none;padding:0}
.seta.ativo{background:#e33}

#centro{flex:1;display:flex;flex-direction:column;align-items:center;justify-content:center;
  gap:2.5vmin;padding:0 3vmin;min-width:0}
#status{font-size:3.8vmin}
#gnome{font-size:6vmin;font-weight:bold}
#marcha{width:100%;height:9vmin;accent-color:#e33}
#rotulos{width:100%;display:flex;justify-content:space-between;font-size:3.5vmin;color:#aaa}
.btn{font-size:4.5vmin;padding:2vmin 5vmin;border:0;border-radius:2vmin;background:#2a7;color:#fff}
</style></head><body>
<div id="app">
  <div class="zona" id="zonaEsq">
    <button class="seta" id="esq">&#9664;</button>
    <button class="seta" id="dir">&#9654;</button>
  </div>

  <div id="centro">
    <div id="status">Conectando...</div>
    <div>Marcha: <span id="gnome">MÉDIA</span></div>
    <input type="range" id="marcha" min="1" max="3" step="1" value="2">
    <div id="rotulos"><span>BAIXA</span><span>MÉDIA</span><span>ALTA</span></div>
    <button class="btn" id="led" onclick="enviarLed()">LED</button>
    <button class="btn" style="background:#444" onclick="telaCheia()">Tela cheia</button>
  </div>

  <div class="zona" id="zonaDir">
    <button class="seta" id="cima">&#9650;</button>
    <button class="seta" id="baixo">&#9660;</button>
  </div>
</div>

<script>
let ws;
const st    = document.getElementById('status');
const slider = document.getElementById('marcha');
const gnome = document.getElementById('gnome');
const NOMES = ['BAIXA', 'MÉDIA', 'ALTA'];
const VEL  = 100;   // frente/ré
const GIRO = 70;    // intensidade da curva
const premido = { cima:false, baixo:false, esq:false, dir:false };

function conectar() {
  ws = new WebSocket('ws://' + location.hostname + ':81/');
  ws.onopen  = () => st.textContent = 'Conectado';
  ws.onclose = () => { st.textContent = 'Reconectando...'; setTimeout(conectar, 1000); };
}
conectar();

slider.addEventListener('input', () => { gnome.textContent = NOMES[slider.value - 1]; });

// cada botão tem o seu próprio estado, então dá pra apertar vários ao mesmo tempo
for (const id in premido) {
  const b = document.getElementById(id);
  const ligar = (v) => (e) => { e.preventDefault(); premido[id] = v; b.classList.toggle('ativo', v); };
  b.addEventListener('pointerdown', ligar(true));
  b.addEventListener('pointerup', ligar(false));
  b.addEventListener('pointercancel', ligar(false));
  b.addEventListener('pointerleave', ligar(false));
  b.addEventListener('contextmenu', e => e.preventDefault());
}

// segurança: se a página perder o foco, solta todos os botões
function soltarTudo() {
  for (const id in premido) { premido[id] = false; document.getElementById(id).classList.remove('ativo'); }
}
window.addEventListener('blur', soltarTudo);
document.addEventListener('visibilitychange', () => { if (document.hidden) soltarTudo(); });

// manda direção + marcha 10x por segundo
setInterval(() => {
  if (!ws || ws.readyState !== 1) return;
  const y = (premido.cima ? VEL : 0) - (premido.baixo ? VEL : 0);
  const x = (premido.dir ? GIRO : 0) - (premido.esq ? GIRO : 0);
  ws.send('M,' + x + ',' + y + ',' + slider.value);
}, 100);

function enviarLed() { if (ws && ws.readyState === 1) ws.send('L'); }

// no Android, tela cheia permite travar a rotação em modo deitado
async function telaCheia() {
  try { await document.documentElement.requestFullscreen(); } catch (e) {}
  try { await screen.orientation.lock('landscape'); } catch (e) {}
}
</script></body></html>
)rawliteral";

// ---------- PWM compatível com ESP32 core 2.x e 3.x ----------
void pwmIniciar(int pino, int canal) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(pino, PWM_FREQ, PWM_RES);
#else
  ledcSetup(canal, PWM_FREQ, PWM_RES);
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

// ---------- Controle dos motores ----------
// vel vai de -100 (ré total) a +100 (frente total); a marcha limita o PWM máximo
void moverMotor(int pinIn1, int pinIn2, int pinEn, int canal, int vel) {
  vel = constrain(vel, -100, 100);
  if (abs(vel) < ZONA_MORTA) vel = 0;

  int pwmMax = PWM_MAX_MARCHA[marcha - 1];
  int pwm = (vel == 0) ? 0 : map(abs(vel), 1, 100, PWM_MIN, pwmMax);

  if (vel > 0) {
    digitalWrite(pinIn1, HIGH);
    digitalWrite(pinIn2, LOW);
  } else if (vel < 0) {
    digitalWrite(pinIn1, LOW);
    digitalWrite(pinIn2, HIGH);
  } else {
    digitalWrite(pinIn1, LOW);
    digitalWrite(pinIn2, LOW);
  }
  pwmEscrever(pinEn, canal, pwm);
}

void parar() {
  moverMotor(IN1, IN2, ENA, CANAL_A, 0);
  moverMotor(IN3, IN4, ENB, CANAL_B, 0);
}

void dirigir(int x, int y) {
  int esq = y - x;   // invertido: curva para o lado certo
  int dir = y + x;
  moverMotor(IN1, IN2, ENA, CANAL_A, esq);
  moverMotor(IN3, IN4, ENB, CANAL_B, dir);
}


// ---------- Mensagens recebidas pelo WebSocket ----------
void eventoWS(uint8_t num, WStype_t tipo, uint8_t* payload, size_t tamanho) {
  switch (tipo) {
    case WStype_DISCONNECTED:
      parar();
      break;

    case WStype_TEXT: {
      String msg = String((char*)payload);
      ultimoComando = millis();

      if (msg.startsWith("M,")) {          // formato: M,x,y,marcha
        int p1 = msg.indexOf(',', 2);
        int p2 = msg.indexOf(',', p1 + 1);
        if (p1 < 0 || p2 < 0) break;       // mensagem incompleta
        int x = msg.substring(2, p1).toInt();
        int y = msg.substring(p1 + 1, p2).toInt();
        marcha = constrain(msg.substring(p2 + 1).toInt(), 1, 3);
        dirigir(x, y);
      } else if (msg == "L") {             // alterna o LED
        ledLigado = !ledLigado;
        digitalWrite(PINO_LED, ledLigado ? HIGH : LOW);
      }
      break;
    }

    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(PINO_LED, OUTPUT);
  pwmIniciar(ENA, CANAL_A);
  pwmIniciar(ENB, CANAL_B);
  parar();

  WiFi.softAP(ssid, senha);
  Serial.print("Rede: "); Serial.println(ssid);
  Serial.print("Acesse: http://"); Serial.println(WiFi.softAPIP());   // normalmente 192.168.4.1

  servidor.on("/", []() { servidor.send_P(200, "text/html; charset=utf-8", PAGINA); });
  servidor.begin();

  ws.begin();
  ws.onEvent(eventoWS);
}

void loop() {
  servidor.handleClient();
  ws.loop();

  // segurança: se o sinal sumir, o carro para sozinho
  if (millis() - ultimoComando > TEMPO_FAILSAFE) {
    parar();
  }
}

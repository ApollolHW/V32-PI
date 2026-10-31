#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

// ---------- Rede Wi-Fi criada pelo ESP32 ----------
const char* ssid  = "CarroESP32";
const char* senha = "12345678";   // mínimo 8 caracteres

// ---------- Pinos do L298N ----------

const int IN1 = 14;
const int IN2 = 26;
const int ENA = 27;

const int IN3 = 32;
const int IN4 = 12;
const int ENB = 33;
const int PINO_LED = 29;
// ---------- PWM ----------
const int PWM_RES          = 8;      // 0 a 255
const int PWM_FREQ_TRACAO  = 1000;   // motor traseiro (menos barulho e aquecimento no L298N)
const int PWM_FREQ_DIRECAO = 10000;  // motor da direção (inaudível). Se ficar fraco: 10000 ou 5000
const int PWM_MIN          = 110;    // força mínima para a tração partir
const int ZONA_MORTA       = 10;
const int CANAL_A          = 0;      // canais em timers diferentes (0 e 2),
const int CANAL_B          = 2;      // senão a frequência de um muda a do outro (core 2.x)

// ---------- Direção ----------
const int  PWM_DIRECAO        = 200;   // força da direção (com 20 kHz o 180 não chegou ao limite)
const bool INVERTER_DIRECAO   = false; // troque para true se virar para o lado errado
const unsigned long TEMPO_MAX_DIRECAO = 5000; // corta a direção após 5 s segurando (0 = desativa)
int ladoAtual = 0;
unsigned long inicioLado = 0;

// ---------- Marchas: PWM máximo da tração em cada uma ----------
//                                   baixa média alta
const int PWM_MAX_MARCHA[3] = {       150,  200,  255 };
int marcha = 2;                       // 1 = baixa, 2 = média, 3 = alta

WebServer servidor(80);
WebSocketsServer ws(81);

unsigned long ultimoComando = 0;
const unsigned long TEMPO_FAILSAFE = 500;  // para o carro se ficar 500 ms sem comando
bool ledLigado = false;

// ---------- Página HTML (tela deitada) ----------
// O vídeo vem direto da ESP32-CAM (IP fixo 192.168.4.50, porta 81).
// O IP precisa ser o mesmo configurado no código da câmera.
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

/* Vídeo da câmera ocupa o fundo; os controles ficam por cima */
#cam{position:absolute;top:0;left:0;width:100%;height:100%;object-fit:cover;
  z-index:0;display:none;background:#000;pointer-events:none}
#app > *:not(#cam){position:relative;z-index:1}

.zona{display:flex;gap:3vmin;align-items:center;justify-content:center}
#zonaEsq{flex-direction:row}      /* ← → lado a lado, polegar esquerdo */
#zonaDir{flex-direction:column}   /* ↑ ↓ empilhados, polegar direito */

.seta{width:30vmin;height:30vmin;font-size:11vmin;border:0;border-radius:5vmin;
  background:rgba(40,40,40,.6);color:#fff;touch-action:none;padding:0}
.seta.ativo{background:rgba(238,51,51,.85)}

#centro{flex:1;display:flex;flex-direction:column;align-items:center;justify-content:center;
  gap:2.5vmin;padding:2vmin 3vmin;margin:0 3vmin;min-width:0;
  background:rgba(0,0,0,.45);border-radius:3vmin;text-shadow:0 1px 3px #000}
#status{font-size:3.8vmin}
#camst{font-size:3.2vmin;color:#ccc}
#gnome{font-size:6vmin;font-weight:bold}
#marcha{width:100%;height:9vmin;accent-color:#e33}
#rotulos{width:100%;display:flex;justify-content:space-between;font-size:3.5vmin;color:#ccc}
.btn{font-size:4.5vmin;padding:2vmin 5vmin;border:0;border-radius:2vmin;background:#2a7;color:#fff}
</style></head><body>
<div id="app">
  <img id="cam" alt="">

  <div class="zona" id="zonaEsq">
    <button class="seta" id="esq">&#9664;</button>
    <button class="seta" id="dir">&#9654;</button>
  </div>

  <div id="centro">
    <div id="status">Conectando...</div>
    <div id="camst">Câmera: procurando...</div>
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
const GIRO = 70;    // intensidade da curva (a direção só usa o sinal: esquerda, centro ou direita)
const premido = { cima:false, baixo:false, esq:false, dir:false };

// ---------- Câmera ----------
const CAM_URL = 'http://192.168.4.50:81/stream';
const cam   = document.getElementById('cam');
const camSt = document.getElementById('camst');

function carregarCam() { cam.src = CAM_URL + '?t=' + Date.now(); }
cam.onload  = () => { cam.style.display = 'block'; camSt.textContent = 'Câmera: ao vivo'; };
cam.onerror = () => {
  cam.style.display = 'none';
  camSt.textContent = 'Câmera: sem sinal';
  setTimeout(carregarCam, 2000);   // tenta de novo sozinho
};
carregarCam();

// ---------- Controle ----------
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

void tracao(int vel) {
  moverMotor(IN1, IN2, ENA, CANAL_A, vel);
}

// ---------- Direção (motor dianteiro, volta ao centro pela mola) ----------
// lado: -1 = esquerda, 0 = centro (motor desligado, a mola centraliza), 1 = direita
void direcao(int lado) {
  // controla há quanto tempo está segurando para o mesmo lado
  if (lado != ladoAtual) {
    ladoAtual = lado;
    inicioLado = millis();
  }
  // proteção: não deixa o motor forçando contra o limite por muito tempo
  if (lado != 0 && TEMPO_MAX_DIRECAO > 0 && millis() - inicioLado > TEMPO_MAX_DIRECAO) {
    lado = 0;
  }
  if (INVERTER_DIRECAO) lado = -lado;

  if (lado < 0) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    pwmEscrever(ENB, CANAL_B, PWM_DIRECAO);
  } else if (lado > 0) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    pwmEscrever(ENB, CANAL_B, PWM_DIRECAO);
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    pwmEscrever(ENB, CANAL_B, 0);
  }
}

void parar() {
  tracao(0);
  direcao(0);
}

// x: curva (positivo = direita), y: frente/ré
void dirigir(int x, int y) {
  tracao(y);

  int lado = 0;
  if (x > ZONA_MORTA)       lado = 1;
  else if (x < -ZONA_MORTA) lado = -1;
  direcao(lado);
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
  pwmIniciar(ENA, CANAL_A, PWM_FREQ_TRACAO);
  pwmIniciar(ENB, CANAL_B, PWM_FREQ_DIRECAO);
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

  // segurança: se o sinal sumir, o carro para sozinho (a direção volta ao centro)
  if (millis() - ultimoComando > TEMPO_FAILSAFE) {
    parar();
  }
}

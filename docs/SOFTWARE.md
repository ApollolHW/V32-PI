Documentação técnica do carro controlado por ESP32.

**Versão:** 1 · **Data:** 10/10/2026


## Índice

1. [Objetivo e requisitos](#1-objetivo-e-requisitos)
2. [Arquitetura geral](#2-arquitetura-geral)
3. [Código](#3-código-explicado)
4. [Riscos e pendências](#4-riscos-e-pendências)

---

## 1. Objetivo e requisitos

Construir um carro RC controlado por ESP32 para disputar uma corrida em uma pista com curvas.
| Requisito | Meta |
|---|---|
| Aceleração | Usar slider da web para controle |
| Direção | Não capotar nem derrapar em curvas |
| Vigilância | Baixa latência, sem perder sinal na pista |
| Confiabilidade | Consistencia nos testes do motor |
| Regras da prova | Conferir limite de tensão, peso, dimensões e tipo de direção **[DECIDIR]** |

## 2. Arquitetura geral

```mermaid
flowchart TD
    A([Liga o ESP32]) --> B["setup()"]
    B --> B1["Configura pinos dos motores e da luz"]
    B1 --> B2["Configura PWM em ENA e ENB"]
    B2 --> B3["executar('S') e executar('D')<br/>carro parado e luz apagada"]
    B3 --> B4["Conecta no WiFi"]
    B4 --> B5["Inicia servidor HTTP e WebSocket"]
    B5 --> C["loop()"]
    C --> C1["ws.cleanupClients()"]
    C1 --> C

    S["Celular abre o IP no navegador"] --> P["Servidor envia a página HTML"]
    P --> W["Página abre o WebSocket em /ws"]
    W --> E["onWsEvent()"]
```

| Modulos | Descrição | Situação |
|---|---|---|
| Configuração WIFI | SSID | Funcionando |
| Direção | Servo motor | A fazer |
| Vigilância | Transcrição da imagem da ESPCAM | A fazer |
| HTTP | Canal de comunicação dupla sempre aberto | Funcionando + recursos são necessários|

## 3. Código explicado

### 3.1 Funções

| Parâmetro | Descrição | Situação |
|---|---|---|
| Câmera | Transcrição sem delay | A fazer |
| Motores | Responsável pela rotação para cada comando | OK |
| Servo | Responsável pela direção do eixo móvel | A fazer |
| HTTP | Vai ser o meio de comunicação adotado | OK |
| Setup | Precisa definir os pinos do ESP32, Estabelecer a comunicação e o estado inicial do carro | Parcial |
| Loop | Retoma as funções posteriores | Parcial |

### 3.2 Câmera


### 3.3 Motores


### 3.3 Servo


### 3.5. HTTP

```mermaid
flowchart TD
    E["onWsEvent()"] --> T{"Tipo do evento"}
    T -->|"WS_EVT_CONNECT"| C1["Mostra cliente conectado no Serial"]
    T -->|"WS_EVT_DISCONNECT"| D1["executar('S')<br/>para o carro por segurança"]
    T -->|"WS_EVT_DATA"| M{"Mensagem de texto completa?"}
    M -->|"Sim"| X["executar(primeiro caractere)"]
    M -->|"Não"| I["Ignora"]
```
Na função onWsEvent, acontece a abertura da comunicação, dando os dados no Serial do ESP32 para ser possivel conectar o celular com o ESP32 e oque acontece em cada situação da conexão.
Logo em seguida, está o HTML da página responsável pela interface de controle, ela faz referencia a algumas funções da aréa mecânica. Está secção do projeto está muito sujeita a alterações, pois o controle final será com um volante e a direção será feita por servos e ainda vai precisar comportar a camêra.

### 3.6. Setup

O setup é responsável por definir os pinos, logicamente, mas além disso, ele estabelece o estado de repouso inicial do carro, e a conexão WiFi do carro, inicializa o serial

## 4. Riscos e pendências

| Risco | Impacto | Mitigação |
|---|---|---|
| Delay da imagem do ESPCAM | Motor queimado | Reduzir a resolução ou usar rede WIFI com roteador local |
| Como alterar a velocidade sem usar joystick | Falta de controle nas curvas | Usar slider na Web |
| Regras da prova desconhecidas | Carro desclassificado | Ler o regulamento antes de comprar peças |

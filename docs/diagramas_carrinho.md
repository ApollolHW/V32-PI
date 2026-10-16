# Carrinho ESP32 controlado por WiFi

## Fluxo geral do programa

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

## Eventos do WebSocket

```mermaid
flowchart TD
    E["onWsEvent()"] --> T{"Tipo do evento"}
    T -->|"WS_EVT_CONNECT"| C1["Mostra cliente conectado no Serial"]
    T -->|"WS_EVT_DISCONNECT"| D1["executar('S')<br/>para o carro por segurança"]
    T -->|"WS_EVT_DATA"| M{"Mensagem de texto completa?"}
    M -->|"Sim"| X["executar(primeiro caractere)"]
    M -->|"Não"| I["Ignora"]
```

## Comandos

```mermaid
flowchart TD
    X["executar(cmd)"] --> K{"cmd"}
    K -->|"F"| F["mover(1, 1)<br/>frente"]
    K -->|"B"| B["mover(-1, -1)<br/>trás"]
    K -->|"L"| L["mover(-1, 1)<br/>gira à esquerda"]
    K -->|"R"| R["mover(1, -1)<br/>gira à direita"]
    K -->|"S"| S["mover(0, 0)<br/>parado"]
    K -->|"A"| A["Luz HIGH"]
    K -->|"D"| D["Luz LOW"]
```

## Comunicação entre celular e ESP32

```mermaid
sequenceDiagram
    participant U as Usuário
    participant C as Celular (navegador)
    participant E as ESP32
    participant M as Motores

    U->>C: Abre o IP do ESP32
    C->>E: GET /
    E-->>C: Página HTML
    C->>E: Abre WebSocket /ws
    U->>C: Segura a seta para cima
    C->>E: "F"
    E->>M: Frente
    U->>C: Solta a seta
    C->>E: "S"
    E->>M: Parar
    Note over C,E: Se a conexão cair, o ESP32 executa "S" sozinho
```

## Ligações dos pinos

| Sinal | GPIO | Função |
|---|---|---|
| ENA | 33 | Velocidade do motor direito (PWM) |
| IN1 | 32 | Motor direito |
| IN2 | 25 | Motor direito |
| ENB | 26 | Velocidade do motor esquerdo (PWM) |
| IN3 | 5 | Motor esquerdo |
| IN4 | 18 | Motor esquerdo |
| Luz | 27 | LED |

## Comandos aceitos

| Letra | Ação |
|---|---|
| `F` | Frente |
| `B` | Trás |
| `L` | Esquerda |
| `R` | Direita |
| `S` | Parar |
| `A` | Acender luz |
| `D` | Desligar luz |

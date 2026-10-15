Documentação técnica do carro controlado por ESP32.

**Versão:** 1 · **Data:** 10/10/2026


## Índice

1. [Objetivo e requisitos](#1-objetivo-e-requisitos)
2. [Arquitetura geral](#2-arquitetura-geral)
3. [Dimensões e massa](#3-dimensões-e-massa)
4. [Tração](#4-tração)
5. [Direção](#5-direção)
6. [Eletrônica e alimentação](#6-eletrônica-e-alimentação)
7. [Peças projetadas e modificadas](#7-peças-projetadas-e-modificadas)
8. [Riscos e pendências](#8-riscos-e-pendências)

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
flowchart LR
    C["Controle<br/>(celular / gamepad)"] -- "Bluetooth ou ESP-NOW" --> E[ESP32]
    E -- "PWM + direção" --> D["Driver de motor<br/>(ponte H)"]
    D --> M[Motores DC]
    E -. "PWM (opcional)" .-> S[Servo de direção]
    B[Bateria] --> SW[Chave geral + fusível]
    SW --> D
    SW --> R["Regulador 5V"]
    R --> E
    R --> S
```

| Modulos | Descrição | Situação |
|---|---|---|
| Configuração WIFI | SSID | Funcionando |
| Direção | Servo motor | A fazer |
| Vigilância | Transcrição da imagem da ESPCAM | A fazer |
| HTTP | Canal de comunicação dupla sempre aberto | Funcionando + recursos são necessários|

## 4. Código explicado

### 4.1 Funções

| Parâmetro | Descrição | Situação |
|---|---|---|
| Câmera | Transcrição sem delay | A fazer |
| Motores | Responsável pela rotação para cada comando | OK |
| Servo | Responsável pela direção do eixo móvel | A fazer |
| HTTP | Vai ser o meio de comunicação adotado | OK |
| Setup | Precisa definir os pinos do ESP32, Estabelecer a comunicação e o estado inicial do carro | Parcial |
| Loop | Retoma as funções posteriores | Parcial |

### 4.2 Câmera


### 4.3 Motores


### 4.4 Servo


## 5. HTTP


## 6. Setup


## 7. Loop


## 8. Riscos e pendências

| Risco | Impacto | Mitigação |
|---|---|---|
| Delay da imagem do ESPCAM | Motor queimado | Reduzir a resolução ou usar rede WIFI com roteador local |
| Como alterar a velocidade sem usar joystick | Falta de controle nas curvas | Usar slider na Web |
| Regras da prova desconhecidas | Carro desclassificado | Ler o regulamento antes de comprar peças |

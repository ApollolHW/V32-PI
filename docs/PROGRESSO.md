# Registro de Progresso — Projeto Integrador

**Equipe:** Equipe VOLGA 32  
**Turma:** MECA3M  
**Professor técnico:** Gustavo Lima

Este arquivo deve registrar, de forma objetiva, o andamento semanal do projeto. A equipe não deve apagar registros anteriores. Caso o planejamento mude, a alteração deve ser descrita e justificada.

---

# Semana 1 — 16/09/2026 a 22/09/2026

## Planejado

- Debate entre usar arduino ou ESP32

## Concluído

- Criar a estrutura inicial do repositório.
- Repositório organizado com `README.md`, `PLANEJAMENTO.md` e `PROGRESSO.md`.

## CONCLUIDO



## Problemas ou impedimentos

- Os pinos digitais do ESP32 nao respondem, ainda é preciso saber se a causa é a IDE ou problemas físicos.
- O driver inicialmente escolhido pode não suportar a corrente de partida dos motores.
- Mal contato nas terminações da ponte h

## Decisões técnicas da semana

- A equipe decidiu comparar duas opções de driver antes da montagem definitiva.-
- Alteração no chassi

## Testes realizados

| Teste | Resultado |
|---|---|
| Alimentação do ESP32 em bancada | SUCESSO |
| Teste individual dos motores | SUCESSO |
| Integração ESP32,Ponte H e Motores DC | SUCESSO |

## Próximas ações

- Medir a corrente dos motores.
- Aprender soldagem eletrnônica
- Definir o driver de potência.
- Iniciar montagem mecânica.
- Implementar recepção dos comandos UDP.

---

# Semana 2 — 23/09/2026 a 29/09/2026

## Planejado

- Escolher a arquitetura de tração e direção.
- Elaborar o primeiro diagrama elétrico.
- Definir a arquitetura geral do veículo.
- Realizar testes no ESP32

## Concluído

- Realizar testes no ESP32
- Usar dois motores na ponte h
- Exercer a logica PWM nos motores
- Soldagem eletrônica


- Possivelmente compraremos outra ponte H para integrar outros 2 motores


## concluído

- ___Escolher a arquitetura de tração e direção._________________
-  Elaborar o primeiro diagrama elétrico
-  Definir a arquitetura geral do veículo

## Problemas ou impedimentos

- Necessidade das pilhas para alimentação embarcada

## Decisões técnicas da semana

- Orientação de trabalhar na base do projeto
- Desenvolvimento do novo chassi

## Testes realizados

| Teste | Resultado |
|---|---|
| __________Dois motores na ponte H__________ | SUCESSO |
| __________Upload do code no ESP__________ | SUCESSO |
| Integração ESP32,Ponte H e Motores DC | SUCESSO |

## Próximas ações

- Desenvolver o controle remoto
- Montar o circuito em cima do chassi
- Organizar as documentações
- Medir a corrente dos motores.
- Definir o driver de potência.
- Aprender comandos UDP.

---

# Semana 3 — 30/09/2026 a 06/10/2026

## Planejado

- Padronizar com o orientador sobre a planilha e documentação
- Desenvolver o controle remoto
- Montar o circuito em cima do chassi
- Organizar as documentações
- Medir a corrente dos motores.
- Definir o driver de potência.
- Aprender comandos UDP.

## Concluído

- Interface de controle
- Controle remoto

## Não concluído

- Desenvolver o controle remoto
- Montar o circuito em cima do chassi
- Organizar as documentações
- Medir a corrente dos motores.
- Definir o driver de potência.
- Aprender comandos UDP.

## Problemas ou impedimentos

- Aprimorar o controle
- Ampliar a gama de comando do controle
- Montegem em cima do chassi
- Alimentação indepente

## Decisões técnicas da semana

- Controle remoto por pagina Web

## Testes realizados

| Teste | Resultado |
|---|---|
| Controle remoto | PARCIAL |

## Próximas ações

- Desenvolver o controle remoto
- Montar o circuito em cima do chassi
- Organizar as documentações
- Medir a corrente dos motores.
- Aprender comandos UDP.

---

# Semana 4 — 07/10/2026 a 13/10/2026

## Planejado

- Desenvolver o controle remoto
- Montar o circuito em cima do chassi
- Organizar as documentações
- Medir a corrente dos motores.
- Definir o driver de potência.
- Aprender comandos UDP.
  
## Concluído

- Desenvolver o controle remoto
- Montar os sistemas em cima do chassi

## Não concluído

- Organizar as documentações
- Medir a corrente dos motores.
- Definir arquitetura mecânica
- Definir chassi novo
- Aprender comandos UDP.

## Problemas ou impedimentos

- Alimentação (resolvido)
  
## Decisões técnicas da semana

- Será necessário aumentar a tensão de alimentação
- Será necessário diminuir o peso do carro

## Testes realizados

| Teste | Resultado |
|---|---|
| Controle remoto do carro montado | SUCESSO |

## Próximas ações

- Definir arquitetura de vigilância
- Organizar as documentações
- Medir a corrente dos motores.
- Definir arquitetura mecânica
- Definir chassi novo
- Aprender comandos UDP e Web.
- Definir como vai ser o controle no circuito final

---

# Semana 5 — 14/10/2026 a 20/10/2026

## Planejado

- Preparar a demonstração mínima para a Avaliação de Progresso 1.
- Revisar a documentação do repositório.
- Comparar o cronograma planejado com o desenvolvimento realizado.

## Concluído

- Avaliação de progresso 1
- Documentação
- Registro do progresso

## Não concluído

- 

## Problemas ou impedimentos

- "patinação" na aceleração (RESOLVIDO)

## Decisões técnicas da semana

- Mudar arquitetura de comunicação para WiFi proprio do esp

## Demonstração preparada para a AP1

Foi mostrado o sistema de comunicação temporário, tração, direção e alimentação.

## Situação geral na AP1

### Entregas concluídas

- Mecânica
- Eletrônica
- Comunicação

### Entregas parcialmente concluídas

- 

### Entregas não concluídas

- 

### Principais alterações em relação ao planejamento original

- Originalmente, era planejado já ter o chassi final, mas decidimos usar momentaneamente o fornecido pela organização do evento.

### Justificativas

- A mudança no chassi foi feita para poder testar outros subsistemas em bancadas e integrados

---

# Semana 6 — 21/10/2026 a 28/10/2026

## Planejado

- Definir arquitetura de vigilância
- Organizar as documentações
- Medir a corrente dos motores
- Definir arquitetura mecânica
- Definir chassi novo
- Aprender comandos UDP e Web.
- Definir como vai ser o controle no circuito final

## Concluído

- Arquitetura de vigilancia definida
- Documentações organizadas
- arquitetura mecânica definida
- Chassi novo decidido
- Controle definido
- Reformulação do controle HTML
- Case para pilhas com chave seletora integrada

## Não concluído

-  Aprender comandos UDP e Web
-  Medir a corrente dos motores

## Problemas ou impedimentos

- 

## Decisões técnicas da semana

- Usar ESPCAM
- Chassi de carro de brinquedo
- Case para pilhas com chave seletora
- Direção com volante da organização
- Tração de dois motores traseiros e haste de direção dianteira


## Testes realizados

| Teste | Resultado |
| Case para pilhas | SUCESSO |
| Nova interface de controle | SUCESSO |

## Próximas ações
# Semana 7 — 29/10/2026 a 04/11/2026

## Planejado

- 

## Concluído

- ...

## Não concluído

- ...

## Problemas ou impedimentos

- ...

## Decisões técnicas da semana

- ...

## Testes realizados

| Teste | Resultado |
|---|---|
| ... | ... |

## Próximas ações

- ...
# Modelo para semanas posteriores

Copiar o bloco abaixo sempre que uma nova semana for iniciada.

```markdown
# Semana 6 — 21/10/2026 a 24/10/2026

## Planejado

- Fazer novo chassi e testar

## Concluído

- ...

## Não concluído

- ...

## Problemas ou impedimentos

- ...

## Decisões técnicas da semana

- ...

## Testes realizados

| Teste | Resultado |
|---|---|
| ... | ... |

## Próximas ações
# Semana 7 — 28/10/2026 a 31/10/2026

## Planejado

- 

## Concluído

- ...

## Não concluído

- ...

## Problemas ou impedimentos

- ...

## Decisões técnicas da semana

- ...

## Testes realizados

| Teste | Resultado |
|---|---|
| ... | ... |

## Próximas ações

- ...
```

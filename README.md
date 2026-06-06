# AquaSafe — Sistema de Monitoramento de Enchentes Urbanas

> Projeto desenvolvido para a Global Solution 2026/1 — FIAP
> Disciplina: Disruptive Architectures: IoT, IoB & Generative IA
> Turma: 2TDSPI — Turmas de Fevereiro

(Nosso serial monitor não funciona)
---

## Sumário

- [Descrição da Solução](#descrição-da-solução)
- [Conexão com o Tema da Global Solution](#conexão-com-o-tema-da-global-solution)
- [Arquitetura do Sistema](#arquitetura-do-sistema)
- [Componentes de Hardware](#componentes-de-hardware)
- [Ligação dos Fios (Pinagem)](#ligação-dos-fios-pinagem)
- [Funcionalidades](#funcionalidades)
- [Endpoints da API REST](#endpoints-da-api-rest)
- [Dashboard Web](#dashboard-web)
- [Matriz de Risco](#matriz-de-risco)
- [Vídeo de Demonstração](#vídeo-de-demonstração)
- [Integrantes](#integrantes)

---

## Descrição da Solução

O **AquaSafe** é um sistema embarcado de monitoramento em tempo real do nível de água em bueiros e canais urbanos, desenvolvido com ESP32-S3. O sistema utiliza um sensor ultrassônico para medir o nível da água e um potenciômetro para simular a intensidade de chuva, combinando essas leituras para calcular o risco de enchente e emitir alertas visuais (LEDs) e locais (display LCD).

Além do monitoramento local, o AquaSafe disponibiliza uma **API REST via Wi-Fi** com três endpoints JSON e um **dashboard web** acessível em tempo real pelo navegador, sem necessidade de aplicativo externo.

---

## Conexão com o Tema da Global Solution

O tema da Global Solution 2026/1 é **A Economia Espacial**: satélites monitoram o clima, evitam desastres e conectam regiões remotas. O AquaSafe se conecta a esse tema ao simular o tipo de sistema que alimenta redes de monitoramento ambiental integradas a dados orbitais, como os utilizados pelo INPE e pelo Space Charter para resposta a desastres climáticos.

A solução responde diretamente ao ODS 11 (Cidades e Comunidades Sustentáveis) e ODS 13 (Ação Contra a Mudança Global do Clima), usando IoT como camada de coleta de dados na ponta, que pode ser integrada a plataformas de análise espacial em larga escala.

---

## Arquitetura do Sistema

```
┌──────────────────────────────────────────────────┐
│                   ESP32-S3                        │
│                                                  │
│  Entradas:                   Saídas:             │
│  ┌─────────────────┐         ┌──────────────┐    │
│  │ HC-SR04          │         │ LED Amarelo  │    │
│  │ (Nível da Água)  │         │ (Risco Médio)│    │
│  └─────────────────┘         └──────────────┘    │
│  ┌─────────────────┐         ┌──────────────┐    │
│  │ Potenciômetro    │         │ LED Vermelho │    │
│  │ (Chuva Simulada) │         │ (Risco Alto/ │    │
│  └─────────────────┘         │  Crítico)    │    │
│                               └──────────────┘    │
│  Interface Local:             Rede:               │
│  ┌─────────────────┐         ┌──────────────┐    │
│  │ LCD 16x2 I2C    │         │ Wi-Fi        │    │
│  │ (Status ao vivo)│         │ WebServer    │    │
│  └─────────────────┘         │ API REST     │    │
│                               │ Dashboard    │    │
│                               └──────────────┘    │
└──────────────────────────────────────────────────┘
```

---

## Componentes de Hardware

| Componente            | Função                                      |
|-----------------------|---------------------------------------------|
| ESP32-S3 DevKit       | Microcontrolador principal                  |
| HC-SR04               | Sensor ultrassônico — mede nível da água    |
| Potenciômetro         | Simula intensidade de chuva (entrada ADC)   |
| LED Amarelo           | Alerta visual para risco médio              |
| LED Vermelho          | Alerta visual para risco alto e crítico     |
| Resistor 220Ω (x2)    | Proteção dos LEDs                           |
| LCD 16x2 com I2C      | Interface local — exibe leituras e status   |

---

## Ligação dos Fios (Pinagem)

### HC-SR04 (Sensor Ultrassônico)

| Pino do HC-SR04 | Pino do ESP32-S3 | Cor do Fio |
|-----------------|------------------|------------|
| VCC             | 5V               | Vermelho   |
| GND             | GND              | Roxo       |
| TRIG            | GPIO 12          | Azul       |
| ECHO            | GPIO 13          | Verde      |

### Potenciômetro (Simulador de Chuva)

| Pino do Potenciômetro | Pino do ESP32-S3 | Cor do Fio |
|-----------------------|------------------|------------|
| VCC (lateral)         | 3.3V             | Vermelho   |
| GND (lateral)         | GND              | Ciano      |
| SIG (central)         | GPIO 4           | Verde      |

### LCD 16x2 com Módulo I2C

| Pino do LCD | Pino do ESP32-S3 | Cor do Fio |
|-------------|------------------|------------|
| VCC         | 5V               | Vermelho   |
| GND         | GND              | Roxo       |
| SDA         | GPIO 8           | Ciano      |
| SCL         | GPIO 9           | Violeta    |

### LEDs

| Componente        | Pino ESP32-S3 | Observação        |
|-------------------|---------------|-------------------|
| LED Amarelo (+)   | GPIO 17       | Via resistor 220Ω |
| LED Amarelo (-)   | GND           |                   |
| LED Vermelho (+)  | GPIO 18       | Via resistor 220Ω |
| LED Vermelho (-)  | GND           |                   |

---

## Funcionalidades

- Leitura contínua do nível de água via sensor ultrassônico (HC-SR04)
- Leitura contínua da intensidade de chuva via potenciômetro (ADC)
- Cálculo automático do nível de risco (BAIXO / MÉDIO / ALTO / CRÍTICO)
- Exibição das leituras e status em tempo real no LCD 16x2
- Acionamento dos LEDs conforme o nível de risco
- Conexão Wi-Fi automática na inicialização
- Servidor HTTP embarcado com dashboard e API REST
- Atualização automática do dashboard a cada 2 segundos

---

## Endpoints da API REST

O servidor HTTP é iniciado na porta **80** e fica acessível pelo IP exibido no LCD após a conexão Wi-Fi.

### `GET /`
Dashboard web — página HTML com atualização automática a cada 2 segundos, exibindo nível da água, volume de chuva, nível de risco e alerta quando o risco é crítico.

---

### `GET /status`
Retorna o status geral do sistema em JSON.

```json
{
  "nivelAgua": 72,
  "volumeChuva": 45,
  "nivelRisco": "ALTO",
  "alertaAtivo": false,
  "statusSensor": "ATIVO"
}
```

---

### `GET /leitura`
Retorna os dados brutos dos sensores e o tempo de atividade em JSON.

**Resposta de exemplo:**
```json
{
  "nivelAgua": 72,
  "volumeChuva": 45,
  "uptime": 120
}
```

---

### `GET /alerta`
Retorna o estado do alerta atual com mensagem descritiva em JSON.

**Resposta de exemplo:**
```json
{
  "tipoAlerta": "NIVEL_AGUA",
  "nivelRisco": "ALTO",
  "mensagem": "Nivel alto. Fique em alerta.",
  "ativo": false
}
```

---

## Dashboard Web

O dashboard é servido diretamente pelo ESP32 e acessível em qualquer navegador na mesma rede. Ele exibe os dados em cards com barras de progresso e muda de cor conforme o nível de risco:

| Cor      | Nível de Risco |
|----------|----------------|
| Verde    | BAIXO          |
| Amarelo  | MÉDIO          |
| Laranja  | ALTO           |
| Vermelho | CRÍTICO        |

---

## Matriz de Risco

| Nível da Água | Risco   | LED          | LCD        |
|---------------|---------|--------------|------------|
| 0% – 39%      | BAIXO   | Nenhum aceso | `NORMAL`   |
| 40% – 69%     | MÉDIO   | LED Amarelo  | `MEDIO`    |
| 70% – 84%     | ALTO    | LED Vermelho | `ALTO`     |
| 85% – 100%    | CRÍTICO | LED Vermelho | `CRITICO!` |

---

## Vídeo de Demonstração

https://youtu.be/f5DcpC2tS2Q
---

## Integrantes

| Nome                              | RM       | Turma   |
|-----------------------------------|----------|---------|
| Enzo Franchin de Souza            | R565677  | 2TDSPI  |
| Lucas da Silva Lima               | RM562118 | 2TDSPI  |
| Riquelme Nascimento de Oliveira   | RM565468 | 2TDSPI  |
| Yasmin Nathalin Miranda dos Santos| RM561365 | 2TDSPI  |

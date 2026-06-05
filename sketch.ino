#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int PIN_TRIG = 12;
const int PIN_ECHO = 13;
const int PIN_POT_CHUVA = 4;
const int LED_AMARELO = 17;
const int LED_VERMELHO = 18;

// rede simulada do Wokwi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

WebServer server(80);

// variaveis globais com as leituras atuais
int nivelAguaPorcentagem = 0;
int chuvaPorcentagem = 0;
String nivelRisco = "BAIXO";
bool alertaAtivo = false;

// calcula o nivel de risco conforme as regras do projeto
String calcularRisco(int nivel) {
  if (nivel >= 85) return "CRITICO";
  if (nivel >= 70) return "ALTO";
  if (nivel >= 40) return "MEDIO";
  return "BAIXO";
}

// endpoint 1 - /status : retorna o status geral do sistema
void handleStatus() {
  String json = "{";
  json += "\"nivelAgua\":" + String(nivelAguaPorcentagem) + ",";
  json += "\"volumeChuva\":" + String(chuvaPorcentagem) + ",";
  json += "\"nivelRisco\":\"" + nivelRisco + "\",";
  json += "\"alertaAtivo\":" + String(alertaAtivo ? "true" : "false") + ",";
  json += "\"statusSensor\":\"ATIVO\"";
  json += "}";
  server.send(200, "application/json", json);
}

// endpoint 2 - /leitura : retorna os dados brutos dos sensores
void handleLeitura() {
  String json = "{";
  json += "\"nivelAgua\":" + String(nivelAguaPorcentagem) + ",";
  json += "\"volumeChuva\":" + String(chuvaPorcentagem) + ",";
  json += "\"uptime\":" + String(millis() / 1000);
  json += "}";
  server.send(200, "application/json", json);
}

// endpoint 3 - /alerta : retorna o alerta ativo (se houver)
void handleAlerta() {
  String mensagem = "Situacao normal.";
  if (nivelRisco == "CRITICO")    mensagem = "Risco critico! Evacuacao imediata.";
  else if (nivelRisco == "ALTO")  mensagem = "Nivel alto. Fique em alerta.";
  else if (nivelRisco == "MEDIO") mensagem = "Nivel em atencao. Monitore.";

  String json = "{";
  json += "\"tipoAlerta\":\"NIVEL_AGUA\",";
  json += "\"nivelRisco\":\"" + nivelRisco + "\",";
  json += "\"mensagem\":\"" + mensagem + "\",";
  json += "\"ativo\":" + String(alertaAtivo ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

// dashboard - / : pagina HTML com os dados em tempo real
void handleDashboard() {
  String corRisco = "#28a745";
  if (nivelRisco == "CRITICO")    corRisco = "#dc3545";
  else if (nivelRisco == "ALTO")  corRisco = "#fd7e14";
  else if (nivelRisco == "MEDIO") corRisco = "#ffc107";

  String corBarra = corRisco;

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<title>AquaSafe</title>";
  html += "<style>";
  html += "body{font-family:Arial,sans-serif;background:#f2f2f2;padding:20px;max-width:480px;margin:auto;}";
  html += "h2{color:#1a1a2e;margin-bottom:4px;}";
  html += "p.sub{color:#888;font-size:13px;margin-top:0;}";
  html += ".card{background:#fff;border-radius:10px;padding:16px 20px;margin-bottom:12px;box-shadow:0 1px 4px rgba(0,0,0,.1);}";
  html += ".label{font-size:13px;color:#666;margin-bottom:4px;}";
  html += ".valor{font-size:26px;font-weight:bold;color:#1a1a2e;}";
  html += ".barra-bg{background:#e0e0e0;border-radius:6px;height:14px;margin-top:8px;}";
  html += ".barra-fill{height:14px;border-radius:6px;transition:width .3s;}";
  html += ".risco{font-size:28px;font-weight:bold;}";
  html += ".alerta{border-left:4px solid #dc3545;background:#fff5f5;}";
  html += ".rodape{color:#aaa;font-size:11px;margin-top:16px;text-align:center;}";
  html += "</style></head><body>";

  html += "<h2>AquaSafe</h2>";
  html += "<p class='sub'>Monitoramento de Enchentes — ADS IoT</p>";

  html += "<div class='card'>";
  html += "<div class='label'>Nivel da Agua</div>";
  html += "<div class='valor'>" + String(nivelAguaPorcentagem) + "%</div>";
  html += "<div class='barra-bg'><div class='barra-fill' style='width:" + String(nivelAguaPorcentagem) + "%;background:" + corBarra + ";'></div></div>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<div class='label'>Volume de Chuva</div>";
  html += "<div class='valor'>" + String(chuvaPorcentagem) + "%</div>";
  html += "<div class='barra-bg'><div class='barra-fill' style='width:" + String(chuvaPorcentagem) + "%;background:#3a86ff;'></div></div>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<div class='label'>Nivel de Risco</div>";
  html += "<div class='risco' style='color:" + corRisco + ";'>" + nivelRisco + "</div>";
  html += "</div>";

  if (alertaAtivo) {
    html += "<div class='card alerta'>";
    html += "<div style='color:#dc3545;font-weight:bold;'>&#9888; ALERTA ATIVO</div>";
    html += "<div style='font-size:14px;margin-top:6px;color:#555;'>Nivel critico detectado. Verifique a regiao monitorada.</div>";
    html += "</div>";
  }

  html += "<div class='rodape'>Atualizando a cada 2 segundos</div>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_POT_CHUVA, INPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  Wire.begin(8, 9);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("AquaSafe");
  lcd.setCursor(0, 1);
  lcd.print("Conectando...");

  // conecta ao Wi-Fi e aguarda conexao
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());

  // exibe o IP no LCD por 2 segundos
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi OK!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(2000);
  lcd.clear();

  // registra as rotas do servidor
  server.on("/", handleDashboard);
  server.on("/status", handleStatus);
  server.on("/leitura", handleLeitura);
  server.on("/alerta", handleAlerta);
  server.begin();

  Serial.println("Servidor iniciado.");
  Serial.println("Dashboard: http://" + WiFi.localIP().toString());
  Serial.println("Endpoints: /status  /leitura  /alerta");
}

void loop() {
  // leitura do sensor ultrasonico (nivel da agua)
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duracao = pulseIn(PIN_ECHO, HIGH);
  int distancia = duracao * 0.034 / 2;

  nivelAguaPorcentagem = map(distancia, 319, 1, 0, 100);
  if (nivelAguaPorcentagem < 0)   nivelAguaPorcentagem = 0;
  if (nivelAguaPorcentagem > 100) nivelAguaPorcentagem = 100;

  // leitura do sensor de chuva (potenciometro simulado)
  int chuva = analogRead(PIN_POT_CHUVA);
  chuvaPorcentagem = map(chuva, 0, 4095, 0, 100);

  // atualiza o nivel de risco e o estado do alerta
  nivelRisco = calcularRisco(nivelAguaPorcentagem);
  alertaAtivo = (nivelRisco == "CRITICO");

  // atualiza o display LCD
  lcd.setCursor(0, 0);
  lcd.print("Ag:");
  lcd.print(nivelAguaPorcentagem);
  lcd.print("% Ch:");
  lcd.print(chuvaPorcentagem);
  lcd.print("%   ");

  lcd.setCursor(0, 1);
  if (nivelRisco == "CRITICO") {
    lcd.print("CRITICO!        ");
    digitalWrite(LED_VERMELHO, HIGH);
    digitalWrite(LED_AMARELO, LOW);
  } else if (nivelRisco == "ALTO") {
    lcd.print("ALTO            ");
    digitalWrite(LED_VERMELHO, HIGH);
    digitalWrite(LED_AMARELO, LOW);
  } else if (nivelRisco == "MEDIO") {
    lcd.print("MEDIO           ");
    digitalWrite(LED_AMARELO, HIGH);
    digitalWrite(LED_VERMELHO, LOW);
  } else {
    lcd.print("NORMAL          ");
    digitalWrite(LED_AMARELO, LOW);
    digitalWrite(LED_VERMELHO, LOW);
  }

  // processa as requisicoes HTTP recebidas
  server.handleClient();
  delay(500);
}

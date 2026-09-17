#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int pinPIR = 2;    
const int pinLDR = A0;   
const int pinLED = 8;    
const int pinRele = 9;   

int limiarEscuro = 700;  

// --- Variáveis Globais de Tempo (millis) ---
unsigned long tempoAnteriorTravessia = 0;
unsigned long tempoUltimaEscuridao = 0;
unsigned long tempoAnteriorLCD = 0;  // <-- Variável global do visor!

const long intervaloContagem = 1000; // 1 segundo para contagem
const long debounceFarol = 3000;     // 3 segundos (Debounce)
const long intervaloLCD = 500;       // Atualiza a tela a cada 0.5s

int tempoRestante = 0;
bool travessiaAtiva = false;
bool estaDeNoite = false;

void setup() {
  Serial.begin(9600);
  pinMode(pinPIR, INPUT);
  pinMode(pinLED, OUTPUT);
  pinMode(pinRele, OUTPUT); 

  digitalWrite(pinRele, HIGH); 

  lcd.init();      
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Sistema Ativo");
  delay(2000); 
  lcd.clear(); 
}

void loop() {
  int nivelLuz = analogRead(pinLDR);
  int movimento = digitalRead(pinPIR);
  unsigned long tempoAtual = millis();

  // 1. FILTRO DE DEBOUNCE
  if (nivelLuz > limiarEscuro) {
    estaDeNoite = true; 
    tempoUltimaEscuridao = tempoAtual; 
  } else {
    if (tempoAtual - tempoUltimaEscuridao > debounceFarol) {
      estaDeNoite = false; 
    }
  }

  // 2. LÓGICA DE DISPARO
  if (estaDeNoite && movimento == HIGH && !travessiaAtiva) {
    travessiaAtiva = true;
    tempoRestante = 15; 
    tempoAnteriorTravessia = tempoAtual;

    digitalWrite(pinLED, HIGH);   
    digitalWrite(pinRele, LOW);   
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Travessia Segura");
  }

  // 3. EXECUÇÃO DA TRAVESSIA
  if (travessiaAtiva) {
    if (tempoAtual - tempoAnteriorTravessia >= intervaloContagem) {
      tempoAnteriorTravessia = tempoAtual; 
      
      lcd.setCursor(0, 1);
      lcd.print("Tempo: ");
      lcd.print(tempoRestante);
      lcd.print(" seg ");
      
      tempoRestante--;

      if (tempoRestante < 0) {
        travessiaAtiva = false;
        digitalWrite(pinLED, LOW);    
        digitalWrite(pinRele, HIGH);  
        lcd.clear(); 
      }
    }
  } 
  // 4. PAINEL DE ESPERA (Monitoramento)
  else {
    // Só atualiza a tela a cada intervaloLCD (500ms)
    if (tempoAtual - tempoAnteriorLCD >= intervaloLCD) {
      tempoAnteriorLCD = tempoAtual; // Reseta o cronômetro da tela

      lcd.setCursor(0, 0);
      lcd.print("Luz: ");
      lcd.print(nivelLuz);
      lcd.print("    "); // O espaço apaga "fantasmas" de números antigos

      lcd.setCursor(0, 1);
      if (movimento == HIGH) {
        lcd.print("Movimento: SIM ");
      } else {
        lcd.print("Movimento: NAO ");
      }
    }
  }
}
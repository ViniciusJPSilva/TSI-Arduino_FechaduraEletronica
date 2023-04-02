#include <Keypad.h> // Biblioteca responsável por controlar as entradas do teclado
#include <string.h>

const int TAMANHO_SENHA = 4, // Tamanho padrão da senha (1 a 9)
    NUM_TENT_ALARME = 5; // Número de tentativas erradas que podem ser feitas antes do alarme tocar

const byte LINHAS = 4; // Número de linhas do teclado
const byte COLUNAS = 4; // Número de colunas do teclado
const char CONTROLE = '*', CODIGO_RESET[] = {'#', '#', '#', '#', '#', '#', '#', '#', '#'};

// Nomeando os PINS
const int VERDE = A2,
    VERMELHO = A0,
    AZUL = A1,
    SOM = A3,
    BOTAO = A4,
    FECHADURA = A5;
    

// Matriz de mapeamento dos caracteres, baseado no número de colunas e linhas
char mapTeclas[LINHAS][COLUNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Define os 4 PINS referentes às linhas do teclado 
byte pinsLinhas[LINHAS] = {9, 8, 7, 6}; 

// Define os 4 PINS referentes às colunas do teclado
byte pinsColunas[COLUNAS] = {5, 4, 3, 2}; 

// Cria um objeto teclado que relaciona a entrada dos pin's e a matriz de mapeamento mapTeclas.
Keypad teclado = Keypad(makeKeymap(mapTeclas), pinsLinhas, pinsColunas, LINHAS, COLUNAS); 

// Variáveis globais de controle 
char tentativa[TAMANHO_SENHA], senha[TAMANHO_SENHA];
int alarme;
bool senhaOK;

/*
  Função de inicialização: é executada uma única vez no inicio da execução do algoritmo.
*/
void setup(){
  Serial.begin(9600);
  
  // Definindo o modo dos pinos de controle
  pinMode(VERMELHO, OUTPUT);
  pinMode(AZUL, OUTPUT);
  pinMode(VERDE, OUTPUT);
  pinMode(FECHADURA, OUTPUT);
  pinMode(SOM, OUTPUT);

  pinMode(BOTAO, INPUT);

  
  // Inicializando a senha com uma sequência numérica, baseando-se no tamanho pré-definido da senha.
  inicializarString(senha, TAMANHO_SENHA);
  
  // Inicializando a contagem para o alarme.
  alarme = 0;
} // setup()
  


/*
  Função de repetição: é executada repetitivamente até a finalização do algoritmo.
*/
void loop(){
  // Lendo a senha
  senhaOK = lerString(tentativa, TAMANHO_SENHA, CONTROLE);
  
  // Verifica se a tecla de controle foi pressionada
  if(senhaOK){
    // Verifica se o código de reset foi inserido
    if(!comparaStrings(tentativa, CODIGO_RESET, TAMANHO_SENHA))
      // Verifica se a senha inserida é igual à senha armazenada
      if(comparaStrings(tentativa, senha,TAMANHO_SENHA)){
        energizarPino(FECHADURA, 1); // Abre a fechadura
        alarme = 0; // Reinicia a contagem para o alarme.
      }else{
        energizarPino(VERMELHO, 1); // Alerta o usuário que a senha inserida é inválida
        alarme++;
      }
    else
      alterarSenha(); // Chama a função para alterar a senha
      
  } else {
    energizarPino(AZUL, VERMELHO, 1); // Alerta o usuário que a leitura foi finalizada (apaga o "buffer")
  } // if(senhaOK)
  
  // Toca o alarme
  if(alarme >= NUM_TENT_ALARME){
    tocarPiezo(SOM, BOTAO, 500, 60000);
    alarme = 0; // Reinicia a contagem para o alarme.
  }
  
} // loop()



/*
  Lê e armazena os caracteres pressionados no teclado em um vetor de caracteres.
    Params:
      texto: string onde os caracteres serão armazenados.
        tam: tamanho da string texto
        controle: caracter de controle que finaliza o loop de leitura
    Retorno:
      true: caso o caracter de controle não tenha sido pressionado, leitura completa.
        false: caso o caracter de controle tenha sido pressionado, leitura incompleta.
*/
bool lerString(char *texto, int tam, char controle){
  for(int cont = 0; cont < TAMANHO_SENHA;){
    //Armazena a tecla pressionada, caso houver
    char teclaPressionada = teclado.getKey();
  
    // Verifica se alguma tecla foi pressionada
    if (teclaPressionada){
      tocarPiezo(SOM, 300, 200);
      
      if(teclaPressionada == CONTROLE) // O caracter de controle finaliza a leitura
        return false; 
      
      texto[cont] = teclaPressionada; // Armazena o caracter
      cont++;
    } 
  } 
  
  return true;
} // lerString()



/*
  Preenche uma string (vetor de caracteres) com o caracter de números inteiros sequenciais.
    Params:
      texto[]: vetor de caracteres que será preenchido.
        tamanho: tamanho do vetor, ou quantidade de números a serem inseridos.
*/
void inicializarString(char *texto, int tamanho){
  for(int i = 0; i < tamanho; i++)
    texto[i] = 49 + i; // 49 = '1'
} // inicializarString()



/*
  Função responsável por confirmar a senha antiga, ler a nova senha e validá-la.
*/
void alterarSenha(){
  digitalWrite(AZUL, HIGH); // Acende o led azul, indicando que entrou no modo de troca de senha
  
  // Confirma a senha atual, por questões de segurança.
  while(!lerString(tentativa, TAMANHO_SENHA, CONTROLE))
    energizarPino(VERMELHO, 1); // Se apagar a string, o led pisca Roxo.
  
  digitalWrite(AZUL, LOW);
  // Verifica se a senha correta foi fornecida
  if(comparaStrings(tentativa, senha, TAMANHO_SENHA)){
    digitalWrite(VERDE, HIGH);
    
    // Salva a senha antiga.
    char senhaAntiga[TAMANHO_SENHA];
    memcpy(senhaAntiga, senha, TAMANHO_SENHA);
    
    // Lê a nova senha.
    while(!lerString(senha, TAMANHO_SENHA, CONTROLE)){
      digitalWrite(VERDE, LOW);  
      energizarPino(VERMELHO, AZUL, 1);
      digitalWrite(VERDE, HIGH);
    }
    
    digitalWrite(VERDE, LOW);
    // Verifica se a nova senha é igual ao código de resete.
    if(comparaStrings(senha, CODIGO_RESET, TAMANHO_SENHA)){
      memcpy(senha, senhaAntiga, TAMANHO_SENHA);
      piscarPino(VERMELHO, 3);
    } else
      piscarPino(VERDE, 3);
       
    return;
  }
  
  piscarPino(VERMELHO, 3);
} // alterarSenha()



/*
  Verifica se as strings fornecidas possuem o mesmo conteúdo (são iguais).
    Params:
      texto1[] e texto2[]: strings que serão comparadas.
    Retorno:
      true: Caso sejam iguais.
        false: Caso não sejam iguais.
*/
bool comparaStrings(const char *texto1, const char *texto2, int tamanho){
  for(int i = 0; i < tamanho; i++)
    if(texto1[i] != texto2[i])
      return false;
  
  return true;
} // comparaStrings()



/*
  Energiza um pino durante um tempo determinado.
    Params: 
      pino: pino que será energizado.
        tempo: tempo que o pino será energizado (em segundos).
*/
void energizarPino(int pino, int tempo){
  digitalWrite(pino, HIGH);
  delay(tempo * 1000);
  digitalWrite(pino, LOW); 
} // energizaPino()



/*
  Sobrecarga: energizarPino
    Energiza dois pinos durante um tempo determinado.
*/
void energizarPino(int pino1, int pino2, int tempo){
  digitalWrite(pino1, HIGH);
  digitalWrite(pino2, HIGH);
  delay(tempo * 1000);
  digitalWrite(pino1, LOW); 
  digitalWrite(pino2, LOW); 
} // energizaPino()



/*
  Energiza e desenergiza um pino uma determinada quantidade de vezes.
    Params:
      pino: pino que será energizado e desenergizado.
        quantidade: quantidade de vezes que o pino piscará

*/
void piscarPino(int pino, int quantidade) {
  // Desernegiza o pino, caso esteja energizado.
  if(digitalRead(pino) == HIGH){
    digitalWrite(pino, LOW); 
    delay(500);
  }
  
  // Pisca o pino
  for(int i = 0; i < quantidade; i++){
    digitalWrite(pino, HIGH);
    delay(500);
    digitalWrite(pino, LOW);
    delay(500);
  }
} // piscarPino()



/*
  Toca um piezo/buzzer (conectado á um pino) em uma frequência, durante um tempo determinado.
    Params:
      piezo: pino onde o piezo/buzzer está conectado.
      parar: pino que para o loop do beep;
      freq: frequência
      tempo: tempo que o piezo tocará (em milisegundos).
*/
void tocarPiezo(int piezo, int parar, int freq, int tempo){
  tone(piezo,freq);
  for(int i = 60; i > 0 && digitalRead(parar) == LOW; i--)
    delay(1000);
  
  noTone(piezo); 
} // tocarPiezo()

void tocarPiezo(int piezo, int freq, int tempo){
  tone(piezo,freq);
  delay(tempo);   
  noTone(piezo); 
} // tocarPiezo()


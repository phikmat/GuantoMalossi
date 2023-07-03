#include "DigitalIn.h"
#include "PinNameAliases.h"
#include "PinNames.h"
#include "PwmOut.h"
#include "ThisThread.h"
#include "Ticker.h"
#include "mbed.h"
#include "BufferedSerial.h"
#include "mbed_thread.h"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "string.h"
#include <regex> 

using namespace std::chrono;

#define MAX_BUFFER_SIZE 200                 // Dimensione massima del buffer 

BufferedSerial bluetooth(D8, D2);           // RX, TX pins per il modulo Bluetooth HC-10 (utilizziamo D2 e D8 in quando D0 e D1 sono dedicati alla comunicazione pc)
BufferedSerial pc(USBTX, USBRX);            // RX, TX pins per pc

//Input
AnalogIn aPiezo(A0);                        // ADC1/O
//AnalogIn bPiezo(A1);
AnalogIn cPiezo(A1);                        // ADC1/1
AnalogIn dPiezo(A2);                        // ADC1/4
AnalogIn ePiezo(A3);                        // ADC1/8
//AnalogIn fPiezo(A2);
//AnalogIn gPiezo(A4);    
//AnalogIn hPiezo(A2);
AnalogIn iPiezo(A5);                        // ADC1/10
AnalogIn lPiezo(A4);                        // ADC1/11
AnalogIn mPiezo(D11);                       // ADC1/7
AnalogIn nPiezo(D12);                       // ADC1/6
AnalogIn oPiezo(D13);                       // ADC1/5
AnalogIn pPiezo(PB_1);                      // ADC1/9
//AnalogIn qPiezo(A2);  
AnalogIn rPiezo(PC_2);                      // ADC1/12
AnalogIn sPiezo(PC_3);                      // ADC1/13
AnalogIn tPiezo(PC_4);                      // ADC1/14
//AnalogIn uPiezo(PC_5);  
AnalogIn vPiezo(PC_5);                      // ADC1/15
//AnalogIn zPiezo(A2);

//Output 
PwmOut aSensor(PC_8);
////PwmOut bSensor(D5);
PwmOut cSensor(PC_6);     
PwmOut dSensor(PA_11);      
PwmOut eSensor(D10);
//PwmOut fSensor(D14);                      // Funzionerebbe con D14, manca componente fisica 
PwmOut gSensor(PB_15);
//PwmOut hSensor(D15);                      // Funzionerebbe con D15, manca componente fisica
PwmOut iSensor(PB_14);
PwmOut lSensor(PB_13);
PwmOut mSensor(PC_9);
PwmOut nSensor(PA_15);
PwmOut oSensor(PB_7);
PwmOut pSensor(D4);
////PwmOut qSensor(D6);
PwmOut rSensor(D3);
PwmOut sSensor(D5);
PwmOut tSensor(D6);
PwmOut uSensor(D7);
PwmOut vSensor(D9);
////PwmOut zSensor(D6);


DigitalIn buttonInvia(BUTTON1);             // Bottone per l'invio del messaggio dal guanto all'app
DigitalOut ledScrittura(D15);               // Led che ci segnala se si sta scrivendo un messaggio sul guanto

float frequency = 200;                      // Frequenza in Hz per il Vibration Motor 
float dutyCycle = 0.8;                      // Duty cycle (0.0 to 1.0) per il Vibration Motor 

string outputMessage = "";                  // Qui salviamo il messaggio da inviare all'app

Ticker timer;                               // Il tiker ci permette di separare le parole

bool isWriting = false;                     // E' la variabile di stato che ci indica se stiamo scrivendo un messaggio sul guanto


//Genera un identificativo alfanumerico randomico che inviamo all'app insieme al messaggio 
std::string gen_random(const int len) {     
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    return tmp_s;
}



// Invia dei messaggi al modulo Bluetooth
void write(string messageStr) {
    string messageId = gen_random(4);                                   //Genero un id randomico formato da 4 caratteri alfanumerici
    string welcomeMessage = "<w id=" + messageId + ">";                 //Mi costruisco il tag di tipo write con l'id del messaggio <w id=000000>
    welcomeMessage += messageStr;                                       //Inserisco il contenuto del messaggio nel tag <w id=000000>Messagio da inviare
    welcomeMessage += "</w>";                                           //Chiudo il tag <w id=000000>Messagio da inviare</w>
    bluetooth.write(welcomeMessage.c_str(), welcomeMessage.size());     //Invio del messaggio al modulo bluetooth:  
                                                                            //buffer: converto la stringa in un buffer con la funzione c_str(),
                                                                            //length: recupeando la dimensione del buffer con la funzione size()
    pc.write(welcomeMessage.c_str(), welcomeMessage.size());            //Invio del messaggio al pc per stamparlo in console 
}



// Separo le parole con uno spazio se necessario
void seperaParole() { 
    if (outputMessage.at(outputMessage.length() - 1) != ' ') {
        outputMessage += " ";
    }
}

// Inizializzo il ticker richiamando la funzione seperaParole() ogni 4 secondi
void resetTicker() {
    isWriting = true;
    timer.attach(seperaParole, 4000ms);
}

// Funzione per la scrittura del messaggio e l'invio all'app 
void writeValueIfNeed() { 
    
    //Se ho scritto un messaggio ed ho cliccato sul bottone invia, mando il messaggio all'app
    if (buttonInvia == 0 && !outputMessage.empty()) {       
        timer.detach();                                 // Stoppo il Ticker
        string temp = outputMessage;                    
        write(temp);                                    // Invio il messaggio all'app 
        ThisThread::sleep_for(400ms);                   
        outputMessage = "";                             //Resetto le mie variabili
        isWriting = false;
        return;
    }

    // Impostiamo le soglie dei piezoelettrici, se il piezo a supera la soglia ci salviamo la lettera associata

    if (aPiezo > 0.45f) {                               // 0.45 è la soglia che identifica l'input della persona sordocieca
        outputMessage += 'a';                           // salvo la lettera A nel nostro messaggio
        printf("A value: %f\n", aPiezo.read()*1.0f);    // stampo il valore in console
        resetTicker();                                  //Resetto il ticker che conta 4 secondi per aggiungere uno spazio tra le parole
        ThisThread::sleep_for(400ms);                   
    } 
    
    /*
    if (bPiezo > 0.75f) {
        outputMessage += 'b';
        printf("B value: %f\n", bPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    } 
    */

    if (cPiezo > 0.75f) {                               // 0.75 è la soglia che identifica l'input della persona sordocieca
        outputMessage += 'c';                           // salvo la lettera C nel nostro messaggio
        printf("C value: %f\n", cPiezo.read()*1.0f);    // stampo il valore in console
        resetTicker();                                  //Resetto il ticker che conta 4 secondi per aggiungere uno spazio tra le parole
        ThisThread::sleep_for(400ms);
    }
    
    if (dPiezo > 0.99f) {                               // 0.99 è la soglia che identifica l'input della persona sordocieca
        outputMessage += 'd';                           // salvo la lettera D nel nostro messaggio
        printf("D value: %f\n", dPiezo.read()*1.0f);    // stampo il valore in console
        resetTicker();                                  //Resetto il ticker che conta 4 secondi per aggiungere uno spazio tra le parole
        ThisThread::sleep_for(400ms);
    }
    
    if (ePiezo > 0.75f) {                               // 0.75 è la soglia che identifica l'input della persona sordocieca
        outputMessage += 'e';                           // salvo la lettera E nel nostro messaggio
        printf("E value: %f\n", ePiezo.read()*1.0f);    // stampo il valore in console
        resetTicker();                                  //Resetto il ticker che conta 4 secondi per aggiungere uno spazio tra le parole
        ThisThread::sleep_for(400ms);
    }
    
    /*
    if (fPiezo > 0.75f) {
        outputMessage += 'f';
        printf("F value: %f\n", fPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    
    if (gPiezo > 0.75f) {
        outputMessage += 'g';
        printf("G value: %f\n", gPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    } 
    
    if (hPiezo > 0.75f) {
        outputMessage += 'h';
        printf("H value: %f\n", hPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    */
    
    
    if (iPiezo > 0.99f) {                               // 0.99 è la soglia che identifica l'input della persona sordocieca
        outputMessage += 'i';                           // salvo la lettera I nel nostro messaggio
        printf("I value: %f\n", iPiezo.read()*1.0f);    // stampo il valore in console
        resetTicker();                                  //Resetto il ticker che conta 4 secondi per aggiungere uno spazio tra le parole
        ThisThread::sleep_for(400ms);
    }
    
    if (lPiezo > 0.7f) {                               // 0.7 è la soglia che identifica l'input della persona sordocieca
        outputMessage += 'l';                           // salvo la lettera L nel nostro messaggio
        printf("L value: %f\n", lPiezo.read()*1.0f);    // stampo il valore in console
        resetTicker();                                  //Resetto il ticker che conta 4 secondi per aggiungere uno spazio tra le parole
        ThisThread::sleep_for(400ms);
    }
    
    if (mPiezo > 0.7f) {                               // 0.7 è la soglia che identifica l'input della persona sordocieca
        outputMessage += 'm';                           // salvo la lettera M nel nostro messaggio
        printf("M value: %f\n", mPiezo.read()*1.0f);    // stampo il valore in console
        resetTicker();                                  //Resetto il ticker che conta 4 secondi per aggiungere uno spazio tra le parole
        ThisThread::sleep_for(400ms);
    }
    
    if (nPiezo > 0.85f) {                               // 0.85 è la soglia che identifica l'input della persona sordocieca
        outputMessage += 'n';                           // salvo la lettera N nel nostro messaggio
        printf("N value: %f\n", nPiezo.read()*1.0f);    // stampo il valore in console
        resetTicker();                                  //Resetto il ticker che conta 4 secondi per aggiungere uno spazio tra le parole
        ThisThread::sleep_for(400ms);
    }
    
    if (oPiezo > 0.6f) {                               // 0.6 è la soglia che identifica l'input della persona sordocieca
        outputMessage += 'o';                           // salvo la lettera O nel nostro messaggio
        printf("O value: %f\n", oPiezo.read()*1.0f);    // stampo il valore in console
        resetTicker();                                  //Resetto il ticker che conta 4 secondi per aggiungere uno spazio tra le parole
        ThisThread::sleep_for(400ms);
    }
    
    if (pPiezo > 0.75f) {                               // 0.75 è la soglia che identifica l'input della persona sordocieca
        outputMessage += 'p';                           // salvo la lettera P nel nostro messaggio
        printf("P value: %f\n", pPiezo.read()*1.0f);    // stampo il valore in console
        resetTicker();                                  //Resetto il ticker che conta 4 secondi per aggiungere uno spazio tra le parole
        ThisThread::sleep_for(400ms);
    }
    
    /*
    if (qPiezo > 0.75f) {
        outputMessage += 'q';
        printf("Q value: %f\n", qPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    */
    
    if (rPiezo > 0.75f) {                               // 0.75 è la soglia che identifica l'input della persona sordocieca
        outputMessage += 'r';                           // salvo la lettera R nel nostro messaggio
        printf("-> R value: %f\n", rPiezo.read()*1.0f); // stampo il valore in console
        resetTicker();                                  //Resetto il ticker che conta 4 secondi per aggiungere uno spazio tra le parole
        ThisThread::sleep_for(400ms);
    }
    
    if (sPiezo > 0.99f) {                               // 0.99 è la soglia che identifica l'input della persona sordocieca
        outputMessage += 's';                           // salvo la lettera S nel nostro messaggio
        printf("S value: %f\n", sPiezo.read()*1.0f);    // stampo il valore in console
        resetTicker();                                  //Resetto il ticker che conta 4 secondi per aggiungere uno spazio tra le parole
        ThisThread::sleep_for(400ms);
    }
    
    if (tPiezo > 0.75f) {                               // 0.75 è la soglia che identifica l'input della persona sordocieca
        outputMessage += 't';                           // salvo la lettera T nel nostro messaggio
        printf("T value: %f\n", tPiezo.read()*1.0f);    // stampo il valore in console
        resetTicker();                                  //Resetto il ticker che conta 4 secondi per aggiungere uno spazio tra le parole
        ThisThread::sleep_for(400ms);
    }
    
    /*
    if (uPiezo > 0.75f) {
        outputMessage += 'u';
        printf("U value: %f\n", uPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    */
    
    
    if (vPiezo > 0.75f) {                               // 0.75 è la soglia che identifica l'input della persona sordocieca
        outputMessage += 'v';                           // salvo la lettera V nel nostro messaggio
        printf("V value: %f\n", vPiezo.read()*1.0f);    // stampo il valore in console
        resetTicker();                                  //Resetto il ticker che conta 4 secondi per aggiungere uno spazio tra le parole
        ThisThread::sleep_for(400ms);
    }

/*
    if (zPiezo > 0.75f) {
        outputMessage += 'z';
        printf("Z value: %f\n", zPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    */
  

}

// Funzione per la lettura dall'app al guanto
void valueToGuanto(char c) { 

    // ogni if fa vibrare il vibration motor della lettera corrispondente

    if (c == 'a' || c == 'A') {
        aSensor.resume();                       //Attivo il Vibration Motor
        aSensor.write(dutyCycle);               //Imposto il dutyCycle per far vibrare 
        ThisThread::sleep_for(500ms);           //Attendo mezzo secondo
        aSensor.write(0);                       //Stoppo la vibrazione
        aSensor.suspend();                      //Disattivo il Vibration Motor
    } else if (c == 'b' || c == 'B') {
        /*
        bSensor.resume();
        bSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        bSensor.write(0);
        bSensor.suspend();
        */
    }else if (c == 'c' || c == 'C') {
        cSensor.resume();                       //Attivo il Vibration Motor
        cSensor.write(dutyCycle);               //Imposto il dutyCycle per far vibrare 
        ThisThread::sleep_for(500ms);           //Attendo mezzo secondo
        cSensor.write(0);                       //Stoppo la vibrazione
        cSensor.suspend();                      //Disattivo il Vibration Motor
    }else if (c == 'd' || c == 'D') {
        dSensor.resume();                       //Attivo il Vibration Motor
        dSensor.write(dutyCycle);               //Imposto il dutyCycle per far vibrare 
        ThisThread::sleep_for(500ms);           //Attendo mezzo secondo
        dSensor.write(0);                       //Stoppo la vibrazione
        dSensor.suspend();                      //Disattivo il Vibration Motor
    }else if (c == 'e' || c == 'E') {
        eSensor.resume();                       //Attivo il Vibration Motor
        eSensor.write(dutyCycle);               //Imposto il dutyCycle per far vibrare 
        ThisThread::sleep_for(500ms);           //Attendo mezzo secondo
        eSensor.write(0);                       //Stoppo la vibrazione
        eSensor.suspend();                      //Disattivo il Vibration Motor
    }else if (c == 'f' || c == 'F') {
        /*
        fSensor.resume();
        fSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        fSensor.write(0);
        fSensor.suspend();
        */
    }else if (c == 'g' || c == 'G') {
        gSensor.resume();                       //Attivo il Vibration Motor
        gSensor.write(dutyCycle);               //Imposto il dutyCycle per far vibrare 
        ThisThread::sleep_for(500ms);           //Attendo mezzo secondo
        gSensor.write(0);                       //Stoppo la vibrazione
        gSensor.suspend();                      //Disattivo il Vibration Motor
    }else if (c == 'h' || c == 'H') {
        /*
        hSensor.resume();
        hSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        hSensor.write(0);
        hSensor.suspend();
        */
    }else if (c == 'i' || c == 'I') {
        iSensor.resume();                       //Attivo il Vibration Motor
        iSensor.write(dutyCycle);               //Imposto il dutyCycle per far vibrare 
        ThisThread::sleep_for(500ms);           //Attendo mezzo secondo
        iSensor.write(0);                       //Stoppo la vibrazione
        iSensor.suspend();                      //Disattivo il Vibration Motor
    }else if (c == 'l' || c == 'L') {
        lSensor.resume();                       //Attivo il Vibration Motor
        lSensor.write(dutyCycle);               //Imposto il dutyCycle per far vibrare 
        ThisThread::sleep_for(500ms);           //Attendo mezzo secondo
        lSensor.write(0);                       //Stoppo la vibrazione
        lSensor.suspend();                      //Disattivo il Vibration Motor
    }else if (c == 'm' || c == 'M') {
        mSensor.resume();                       //Attivo il Vibration Motor
        mSensor.write(dutyCycle);               //Imposto il dutyCycle per far vibrare 
        ThisThread::sleep_for(500ms);           //Attendo mezzo secondo
        mSensor.write(0);                       //Stoppo la vibrazione
        mSensor.suspend();                      //Disattivo il Vibration Motor
    }else if (c == 'n' || c == 'N') {
        nSensor.resume();                       //Attivo il Vibration Motor
        nSensor.write(dutyCycle);               //Imposto il dutyCycle per far vibrare 
        ThisThread::sleep_for(500ms);           //Attendo mezzo secondo
        nSensor.write(0);                       //Stoppo la vibrazione
        nSensor.suspend();                      //Disattivo il Vibration Motor
    }else if (c == 'o' || c == 'O') {
        oSensor.resume();                       //Attivo il Vibration Motor
        oSensor.write(dutyCycle);               //Imposto il dutyCycle per far vibrare 
        ThisThread::sleep_for(500ms);           //Attendo mezzo secondo
        oSensor.write(0);                       //Stoppo la vibrazione
        oSensor.suspend();                      //Disattivo il Vibration Motor
    }else if (c == 'p' || c == 'P') {
        pSensor.resume();                       //Attivo il Vibration Motor
        pSensor.write(dutyCycle);               //Imposto il dutyCycle per far vibrare 
        ThisThread::sleep_for(500ms);           //Attendo mezzo secondo
        pSensor.write(0);                       //Stoppo la vibrazione
        pSensor.suspend();                      //Disattivo il Vibration Motor
    }else if (c == 'q' || c == 'Q') {
        /*
        qSensor.resume();
        qSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        qSensor.write(0);
        qSensor.suspend();
        */
    }else if (c == 'r' || c == 'R') {
        rSensor.resume();                       //Attivo il Vibration Motor
        rSensor.write(dutyCycle);               //Imposto il dutyCycle per far vibrare 
        ThisThread::sleep_for(500ms);           //Attendo mezzo secondo
        rSensor.write(0);                       //Stoppo la vibrazione
        rSensor.suspend();                      //Disattivo il Vibration Motor
    }else if (c == 's' || c == 'S') {
        sSensor.resume();                       //Attivo il Vibration Motor
        sSensor.write(dutyCycle);               //Imposto il dutyCycle per far vibrare 
        ThisThread::sleep_for(500ms);           //Attendo mezzo secondo
        sSensor.write(0);                       //Stoppo la vibrazione
        sSensor.suspend();                      //Disattivo il Vibration Motor
    }else if (c == 't' || c == 'T') {
        tSensor.resume();                       //Attivo il Vibration Motor
        tSensor.write(dutyCycle);               //Imposto il dutyCycle per far vibrare 
        ThisThread::sleep_for(500ms);           //Attendo mezzo secondo
        tSensor.write(0);                       //Stoppo la vibrazione
        tSensor.suspend();                      //Disattivo il Vibration Motor
    }else if (c == 'u' || c == 'U') {
        uSensor.resume();                       //Attivo il Vibration Motor
        uSensor.write(dutyCycle);               //Imposto il dutyCycle per far vibrare 
        ThisThread::sleep_for(500ms);           //Attendo mezzo secondo
        uSensor.write(0);                       //Stoppo la vibrazione
        uSensor.suspend();                      //Disattivo il Vibration Motor
    }else if (c == 'v' || c == 'V') {
        vSensor.resume();                       //Attivo il Vibration Motor
        vSensor.write(dutyCycle);               //Imposto il dutyCycle per far vibrare 
        ThisThread::sleep_for(500ms);           //Attendo mezzo secondo
        vSensor.write(0);                       //Stoppo la vibrazione
        vSensor.suspend();                      //Disattivo il Vibration Motor
    }else if (c == 'z' || c == 'Z') {
        /*
        zSensor.resume();
        zSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        zSensor.write(0);
        zSensor.suspend();
        */
    }

}

// Ricevere dati dal modulo Bluetooth
void readValueIfNeed() {
    //inizializzo il buffer dove poter memorizzare temporaneamente la frase ricevuta dal modulo bluetooth
    char buffer[MAX_BUFFER_SIZE] = {};

    bool recivedValidMessage = false;
    string fullMessage = "";                // Variabile di apoggio dell'intero messaggio (compreso id e tipo di messaggio)
    string validMessage = "";               // Variabile di apoggio del messaggio testuale da inviare al guanto
    string idMessage = "";                  // Variabile di appoggio dell'id del messaggio


    // Verifico che il modulo bluetooth sia collegato correttamente e in modalità lettura abilitata
    if(bluetooth.readable() && !isWriting){
        // Leggo una frase che ho ricevuto dal modulo bluetooth
        bluetooth.read(&buffer, MAX_BUFFER_SIZE);

        // Recupero il messaggio completo e lo salvo nella variabile fullMessage
        for (int i = 0; i < MAX_BUFFER_SIZE; i++) {
            char c = buffer[i];
            fullMessage += c;
        }

        // Stampo il messaggio in console
        printf("\nFull Message: %s\n", fullMessage.c_str());

        // Recupero l'ID del messaggio, il messaggio da inviare al guanto grazie all'utilizzo di una apposita REGEX (regular expression)
        regex regexId(".*id=([^>]+)>([^<]+)?<\\/([^>]+)>");     // Creo l'espressione regolare
        smatch message;                                         // Inizializzo la variabile dove salvare i matchs rispetto alla regex
        regex_search(fullMessage, message, regexId);            // Uso la funzione search per cercare testi che rispettano l'espressione regolare
        idMessage += message[1];                                // Il primo match identifica l'ID del messaggio
        validMessage += message[2];                             // Il secondo match identifica il messaggio da inviare al guanto
                                                                // Eventualmente avrei potuto ottenere un terzo match che mi identificava la tipologia di messaggio w/r


        // Ciclo le lettere presenti nel buffer
        for (int i = 0; i < validMessage.size(); i++) {
            
            char c = validMessage[i];                           // Seleziono la lettera del buffer in posizione i
            
            if (isalpha(c)) {                                   // Verifico se è una lettera
                valueToGuanto(c);                               // Mandare segnale al guanto per far vibrare la lettera
                printf("%c", c);                                // Stampo lettera in console
                recivedValidMessage = true;                     // Essendoci almeno una lettera dichiaro il messaggio valido
                printf("\n");
                ThisThread::sleep_for(500ms);                   // Attendo mezzo secondo prima di far vibrare la lettera successiva

            } else if (c == ' ') {                              // Verifico se è uno spazio
                ThisThread::sleep_for(1s);                      // Attendo 1 secondo tra una parola ed un'altra

            } else if (c == ',') {                              // Verifico se è una virgola
                ThisThread::sleep_for(2s);                      // Attendo 2 secondi di pausa per la virgola

            } else if (c == '.') {                              // Verifico se è un punto
                ThisThread::sleep_for(4s);                      // Attendo 4 secondi di pausa tra una frase ed un'altra
            }
            
        }
        
        //Al termine della lettura del messaggio arrivato dall'app, invio un messaggio di avvenuta lettura all'app, inviando l'ID del messaggio letto
        if (recivedValidMessage && bluetooth.writable()) {
            string readedMessage = "<r id=" + idMessage + "></r>";          // Costruisco il messaggio da inviare con un TAG di tipo r e l'identificativo del messaggio
            pc.write(readedMessage.c_str(), readedMessage.size());          // Stpampo in console ciò che invio
            bluetooth.write(readedMessage.c_str(), readedMessage.size());   // Invio il messaggio all'app tramite il modulo bluetooth
            outputMessage = "";                                             // Pulisco le variabili globali 
            ThisThread::sleep_for(500ms);                                   // Attendo mezzo secondo
        }
    }
}

int main() {
    bluetooth.set_baud(9600);           // Imposto il baud rate per la comunicazione UART del modulo bluetooth
    pc.set_baud(9600);                  // Imposto il baud rate anche sul pc

    aSensor.period(1.0 / frequency);    // Set the PWM period based on the desired frequency
    //bSensor.period(1.0 / frequency);
    cSensor.period(1.0 / frequency);    // Set the PWM period based on the desired frequency
    dSensor.period(1.0 / frequency);    // Set the PWM period based on the desired frequency
    eSensor.period(1.0 / frequency);    // Set the PWM period based on the desired frequency
    //fSensor.period(1.0 / frequency); 
    gSensor.period(1.0 / frequency);    // Set the PWM period based on the desired frequency
    //hSensor.period(1.0 / frequency); 
    iSensor.period(1.0 / frequency);    // Set the PWM period based on the desired frequency
    lSensor.period(1.0 / frequency);    // Set the PWM period based on the desired frequency
    mSensor.period(1.0 / frequency);    // Set the PWM period based on the desired frequency
    nSensor.period(1.0 / frequency);    // Set the PWM period based on the desired frequency
    oSensor.period(1.0 / frequency);    // Set the PWM period based on the desired frequency
    pSensor.period(1.0 / frequency);    // Set the PWM period based on the desired frequency
    //qSensor.period(1.0 / frequency); 
    rSensor.period(1.0 / frequency);    // Set the PWM period based on the desired frequency
    sSensor.period(1.0 / frequency);    // Set the PWM period based on the desired frequency
    tSensor.period(1.0 / frequency);    // Set the PWM period based on the desired frequency
    uSensor.period(1.0 / frequency);    // Set the PWM period based on the desired frequency
    vSensor.period(1.0 / frequency);    // Set the PWM period based on the desired frequency
    //zSensor.period(1.0 / frequency); 


    
    write("Bluetooth initialized");     //Test di scrittura dal guanto all'app

    while(1){
        writeValueIfNeed();
        readValueIfNeed();
        ledScrittura = !outputMessage.empty();
    }

}
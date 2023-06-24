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

#define MAX_BUFFER_SIZE 200

BufferedSerial bluetooth(D8, D2); // RX, TX pins for HC-10 Bluetooth module
BufferedSerial pc(USBTX, USBRX); // RX, TX pins for pc

//Input
AnalogIn aPiezo(A0);
AnalogIn bPiezo(A1);
AnalogIn cPiezo(A2);
AnalogIn dPiezo(A2);
AnalogIn ePiezo(A2);
AnalogIn fPiezo(A2);
AnalogIn gPiezo(A2);
AnalogIn hPiezo(A2);
AnalogIn iPiezo(A2);
AnalogIn lPiezo(A2);
AnalogIn mPiezo(A2);
AnalogIn nPiezo(A2);
AnalogIn oPiezo(A2);
AnalogIn pPiezo(A2);
AnalogIn qPiezo(A2);
AnalogIn rPiezo(A2);
AnalogIn sPiezo(A2);
AnalogIn tPiezo(A2);
AnalogIn uPiezo(A2);
AnalogIn vPiezo(A2);
AnalogIn zPiezo(A2);

//TODO: Sostituire i led 
PwmOut aSensor(D3);
PwmOut bSensor(D5);
PwmOut cSensor(D6);
PwmOut dSensor(D6);
PwmOut eSensor(D6);
PwmOut fSensor(D6);
PwmOut gSensor(D6);
PwmOut hSensor(D6);
PwmOut iSensor(D6);
PwmOut lSensor(D6);
PwmOut mSensor(D6);
PwmOut nSensor(D6);
PwmOut oSensor(D6);
PwmOut pSensor(D6);
PwmOut qSensor(D6);
PwmOut rSensor(D6);
PwmOut sSensor(D6);
PwmOut tSensor(D6);
PwmOut uSensor(D6);
PwmOut vSensor(D6);
PwmOut zSensor(D6);

DigitalIn buttonInvia(BUTTON1);

float frequency = 200; // Vibration frequency in Hz
float dutyCycle = 0.5; // Duty cycle (0.0 to 1.0)

string outputMessage = "";

Ticker timer;

bool isWriting = false;

//SOS: 


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
    string messageId = gen_random(4);                               //Genero un id randomico formato da 4 caratteri alfanumerici
    string welcomeMessage = "<w id=" + messageId + ">";             //Mi costruisco il tag di tipo write con l'id del messaggio <w id=000000>
    welcomeMessage += messageStr;                                   //Inserisco il contenuto del messaggio nel tag <w id=000000>Messagio da inviare
    welcomeMessage += "</w>";                                       //Chiudo il tag <w id=000000>Messagio da inviare</w>
    bluetooth.write(welcomeMessage.c_str(), welcomeMessage.size()); //Invio del messaggio al modulo bluetooth:  
                                                                        //buffer: converto la stringa in un buffer con la funzione c_str(),
                                                                        //length: recupeando la dimensione del buffer con la funzione size()
    pc.write(welcomeMessage.c_str(), welcomeMessage.size());        //Invio del messaggio al pc per stamparlo in console 
}



//prendere l'input dal guanto e mandarlo al bluethoot
void seperaParole() { 
    if (outputMessage.at(outputMessage.length() - 1) != ' ') {
        outputMessage += " ";
    }
}

void resetTicker() {
    isWriting = true;
    timer.attach(seperaParole, 4000ms);
}

void writeValueIfNeed() { //funzione per la scrittura dal guanto all'app
    
    if (buttonInvia == 0 && !outputMessage.empty()) {
        //TODO: Se premuto più di 3 secondi -> Invio stringa "SOS"
        timer.detach();
        string temp = outputMessage;
        write(temp);
        ThisThread::sleep_for(400ms);
        outputMessage = "";
        isWriting = false;
        return;
    }

    if (aPiezo > 0.75f) {    //0.75 è la soglia che identifica l'input della persona sordocieca
        outputMessage += 'a';
        printf("A value: %f\n", aPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    } 
    
    if (bPiezo > 0.75f) {
        outputMessage += 'b';
        printf("B value: %f\n", bPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    } 


    if (cPiezo > 0.75f) {
        outputMessage += 'c';
        printf("C value: %f\n", cPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    
    if (dPiezo > 0.75f) {
        outputMessage += 'd';
        printf("D value: %f\n", dPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    
    if (ePiezo > 0.75f) {
        outputMessage += 'e';
        printf("E value: %f\n", ePiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    
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
    
    if (iPiezo > 0.75f) {
        outputMessage += 'i';
        printf("I value: %f\n", iPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    
    if (lPiezo > 0.75f) {
        outputMessage += 'l';
        printf("L value: %f\n", lPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    
    if (mPiezo > 0.75f) {
        outputMessage += 'm';
        printf("M value: %f\n", mPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    
    if (nPiezo > 0.75f) {
        outputMessage += 'n';
        printf("N value: %f\n", nPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    
    if (oPiezo > 0.75f) {
        outputMessage += 'o';
        printf("O value: %f\n", oPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    
    if (pPiezo > 0.75f) {
        outputMessage += 'p';
        printf("P value: %f\n", pPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    
    if (qPiezo > 0.75f) {
        outputMessage += 'q';
        printf("Q value: %f\n", qPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    
    if (rPiezo > 0.75f) {
        outputMessage += 'r';
        printf("R value: %f\n", rPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    
    if (sPiezo > 0.75f) {
        outputMessage += 's';
        printf("S value: %f\n", sPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    
    if (tPiezo > 0.75f) {
        outputMessage += 't';
        printf("T value: %f\n", tPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    
    if (uPiezo > 0.75f) {
        outputMessage += 'u';
        printf("U value: %f\n", uPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    
    if (vPiezo > 0.75f) {
        outputMessage += 'v';
        printf("V value: %f\n", vPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
    
    if (zPiezo > 0.75f) {
        outputMessage += 'z';
        printf("Z value: %f\n", zPiezo.read()*1.0f);
        resetTicker();
        ThisThread::sleep_for(400ms);
    }
  

}

void valueToGuanto(char c) { //funzione per la lettura dall'app al guanto

    if (c == 'a' || c == 'A') { //ogni if fa vibrare il vibration motor della lettera corrispondente
        aSensor.resume();
        aSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        aSensor.write(0);
        aSensor.suspend();
    } else if (c == 'b' || c == 'B') {
        bSensor.resume();
        bSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        bSensor.write(0);
        bSensor.suspend();
    }else if (c == 'c' || c == 'C') {
        cSensor.resume();
        cSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        cSensor.write(0);
        cSensor.suspend();
    }else if (c == 'd' || c == 'D') {
        dSensor.resume();
        dSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        dSensor.write(0);
        dSensor.suspend();
    }else if (c == 'e' || c == 'E') {
        eSensor.resume();
        eSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        eSensor.write(0);
        eSensor.suspend();
    }else if (c == 'f' || c == 'F') {
        fSensor.resume();
        fSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        fSensor.write(0);
        fSensor.suspend();
    }else if (c == 'g' || c == 'G') {
        gSensor.resume();
        gSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        gSensor.write(0);
        gSensor.suspend();
    }else if (c == 'h' || c == 'H') {
        hSensor.resume();
        hSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        hSensor.write(0);
        hSensor.suspend();
    }else if (c == 'i' || c == 'I') {
        iSensor.resume();
        iSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        iSensor.write(0);
        iSensor.suspend();
    }else if (c == 'l' || c == 'L') {
        lSensor.resume();
        lSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        lSensor.write(0);
        lSensor.suspend();
    }else if (c == 'm' || c == 'M') {
        mSensor.resume();
        mSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        mSensor.write(0);
        mSensor.suspend();
    }else if (c == 'n' || c == 'N') {
        nSensor.resume();
        nSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        nSensor.write(0);
        nSensor.suspend();
    }else if (c == 'o' || c == 'O') {
        oSensor.resume();
        oSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        oSensor.write(0);
        oSensor.suspend();
    }else if (c == 'p' || c == 'P') {
        pSensor.resume();
        pSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        pSensor.write(0);
        pSensor.suspend();
    }else if (c == 'q' || c == 'Q') {
        qSensor.resume();
        qSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        qSensor.write(0);
        qSensor.suspend();
    }else if (c == 'r' || c == 'R') {
        rSensor.resume();
        rSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        rSensor.write(0);
        rSensor.suspend();
    }else if (c == 's' || c == 'S') {
        sSensor.resume();
        sSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        sSensor.write(0);
        sSensor.suspend();
    }else if (c == 't' || c == 'T') {
        tSensor.resume();
        tSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        tSensor.write(0);
        tSensor.suspend();
    }else if (c == 'u' || c == 'U') {
        uSensor.resume();
        uSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        uSensor.write(0);
        uSensor.suspend();
    }else if (c == 'v' || c == 'V') {
        vSensor.resume();
        vSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        vSensor.write(0);
        vSensor.suspend();
    }else if (c == 'z' || c == 'Z') {
        zSensor.resume();
        zSensor.write(dutyCycle);
        ThisThread::sleep_for(500ms);
        zSensor.write(0);
        zSensor.suspend();
    }



    
    cSensor = (c == 'c' || c == 'C');
    dSensor = (c == 'd' || c == 'D');
    eSensor = (c == 'e' || c == 'E');
    fSensor = (c == 'f' || c == 'F');
    gSensor = (c == 'g' || c == 'G');
    hSensor = (c == 'h' || c == 'H');
    iSensor = (c == 'i' || c == 'I');
    lSensor = (c == 'l' || c == 'L');
    mSensor = (c == 'm' || c == 'M');
    nSensor = (c == 'n' || c == 'N');
    oSensor = (c == 'o' || c == 'O');
    pSensor = (c == 'p' || c == 'P');
    qSensor = (c == 'q' || c == 'Q');
    rSensor = (c == 'r' || c == 'R');
    sSensor = (c == 's' || c == 'S');
    tSensor = (c == 't' || c == 'T');
    uSensor = (c == 'u' || c == 'U');
    vSensor = (c == 'v' || c == 'V');
    zSensor = (c == 'z' || c == 'Z');
}

// Ricevere dati dal modulo Bluetooth

void readValueIfNeed() {
    //inizializzo il buffer dove poter memorizzare temporaneamente la frase ricevuta dal modulo bluetooth
    char buffer[MAX_BUFFER_SIZE] = {};

    bool recivedValidMessage = false;
    string fullMessage = "";
    string validMessage = "";
    string idMessage = "";

   //ThisThread::sleep_for(500ms);

    //Verifico che il modulo bluetooth sia collegato correttamente e in modalità lettura abilitata
    if(bluetooth.readable() && !isWriting){
        //Leggo una frase che ho ricevuto dal modulo bluetooth
        bluetooth.read(&buffer, MAX_BUFFER_SIZE);

        //Recupero il dato completo
        for (int i = 0; i < MAX_BUFFER_SIZE; i++) {
            char c = buffer[i];
            fullMessage += c;
        }

        //Recupero l'ID del messaggio
        regex regexId(".id=([^>]+)>([^<]+).");
        smatch message; 
        regex_search(fullMessage, message, regexId); 
        idMessage += message[1];
        validMessage += message[2];


        //Ciclo le lettere presenti nel buffer
        for (int i = 0; i < validMessage.size(); i++) {
            //Seleziono la lettera del buffer in posizione i
            char c = validMessage[i];
            
            if (isalpha(c)) {   //Verifico se è una lettera
                //mandare segnale al guanto per far vibrare la lettera
                valueToGuanto(c);
                printf("%c", c);
                recivedValidMessage = true;

                printf("\n");
                ThisThread::sleep_for(500ms);
            } else if (c == ' ') {  //Verifico se è uno spazio
                //attendo 1 secondo tra una parola ed un'altra
                ThisThread::sleep_for(1s);
            } else if (c == ',') {  //Verifico se è una virgola
                //attendo 2 secondi di pausa per la virgola
                ThisThread::sleep_for(2s);
            } else if (c == '.') {  //Verifico se è un punto
                //attendo 4 secondi di pausa tra una frase ed un'altra
                ThisThread::sleep_for(4s);
            }
            
        }
        

        if (recivedValidMessage && bluetooth.writable()) {
            string readedMessage = "<r id=" + idMessage + "></r>";
            pc.write(readedMessage.c_str(), readedMessage.size());
            bluetooth.write(readedMessage.c_str(), readedMessage.size());
            outputMessage = "";
            ThisThread::sleep_for(500ms);
        }
    }
}

int main() {
    bluetooth.set_baud(9600);
    pc.set_baud(9600);

    aSensor.period(1.0 / frequency); // Set the PWM period based on the desired frequency
    aSensor.suspend();

    bSensor.period(1.0 / frequency);
    bSensor.suspend();

    /*TODO: Dato che il modulo HM-10 invia un massimo di 20 caratteri per volta, bisogna capire come impacchettare i messaggi, 
    in modo che l'app riesca a capire dove inizia e dove finisce il mesaggio, e renderizzare un'unica nuvoletta. 
    
    Sto immaginando di implementare due TAG <r>MessageUUID</r> e <w id=MessageUUID>Messaggio</w> 
    per indicare se il messaggio che sto inviando è una notifica di lettura oppure è un messaggio da stampare nella chat
    ed aggiungere la proprietà id in modo da poter rendere univoci i messaggi inviati e gestirli correttamente ambo i lati
    */

    //Test di scrittura dal guanto all'app
    write("Bluetooth initialized");

    while(1){
        writeValueIfNeed();
        readValueIfNeed();
    }

}
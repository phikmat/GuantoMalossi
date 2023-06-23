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
DigitalOut cSensor(D6);
DigitalOut dSensor(D6);
DigitalOut eSensor(D6);
DigitalOut fSensor(D6);
DigitalOut gSensor(D6);
DigitalOut hSensor(D6);
DigitalOut iSensor(D6);
DigitalOut lSensor(D6);
DigitalOut mSensor(D6);
DigitalOut nSensor(D6);
DigitalOut oSensor(D6);
DigitalOut pSensor(D6);
DigitalOut qSensor(D6);
DigitalOut rSensor(D6);
DigitalOut sSensor(D6);
DigitalOut tSensor(D6);
DigitalOut uSensor(D6);
DigitalOut vSensor(D6);
DigitalOut zSensor(D6);

DigitalIn buttonInvia(BUTTON1);

float frequency = 200;
float dutyCycle = 0.5;

float frequencyMini = 1000;
float dutyCycleMini = 0.5;

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
    string messageId = gen_random(6);                               //Genero un id randomico formato da 6 caratteri alfanumerici
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

void writeValueIfNeed() {
    
    if (buttonInvia == 0 && !outputMessage.empty()) {
        //Se premuto più di 3 secondi -> Invio stringa "SOS"
        timer.detach();
        string temp = outputMessage;
        write(temp);
        ThisThread::sleep_for(400ms);
        outputMessage = "";
        isWriting = false;
        return;
    }

    if (aPiezo > 0.75f) {
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

}

void valueToGuanto(char c) {

    if (c == 'a' || c == 'A') {
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

    //Verifico che il modulo bluetooth sia in collegato correttamente e in modalità lettura abilitata
    if(bluetooth.readable() && !isWriting){
        //Leggo una frase che ho ricevuto dal modulo bluetooth
        bluetooth.read(&buffer, MAX_BUFFER_SIZE);

        //Recupero il dato completo
        for (int i = 0; i < MAX_BUFFER_SIZE; i++) {
            char c = buffer[i];
            fullMessage += c;
        }

        //Recupero l'ID del messaggio
        regex regexId(".*id=([^>]+)>([^<]+).*");
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
                //attendo 2 secondo di pausa per la virgola
                ThisThread::sleep_for(2s);
            } else if (c == '.') {  //Verifico se è un punto
                //attendo 4 secondo di pausa tra una frase ed un'altra
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

    aSensor.period(1.0 / frequency);
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



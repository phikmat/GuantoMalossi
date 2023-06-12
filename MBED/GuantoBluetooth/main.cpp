#include "PinNameAliases.h"
#include "PinNames.h"
#include "ThisThread.h"
#include "mbed.h"
#include "BufferedSerial.h"
#include "mbed_thread.h"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "string.h"
#include <regex> 

#define MAX_BUFFER_SIZE 200

BufferedSerial bluetooth(D8, D2); // RX, TX pins for HC-10 Bluetooth module
BufferedSerial pc(USBTX, USBRX); // RX, TX pins for pc

//TODO: Sostituire i led 
DigitalOut aSensor(LED1);
DigitalOut bSensor(D5);
DigitalOut cSensor(D5);
DigitalOut dSensor(D5);
DigitalOut eSensor(D5);
DigitalOut fSensor(D5);
DigitalOut gSensor(D5);
DigitalOut hSensor(D5);
DigitalOut iSensor(D5);
DigitalOut lSensor(D5);
DigitalOut mSensor(D5);
DigitalOut nSensor(D5);
DigitalOut oSensor(D5);
DigitalOut pSensor(D5);
DigitalOut qSensor(D5);
DigitalOut rSensor(D5);
DigitalOut sSensor(D5);
DigitalOut tSensor(D5);
DigitalOut uSensor(D5);
DigitalOut vSensor(D5);
DigitalOut zSensor(D5);

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


// Invia dati al modulo Bluetooth
void writeValue(char c[MAX_BUFFER_SIZE]) {

    bluetooth.write(&c,MAX_BUFFER_SIZE);
    ThisThread::sleep_for(1s);
}

//prendere l'input dal guanto e mandarlo al bluethoot
void writeValueIfNeed() {
    //TODO
}

void valueToGuanto(char c) {
    aSensor = (c == 'a' || c == 'A');
    bSensor = (c == 'b' || c == 'B');
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

   ThisThread::sleep_for(500ms);

    //Verifico che il modulo bluetooth sia in collegato correttamente e in modalità lettura abilitata
    if(bluetooth.readable()){
        //Leggo una frase che ho ricevuto dal modulo bluetooth
        bluetooth.read(&buffer, MAX_BUFFER_SIZE);

        //Recupero il dato completo
        for (int i = 0; i < MAX_BUFFER_SIZE; i++) {
            char c = buffer[i];
            fullMessage += c;
        }

        //Recupero l'ID del messaggio
        regex regexId(".*id=([^>]+)>(\\w+).*");
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
            string readedMessage = "<read id=" + idMessage + ">" + validMessage + "</read>";
            pc.write(readedMessage.c_str(), readedMessage.size());
            bluetooth.write(readedMessage.c_str(), readedMessage.size());
        }
    }
}

int main() {
    bluetooth.set_baud(9600);
    pc.set_baud(9600);

    /*TODO: Dato che il modulo HM-10 invia un massimo di 20 caratteri per volta, bisogna capire come impacchettare i messaggi, 
    in modo che l'app riesca a capire dove inizia e dove finisce il mesaggio, e renderizzare un'unica nuvoletta. 
    
    Sto immaginando di implementare due TAG <read>MessageUUID</read> e <write id=MessageUUID>Messaggio</write> 
    per indicare se il messaggio che sto inviando è una notifica di lettura oppure è un messaggio da stampare nella chat
    ed aggiungere la proprietà id in modo da poter rendere univoci i messaggi inviati e gestirli correttamente ambo i lati
    */

    //Test di scrittura dal guanto all'app
    string uuid = gen_random(32);
    string welcomeMessage = "<write id=" + uuid + ">";
    welcomeMessage += "Bluetooth initialization ended!";
    welcomeMessage += "</write>";
    bluetooth.write(welcomeMessage.c_str(), welcomeMessage.size());
    

    while(1){
        readValueIfNeed();
        writeValueIfNeed();
    }

}



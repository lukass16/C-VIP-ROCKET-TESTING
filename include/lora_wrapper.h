#pragma once
#include <LoRa.h>
#include <SPI.h>

// https://randomnerdtutorials.com/ttgo-lora32-sx1276-arduino-ide/
// https://github.com/sandeepmistry/arduino-LoRa/blob/master/examples/LoRaDuplex/LoRaDuplex.ino

namespace lora {

    #define SCK 5
    #define MISO 19
    #define MOSI 27
    #define SS 18
    #define RST 14
    #define DIO0 26

    String outgoing; // outgoing message

    int syncWord = 0xF3;

    byte _localAddress = 0xFF; // address of this device
    byte _destination = 0xFF;  // destination to send to
    boolean _canPrintHeaderMessage = false;

    void sendMessage(String outgoing, int lora_message_id);

    void setup(double frequency = 868E6, boolean canPrintHeaderMessage = false)
    {
        _canPrintHeaderMessage = canPrintHeaderMessage;

        //SPI LoRa pins
        SPI.begin(SCK, MISO, MOSI, SS);
        //setup LoRa transceiver module
        LoRa.setPins(SS, RST, DIO0);

        if (!LoRa.begin(frequency))
        {
            Serial.println("Starting LoRa failed!");
            while (1);
        }

        //setting paramaters
        LoRa.setSyncWord(0xF3);
        LoRa.setTxPower(14);
        LoRa.setSpreadingFactor(10);
        LoRa.setCodingRate4(6);
        LoRa.setSignalBandwidth(62.5E3);

        Serial.println("Lora connected");
        Serial.println(LoRa.available());
    }

    String readMessage(){
        int packetSize = LoRa.parsePacket();
        String message = "";
        if(packetSize){
            // read packet header bytes:
            int recipient = LoRa.read();          // recipient address
            byte sender = LoRa.read();            // sender address
            byte incomingMsgId = LoRa.read();     // incoming msg ID
            byte incomingLength = LoRa.read();    // incoming msg length

            if (recipient != _localAddress && recipient != 0xFF) {
                Serial.println("This message is not for me, recipient: " + String(recipient));
                return "NULL";                            
            }   
            while(LoRa.available()){
                message += (char)LoRa.read();
            }
            Serial.print("Message: " + message);
        }
        else {
            return "NULL";
        }
        Serial.println(" RSSI: " + String(LoRa.packetRssi()));
        return message;
    }

    void sendMessage(String outgoing, int lora_message_id)
    {
        LoRa.beginPacket();            // start packet
        LoRa.write(_destination);      // add destination address
        LoRa.write(_localAddress);     // add sender address
        LoRa.write(lora_message_id);   // add message ID
        LoRa.write(outgoing.length()); // add payload length
        LoRa.print(outgoing);          // add payload
        LoRa.endPacket();              // finish packet and send it
    }

    int getPacketRssi()
    {
        return LoRa.packetRssi();
    }

    void test()
    {
        String msg = "";
        for (int i = 0; i < 4; i++)
        {
            switch (i)
            {
            case 1:
                msg = "First LoRa message";
                break;
            case 2:
                msg = "Second LoRa message";
                break;
            case 3:
                msg = "Third LoRa message";
                break;
            default:
                msg = "Default LoRa message";
            }
        }
        Serial.println("Sending message: " + msg);
        sendMessage(msg, 0);
    }

}

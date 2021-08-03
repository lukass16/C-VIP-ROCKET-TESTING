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


    String outgoing;              // outgoing message

    int syncWord = 0xF3;

    byte _localAddress = 0xFF;     // address of this device
    byte _destination = 0xBB;      // destination to send to
    long lastSendTime = 0;        // last send time
    int interval = 2000;          // interval between sends
    boolean _canPrintHeaderMessage = false;

    void sendMessage(String outgoing, int lora_message_id);
    void onReceive(int packetSize);

    void setup(double frequency = 868E6, boolean canPrintHeaderMessage = false){
        
        _canPrintHeaderMessage = canPrintHeaderMessage;


        //SPI LoRa pins
        SPI.begin(SCK, MISO, MOSI, SS);
        //setup LoRa transceiver module
        LoRa.setPins(SS, RST, DIO0);
            
        if (!LoRa.begin(frequency)) {
            Serial.println("Starting LoRa failed!");
            while (1);
        }


        //setting paramaters

        LoRa.setSyncWord(0xF3);
        LoRa.setTxPower(20);
        LoRa.setSpreadingFactor(9);
        LoRa.setCodingRate4(7);
        LoRa.setSignalBandwidth(31.25E3);
                

        Serial.println("Lora connected");
        Serial.println(LoRa.available());
    }

    void readMessage() {
        onReceive(LoRa.parsePacket());
    }

    void sendMessage(String outgoing, int lora_message_id) {
        LoRa.beginPacket();                   // start packet
        LoRa.write(_destination);             // add destination address
        LoRa.write(_localAddress);            // add sender address
        LoRa.write(lora_message_id);          // add message ID
        LoRa.write(outgoing.length());        // add payload length
        LoRa.print(outgoing);                 // add payload
        LoRa.endPacket();                     // finish packet and send it
    }

    int getPacketRssi()
    {
        return LoRa.packetRssi();
    }


    void onReceive(int packetSize) {
        if (packetSize == 0) return;          // if there's no packet, return

        // read packet header bytes:
        int recipient = LoRa.read();          // recipient address
        byte sender = LoRa.read();            // sender address
        byte incomingMsgId = LoRa.read();     // incoming msg ID
        byte incomingLength = LoRa.read();    // incoming msg length

        String incoming = "";
        while (LoRa.available()) {
            incoming += (char)LoRa.read();
        }

        if (incomingLength != incoming.length()) {   // check length for error
            Serial.println("error: message length does not match length");
            return;                             // skip rest of function
        }

        //   if the recipient isn't this device or broadcast,
        if (recipient != _localAddress && recipient != 0xFF) {
            Serial.println("This message is not for me.");
            return;                             // skip rest of function
        }

        // if message is for this device, or broadcast, print details:
        if(_canPrintHeaderMessage) {
            Serial.println("Received from: 0x" + String(sender, HEX));
            Serial.println("Sent to: 0x" + String(recipient, HEX));
            Serial.println("Message ID: " + String(incomingMsgId));
            Serial.println("Message length: " + String(incomingLength));
        }

        Serial.println("Message: " + incoming);
        // Serial.println("RSSI: " + String(LoRa.packetRssi()));
        // Serial.println("Snr: " + String(LoRa.packetSnr()));
        Serial.println();
    }

    void test() {
        String msg = "";
        for(int i = 0; i < 4; i++)
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

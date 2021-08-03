
// #include "states/test_state.cpp"
// #include "states/i2c_state.cpp"
#include "states/preperation_state.cpp"


// int t, lastSendTime;
Context *mode = new Context(new PreperationState);
// Context *mode = new Context(new I2cState);
//Context *mode = new Context(new TestState);

void setup() {
    Serial.begin(115200);

    mode->Start();
}

void loop() {
    
}



#include <Homie.h>

#define FW_NAME "sonoff-s20-firmware"
#define FW_VERSION "1.0.0"

const int RELAY_PIN = 12;
const int LED_PIN = 13;
const int BUTTON_PIN = 0;
const int BUTTON_SHORT_PRESS = 300;
const int BUTTON_LONG_PRESS = 5000;

const int OFF = 0;
const int ON = 1;

int plugState = OFF;
unsigned long millisSinceChange = 0;
unsigned long millisSincePress = 0;
bool buttonStateHasJustChanged = false;
bool flaggedForReset = false;

HomieNode plugNode("plug", "switch");

void on() {
    digitalWrite(RELAY_PIN, HIGH);
    plugState = ON;
    if (Homie.isConnected()) {
        plugNode.setProperty("state").send("on");
    }
    Homie.getLogger() << "switch on plug..." << endl;
}

void off() {
    digitalWrite(RELAY_PIN, LOW);
    plugState = OFF;
    if (Homie.isConnected()) {
        plugNode.setProperty("state").send("off");
    }
    Homie.getLogger() << "switch off plug..." << endl;

}

bool plugStateHandler(const HomieRange &range, const String &value) {
    if (value == "on") {
        on();
    } else if (value == "off") {
        off();
    } else {
        return false;
    }
    return true;
}

void handleButton() {
    if (digitalRead(BUTTON_PIN) == HIGH) {
        buttonStateHasJustChanged = false;
        millisSincePress = millis();
    }

    if (digitalRead(BUTTON_PIN) == LOW && (millis() - millisSinceChange) > BUTTON_SHORT_PRESS &&
        !buttonStateHasJustChanged) {
        millisSinceChange = millis();
        plugState == ON ? off() : on();
        buttonStateHasJustChanged = true;
    } else if (digitalRead(BUTTON_PIN) == LOW && (millis() - millisSincePress) > BUTTON_LONG_PRESS &&
               !flaggedForReset) {
        Homie.getLogger() << "reset by button long press..." << endl;
        flaggedForReset = true;
        Homie.reset();
    }
}

void onHomieEvent(const HomieEvent &event) {
    switch (event.type) {
        case HomieEventType::CONFIGURATION_MODE:
            digitalWrite(LED_PIN, LOW);
            break;
        case HomieEventType::MQTT_READY:
            digitalWrite(LED_PIN, HIGH);
            break;
        default:
            break;
    }
}

void setup() {
    Serial.begin(115200);
    Serial << endl << endl;

    pinMode(RELAY_PIN, OUTPUT);

    off();

    Homie_setFirmware(FW_NAME, FW_VERSION);
    Homie.setLedPin(LED_PIN, LOW);
    Homie.onEvent(onHomieEvent);
    plugNode.advertise("state").settable(plugStateHandler);
    Homie.setup();
}

void loop() {
    handleButton();
    Homie.loop();
    yield();
}

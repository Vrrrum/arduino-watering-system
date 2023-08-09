#include <Arduino.h>
#line 1 "/home/kamil/Projects/Podlewanie/Podlewanie.ino"
// Podlewanie - 10.07.2023
#include "ArduinoSTL.h"
#include <ezButton.h>
// #include "section.h"

using namespace std;

#define DEFAULT_WORK_TIME 1000
#define DEFAULT_TRANSITION_TIME 1000

class Section {
  private:
    bool state = false;
    bool isActive = false;
    bool worksAtRain = false;
    uint8_t pin;
    uint8_t button;

    uint32_t savedWorkTime = 0;
    uint32_t savedTransitionTime = 0;
    uint32_t workTime;
    uint32_t transitionTime;

    void on() {
      digitalWrite(pin, HIGH);
    }
    void off() {
      digitalWrite(pin, LOW);
    }
  public:
    Section(uint8_t ledPin, uint8_t btn, uint32_t wt, uint32_t tt, bool wat = false) {
      pin = ledPin;
      button = btn;
      workTime = wt;
      transitionTime = tt;
      worksAtRain = wat;
    }

    bool getState() {
      return state;
    }
    void setWorksAtRain(bool value) {
      worksAtRain = value;
    }

    void updateState() {
      // printf("state: %d - %d", button, digitalRead(button));
      if(digitalRead(button) == LOW)
        state = true;
      else
        state = false;
    }
    void startSection() {
      savedWorkTime = millis();
      isActive = true;
      while(savedWorkTime < millis() - workTime) {
        on();
      }
    }
    void stopSection() {
      isActive = false;
      off();
    }
    void transition(Section &nextSection) {
      savedTransitionTime = millis();
      while(millis() - savedTransitionTime < transitionTime) {
        on();
        nextSection.on();
      }
      stopSection();
    }
    void sectionLoop(Section *nextSection, uint8_t &pos) {
      if(!isActive) {
        savedWorkTime = millis();
        printf("savedWorkTime: %d", savedWorkTime);
        isActive = true;
      }
      if(millis() - savedWorkTime > workTime || (isActive && !state)) {
        pos++;
        if(nextSection->getState())
          transition(*nextSection);
      } else {
        on();
      }
    }
    void sectionLoop(bool &relay) {
      if(!isActive) {
        savedWorkTime = millis();
        isActive = true;
      }
      if(millis() - savedWorkTime < workTime) {
        on();
      } else {
        stopSection();
        end(relay);
      }
    }
    void end(bool &relay) {
      relay = LOW;
    }
};

void checkStates(vector<Section> &sections);
void updatePool(vector<Section> &sections, vector<Section*> &ap);
void end(bool &relay);

Section kropleSzopa(10, 1, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);
Section skalniak(3, A0, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME, true);
Section trawaZachod(4, A1, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);
Section trawaSrodek(5, A2, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);
Section trawaWschod(6, A3, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);
Section trawaPrzodWschod(7, A4, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);
Section trawaPrzodZachod(8, A5, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);
Section rabatkaPrzod(9, 0, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);
Section skwerBronka(11, 2, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);

vector<Section> wateringSystem{skalniak, trawaZachod, trawaSrodek, trawaWschod, trawaPrzodWschod, trawaPrzodZachod, rabatkaPrzod, kropleSzopa, skwerBronka};
vector<Section*> activePool;

bool relayStatus = HIGH;
uint8_t position = 0;

#line 123 "/home/kamil/Projects/Podlewanie/Podlewanie.ino"
void setup();
#line 155 "/home/kamil/Projects/Podlewanie/Podlewanie.ino"
void loop();
#line 123 "/home/kamil/Projects/Podlewanie/Podlewanie.ino"
void setup()
{
  Serial.begin(9600);

  pinMode(A0, INPUT_PULLUP); // Btn - sekcja1 skalniak
  pinMode(A1, INPUT_PULLUP); // Btn - sekcja2 trawa zachod
  pinMode(A2, INPUT_PULLUP); // Btn - sekcja3 trawa srodek
  pinMode(A3, INPUT_PULLUP); // Btn - sekcja4 trawa wschod
  pinMode(A4, INPUT_PULLUP); // Btn - sekcja5 trawaPrzód wschod
  pinMode(A5, INPUT_PULLUP); // Btn - sekcja6 trawaPrzód zachod
  pinMode(0, INPUT_PULLUP);  // Btn - sekcja7 kropleOczkoPrzód
  pinMode(1, INPUT_PULLUP);  // Btn - sekcja8 kropleTrawaSzopa
  pinMode(2, INPUT_PULLUP);  // Btn - sekcja9 kropleBronek
  pinMode(3, OUTPUT);        // skalniak
  pinMode(4, OUTPUT);        // trawa sekcja 1 (AnnaMaria)
  pinMode(5, OUTPUT);        // trawa sekcja 2 (środek)
  pinMode(6, OUTPUT);        // trawa sekcja 3 (Sejmowa)
  pinMode(7, OUTPUT);        // trawa przód wschód
  pinMode(8, OUTPUT);        // trawa przód zachód
  pinMode(9, OUTPUT);        // rabata sekcja 1 (oczko z przodu)
  pinMode(13, OUTPUT);       // przekaźnik podtrzymujący
  pinMode(10, OUTPUT);       // trawa sekcja 4 (węże pod szopą)
  pinMode(11, OUTPUT);       // rabata sekcja 2 (skwer Bronka)
  pinMode(12, INPUT_PULLUP); // czujnik opadów

  updatePool(wateringSystem, activePool);

  Serial.println("Start");
  digitalWrite(13, relayStatus);
  delay(3000);
}
  
void loop() 
{
  digitalWrite(13, relayStatus);
  checkStates(wateringSystem);

  if(digitalRead(12) == HIGH) {
    end(relayStatus);
  }  

  if(position == activePool.size()-1) {
    activePool.at(position)->sectionLoop(relayStatus);
  } else {
    activePool.at(position)->sectionLoop(activePool.at(position+1), position);
  }
}

void checkStates(vector<Section> &sections) {
  for(Section &section : sections) {
    section.updateState();
  }
}

void updatePool(vector<Section> &sections, vector<Section*> &ap) {
  ap.clear();
  for(Section &section : sections) {
    section.updateState();
    if(section.getState()){
      ap.push_back(&section);
    }
  }
}

void end(bool &relay) {
  relay = LOW;
}

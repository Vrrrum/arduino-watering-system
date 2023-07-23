#include <Arduino.h>
#line 1 "/home/kamil/Projects/Podlewanie/Podlewanie.ino"
// Podlewanie - 10.07.2023
#include "ArduinoSTL.h"
#include <ezButton.h>

using namespace std;

#define DEFAULT_WORK_TIME 2000
#define DEFAULT_TRANSITION_TIME 1000

class TimerSection {
  private:
    bool state = false;
    bool isActive = false;
    uint8_t pin;
    uint8_t button;

    uint16_t savedWorkTime = 0;
    uint16_t savedTransitionTime = 0;
    uint16_t workTime;
    uint16_t transitionTime;

    TimerSection* nextSection = NULL;

    void on() {
      digitalWrite(pin, HIGH);
    }
    void off() {
      digitalWrite(pin, LOW);
    }
  public:
    TimerSection(uint8_t ledPin, uint8_t btn, uint16_t wt, uint16_t tt) {
      pin = ledPin;
      button = btn;
      workTime = wt;
      transitionTime = tt;
    }

    bool getState() {
      return state;
    }

    void setNextSection(TimerSection &next) {
      nextSection = &next;
    }

    void updateState() {
      printf("state: %d - %d", button, digitalRead(button));
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
    void transition(TimerSection &nextSection) {
      savedTransitionTime = millis();
      while(millis() - savedTransitionTime < transitionTime) {
        on();
        nextSection.on();
      }
      stopSection();
    }
    // void sectionLoop(TimerSection &nextSection) {
    //   if(!isActive) {
    //     savedWorkTime = millis();
    //     isActive = true;
    //   }
    //   if(millis() - savedWorkTime > workTime || (isActive && !state)) {
    //     transition(nextSection);
    //   } else {
    //     on();
    //   }
    // }
    // void sectionLoop(bool &relay) {
    //   if(!isActive) {
    //     savedWorkTime = millis();
    //     isActive = true;
    //   }
    //   if(millis() - savedWorkTime < workTime) {
    //     on();
    //   } else {
    //     stopSection();
    //     end(relay);
    //   }
    // }

    void sectionLoop(bool &relay) {
      if(!isActive) {
        savedWorkTime = millis();
        isActive = true;
      }
      if(millis() - savedWorkTime > workTime || (isActive && !state)) {
        if(nextSection == NULL){
          end(relay);
        }
        transition(*nextSection);
      } else {
        on();
      }
    }
    void end(bool &relay) {
      relay = LOW;
    }
};

void checkStates(vector<TimerSection> &sections);

TimerSection kropleSzopa(10, 1, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);
TimerSection skalniak(3, A0, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);
TimerSection trawaZachod(4, A1, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);
TimerSection trawaSrodek(5, A2, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);
TimerSection trawaWschod(6, A3, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);
TimerSection trawaPrzodWschod(7, A4, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);
TimerSection trawaPrzodZachod(8, A5, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);
TimerSection rabatkaPrzod(9, 0, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);
TimerSection skwerBronka(11, 2, DEFAULT_WORK_TIME, DEFAULT_TRANSITION_TIME);

vector<TimerSection> wateringSystem{skalniak, trawaZachod, trawaSrodek, trawaWschod, trawaPrzodWschod, trawaPrzodZachod, rabatkaPrzod, kropleSzopa, skwerBronka};

uint8_t position = 0;
bool relayStatus = HIGH;

#line 132 "/home/kamil/Projects/Podlewanie/Podlewanie.ino"
void setup();
#line 165 "/home/kamil/Projects/Podlewanie/Podlewanie.ino"
void loop();
#line 132 "/home/kamil/Projects/Podlewanie/Podlewanie.ino"
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

  Serial.println("Start");
  
  for(int i = 0; i < wateringSystem.size()-1; i++) {
    wateringSystem.at(i).setNextSection(wateringSystem.at(i+1));
  }
}
  
int i = 0;
void loop() 
{
  digitalWrite(13, relayStatus);
  checkStates(wateringSystem);

  int next = 1;

  if(digitalRead(12) == LOW) {
      // if(wateringSystem.size() > position + 1) {
      //   wateringSystem.at(position).sectionLoop(wateringSystem.at(position+1), position);
      // }
      // else {
      //   wateringSystem.at(position).sectionLoop(position, relayStatus);
      // }

      // while(i <= wateringSystem.size()) {
      //   while(wateringSystem[i+next].getState() == false) {
      //     next++;
      //   }
      //   i+=next;
      // }
      wateringSystem.at(i).sectionLoop(relayStatus);
  }
}

void checkStates(vector<TimerSection> &sections) {
  for(TimerSection &section : sections) {
    section.updateState();
  }
  kropleSzopa.updateState();
}

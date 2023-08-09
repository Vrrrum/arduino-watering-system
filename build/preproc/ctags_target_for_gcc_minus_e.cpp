# 1 "/home/kamil/Projects/Podlewanie/Podlewanie.ino"
// Podlewanie - 10.07.2023
# 3 "/home/kamil/Projects/Podlewanie/Podlewanie.ino" 2
# 4 "/home/kamil/Projects/Podlewanie/Podlewanie.ino" 2
// #include "section.h"

using namespace std;




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
      digitalWrite(pin, 0x1);
    }
    void off() {
      digitalWrite(pin, 0x0);
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
      if(digitalRead(button) == 0x0)
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
      relay = 0x0;
    }
};

void checkStates(vector<Section> &sections);
void updatePool(vector<Section> &sections, vector<Section*> &ap);
void end(bool &relay);

Section kropleSzopa(10, 1, 1000, 1000);
Section skalniak(3, A0, 1000, 1000, true);
Section trawaZachod(4, A1, 1000, 1000);
Section trawaSrodek(5, A2, 1000, 1000);
Section trawaWschod(6, A3, 1000, 1000);
Section trawaPrzodWschod(7, A4, 1000, 1000);
Section trawaPrzodZachod(8, A5, 1000, 1000);
Section rabatkaPrzod(9, 0, 1000, 1000);
Section skwerBronka(11, 2, 1000, 1000);

vector<Section> wateringSystem{skalniak, trawaZachod, trawaSrodek, trawaWschod, trawaPrzodWschod, trawaPrzodZachod, rabatkaPrzod, kropleSzopa, skwerBronka};
vector<Section*> activePool;

bool relayStatus = 0x1;
uint8_t position = 0;

void setup()
{
  Serial.begin(9600);

  pinMode(A0, 0x2); // Btn - sekcja1 skalniak
  pinMode(A1, 0x2); // Btn - sekcja2 trawa zachod
  pinMode(A2, 0x2); // Btn - sekcja3 trawa srodek
  pinMode(A3, 0x2); // Btn - sekcja4 trawa wschod
  pinMode(A4, 0x2); // Btn - sekcja5 trawaPrzód wschod
  pinMode(A5, 0x2); // Btn - sekcja6 trawaPrzód zachod
  pinMode(0, 0x2); // Btn - sekcja7 kropleOczkoPrzód
  pinMode(1, 0x2); // Btn - sekcja8 kropleTrawaSzopa
  pinMode(2, 0x2); // Btn - sekcja9 kropleBronek
  pinMode(3, 0x1); // skalniak
  pinMode(4, 0x1); // trawa sekcja 1 (AnnaMaria)
  pinMode(5, 0x1); // trawa sekcja 2 (środek)
  pinMode(6, 0x1); // trawa sekcja 3 (Sejmowa)
  pinMode(7, 0x1); // trawa przód wschód
  pinMode(8, 0x1); // trawa przód zachód
  pinMode(9, 0x1); // rabata sekcja 1 (oczko z przodu)
  pinMode(13, 0x1); // przekaźnik podtrzymujący
  pinMode(10, 0x1); // trawa sekcja 4 (węże pod szopą)
  pinMode(11, 0x1); // rabata sekcja 2 (skwer Bronka)
  pinMode(12, 0x2); // czujnik opadów

  updatePool(wateringSystem, activePool);

  Serial.println("Start");
  digitalWrite(13, relayStatus);
  delay(3000);
}

void loop()
{
  digitalWrite(13, relayStatus);
  checkStates(wateringSystem);

  if(digitalRead(12) == 0x1) {
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
  relay = 0x0;
}

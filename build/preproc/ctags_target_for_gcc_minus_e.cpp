# 1 "/home/kamil/Projects/Podlewanie/Podlewanie.ino"
// Podlewanie - 10.07.2023
# 3 "/home/kamil/Projects/Podlewanie/Podlewanie.ino" 2
# 4 "/home/kamil/Projects/Podlewanie/Podlewanie.ino" 2

using namespace std;




class Section {
  protected:
    bool state = false;
    bool isActive = false;
    uint8_t pin;
    uint8_t button;
  public:
    Section(uint8_t ledPin, uint8_t btn) {
      pin = ledPin;
      button = btn;
    }

    void setState(bool newState) {
      state = newState;
    }
    bool getState() {
      return state;
    }
    void setPin(uint8_t newPin) {
      pin = newPin;
    }
    uint8_t getPin() {
      return pin;
    }

    void on() {
      digitalWrite(pin, 0x1);
    }
    void off() {
      digitalWrite(pin, 0x0);
    }
    void startSection() {
      on();
    }
    void stopSection(){
      off();
    }
    void updateState() {
      printf("state: %d - %d", button, digitalRead(button));
      if(digitalRead(button) == 0x0)
        state = true;
      else
        state = false;
    }
};

class TimerSection: public Section {
  private:
    uint16_t savedWorkTime = 0;
    uint16_t savedTransitionTime = 0;
    uint16_t workTime;
    uint16_t transitionTime;
  public:
    TimerSection(uint8_t ledPin, uint8_t btn, uint16_t wt, uint16_t tt) : Section(ledPin, btn) {
      pin = ledPin;
      workTime = wt;
      transitionTime = tt;
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
    void sectionLoop(TimerSection &nextSection, uint8_t &pos) {
      if(!isActive) {
        savedWorkTime = millis();
        isActive = true;
      }
      if(millis() - savedWorkTime > workTime || (isActive && !state)) {
        transition(nextSection);
        pos++;
      } else {
        on();
      }
    }
    void sectionLoop(uint8_t &pos, bool &relay) {
      if(!isActive) {
        savedWorkTime = millis();
        isActive = true;
      }
      if(millis() - savedWorkTime < workTime) {
        on();
      } else {
        stopSection();
        end(relay);
        pos = 0;
      }
    }
    void end(bool &relay) {
      relay = 0x0;
    }
};

void checkStates(vector<TimerSection> &sections, Section kropleSzopa);
void loopButtons(vector<TimerSection> &sections, Section kropleSzopa ,ezButton &rainMeter);

TimerSection kropleSzopa(10, 1, 2000, 1000);
TimerSection skalniak(3, A0, 2000, 1000);
TimerSection trawaZachod(4, A1, 2000, 1000);
TimerSection trawaSrodek(5, A2, 2000, 1000);
TimerSection trawaWschod(6, A3, 2000, 1000);
TimerSection trawaPrzodWschod(7, A4, 2000, 1000);
TimerSection trawaPrzodZachod(8, A5, 2000, 1000);
TimerSection rabatkaPrzod(9, 0, 2000, 1000);
TimerSection skwerBronka(11, 2, 2000, 1000);

vector<TimerSection> wateringSystem{skalniak, trawaZachod, trawaSrodek, trawaWschod, trawaPrzodWschod, trawaPrzodZachod, rabatkaPrzod, kropleSzopa, skwerBronka};

uint8_t position = 0;
bool relayStatus = 0x1;

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

  Serial.println("Start");

}

void loop()
{
  digitalWrite(13, relayStatus);
  // loopButtons(wateringSystem, kropleSzopa, btn_czujnikOpadow);
  checkStates(wateringSystem, kropleSzopa);

  if(digitalRead(12) == 0x0) {
      if(wateringSystem.size() > position + 1) {
        wateringSystem.at(position).sectionLoop(wateringSystem.at(position+1), position);
      }
      else {
        wateringSystem.at(position).sectionLoop(position, relayStatus);
      }
  }
}

void checkStates(vector<TimerSection> &sections, Section kropleSzopa) {
  for(TimerSection &section : sections) {
    section.updateState();
    // printf("state: %d ", section.getState());
  }
  kropleSzopa.updateState();
}

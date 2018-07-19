#define CYCLE_TIME_SECONDS 2

// Utility defines
#define SECONDS_TO_MILLISECONDS 1000
#define MINUTES_TO_SECONDS 60
#define HOURS_TO_MINUTES 60
#define HOURS_TO_SECONDS (HOURS_TO_MINUTES*MINUTES_TO_SECONDS)

// Pin setup
#define RELAY_DIGITAL_PIN 0
#define LED_DIGITAL_PIN 1
// Weird pin numbering, A1 is digital 2
#define BRIGHTNESS_ANALOG_PIN 1

// Statistics
bool isOn = false;
bool hasBeenOn = false;
bool hasBeenOff = false;
signed long timeOnSeconds = 0;
signed long timeOffSeconds = 0;

// Initial cooldown
bool startingUp = true;
#define STARTUP_TIME_SECONDS (15*MINUTES_TO_SECONDS)

// Brightness-controlled switching intervals
enum EBrightnessCategory {night, cloudy, sun};
#define NIGHT_ON_INTERVAL_SECONDS (30*MINUTES_TO_SECONDS)
#define NIGHT_OFF_INTERVAL_SECONDS (90*MINUTES_TO_SECONDS)
#define NIGHT_CLOUDY_THRESHOLD 240
#define CLOUDY_ON_INTERVAL_SECONDS (45*MINUTES_TO_SECONDS)
#define CLOUDY_OFF_INTERVAL_SECONDS (45*MINUTES_TO_SECONDS)
#define CLOUDY_SUN_THRESHOLD 120
// Day is 100% on

// Minimum time for the cooling to have any effect
#define MIN_ON_TIME_SECONDS (15*MINUTES_TO_SECONDS)
// Short-term stress safety, avoiding too much switching
#define MIN_OFF_TIME_SECONDS (15*MINUTES_TO_SECONDS)

// Mid-term stress safety, again avoid too much switching
#define MIN_ON_OFF_TIME_SECONDS (45*MINUTES_TO_SECONDS)

// Long-term stress safety - avoid running the cooling box for too long, according to spec
#define STRESSLEVEL_THRESHOLD_SECONDS (5*HOURS_TO_SECONDS)
#define DESTRESS_TIME_SECONDS (1*HOURS_TO_SECONDS)
#define OFFTIME_DESTRESS_FACTOR (STRESSLEVEL_THRESHOLD_SECONDS / DESTRESS_TIME_SECONDS)
#define MINIMUM_OFFTIME_BEFORE_DESTRESS_SECONDS (15*MINUTES_TO_SECONDS)
bool stressingOff = false;
signed long stressLevelSeconds = 0;

// the setup routine runs once when you press reset:
void setup() {
  // initialize the digital pin as an output.
  pinMode(LED_DIGITAL_PIN, OUTPUT);

  digitalWrite(RELAY_DIGITAL_PIN, LOW);
  pinMode(RELAY_DIGITAL_PIN, OUTPUT);

  startingUp = true;
}

// the loop routine runs over and over again forever:
void loop() {
  if (startingUp && timeOnSeconds > STARTUP_TIME_SECONDS)
  {
    startingUp = false; 
  }
  
  EBrightnessCategory brightnessCategory = determineBrightnessCategory();
  bool brightnessControlValue = controlValueFromBrightness(brightnessCategory);
  bool minMaxEnsured = ensureMinMaxRuntimes(
    startingUp ? true : brightnessControlValue
    );
  updateStressLevelAndSetRelay(minMaxEnsured);

  if (stressingOff)
  {
    digitalWrite(LED_DIGITAL_PIN, HIGH); 
    delay(CYCLE_TIME_SECONDS*SECONDS_TO_MILLISECONDS / 2);
    digitalWrite(LED_DIGITAL_PIN, LOW);
    delay(CYCLE_TIME_SECONDS*SECONDS_TO_MILLISECONDS / 2);
  }
/*  else if (startingUp)
  {
    digitalWrite(LED_DIGITAL_PIN, HIGH); 
    delay(CYCLE_TIME_SECONDS*SECONDS_TO_MILLISECONDS / 4);
    digitalWrite(LED_DIGITAL_PIN, LOW);
    delay(CYCLE_TIME_SECONDS*SECONDS_TO_MILLISECONDS / 4);
    digitalWrite(LED_DIGITAL_PIN, HIGH); 
    delay(CYCLE_TIME_SECONDS*SECONDS_TO_MILLISECONDS / 4);
    digitalWrite(LED_DIGITAL_PIN, LOW);
    delay(CYCLE_TIME_SECONDS*SECONDS_TO_MILLISECONDS / 4);
  }*/
  else
  {
    if (brightnessCategory == night)
    {
      digitalWrite(LED_DIGITAL_PIN, HIGH);
      delay(50);
      digitalWrite(LED_DIGITAL_PIN, LOW);
      delay(150);
      delay(CYCLE_TIME_SECONDS*SECONDS_TO_MILLISECONDS - 200);
    }
    else if (brightnessCategory == cloudy)
    {
      digitalWrite(LED_DIGITAL_PIN, HIGH);
      delay(50);
      digitalWrite(LED_DIGITAL_PIN, LOW);
      delay(150);
      digitalWrite(LED_DIGITAL_PIN, HIGH);
      delay(50);
      digitalWrite(LED_DIGITAL_PIN, LOW);
      delay(150);
      delay(CYCLE_TIME_SECONDS*SECONDS_TO_MILLISECONDS - 400);
    }
    else if (brightnessCategory == sun)
    {
      digitalWrite(LED_DIGITAL_PIN, HIGH);
      delay(150);
      digitalWrite(LED_DIGITAL_PIN, LOW);
      delay(50);
      digitalWrite(LED_DIGITAL_PIN, HIGH);
      delay(150);
      digitalWrite(LED_DIGITAL_PIN, LOW);
      delay(50);
      delay(CYCLE_TIME_SECONDS*SECONDS_TO_MILLISECONDS - 400);
    }
  }
}

EBrightnessCategory determineBrightnessCategory()
{
  int brightness = analogRead(BRIGHTNESS_ANALOG_PIN);
  if (brightness < CLOUDY_SUN_THRESHOLD)
  {
    return sun;
  }

  if (brightness < NIGHT_CLOUDY_THRESHOLD)
  {
    return cloudy;
  }

  return night;
}

bool controlValueFromBrightness(EBrightnessCategory brightness)
{
  if (brightness == sun)
  {
    return true;
  }

  if (brightness == cloudy)
  {
    if (isOn)
    {
      return timeOnSeconds < CLOUDY_ON_INTERVAL_SECONDS ? true : false;
    }
    else
    {
      return timeOffSeconds < CLOUDY_OFF_INTERVAL_SECONDS ? false : true;
    }
  }

  if (brightness == night)
  {
    if (isOn)
    {
      return timeOnSeconds < NIGHT_ON_INTERVAL_SECONDS ? true : false;
    }
    else
    {
      return timeOffSeconds < NIGHT_OFF_INTERVAL_SECONDS ? false : true;
    }
  }

  // This should never happen, but if it for any reason does (e.g. future development)
  // turn off for safety
  return false;
}

bool ensureMinMaxRuntimes(bool wantOn)
{
  if (isOn)
  {
    if (hasBeenOn && timeOnSeconds < MIN_ON_TIME_SECONDS)
    {
      // Keep running
      return true;  
    }
  }
  else
  {
    if (hasBeenOff && timeOffSeconds < MIN_OFF_TIME_SECONDS)
    {
      // Keep off
      return false;  
    }
  }

  if (hasBeenOn && hasBeenOff
    && (timeOnSeconds + timeOffSeconds) < MIN_ON_OFF_TIME_SECONDS)
    {
      // Keep what-ever the state already is
      return isOn;
    }
  
  return wantOn;
}

void updateStressLevelAndSetRelay(bool wantOn)
{
  if (stressLevelSeconds > STRESSLEVEL_THRESHOLD_SECONDS)
  {
    stressingOff = true;
  }
  
  if (stressingOff)
  {
    if (timeOffSeconds >= DESTRESS_TIME_SECONDS)
    {
      stressLevelSeconds = 0;
      stressingOff = false;
    }
  }

  bool toBeOn = stressingOff ? false : wantOn;

  if (toBeOn != isOn)
  {
    if (toBeOn)
    {
      timeOnSeconds = 0;
    }
    else
    {
      timeOffSeconds = 0;
    }
  }

  if (toBeOn)
  {
    isOn = true;
    hasBeenOn = true;
    digitalWrite(RELAY_DIGITAL_PIN, HIGH);
    timeOnSeconds += CYCLE_TIME_SECONDS;
    stressLevelSeconds += CYCLE_TIME_SECONDS;
  }
  else
  {
    isOn = false;
    hasBeenOff = true;
    digitalWrite(RELAY_DIGITAL_PIN, LOW);
    timeOffSeconds += CYCLE_TIME_SECONDS;
    if (timeOffSeconds >= MINIMUM_OFFTIME_BEFORE_DESTRESS_SECONDS)
    {
      stressLevelSeconds -= CYCLE_TIME_SECONDS*OFFTIME_DESTRESS_FACTOR;
    }
  }
}
    

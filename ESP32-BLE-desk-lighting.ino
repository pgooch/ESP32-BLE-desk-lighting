// Import all the things
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <NeoPixelBus.h>

/*
 * UUID managment. 
 * This could probably be a library but since I am not generating the UUIDs the proper way I'll just leave the hack as is.
 */
#define UUID_BASE "4fafc201-1fb5-459e-8fcc-000000000000" // The base shouls end with 12 0's but be different for every device
int UUID_COUNT = 0;
// This function generates a UUID
char* getUUID(int service = 0, int characteristic = 0xfffffff){
  // Copy over the base UUID
  char* FinalUUID = new char[37];
  memmove(FinalUUID,UUID_BASE,37);

  // Add the incrementing bit
  char hex[12]; 
  itoa(UUID_COUNT,hex,16);
  int hexLength = strlen(hex);
  memmove(FinalUUID+36-hexLength,hex,hexLength);

  // Outout a little debug info
//  Serial.print("UUID generated for service ");
//  Serial.print(serviceHex);
//  Serial.print(" characteristic ");
//  Serial.print(characteristicHex);
//  Serial.print(" as ");
//  Serial.println(FinalUUID);

  // Tick the number up and return
  UUID_COUNT++;
  return FinalUUID;
}

// Structures for some additional data I'm going to need to use.
typedef struct {
  char* name;
  int characteristics[2];
} BT_Service;
typedef struct {
  char* name;
  std::string value;
} BT_Characteristics;
typedef struct {
  int service;
  int characteristic;
  std::string value;
} BT_Values;

// Prepare for the BT connection and sever
#define DEVICE_NAME "ESP32 BT Desk Lighting"
BT_Service Features[] = {
  // Name, Characteristics, LED Strip
  {"System",              {3}   },
  {"Left Monitor Light",  {2,1} },
  {"Right Monitor Light", {2,1} },
  {"Under Desk Light",    {2,1} },
};
BT_Characteristics Options[] = {
  // Name, Default
  {"NULL",        "NULL"}, // This intentially not used so empty array spaces will auto filter out
  {"Color",       "#FFBB55"},
  {"Brightness",  "512"}, // No longer used, must switch back to NeoPixelBrightnessBus to use again.
  {"Power",       "On"},
};
// Here are some magic numbers, since it needs to know values at runtime and won't run any code to calculate them.
const int FeatureCount = 4; //Count be sizeof(Features)/sizeof(Features[0]);, it's the number of services
const int OptionCount = 7; // This is the sum of all non zero options acrosss all features

// Prepare some of the BLE arrays and a bool
BLEService *ServiceArray[FeatureCount];
BLECharacteristic *CharacteristicArray[OptionCount];
BT_Values CharacteristicValues[OptionCount];

// Create the LED strip, because we gotta use the RMT so we can have more than enough strips, they gotta be out in the open
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0800KbpsMethod> s0strip(72, 12);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt1800KbpsMethod> s1strip(72, 13);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt2800KbpsMethod> s2strip(58, 25);
#define MIN_BRIGHTNESS 10
#define MAX_BRIGHTNESS 255

// Connection State managment, will restart advertising once a client has disconnected.
bool BLEDeviceConnected = false;
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* Server) {
      BLEDeviceConnected = true;
      BLEDevice::startAdvertising();
    };
    void onDisconnect(BLEServer* pServer) {
      BLEDeviceConnected = false;
    }
};




void setup(){
  Serial.begin(115200);
  Serial.println("\n");
  Serial.print("Setting up the ");
  Serial.println(DEVICE_NAME);

  // Init BLE
  BLEDevice::init(DEVICE_NAME);
  BLEServer *Server = BLEDevice::createServer();
  Server->setCallbacks(new ServerCallbacks());
  BLEAdvertising *Advertising = BLEDevice::getAdvertising();

  // loop the services, adding them as we go
  Serial.println("The following JSON can be used for the web interface:");
  Serial.println("{");
  Serial.print("\t\"_name\": \"");
  Serial.print(DEVICE_NAME);
  Serial.println("\",");
  int characteristicPointer = 0; // Needed for adding characteristincs, needs to keep counting between all services
  for(int s=0; s<FeatureCount; s++){
    char* serviceID = getUUID(s);
    ServiceArray[s] = Server->createService(serviceID);
    Advertising->addServiceUUID(serviceID);
    
    Serial.print("\t\"");
    Serial.print(Features[s].name);
    Serial.print("\": {\n");
    Serial.print("\t\t\"UUID\": \"");
    Serial.print(serviceID);
    Serial.print("\",\n");
    Serial.print("\t\t\"characteristics\": [\n");

    // Add the services characteristics
    int characteristicsInService = sizeof(Features[s].characteristics)/sizeof(Features[s].characteristics[0]);
    for(int c=0; c<characteristicsInService; c++){
      if(Features[s].characteristics[c]==0){
        // Were not gonna deal with zeros
        
      }else{
        char* charUUID = getUUID(s,c);

        // Add the comma and line break on every after the first
        if(c>0){
          Serial.print(",");
          Serial.print("\n");
        }
        Serial.print("\t\t\t");

        Serial.print("{\"");
        Serial.print(Options[Features[s].characteristics[c]].name);
        Serial.print("\": \"");
        Serial.print(charUUID);
        Serial.print("\"}");
  
        CharacteristicArray[characteristicPointer] = ServiceArray[s]->createCharacteristic(charUUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
        CharacteristicArray[characteristicPointer]->setValue(Options[Features[s].characteristics[c]].value);
        CharacteristicValues[characteristicPointer] = {s,Features[s].characteristics[c],""};
        characteristicPointer++;
      }
    }
    
    Serial.print("\n");
    Serial.print("\t\t]\n");
    Serial.print("\t}");
    if(s<FeatureCount-1){
      Serial.print(",");
    }
    Serial.print("\n");

    // Now that everything is prepped we can start the ble service and clean up that JSON
    ServiceArray[s]->start();
  }
  Serial.println("}");

  // Advertise we got that bluetooth going on
  Advertising->setScanResponse(true);
  Advertising->setMinPreferred(0x06);
  BLEDevice::startAdvertising();
  Serial.println("Setup Complete, starting initial update.");

  // Prep the LEDs
  for(int f=0; f<FeatureCount; f++){
    switch(f){
      case 0:
        s0strip.Begin();
      break;
      case 1:
        s1strip.Begin();
      break;
      case 2:
        s2strip.Begin();
      break;
      default:
      break;
    }
  }
};


// Loop through all the characteristcs, if one has changed figure out the details
bool lightsOn = true;
void loop(){
  for(int c=0; c<OptionCount; c++){
    if(CharacteristicValues[c].value != CharacteristicArray[c]->getValue()){

      // Update the previous with the new previious
      CharacteristicValues[c].value = CharacteristicArray[c]->getValue();

      // First and foremost, if it's off set the strip to black (not really off but I'm not worried about sleeping it at this time
      if(Options[CharacteristicValues[c].characteristic].name=="Power"){
        const char* state = CharacteristicArray[c]->getValue().c_str();
        // If on then change slightsOn and clear the stored values so the below will reprocess.
        if(!lightsOn && strcmp(state,"On")==0){
          // Serial.println("Lights turned on");
          lightsOn = true;
          for(int c=0; c<OptionCount; c++){
            CharacteristicValues[c].value = "";
          }

        // zif your turning it off then clear the LED strips back to solid black.
        }else if(lightsOn && strcmp(state,"Off")==0){
          // Serial.println("Lights turned off");
          lightsOn = false;
          RgbColor color(0,0,0);
          s0strip.ClearTo(color);
          s0strip.Show();
          s1strip.ClearTo(color);
          s1strip.Show();
          s2strip.ClearTo(color);
          s2strip.Show();
        }

      // Update things as needed, starting with color
      }else if(Options[CharacteristicValues[c].characteristic].name=="Color"){
        // Parse the HTML color (the built in thing is not working)
        const char* string = CharacteristicArray[c]->getValue().c_str();
        // lifted from https://stackoverflow.com/questions/28104559/arduino-strange-behavior-converting-hex-to-rgb cause bit shifting is scary
        long number = (long) strtol( &string[1], NULL, 16);
        int r = number >> 16;
        int g = number >> 8 & 0xFF;
        int b = number & 0xFF;
        RgbColor color(r,g,b);
        
        // Set strip color and update
        // Serial.print("Updating color for ");
        // Serial.print(Features[CharacteristicValues[c].service].name);
        // Serial.print(" to ");
        // Serial.print(r);
        // Serial.print(",");
        // Serial.print(g);
        // Serial.print(",");
        // Serial.print(b);
        if(Features[CharacteristicValues[c].service].name == "Left Monitor Light"){
            s0strip.ClearTo(color);
            s0strip.Show();
            // Serial.println(" done.");
        }else if(Features[CharacteristicValues[c].service].name == "Right Monitor Light"){
            s1strip.ClearTo(color);
            s1strip.Show();
            // Serial.println(" done.");
        }else if(Features[CharacteristicValues[c].service].name == "Under Desk Light"){
            s2strip.ClearTo(color);
            s2strip.Show();
            // Serial.println(" done.");
        }else{
          Serial.println(" ERROR; no addressable LED strip for service.");
        }


 
      // the brighness is much simpler, and disabled since I'm no longer using the brightness bus due to how it morphs colors
      }else if(Options[CharacteristicValues[c].characteristic].name=="Brightness"){
      /*
        int b = atoi(CharacteristicArray[c]->getValue().c_str());
        b = map(b,0,1023,MIN_BRIGHTNESS,MAX_BRIGHTNESS);

        Serial.print("Updating brightness for ");
        Serial.print(Features[CharacteristicValues[c].service].name);
        Serial.print(" to ");
        Serial.print(b);
       if(Features[CharacteristicValues[c].service].name == "Left Monitor Light"){
            s0strip.SetBrightness(b);
            s0strip.ClearTo(b);
            s0strip.Show();
            Serial.println(" done.");
        }else if(Features[CharacteristicValues[c].service].name == "Right Monitor Light"){
            s1strip.SetBrightness(b);
            s1strip.Show();
            Serial.println(" done.");
        }else if(Features[CharacteristicValues[c].service].name == "Under Desk Light"){
            s2strip.SetBrightness(b);
            s2strip.Show();
            Serial.println(" done.");
        }else{
          Serial.println(" ERROR; no addressable LED strip for service.");
        }
      */

      // Ya done goofed (or the update is passive)
      }else{
        Serial.print("Unable to update the \"");
        Serial.print(Features[CharacteristicValues[c].service].name);
        Serial.print("\" features \"");
        Serial.print(Options[CharacteristicValues[c].characteristic].name);
        Serial.print("\" option to \"");
        Serial.print(CharacteristicValues[c].characteristic);
        Serial.println("\", no appropriate update logic found.");
      }
    }
  }
  delay(1);
};

// Import all the things
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <NeoPixelBus.h>


typedef struct {
  char* name;
  int characteristics[2];
} BT_Service;
typedef struct {
  char* name;
  std::string value;
  std::string default;
} BT_Characteristics;
;

// Prepare for the BT connection and sever
#define DEVICE_NAME "ESP32 BT Desk Lighting"
BT_Service Features[] = {
  // Name, Characteristics, LED Strip
  {"System",              {0,3}},
  {"Left Monitor Light",  {2,1}},
  {"Right Monitor Light", {2,1}},
  {"Under Desk Light",    {2,1}},
};
BT_Characteristics Options[] = {
  // Name, Default
  {"NULL",        "#NULL"},
  {"Color",       "#FFA500"},
  {"Brightness",  "20"},
  {"Power",       "On"},
};
// Calculate a few numbers, create an array to store the characteristic to service relation
const int FeatureCount = sizeof(Features)/sizeof(Features[0]);
const int OptionCount = FeatureCount*(sizeof(Options)/sizeof(Options[0]));
int CharacteristicToFeature[OptionCount];
int CharacteristicToOption[OptionCount];
// and build out some needed arrays, this math will get more complicated if more than just LEDs are in plat
BLEService *ServiceArray[FeatureCount];
BLECharacteristic *CharacteristicArray[OptionCount];
std::__cxx11::string PreviousCharacteristicArray[OptionCount]; // This funky format is what the BLECharacteristics getValue returns

// Create the LED strip, because we gotta use the RMT so we can have more than enough strips, they gotta be out in the open
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0800KbpsMethod> s0strip(72, 25);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt1800KbpsMethod> s1strip(72, 26);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt2800KbpsMethod> s2strip(72, 14);




/*
 * UUID managment. 
 * This could probably be a library but since I am not generating the UUIDs the proper way I'll just 
 * leave the hack as is.
 */
#define UUID_BASE "4fafc201-1fb5-459e-8fcc-000000000000" // The base shouls end with 12 0's but be different for every device

// This function generates a UUID
char* getUUID(int service = 0, int characteristic = 0xfffffff){
  // Copy over the base UUID
  char* FinalUUID = new char[37];
  memmove(FinalUUID,UUID_BASE,37);

  // Service Part
  char serviceHex[4]; 
  itoa(service,serviceHex,16);
  int serviceHexLength = strlen(serviceHex);
  memmove(FinalUUID+36-12,serviceHex,serviceHexLength);

  // The characteristc part
  char characteristicHex[8]; 
  itoa(characteristic,characteristicHex,16);
  int characteristicHexLength = strlen(characteristicHex);
  memmove(FinalUUID+36-characteristicHexLength,characteristicHex,characteristicHexLength);

  // Outout a little debug info
//  Serial.print("UUID generated for service ");
//  Serial.print(serviceHex);
//  Serial.print(" characteristic ");
//  Serial.print(characteristicHex);
//  Serial.print(" as ");
//  Serial.println(FinalUUID);

  // Tick the number up and return
  return FinalUUID;
}
// Define some data structures
typedef struct {
  char* name;
  char type[4];
  char UUID = getUUID();
} BLEConfig;





void setup(){
  Serial.begin(115200);
  Serial.println("\n");
  Serial.print("Setting up the ");
  Serial.println(DEVICE_NAME);

  // Init BLE
  BLEDevice::init(DEVICE_NAME);
  BLEServer *Server = BLEDevice::createServer();
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
        // Were gonna keep putting things in the a
        
      }else{
        char* charUUID = getUUID(s,c);
  
        Serial.print("\t\t\t{\"");
        Serial.print(Options[Features[s].characteristics[c]].name);
        Serial.print("\": \"");
        Serial.print(charUUID);
        Serial.print("\"}");
        if(c<characteristicsInService-1){
          Serial.print(",");
        }
        Serial.print("\n");
  
        CharacteristicArray[characteristicPointer] = ServiceArray[s]->createCharacteristic(charUUID,BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
        CharacteristicArray[characteristicPointer]->setValue(Options[Features[s].characteristics[c]].value);
        //PreviousCharacteristicArray[CHARACTERISTIC_POINTER] = CHARACTERISTICS[c][1]; // Can set defaults but instead going to use that first batch is mis-matches to set the initials
        CharacteristicToFeature[characteristicPointer] = s;
        CharacteristicToOption[characteristicPointer] = Features[s].characteristics[c];
        characteristicPointer++;
      }
    }
    
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
  Serial.println("Setup Complete.");

  // Prep the LEDs
  for(int f=0; f<FeatureCount; f++){
    switch(f){
      case 0:
        s0strip.Begin();
      //  s0strip.Show();
      break;
      case 1:
        s1strip.Begin();
      //  s1strip.Show();
      break;
      case 2:
        s2strip.Begin();
      //  s2strip.Show();
      break;
      default:
      break;
    }
  }
};


// Loop through all the characteristcs, if one has changed figure out the details
void loop(){
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("Loop Start");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  for(int c=0; c<OptionCount-2; c++){
    Serial.print(" --- for: c");
    Serial.println(c);
    if(PreviousCharacteristicArray[c] != CharacteristicArray[c]->getValue()){

      // Output a message about this
      Serial.println("");
      Serial.print(Features[CharacteristicToFeature[c]].name);
      Serial.print(" ");
      Serial.print(Options[CharacteristicToOption[c]].name);
      Serial.print(" changed from ");
      Serial.print(PreviousCharacteristicArray[c].c_str());
      Serial.print(" to ");
      Serial.print(CharacteristicArray[c]->getValue().c_str());
      Serial.print(", ");

      // Update the previous with the new previious
      PreviousCharacteristicArray[c] = CharacteristicArray[c]->getValue();



      // Update things as needed, starting with color
      if(Options[CharacteristicToOption[c]].name=="Color"){
        // Parse the HTML color (the built in thing is not working)
        const char* string = CharacteristicArray[c]->getValue().c_str();
        // lifted from https://stackoverflow.com/questions/28104559/arduino-strange-behavior-converting-hex-to-rgb cause bit shifting is scary
        long number = (long) strtol( &string[1], NULL, 16);
        int r = number >> 16;
        int g = number >> 8 & 0xFF;
        int b = number & 0xFF;
        RgbColor color(r,g,b);
        
        // Set strip color and update
        switch(CharacteristicToFeature[c]){
          case 0:
            s0strip.ClearTo(color);
            s0strip.Show();
          break;
          case 1:
            s1strip.ClearTo(color);
            s1strip.Show();
          break;
          case 2:
            s2strip.ClearTo(color);
            s2strip.Show();
          break;
          default:
          break;
        }
        Serial.print("done.");

                              Serial.println(">>>>> 249");

 
      // the brighness is much simpler
      }else if(Options[CharacteristicToOption[c]].name=="Brightness"){
        int b = atoi(CharacteristicArray[c]->getValue().c_str());
        Serial.print("done.");

                              Serial.println(">>>>> 257");


      // Ya done goofed (or the update is passive)
      }else{
        Serial.print("no update logic.");

                              Serial.println(">>>>> 264");
      }
                              Serial.println(">>>>> 266");
    }
                              Serial.println(">>>>> 268");
  }
                              Serial.println(">>>>> 270");
  delay(1000);
};

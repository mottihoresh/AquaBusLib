// AquaBus Library
// Copyright (C) 2017
//
// This software is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0
// International License.
//
// You can redistribute and/or modify this software for non-commerical purposes under the terms
// of the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//
// This software is provided "as is" without express or implied warranty.

// Debug related definitions
// #define DEBUG
#ifdef DEBUG
  #define DEBUG_LOG(string) Serial.print(string)
  #define DEBUG_LOG_LN(string) Serial.println(string)
#else
  #define DEBUG_LOG(string)
  #define DEBUG_LOG_LN(string)
#endif
#define DEBUG_LOG_FREE_RAM() DEBUG_LOG(F("Free RAM: ")); DEBUG_LOG_LN(FreeRam())

// Include header files
#include <string.h>
#include "AquaBusLib.h"

// Static member variables
static AquaBusDev** AquaBusLib::devices = NULL;
static byte AquaBusLib::devicesCount = 0;

// Static callback function for handling initial probing
eMBException AquaBusLib::probeCallback(byte address, byte* frame, unsigned short* length)
{
  // Check the length of the frame
  if (*length == 8) // Probe stages 1, 2, 3, and 5
  {
    // Define the request structure for frames with this length
    struct Request
    {
      byte code;
      byte stage;
      byte nextAddress;
      unsigned short hwSerial;
      byte unknown[3];
    };

    // Loop through the devices
    for (byte i = 0; i < devicesCount; ++i)
    {
      // Create the response structure
      struct
      {
        byte code;
        byte stage;
        byte hwId;
        byte hwRevision;
        byte swRevision;
        byte nextAddress;
        unsigned short hwSerial;
        byte unknown[3] = {0, 0, 0};
      } response;
      response.code = ((Request*)frame)->code;
      response.stage = ((Request*)frame)->stage;
      response.hwId = devices[i]->hwId;
      response.hwRevision = devices[i]->hwRevision;
      response.swRevision = devices[i]->swRevision;
      response.nextAddress = ((Request*)frame)->nextAddress;
      response.hwSerial = devices[i]->hwSerial;

      // Send the response
      devices[i]->sendData(devices[i]->abAddress, (byte*)&response, sizeof(response));
    }
  }
  else if (*length == 5) // Probe stage 4
  {
    // Define the request structure for frames with this length
    struct Request
    {
      byte code;
      byte stage;
      byte abAddress;
      byte unknown[2];
    };
  }

  return MB_EX_NONE;
}

// Static callback function for handling device communication
eMBException AquaBusLib::deviceCallback(byte address, byte* frame, unsigned short* length)
{
  // Loop through the devices
  for (byte i = 0; i < devicesCount; ++i)
  {
    // Check if the address matches the device's AquaBus address
    if (address == devices[i]->abAddress)
    {
      // Call the device's processData function
      devices[i]->processData(frame, length);
    }
  }

  return MB_EX_NONE;
}

// Constructor
AquaBusLib::AquaBusLib(byte numberOfDevices)
{
  // Check if the devices array has to be allocated
  if (devices == NULL)
  {
    // Allocate and initalize memory for the number of devices specified and save the devices count
    devices = new AquaBusDev*[numberOfDevices]();
    devicesCount = numberOfDevices;
  }
#ifdef DEBUG
  // Check if a different device count was specified than before
  else if (numberOfDevices != devicesCount)
  {
    DEBUG_LOG_LN("WARNING: Creating a second AquaBusLib object with a different number of devices");
  }
  else
  {
    DEBUG_LOG_LN("WARNING: Creating a second AquaBusLib object");
  }
#endif
}

// Function called from the sketch's setup function
void AquaBusLib::setup()
{
  // Initialize the FreeModBus library
  eMBInit(MB_RTU, MB_ADDRESS_MAX, 0, 19200, MB_PAR_EVEN);

  // Register the probe and device callback functions with the FreeModBus library
  eMBRegisterCB(0x01, probeCallback);
  eMBRegisterCB(0x7F, deviceCallback);

  // Enable the FreeModBus library
  eMBEnable();
}

// Function called from the sketch's loop function
void AquaBusLib::loop()
{
  eMBPoll();
}

// Function called to add a device
void AquaBusLib::addDevice(AquaBusDev* device)
{
  // Loop through the devices
  byte i = 0;
  for (; i < devicesCount; ++i)
  {
    // Check if this slot is available and save the passed in device
    if (devices[i] == NULL)
      devices[i] = device;
  }

#ifdef DEBUG
  // Check if the devices array was full
  if (i == devicesCount)
    DEBUG_LOG_LN("WARNING: Trying to add too many devices");
#endif
}
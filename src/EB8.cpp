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

// Include header files
#include "EB8.h"

// Function called to get the state of an outlet
bool EB8::getOutletState(byte outlet)
{
  return (outletStates >> outlet) & 1;
}

// Function called to process recieved data
void EB8::processData(byte* data, unsigned short length)
{
  // Define the request structure
  struct Request
  {
    byte hwId;
    byte stage;
    byte outletStates;
    byte unknown;
  };

  // Check if we should update the outlet states
  if (((Request*)data)->stage == 1)
    outletStates = ((Request*)data)->outletStates;

  // Create the response structure
  struct
  {
    byte hwId;
    byte stage;
    byte outletStates;
    byte unknown;
    unsigned short legacyCurrent;
    unsigned short frequency;
    unsigned long rawCurrent;
  } response;
  response.hwId = hwId;
  response.stage = (((Request*)data)->stage);
  response.outletStates = outletStates;
  response.unknown = outletStates; // Seems to match the outletStates variable usually
  response.legacyCurrent = 0;
  response.frequency = 0;
  response.rawCurrent = 0;

  // Send the response
  sendData((byte*)&response, sizeof(response));
}
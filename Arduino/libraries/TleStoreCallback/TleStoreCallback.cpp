/* TleEEPROM.cpp -- a  library for compactly storing keplerian elements
and satellite modelines in EEPROM memory, and for accessing data stored in
this way.

Copyright (c) 2009 Bruce Robertson, VE9QRP. All rights reserved.


This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation; either version 2.1 of the License, or (at your option)
any later version.

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details.

You should have received a copy of the GNU Lesser General Public License along
with this library; if not, write to the Free Software Foundation, Inc., 51
Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

The latest version of this software is available at
*/
#include "WProgram.h"
#include "TleStoreCallback.h"


TleStoreCallback::TleStoreCallback(readCallback in, writeCallback out, int delayIn)
{
	read = in;
	write = out;
	// you don't know how much a given chip or context is going to need to wait
	//so we're parameterizing this
	writeDelay = delayIn;
}


void TleStoreCallback::greeting() {
	Serial.println("QRPT");
}
void TleStoreCallback::handshake() {
	Serial.println("VE9");
}

/* This is not properly implmemented for abstract class. We need 
   to ask what the size is, then write to that
*/
void TleStoreCallback::erase() {
  for (int x = 0; x < 512; x++) {
    write(x,0);
  }
}

char * TleStoreCallback::getSatNameFromAddress(uint16_t satAddress) {
  char satName[6];
  readAnything(satAddress, satName);
  return satName;
}

void TleStoreCallback::echoWrite() {
  erase();
handshake();
  char value;
  int x = 0;
  while (true) {
    while (Serial.available() < 1) {
      ;
    }
    while (Serial.available()) {
      value = Serial.read();
      Serial.print(value);
      write(x,value);
	  delay(writeDelay);
      x++;
    }
  }
}

void TleStoreCallback::readTLE() {

   erase();
   handshake();
  char header[4];
  int x = 0;

  for (x = 0; x < 4; x++) {
            while (Serial.available() < 1) {
      ;
    }
    while (Serial.available() > 0) {
    header[x] = Serial.read();
    echoWriteChar(x,header[x]);
    x++;
    continue;
  }
      
    }
   
  readHeader();
  //numberOfTles = ((header[0] << 0) & 0xFF) + ((header[1] << 8) & 0xFF00);
  //numberOfModelines = ((header[2] << 0) & 0xFF) + ((header[3] << 8) & 0xFF00);
  unsigned int tleBytes = TLE_RECORD_SIZE * numberOfTles;
  char in;
  for (unsigned int x = 0; x < tleBytes; x++) {
                while (Serial.available() < 1) {
      ;
    }
    while (Serial.available() > 0) {
      in = Serial.read();
      echoWriteChar(x,in);
      x++;
    }
  }

  unsigned int modelineBytes = MODELINE_RECORD_SIZE * numberOfModelines;


  for (unsigned int x = 0; x < modelineBytes; x++) {
                    while (Serial.available() < 1) {
      ;
    }
    while (Serial.available() > 0){
      in = Serial.read();
      echoWriteChar(x,in);
      x++;
    }
  }

}

void TleStoreCallback::echoWriteChar(unsigned int address, char in) {
  Serial.print(in);
  write(address,in);
  delay(10);
}

byte TleStoreCallback::getVersion() {
	return read(0);
}

float TleStoreCallback::getLatitude() {
	float latitude;
	readAnything(1,latitude);
	return latitude;
}

float TleStoreCallback::getLongitude() {
	float longitude;
	readAnything(5,longitude);
	return longitude;
}

unsigned int TleStoreCallback::getAltitude() {
	unsigned int altitude;
	readAnything (9,altitude);
	return altitude;
}

void TleStoreCallback::dumpStore() {
  readHeader();
  Serial.print("Number of TLEs: " ); ;
  Serial.print(numberOfTles);
  Serial.print("Number of Modelines: ");
  Serial.println(numberOfModelines);
  Serial.println("------------");
  for (int i = 0; i < 512; i++) {
    Serial.print((int) read(i));
    Serial.print(' ');
  }

}

struct tleStruct TleStoreCallback::getTle(byte place) {
	struct tleStruct s;
	readAnything(HEADER_SIZE + place * TLE_RECORD_SIZE,s.name);
	readAnything(HEADER_SIZE + 6 +  place *  TLE_RECORD_SIZE, s.YE);
    readAnything(HEADER_SIZE + 10 +  place *  TLE_RECORD_SIZE, s.TE);
    readAnything(HEADER_SIZE + 14 +  place *  TLE_RECORD_SIZE, s.M2);
    readAnything(HEADER_SIZE + 18 +  place *  TLE_RECORD_SIZE, s.IN);
    readAnything(HEADER_SIZE + 22 +  place *  TLE_RECORD_SIZE, s.RA);
    readAnything(HEADER_SIZE + 26 +  place *  TLE_RECORD_SIZE, s.EC);
    readAnything(HEADER_SIZE + 30 +  place *  TLE_RECORD_SIZE, s.WP);
    readAnything(HEADER_SIZE + 34 +  place *  TLE_RECORD_SIZE, s.MA);
    readAnything(HEADER_SIZE + 38 +  place *  TLE_RECORD_SIZE, s.MM);
    readAnything(HEADER_SIZE + 42 +  place *  TLE_RECORD_SIZE, s.RV);
	return s;
}

unsigned int TleStoreCallback::getSatAddress(byte rank) {
	return HEADER_SIZE + TLE_RECORD_SIZE * rank;
}

/*
struct modelineStruct TleStoreCallback::getModelineForSatNumber(int rank) {
	return getModeline(getSatAddress(rank));
}
*/
byte TleStoreCallback::countModelinesForTle(unsigned int tleAddress) {
	unsigned int modelineStart = HEADER_SIZE + TLE_RECORD_SIZE * numberOfTles;
	unsigned int satAddress;
    int modelineCounter = 0;
	for (int i = 0; i < numberOfModelines; i++) {
		int currentModelineAddress = modelineStart + i * MODELINE_RECORD_SIZE;
	    readAnything(modelineStart + i * MODELINE_RECORD_SIZE,satAddress);
	    if (satAddress == tleAddress) {
			modelineCounter++;
		}
	}
	return modelineCounter;
} 

/*Returns EEPROM address of the modelineRank-th modeline for the satellite at tleAddress

*/

 unsigned int  TleStoreCallback::getModelineStart(unsigned int tleAddress, byte modelineRank) {
		unsigned int modelinesStart = HEADER_SIZE + TLE_RECORD_SIZE * numberOfTles;
		unsigned int currentModelineStart;
		unsigned int satAddress;
		byte modelineCounter = 0;
		/*
		Serial.print("TLE address: ");
		Serial.println(tleAddress);
		Serial.print("modeline#s: ");
		Serial.println(numberOfModelines);
		*/
		// find the first modeline that pertains
		for (int i = 0; i < numberOfModelines; i++) {
		    currentModelineStart = modelinesStart + i * MODELINE_RECORD_SIZE;
		    readAnything(currentModelineStart,satAddress);
		    /*Serial.print(i);
		    Serial.print("At ");
		    Serial.println(currentModelineStart);
		    Serial.print("satAddress is ");
		    Serial.println(satAddress);
		    */
		    if (satAddress == tleAddress) {
		    	 if  (modelineRank == modelineCounter) {
			    return currentModelineStart;
			}
			else {
				modelineCounter++;
			}
		    }
	
	}
	return 0;// This is not a sensible number, so we use it as an indication that
	         //you've given a number without a corresponding modeline.
} 

struct modelineStruct TleStoreCallback::getModelineForAddress(unsigned int modelineStart) {
	modelineStruct modeline;
   modeline.modeName[5] = '\0';
   readAnything(modelineStart + 2, modeline.modeName);
   readAnything(modelineStart + 7, modeline.dlLong);
   readAnything(modelineStart + 11, modeline.ulLong);
   readAnything(modelineStart + 15, modeline.dlMode);
   readAnything(modelineStart + 16, modeline.ulMode);
   readAnything(modelineStart + 17, modeline.polarity);
   readAnything(modelineStart + 18, modeline.dlShift);
   readAnything(modelineStart + 22, modeline.ulShift);
   readAnything(modelineStart + 26, modeline.tone);
   return modeline;
}
/*
struct modelineStruct TleStoreCallback::getModeline(unsigned int tleAddress) {
	unsigned int modelineStart = HEADER_SIZE + TLE_RECORD_SIZE * numberOfTles;
	unsigned int satAddress;
	modelineStruct modeline;
	// find the first modeline that pertains
	for (int i = 0; i < numberOfModelines; i++) {
	    readAnything(modelineStart + i * MODELINE_RECORD_SIZE,satAddress);
	    if (satAddress == tleAddress) {
		   // reverse these?
		    modeline.modeName[5] = '\0';
			readAnything(modelineStart + 2 + i * MODELINE_RECORD_SIZE, modeline.modeName);
		    readAnything(modelineStart + 7 + i * MODELINE_RECORD_SIZE, modeline.dlLong);
		    readAnything(modelineStart + 11 + i * MODELINE_RECORD_SIZE, modeline.ulLong);
		    readAnything(modelineStart + 15 + i * MODELINE_RECORD_SIZE, modeline.dlMode);
		    readAnything(modelineStart + 16 + i * MODELINE_RECORD_SIZE, modeline.ulMode);
		    readAnything(modelineStart + 17 + i * MODELINE_RECORD_SIZE, modeline.polarity);
		    readAnything(modelineStart + 18 + i * MODELINE_RECORD_SIZE, modeline.dlShift);
		    readAnything(modelineStart + 22 + i * MODELINE_RECORD_SIZE, modeline.ulShift);
			return modeline;
	}
	
}
return modeline;
}
*/
 void TleStoreCallback::listTlesAndModelines() {
  readHeader();
  char satName[6];
  float YE,TE,M2,IN,RA,EC,WP,MA,MM,RV;
  for (int i = 0; i < numberOfTles; i++) {
    readAnything(HEADER_SIZE + i * TLE_RECORD_SIZE,satName);
    satName[6] = '\0';
    Serial.println(satName);
    readAnything(HEADER_SIZE + 6 +  i *  TLE_RECORD_SIZE, YE);
    Serial.print("\tYE: ");
    Serial.println(YE);
    readAnything(HEADER_SIZE + 10 +  i *  TLE_RECORD_SIZE, TE);
    Serial.print("\tTE: ");
    Serial.println(TE);
    readAnything(HEADER_SIZE + 14 +  i *  TLE_RECORD_SIZE, M2);
    Serial.print("\tM2: ");
    Serial.println(M2);
    readAnything(HEADER_SIZE + 18 +  i *  TLE_RECORD_SIZE, IN);
    Serial.print("\tIN: ");
    Serial.println(IN);
    readAnything(HEADER_SIZE + 22 +  i *  TLE_RECORD_SIZE, RA);
    Serial.print("\tRA: ");
    Serial.println(RA);
    readAnything(HEADER_SIZE + 26 +  i *  TLE_RECORD_SIZE, EC);
    Serial.print("\tEC: ");
    Serial.println(EC);
    readAnything(HEADER_SIZE + 30 +  i *  TLE_RECORD_SIZE, WP);
    Serial.print("\tWP: ");
    Serial.println(WP);
    readAnything(HEADER_SIZE + 34 +  i *  TLE_RECORD_SIZE, MA);
    Serial.print("\tMA: ");
    Serial.println(MA);
    readAnything(HEADER_SIZE + 38 +  i *  TLE_RECORD_SIZE, MM);
    Serial.print("\tMM: ");
    Serial.println(MM);
    readAnything(HEADER_SIZE + 42 +  i *  TLE_RECORD_SIZE, RV);
    Serial.print("\tRV: ");
    Serial.println(RV);
    Serial.print("\tNumber of Modes: ");
    int noOfModes = countModelinesForTle(HEADER_SIZE +  i *  TLE_RECORD_SIZE);
    Serial.println(noOfModes);
    modelineStruct ml;
    for (int x = 0; x < noOfModes; x++) {
    unsigned int foo = getModelineStart(HEADER_SIZE +  i *  TLE_RECORD_SIZE, x);
    ml = getModelineForAddress(foo);
    Serial.print("\t\t");
    Serial.print(x);
    Serial.print(": ");
    Serial.print(ml.modeName);
    Serial.println(" ");
  }
  }
  uint16_t satAddress;
  char modeName[5];
  uint32_t ulLong, dlLong;
  char dlMode, ulMode, polarity;
  float ulShift, dlShift;
  unsigned int tone;
  int modelineStart = HEADER_SIZE + TLE_RECORD_SIZE * numberOfTles;
  for (int i = 0; i < numberOfModelines; i++) {
    readAnything(modelineStart + i * MODELINE_RECORD_SIZE,satAddress);
    Serial.print(satAddress);
    Serial.print(" (");
    char satName[6];
    readAnything(satAddress, satName);
    satName[6] = '\0';
    Serial.print(satName);
    Serial.println(")");
    readAnything(modelineStart + 2 + i * MODELINE_RECORD_SIZE, modeName);
    modeName[5] = '\0';
    Serial.print("\tmodeName: ");
    Serial.println(modeName);
    readAnything(modelineStart + 7 + i * MODELINE_RECORD_SIZE, dlLong);
    Serial.print("\tdownlink: ");
    Serial.println(dlLong);
    readAnything(modelineStart + 11 + i * MODELINE_RECORD_SIZE, ulLong);
    Serial.print("\tuplink: ");
    Serial.println(ulLong);
    readAnything(modelineStart + 15 + i * MODELINE_RECORD_SIZE, dlMode);
    Serial.print("\tdownlink Mode: ");
    Serial.println(dlMode,DEC);
    readAnything(modelineStart + 16 + i * MODELINE_RECORD_SIZE, ulMode);
    Serial.print("\tuplink Mode: ");
    Serial.println(ulMode,DEC);
    readAnything(modelineStart + 17 + i * MODELINE_RECORD_SIZE, polarity);
    Serial.print("\tpolarity: ");
    Serial.println(polarity,DEC);
    readAnything(modelineStart + 18 + i * MODELINE_RECORD_SIZE, dlShift);
    Serial.print("\tdlshift: ");
    Serial.println(dlShift);
    readAnything(modelineStart + 22 + i * MODELINE_RECORD_SIZE, ulShift);
    Serial.print("\tuplink shift: ");
    Serial.println(ulShift);
    readAnything(modelineStart + 26 + i * MODELINE_RECORD_SIZE, tone);
    Serial.print("\ttone: ");
    Serial.println(tone);
  }
}

boolean TleStoreCallback::readHeader() {
	readAnything(11, numberOfTles);
	readAnything(13, numberOfModelines);
//  numberOfTles = ((read(11) << 0) & 0xFF) + ((read(12) << 8) & 0xFF00);
//  numberOfModelines = ((read(13) << 0) & 0xFF) + ((read(14) << 8) & 0xFF00);
if (numberOfTles < 1 || numberOfTles > MAX_TLES) {return false;}
else {return true;}
}


	template <class T>  int TleStoreCallback::readAnything(int ee, T& value)
	{
	  byte* p = (byte*)(void*)&value;
	  int i;
	  for (i = 0; i < sizeof(value); i++)
	    *p++ = read(ee++);
	  return i;
	}
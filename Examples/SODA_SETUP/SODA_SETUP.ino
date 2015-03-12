//SODA HE 1.0 Project
//Peter Gould, 2014-05-13
//Setup initial time and ID for SODA HE 1.0 Board
//change the value of LOGGERID below to your choice of logger ID number.
//Always end with an L to designate it as a long type.  
//The clock is only updated and the SD card is only formatted when the LOGGERID
//changes to avoid resetting the clock every time the SODA HE is powered up.
#define LOGGERID 10001L 
#define override 0
//if you don't want to format the SD card, change the value below to zero.
#define FORMAT_SD 1
//OFFSET in seconds used to account for the difference between compile and upload time.
//It can be adjusted as needed but it only affects secs, mins, hrs, not days, months, yrs.
#define OFFSET 6 
#include <EEPROM.h>
#include "SODA.h"
#include <SdFat.h>
#include <Wire.h>
byte monthID[] = {
  25,13,32,35,39,45,43,29,40,38,51,12}; //values to look up month
const char dt[] = __DATE__ " " __TIME__; //generates date and time of compilation
//date-times values are in format:
//Feb 18 2014 15:42:38
//01234567890123456789       
//          1111111111 
int timeSet[6];
SODA soda;
//the following are used to format the SD card
//are are taken from the SDFormatter examples
//in the SDFat library
#define DEBUG_PRINT 0
#include <SdFat.h>
#if DEBUG_PRINT
#include <SdFatUtil.h>
#endif  // DEBUG_PRINT
#define sdError(msg) sdError_P(PSTR(msg))
//
// Change the value of chipSelect if your hardware does
// not use the default value, SS.  Common values are:
// Arduino Ethernet shield: pin 4
// Sparkfun SD shield: pin 8
// Adafruit SD shields and modules: pin 10
const uint8_t chipSelect = 17;

// Change spiSpeed to SPI_FULL_SPEED for better performance
// Use SPI_QUARTER_SPEED for even slower SPI bus speed
const uint8_t spiSpeed = SPI_HALF_SPEED;

// Serial output stream
ArduinoOutStream cout(Serial);

Sd2Card sdcard;
uint32_t cardSizeBlocks;
uint16_t cardCapacityMB;

// cache for SD block
cache_t cache;

// MBR information
uint8_t partType;
uint32_t relSector;
uint32_t partSize;

// Fake disk geometry
uint8_t numberOfHeads;
uint8_t sectorsPerTrack;

// FAT parameters
uint16_t reservedSectors;
uint8_t sectorsPerCluster;
uint32_t fatStart;
uint32_t fatSize;
uint32_t dataStart;

// constants for file system structure
uint16_t const BU16 = 128;
uint16_t const BU32 = 8192;

//  strings needed in file system structures
char noName[] = "NO NAME    ";
char fat16str[] = "FAT16   ";
char fat32str[] = "FAT32   ";

void setup(){
  //Only proceed if the ID number is new or override = 1.
  //This avoids reseting the time each time the logger is powered up.
  long currentID = soda.getID();
  if(LOGGERID != currentID || override != 0){
    soda.blinks(5); //indicate that setup is underway
    //first set time
    char temp1[] = {
      dt[7],dt[8],dt[9],dt[10],'\0'        };
    timeSet[0] = atoi(temp1);
    //determine the month using unique combinations of ASCII values
    byte check1 = dt[0] + dt[1] + dt[2];
    for(int k = 0; k<12; k++){
      if(check1 == monthID[k]) timeSet[1] = k + 1;
    }
    int j = 2;
    int spot = 4;
    while(j < 6){ 
      char temp2[] = {
        dt[spot],dt[spot+1],'\0'            };
      timeSet[j] = atoi(temp2);
      if(spot > 4) spot = spot + 3;
      if(spot == 4) spot = 12;
      j++;
    }
    //adjust for offset
    timeSet[5] = timeSet[5] + OFFSET;
    int max1 = 60;
    for(j = 5; j>2; j--){
      if(j==2) max1 = 24;
      if(timeSet[j] >= max1){
        timeSet[j] = timeSet[j] - max1;
        timeSet[j-1]++;
      }
    }
    //load into class time array
    for(j = 5; j>=0; j--){
      soda.updateTime(timeSet[j],j);
    }
    soda.setTime(); 
    //now set ID
    soda.setID(LOGGERID);
   if(FORMAT_SD == 1){
     //format the SD card
     if(!sdcard.init(SPI_HALF_SPEED,17)) soda.blinks(30);
     cardSizeBlocks = sdcard.cardSize();
    if (cardSizeBlocks == 0) sdError("cardSize");
    cardCapacityMB = (cardSizeBlocks + 2047)/2048;
    cout << pstr("Card Size: ") << cardCapacityMB;
    cout << pstr(" MB, (MB = 1,048,576 bytes)") << endl;
    eraseCard();
    formatCard();
   }
  writeSD();
  } //end if-then for new id
}

void loop(){
  soda.getTime();
  Serial.print("LOGGER ID = ");
  Serial.println(soda.getID());
  Serial.print("Current Time = ");
  soda.printBuffer();
  readSD();
  Serial.println();
  delay(2000);
}

void writeSD(){
  SdFat card2;
  SdFile file;
  card2.begin(17,SPI_HALF_SPEED);
  delay(100);
  file.open("TEST.TXT", O_CREAT | O_APPEND | O_WRITE | O_READ);
  file.println("This line was written to the SD Card.");
  file.sync();  //make sure the file is updated before closing
  file.close();
  delay(200);
}

void readSD(){
  SdFat card2;
  SdFile file;
  card2.begin(17,SPI_HALF_SPEED);
  delay(100);
  Serial.println("Trying to read from SD card...");
  file.open("TEST.TXT", O_CREAT | O_APPEND | O_WRITE | O_READ);
  char data;
  while ((data = file.read()) >= 0) Serial.print(char(data));
  file.close();
  Serial.println("...Testing Complete");
}

//////////////////////////////////////////////////////////////////////
//The following functions are borrowed from the sdFormatter sketch
//from the sdFat library examples
/////////////////////////////////////////////////////////////////////

void sdError_P(const char* str) {
  cout << pstr("error: ");
  cout << pgm(str) << endl;
  if (sdcard.errorCode()) {
    cout << pstr("SD error: ") << hex << int(sdcard.errorCode());
    cout << ',' << int(sdcard.errorData()) << dec << endl;
  }
  while (1);
}
//------------------------------------------------------------------------------
#if DEBUG_PRINT
void debugPrint() {
  cout << pstr("FreeRam: ") << FreeRam() << endl;
  cout << pstr("partStart: ") << relSector << endl;
  cout << pstr("partSize: ") << partSize << endl;
  cout << pstr("reserved: ") << reservedSectors << endl;
  cout << pstr("fatStart: ") << fatStart << endl;
  cout << pstr("fatSize: ") << fatSize << endl;
  cout << pstr("dataStart: ") << dataStart << endl;
  cout << pstr("clusterCount: ");
  cout << ((relSector + partSize - dataStart)/sectorsPerCluster) << endl;
  cout << endl;
  cout << pstr("Heads: ") << int(numberOfHeads) << endl;
  cout << pstr("Sectors: ") << int(sectorsPerTrack) << endl;
  cout << pstr("Cylinders: ");
  cout << cardSizeBlocks/(numberOfHeads*sectorsPerTrack) << endl;
}
#endif  // DEBUG_PRINT
//------------------------------------------------------------------------------
// write cached block to the card
uint8_t writeCache(uint32_t lbn) {
  return sdcard.writeBlock(lbn, cache.data);
}
//------------------------------------------------------------------------------
// initialize appropriate sizes for SD capacity
void initSizes() {
  if (cardCapacityMB <= 6) {
    sdError("Card is too small.");
  } else if (cardCapacityMB <= 16) {
    sectorsPerCluster = 2;
  } else if (cardCapacityMB <= 32) {
    sectorsPerCluster = 4;
  } else if (cardCapacityMB <= 64) {
    sectorsPerCluster = 8;
  } else if (cardCapacityMB <= 128) {
    sectorsPerCluster = 16;
  } else if (cardCapacityMB <= 1024) {
    sectorsPerCluster = 32;
  } else if (cardCapacityMB <= 32768) {
    sectorsPerCluster = 64;
  } else {
    // SDXC cards
    sectorsPerCluster = 128;
  }

  cout << pstr("Blocks/Cluster: ") << int(sectorsPerCluster) << endl;
  // set fake disk geometry
  sectorsPerTrack = cardCapacityMB <= 256 ? 32 : 63;

  if (cardCapacityMB <= 16) {
    numberOfHeads = 2;
  } else if (cardCapacityMB <= 32) {
    numberOfHeads = 4;
  } else if (cardCapacityMB <= 128) {
    numberOfHeads = 8;
  } else if (cardCapacityMB <= 504) {
    numberOfHeads = 16;
  } else if (cardCapacityMB <= 1008) {
    numberOfHeads = 32;
  } else if (cardCapacityMB <= 2016) {
    numberOfHeads = 64;
  } else if (cardCapacityMB <= 4032) {
    numberOfHeads = 128;
  } else {
    numberOfHeads = 255;
  }
}
//------------------------------------------------------------------------------
// zero cache and optionally set the sector signature
void clearCache(uint8_t addSig) {
  memset(&cache, 0, sizeof(cache));
  if (addSig) {
    cache.mbr.mbrSig0 = BOOTSIG0;
    cache.mbr.mbrSig1 = BOOTSIG1;
  }
}
//------------------------------------------------------------------------------
// zero FAT and root dir area on SD
void clearFatDir(uint32_t bgn, uint32_t count) {
  clearCache(false);
  if (!sdcard.writeStart(bgn, count)) {
    sdError("Clear FAT/DIR writeStart failed");
  }
  for (uint32_t i = 0; i < count; i++) {
    if ((i & 0XFF) == 0) cout << '.';
    if (!sdcard.writeData(cache.data)) {
      sdError("Clear FAT/DIR writeData failed");
    }
  }
  if (!sdcard.writeStop()) {
    sdError("Clear FAT/DIR writeStop failed");
  }
  cout << endl;
}
//------------------------------------------------------------------------------
// return cylinder number for a logical block number
uint16_t lbnToCylinder(uint32_t lbn) {
  return lbn / (numberOfHeads * sectorsPerTrack);
}
//------------------------------------------------------------------------------
// return head number for a logical block number
uint8_t lbnToHead(uint32_t lbn) {
  return (lbn % (numberOfHeads * sectorsPerTrack)) / sectorsPerTrack;
}
//------------------------------------------------------------------------------
// return sector number for a logical block number
uint8_t lbnToSector(uint32_t lbn) {
  return (lbn % sectorsPerTrack) + 1;
}
//------------------------------------------------------------------------------
// format and write the Master Boot Record
void writeMbr() {
  clearCache(true);
  part_t* p = cache.mbr.part;
  p->boot = 0;
  uint16_t c = lbnToCylinder(relSector);
  if (c > 1023) sdError("MBR CHS");
  p->beginCylinderHigh = c >> 8;
  p->beginCylinderLow = c & 0XFF;
  p->beginHead = lbnToHead(relSector);
  p->beginSector = lbnToSector(relSector);
  p->type = partType;
  uint32_t endLbn = relSector + partSize - 1;
  c = lbnToCylinder(endLbn);
  if (c <= 1023) {
    p->endCylinderHigh = c >> 8;
    p->endCylinderLow = c & 0XFF;
    p->endHead = lbnToHead(endLbn);
    p->endSector = lbnToSector(endLbn);
  } else {
    // Too big flag, c = 1023, h = 254, s = 63
    p->endCylinderHigh = 3;
    p->endCylinderLow = 255;
    p->endHead = 254;
    p->endSector = 63;
  }
  p->firstSector = relSector;
  p->totalSectors = partSize;
  if (!writeCache(0)) sdError("write MBR");
}
//------------------------------------------------------------------------------
// generate serial number from card size and micros since boot
uint32_t volSerialNumber() {
  return (cardSizeBlocks << 8) + micros();
}
//------------------------------------------------------------------------------
// format the SD as FAT16
void makeFat16() {
  uint32_t nc;
  for (dataStart = 2 * BU16;; dataStart += BU16) {
    nc = (cardSizeBlocks - dataStart)/sectorsPerCluster;
    fatSize = (nc + 2 + 255)/256;
    uint32_t r = BU16 + 1 + 2 * fatSize + 32;
    if (dataStart < r) continue;
    relSector = dataStart - r + BU16;
    break;
  }
  // check valid cluster count for FAT16 volume
  if (nc < 4085 || nc >= 65525) sdError("Bad cluster count");
  reservedSectors = 1;
  fatStart = relSector + reservedSectors;
  partSize = nc * sectorsPerCluster + 2 * fatSize + reservedSectors + 32;
  if (partSize < 32680) {
    partType = 0X01;
  } else if (partSize < 65536) {
    partType = 0X04;
  } else {
    partType = 0X06;
  }
  // write MBR
  writeMbr();
  clearCache(true);
  fat_boot_t* pb = &cache.fbs;
  pb->jump[0] = 0XEB;
  pb->jump[1] = 0X00;
  pb->jump[2] = 0X90;
  for (uint8_t i = 0; i < sizeof(pb->oemId); i++) {
    pb->oemId[i] = ' ';
  }
  pb->bytesPerSector = 512;
  pb->sectorsPerCluster = sectorsPerCluster;
  pb->reservedSectorCount = reservedSectors;
  pb->fatCount = 2;
  pb->rootDirEntryCount = 512;
  pb->mediaType = 0XF8;
  pb->sectorsPerFat16 = fatSize;
  pb->sectorsPerTrack = sectorsPerTrack;
  pb->headCount = numberOfHeads;
  pb->hidddenSectors = relSector;
  pb->totalSectors32 = partSize;
  pb->driveNumber = 0X80;
  pb->bootSignature = EXTENDED_BOOT_SIG;
  pb->volumeSerialNumber = volSerialNumber();
  memcpy(pb->volumeLabel, noName, sizeof(pb->volumeLabel));
  memcpy(pb->fileSystemType, fat16str, sizeof(pb->fileSystemType));
  // write partition boot sector
  if (!writeCache(relSector)) {
    sdError("FAT16 write PBS failed");
  }
  // clear FAT and root directory
  clearFatDir(fatStart, dataStart - fatStart);
  clearCache(false);
  cache.fat16[0] = 0XFFF8;
  cache.fat16[1] = 0XFFFF;
  // write first block of FAT and backup for reserved clusters
  if (!writeCache(fatStart)
    || !writeCache(fatStart + fatSize)) {
    sdError("FAT16 reserve failed");
  }
}
//------------------------------------------------------------------------------
// format the SD as FAT32
void makeFat32() {
  uint32_t nc;
  relSector = BU32;
  for (dataStart = 2 * BU32;; dataStart += BU32) {
    nc = (cardSizeBlocks - dataStart)/sectorsPerCluster;
    fatSize = (nc + 2 + 127)/128;
    uint32_t r = relSector + 9 + 2 * fatSize;
    if (dataStart >= r) break;
  }
  // error if too few clusters in FAT32 volume
  if (nc < 65525) sdError("Bad cluster count");
  reservedSectors = dataStart - relSector - 2 * fatSize;
  fatStart = relSector + reservedSectors;
  partSize = nc * sectorsPerCluster + dataStart - relSector;
  // type depends on address of end sector
  // max CHS has lbn = 16450560 = 1024*255*63
  if ((relSector + partSize) <= 16450560) {
    // FAT32
    partType = 0X0B;
  } else {
    // FAT32 with INT 13
    partType = 0X0C;
  }
  writeMbr();
  clearCache(true);

  fat32_boot_t* pb = &cache.fbs32;
  pb->jump[0] = 0XEB;
  pb->jump[1] = 0X00;
  pb->jump[2] = 0X90;
  for (uint8_t i = 0; i < sizeof(pb->oemId); i++) {
    pb->oemId[i] = ' ';
  }
  pb->bytesPerSector = 512;
  pb->sectorsPerCluster = sectorsPerCluster;
  pb->reservedSectorCount = reservedSectors;
  pb->fatCount = 2;
  pb->mediaType = 0XF8;
  pb->sectorsPerTrack = sectorsPerTrack;
  pb->headCount = numberOfHeads;
  pb->hidddenSectors = relSector;
  pb->totalSectors32 = partSize;
  pb->sectorsPerFat32 = fatSize;
  pb->fat32RootCluster = 2;
  pb->fat32FSInfo = 1;
  pb->fat32BackBootBlock = 6;
  pb->driveNumber = 0X80;
  pb->bootSignature = EXTENDED_BOOT_SIG;
  pb->volumeSerialNumber = volSerialNumber();
  memcpy(pb->volumeLabel, noName, sizeof(pb->volumeLabel));
  memcpy(pb->fileSystemType, fat32str, sizeof(pb->fileSystemType));
  // write partition boot sector and backup
  if (!writeCache(relSector)
    || !writeCache(relSector + 6)) {
    sdError("FAT32 write PBS failed");
  }
  clearCache(true);
  // write extra boot area and backup
  if (!writeCache(relSector + 2)
    || !writeCache(relSector + 8)) {
    sdError("FAT32 PBS ext failed");
  }
  fat32_fsinfo_t* pf = &cache.fsinfo;
  pf->leadSignature = FSINFO_LEAD_SIG;
  pf->structSignature = FSINFO_STRUCT_SIG;
  pf->freeCount = 0XFFFFFFFF;
  pf->nextFree = 0XFFFFFFFF;
  // write FSINFO sector and backup
  if (!writeCache(relSector + 1)
    || !writeCache(relSector + 7)) {
    sdError("FAT32 FSINFO failed");
  }
  clearFatDir(fatStart, 2 * fatSize + sectorsPerCluster);
  clearCache(false);
  cache.fat32[0] = 0x0FFFFFF8;
  cache.fat32[1] = 0x0FFFFFFF;
  cache.fat32[2] = 0x0FFFFFFF;
  // write first block of FAT and backup for reserved clusters
  if (!writeCache(fatStart)
    || !writeCache(fatStart + fatSize)) {
    sdError("FAT32 reserve failed");
  }
}
//------------------------------------------------------------------------------
// flash erase all data
uint32_t const ERASE_SIZE = 262144L;
void eraseCard() {
  cout << endl << pstr("Erasing\n");
  uint32_t firstBlock = 0;
  uint32_t lastBlock;
  uint16_t n = 0;

  do {
    lastBlock = firstBlock + ERASE_SIZE - 1;
    if (lastBlock >= cardSizeBlocks) lastBlock = cardSizeBlocks - 1;
    if (!sdcard.erase(firstBlock, lastBlock)) sdError("erase failed");
    cout << '.';
    if ((n++)%32 == 31) cout << endl;
    firstBlock += ERASE_SIZE;
  } while (firstBlock < cardSizeBlocks);
  cout << endl;

  if (!sdcard.readBlock(0, cache.data)) sdError("readBlock");
  cout << hex << showbase << setfill('0') << internal;
  cout << pstr("All data set to ") << setw(4) << int(cache.data[0]) << endl;
  cout << dec << noshowbase << setfill(' ') << right;
  cout << pstr("Erase done\n");
}
//------------------------------------------------------------------------------
void formatCard() {
  cout << endl;
  cout << pstr("Formatting\n");
  initSizes();
  if (sdcard.type() != SD_CARD_TYPE_SDHC) {
    cout << pstr("FAT16\n");
    makeFat16();
  } else {
    cout << pstr("FAT32\n");
    makeFat32();
  }
#if DEBUG_PRINT
  debugPrint();
#endif  // DEBUG_PRINT
  cout << pstr("Format done\n");
}






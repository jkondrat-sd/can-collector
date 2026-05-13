#include "FreematicsSD.h"

#define PIN_SD_OBD 5

void TaskSD::checkSD() {  
  if (!SD.begin(PIN_SD_OBD)) 
  {
    Serial.println("SD Card MOUNT FAIL");
  }
  else 
  {
    Serial.println("SD Card MOUNT SUCCESS");
    Serial.println("");
    uint32_t cardSize = SD.cardSize() / (1024 * 1024);
    String str = "SDCard Size: " + String(cardSize) + "MB";
    Serial.println(str);
    uint8_t cardType = SD.cardType();
    if(cardType == CARD_NONE)
    {
      Serial.println("No SD card attached");
      initSd = false;
      return;
    }
    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC)
    {
      Serial.println("MMC");
    } 
    else if(cardType == CARD_SD)
    {
      Serial.println("SDSC");
    } 
    else if(cardType == CARD_SDHC)
    {
      Serial.println("SDHC");
    } 
    else 
    {
      Serial.println("UNKNOWN");
    }
    initSd = true;
    myFile = SD.open("/");
    printDirectory(myFile, 0);
    myFile.close();
    Serial.println("");
  }
  return;
}

uint32_t TaskSD::fileSD() {
  if(!initSd) {
    Serial.println("No SD card init");
    return 0;
  }
  File root = SD.open("/DATA");
  m_id = getFileID(root);
  if (m_id == 0) {
    SD.mkdir("/DATA");
    m_id = 1;
  }
  char path[24];
  sprintf(path, "/DATA/%u.CSV", m_id);
  Serial.printf("File: ");
  Serial.println(path);
  m_dataCount = 0;
  m_size = 0;
  myFile = SD.open(path, FILE_WRITE);
  if (!myFile) {
    Serial.println("File error");
    m_id = 0;
  }
  return m_id;
}

void TaskSD::flush() {
  char path[24];
  sprintf(path, "/DATA/%u.CSV", m_id);
  myFile.close();
  myFile = SD.open(path, FILE_APPEND);
  if (!myFile) {
    Serial.println("File error");
  }
}

void TaskSD::write(const char* buf, byte len) {
  if (m_next) m_next->write(buf,len);
  myFile.print(m_dataCount);
  myFile.write(',');
  myFile.write((uint8_t*)buf, len);
  // myFile.write('\n');
  m_dataCount++;
}

void TaskSD::writeHeader(const char* buf, byte len) {
  if (m_next) m_next->write(buf,len);
  myFile.write((uint8_t*)buf, len);
}

void TaskSD::close() {
  myFile.close();
  m_id = 0;
  m_dataCount = 0;
}

void TaskSD::tempClose() {
  myFile.close();
}

void TaskSD::tempOpen() {
  char path[24];
  sprintf(path, "/DATA/%u.CSV", m_id);
  myFile = SD.open(path, FILE_APPEND);
  if (!myFile) {
    Serial.println("File error");
    fileOpen = false;
  }
  fileOpen = true;
}

uint32_t TaskSD::size() {
  // return myFile.position();
  return myFile.size();
}

uint32_t TaskSD::getDataCount() {
  return m_dataCount;
}

bool TaskSD::statusSD() {
  return initSd;
}

// printDirectory
void TaskSD::printDirectory(File dir, int numTabs) 
{
  while (true) 
  {
    File entry =  dir.openNextFile();
    if (! entry) 
    {
      
     // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) 
    {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) 
    {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } 
    else 
    {

      // Files have sizes, directories do not.
      Serial.print("\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

void TaskSD::logData(char* name, int value) {
  char buf[24];
  byte len = sprintf(buf, "%s,%d", name, value);
  write(buf, len);
}

void TaskSD::logDataCan(int value) {
  char buf[24];
  byte len = sprintf(buf,"%d",value);
  writeHeader(buf, len);
}

void TaskSD::logDataChar(const char* value) {
  char buf[30];
  byte len = sprintf(buf, "%s", value);
  writeHeader(buf, len);
}
void TaskSD::logDataFloat(char* name, float value) {
  char buf[25];
  byte len = sprintf(buf, "%s,%2f", name, value);
  write(buf, len);
}

void TaskSD::logDataFloatCan(float value) {
  char buf[25];
  byte len = sprintf(buf, "%2f", value);
  writeHeader(buf, len);
}

void TaskSD::logDataMultiFloat(char* name, float value1, float value2, float value3) {
  char buf[50];
  byte len = sprintf(buf,"%s, %2f, %2f, %2f",name, value1, value2, value3);
  write(buf, len);
}

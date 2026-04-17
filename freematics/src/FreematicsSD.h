#include "SD.h"
#include "SPI.h"
#include "FS.h"

#ifndef FREEMATICS_SD
#define FREEMATICS_SD

class TaskSD {
public:
  void checkSD(); // create new directory if not exists
  uint32_t fileSD(); // /create new file with new numeration
  void flush(); 
  void write(const char* buf, byte len); // write new line in file
  void close(); // close file
  void tempClose();
  void tempOpen();
  uint32_t size(); // return size file
  uint32_t getDataCount();
  bool statusSD();
  void printDirectory(File dir, int numTabs);

  void logData(char* name, int value);
  void logDataFloat(char* name, float value);
  void logDataMultiFloat(char* name, float value1, float value2, float value3);

protected:
  int getFileID(File& root) {
    m_dataCount = 0;
    if (root) {
      int id = 0;
      File file;
      while (file = root.openNextFile()) {
        char *p = strrchr(file.name(), '/');
        unsigned int n = atoi(p ? p + 1 : file.name());
        if(n > id) id = n;
      }
      return id + 1;
    }
    return 0;
  }

  bool initSd = false;
  bool fileOpen = false;
  uint32_t m_dataTime = 0;
  uint32_t m_dataCount = 0;
  uint32_t m_id = 0;
  uint32_t m_size = 0;
  File myFile;
  TaskSD* m_next = 0;
};


#endif
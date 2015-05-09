/*
 * Generic Data Logger
 * A starting framework to make easy-to-manage dataloggers.
 * Created by Ashley Gillman, 09/05/15
*/

/*
#if defined(ARDUINO) && ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
*/

// Function Pointer types for getting and putting data.
typedef void(*ReadingFnPointer)(char*); // function w/ no args that returns a str
typedef void(*DumpFnPointer)(char*, short, short); // function takes str array w/ no return

class DataLogger {
public:
  DataLogger();
  DataLogger(short nSensors, short maxReadingLen, short nDumps);
  
  void addReading(ReadingFnPointer rf);
  void addDump(DumpFnPointer df);
  void takeReadings();
  void dumpReadings();

private:
  short _nSensors;
  short _nDumps;
  short _readingWidth;
  short _sensorInd = 0;
  short _dumpInd = 0;
  ReadingFnPointer* readingFs;
  DumpFnPointer* dumpFs;
  char sensorData[100];
};



#include "DataLogger.h"

DataLogger::DataLogger()
{
  DataLogger(0, 0, 0);
}

DataLogger::DataLogger(short nSensors, short maxReadingLen, short nDumps)
{
  _nSensors = nSensors;
  _nDumps = nDumps;
  _readingWidth = maxReadingLen;
  readingFs = new ReadingFnPointer [nSensors];
  dumpFs = new DumpFnPointer [nDumps];
  //sensorData = (char*)malloc(nSensors * maxReadingLen * sizeof(char));// new char[nSensors * maxReadingLen]();
}

void DataLogger::addReading(ReadingFnPointer f)
{
  if (_sensorInd < _nSensors)
    readingFs[_sensorInd++] = f;
  //else
    //Serial.println("DataLogger err: 1");
}

void DataLogger::addDump(DumpFnPointer f)
{
  if (_dumpInd < _nDumps)
    dumpFs[_dumpInd++] = f;
  //else
    //Serial.println("DataLogger err: 2");
}

void DataLogger::takeReadings()
{
  for (int i=0; i<_nSensors; ++i) {
    char buf[_readingWidth+1];
    readingFs[i](buf);
    for (int j=0; j<_readingWidth; ++j) {
      if (buf[j] != '\0')
        sensorData[i*_readingWidth+j] = buf[j];
      else
        sensorData[i*_readingWidth+j] = 'x';
    }
  }
}

void DataLogger::dumpReadings()
{
  for (int i=0; i<_nDumps; ++i) {
    dumpFs[i](sensorData, _nSensors, _readingWidth);
  }
}

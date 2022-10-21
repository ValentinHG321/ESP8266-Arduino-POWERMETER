void SDSave(int timeDay, int timeMonth, int timeYear, String timeHS, String timeMinS, String timeSecS, double avgV , double avgA , double avgCF, double avgSF , String file)
{
  char buff[75];
  String avgVS = String(avgV , 2);
  String avgAS = String(avgA , 2);
  String avgSFS = String(avgSF , 2);
  String avgCFS = String(avgCF , 2);

  sprintf(buff, "%d/%d/%d|%s:%s:%s|%s|%s|%s|%s", timeDay, timeMonth, timeYear, timeHS, timeMinS, timeSecS, avgVS , avgAS, avgCFS, avgSFS);
            //   30/12/99|24:60:60|230.33|120.33|0.99|0.99
  delay(1);

  Serial.print("Saved to ");
  Serial.print(file);
  Serial.print(": ");
  Serial.println(buff);

  File dataFile = SD.open(file, FILE_WRITE);
  if (dataFile)
  {
    dataFile.print(buff);
    dataFile.print(']');
    dataFile.close();
  }
  else
  {
    Serial.print("Error: Can't Save into ");
    Serial.println(file);
  }
  delay(10);
}

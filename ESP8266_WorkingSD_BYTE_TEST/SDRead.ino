String SDRead(String file)
{
  String output = "";
  File dataByteFile = SD.open(file);
  if (dataByteFile)
  {

    while (dataByteFile.available())
    {
      char read = dataByteFile.read();
      output += read;
    }
    dataByteFile.close();
    delay(10);
    return output;
  } /*else {
    Serial.println("Error: Can't Read ");
    Serial.println(file);
  }*/
  
}

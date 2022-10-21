void calcVI()
{
  const double ADC_COUNTS = 1024;
  unsigned int crossCount = 0;
  unsigned int crossCountA = 0;
  unsigned int numberOfSamples = 0;
  unsigned int numberOfSamplesA = 0;
  unsigned int crossings = 8;
  unsigned int timeout = 3000;
  double filteredV = 0 , filteredI = 0;
  double sqV = 0, sumV = 0, sqI = 0, sumI = 0, instP = 0, sumP = 0, Vrms = 0, Irms = 0, powerFactor = 0, apparentPower = 0, realPower = 0;
  int startV = 0, sampleV = 0, sampleI = 0;
  bool lastVCross  , checkVCross ;
  bool lastACross  , checkACross ;
  double offsetV = 512;
  double offsetI = 512;
  unsigned long sumMicros = 0;
  unsigned long initialMicros = 0 ;
  unsigned long secondMicros = 0;

  unsigned long start = millis();

  while (1)    //waiting for zero cross
  {
    startV = analogRead(A1);

    if ((startV < (ADC_COUNTS * 0.55)) && (startV > (ADC_COUNTS * 0.45))) break;

    if ((millis() - start) > timeout) break;
  }

  start = millis();

  while ((crossCount < crossings) && ((millis() - start) < timeout))
  {
    numberOfSamples++;             //number of volt samples

    sampleV = analogRead(A1);
    sampleI = analogRead(A0);

    if (sampleI >= 511 && sampleI <= 513) //ignoring measurig error
    {
      sampleI = 512;
    }

    filteredV = sampleV - offsetV;
    filteredI = sampleI - offsetI;

    sqV = filteredV * filteredV;
    sumV += sqV;

    sqI = filteredI * filteredI;
    sumI += sqI;

    //volts sine zero crossCheck
    lastVCross = checkVCross;

    if (sampleV > startV)
    {
      checkVCross = true;
    }
    else
    {
      checkVCross = false;
    }

    if (numberOfSamples == 1)
    {
      lastVCross = checkVCross;
      initialMicros = micros();
    }

    if (lastVCross != checkVCross)
    {
      crossCount++; //number of zero crossings
      initialMicros =  micros();  //geting current timestamp of volts sine wave zero cross in microsec
    }

    //amps sine zero crossCheck
    lastACross = checkACross;

    if (sampleI > 512)
    {
      checkACross = true;
    }
    else
    {
      checkACross = false;
    }

    if (numberOfSamplesA == 1)
    {
      lastACross = checkACross;
    }

    if (lastACross != checkACross)
    {
      crossCountA++;
      secondMicros = micros();  //geting current timestamp of amps sine wave zero cross in microsec

      sumMicros += (secondMicros - initialMicros);  //substracting from volt zero cross timestamp with amps zero cross timestamp to get the time delay for the phaseshift

    }
  }

  double V_RATIO = 1.32;
  Vrms = V_RATIO * sqrt(sumV / numberOfSamples);

  double I_RATIO = 0.048;
  Irms = I_RATIO * sqrt(sumI / numberOfSamples);

  double doubleSumMicros = sumMicros;
  doubleSumMicros /=  crossCountA;
  int phaseCalibration = 1850;
  double phase = doubleSumMicros - phaseCalibration;

  double rad = (6.283185 * 50 * phase / 1000000);
  double cosF = cos(rad);
  double sinF = sin(rad);

  //realPower = Vrms * Irms * cosF;

  //double reactivePower = Vrms * Irms * sinF;

  sumV = 0;
  sumI = 0;
  sumP = 0;
  sumMicros = 0;

  if (cosF < 0 || isnan(cosF))
  {
    cosF = 0;
  }

  if (sinF < 0.05 || isnan(sinF))
  {
    sinF = 0;
  }

  phase = 0;

  espSerial.print("|");
  espSerial.print("/");
  espSerial.print(Vrms);
  espSerial.print(":");
  espSerial.print(Irms);
  espSerial.print(";");
  espSerial.print(cosF);
  espSerial.print("<");
  espSerial.print(sinF);
  espSerial.println("=");
  
  delay(1);
  
  Serial.print("Vrms: ");
  Serial.print(Vrms);
  Serial.print(", Irms: ");
  Serial.print(Irms);
  Serial.print(", cosF: ");
  Serial.print(cosF);
  Serial.print(", sinF: ");
  Serial.print(sinF);
  Serial.print(", micros: ");
  Serial.print(doubleSumMicros);
  Serial.print(", phase: ");
  Serial.println(phase);
  //crossCount = 0;
  //crossCountA = 0;

  // |/230.33:120.33;0.99<0.99=
  delay(10);
}

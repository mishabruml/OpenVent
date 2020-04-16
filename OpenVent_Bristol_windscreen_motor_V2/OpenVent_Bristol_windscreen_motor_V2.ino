/*
  OpenVent-Bristol basic design based on windscreen motor from MK1 Suzuki Swift
  with aim of meeting design specification: https://www.gov.uk/government/publications/coronavirus-covid-19-ventilator-supply-specification/rapidly-manufactured-ventilator-system-specification

  Relevant requirements from MHRA UK spec document:
  - Default: "default settings of 90-100% oxygen, 400ml/s tidal volume and / or inspiratory plateau pressure 35 cmH2O, 15 cmH2O PEEP, rate 20 breaths min-1 ."
  - Display: "Must show the current settings of tidal volume, frequency, PEEP, FiO2, ventilation mode"
  - Display: "Must show the actual current airway pressure"
  - Alarm on:
            - Gas or electricity supply failure
            - Machine switched off while in mandatory ventilation mode
            - Inspiratory airway pressure exceeded
            - Inspiratory and PEEP pressure not achieved (equivalent to disconnection alarm)
            - Tidal volume not achieved or exceeded.
*/

#include <LiquidCrystal.h>
// select the pins used on the Velleman LCD button Arduino shield
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
#define buttonsLCDpin A3        // Analogue input for buttons on LCD shield
#define btnRIGHT 0
#define btnUP 1
#define btnDOWN 2
#define btnLEFT 3
#define btnSELECT 4
#define btnNONE 5

#define maxPressure 35                // set in NHS spec
#define minPressure 10                // 10 is set higher than the lowest PEEP for failsafe

#define pressureSensorPin A2          // A0 & A1 are motor current sense pins
#define flowSensePin A4           // pin for pressure sensor in flow sensor

// Status
#define inhale 0
#define exhale 1

static int exhalePeriod = 0;
static int inhalePeriod = 0;


void setup() {
  Serial.begin(115200);

  lcd.begin(16, 2);

  lcd.setCursor(0, 0);
  lcd.print("OpenVent-Bristol");    // headings to LCD
  lcd.setCursor(0, 1);
  lcd.print("Alert system");
  delay(2000);
  lcd.setCursor(0, 0);
  lcd.print("Scroll using    ");    // headings to LCD
  lcd.setCursor(0, 1);
  lcd.print("lft & rgt keys  ");
  delay(2000);

  setDisplayHeadings();
}

int setDisplayHeadings ()
{
  // requires display of settings: tidal volume, frequency, PEEP, FiO2, ventilation mode & actual current airway pressure
  lcd.setCursor(0, 0);
  //lcd.print("BPM cmH2O VOLml I:E Status");      // headings to LCD
  lcd.print("BPM cmH2O VOLml I:E Mode");          // headings to LCD
  lcd.setCursor(0, 1);
  lcd.print("                            ");      // clear number display
  lcd.setCursor(4, 1);
  lcd.print("PK:");                               // print before the number
  lcd.setCursor(16, 1);
  lcd.print("1:");                                // print the static part of the I:E ratio
}

void loop() {
  // ********* UI stuff **********
  static int cursorPos;               // position variable of select cursor
  static unsigned long buttonTimer;   // lof time for how long the button has been pressed for for debounce
  static int buttonState;
  static int lastButtonState;
  const int debounceTime = 150;       // time period for debouncing the buttons
  static int screenLatPos = 0;
  static int screenLatPosTarget = 0;

  // location of numbers on LCD display below their corrisponding headings
  const int BPM_cell = 0;
  const int cmH2O_cell = 7;
  const int vol_cell = 10;
  const int IE_cell = 18;
  const int mode_cell = 20;

  int lcd_key = read_LCD_buttons(); // read the buttons
  if (lcd_key != lastButtonState) buttonTimer = millis();    // reset the debouncing timer if state change
  if (millis() - buttonTimer > debounceTime)                 // if button pressed for longer than delay
  {
    if (lcd_key != buttonState)
    {
      buttonState = lcd_key;       // if there is a new button reading
      switch (buttonState)                                     //
      {
        case btnRIGHT:        // change cursor position
          {
            screenLatPosTarget++;
            //if (cursorPos == BPM_cell) cursorPos = cmH2O_cell;              // only needed for changing values
            //else if (cursorPos == cmH2O_cell) cursorPos = vol_cell;
            //else if (cursorPos == vol_cell) cursorPos = IE_cell;
            //else if (cursorPos == IE_cell) cursorPos = status_cell;
            //cursorPos = constrain(cursorPos, 0, status_cell);
            //lcd.setCursor(cursorPos, 1);         // format (cell, line)
            break;
          }
        case btnLEFT:         // change cursor position
          {
            screenLatPosTarget--;
            //if (cursorPos == status_cell) cursorPos = IE_cell;              // only needed for changing values
            //else if (cursorPos == IE_cell) cursorPos = vol_cell;
            //else if (cursorPos == vol_cell) cursorPos = cmH2O_cell;
            //else if (cursorPos == cmH2O_cell) cursorPos = BPM_cell;
            //cursorPos = constrain(cursorPos, 0, status_cell);
            //lcd.setCursor(cursorPos, 1);         // format (cell, line)
            break;
          }
        case btnUP:           // change target value
          {
            break;
          }
        case btnDOWN:         // change target value
          {
            break;
          }
      }
    }
  }
  lastButtonState = lcd_key;


  // read new values
  int cmH2Oreading = 0;
  int volumeReading = getVolumePerBreath();
  int breathStatusReading = getBreathStatus();
  int BPMreading = getBPM();
  int IEratioReading = getIEratio();

  // variables to remember previsous values. Statics are only run once by the code
  static int lastbreathStatusReading = getBreathStatus();

  // write targets to LCD     CHANGE TO ONLY UPDATE WHEN NEW VALUES (haven't implimented this yet, this will speed up code a bit)

  // write targets to LCD only at srart of exhale
  if (breathStatusReading != lastbreathStatusReading)     // Note: this doesn't work brilliantly, it gets called a few times instead of once
  {
    if (breathStatusReading == inhale)
    {
      lcd.setCursor(mode_cell, 1);    // move cursor below Mode
      lcd.print("inhale");
      //Serial.println("inhale");
    }
    if (breathStatusReading == exhale)
    {
      setDisplayHeadings();           // clear display
      lcd.setCursor(cmH2O_cell, 1);   // move cursor below cmH2O
      lcd.print(cmH2Oreading);
      lcd.setCursor(IE_cell, 1);      // move cursor below I:E
      lcd.print(IEratioReading);
      lcd.setCursor(BPM_cell, 1);     // move cursor below BPM
      lcd.print(BPMreading);
      lcd.setCursor(vol_cell, 1);     // move cursor below Mode
      lcd.print(volumeReading);
      lcd.setCursor(mode_cell, 1);    // move cursor below Mode
      lcd.print("exhale");
      //Serial.println("       exhale");
    }
    //lcd.setCursor(cursorPos, 1);              // cursor positioning is needed for changing setting values
    //lcd.blink();                              // blink in current cusor position
  }
  lastbreathStatusReading = breathStatusReading;

  // shift characters along if needed
  screenLatPosTarget = constrain(screenLatPosTarget, 0, 10);
  if (screenLatPosTarget != screenLatPos)
  {
    if (screenLatPosTarget > screenLatPos) lcd.scrollDisplayLeft();
    if (screenLatPosTarget < screenLatPos) lcd.scrollDisplayRight();
    screenLatPos = screenLatPosTarget;
  }
}

int getFlowRate ()
{
  // ADC count, flow (ml/s) lookup table
  static int flowArray[644] = {
    0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 0
    , 159
    , 166
    , 177
    , 189
    , 200
    , 211
    , 221
    , 231
    , 240
    , 249
    , 257
    , 266
    , 274
    , 282
    , 289
    , 297
    , 304
    , 311
    , 317
    , 324
    , 331
    , 337
    , 343
    , 349
    , 355
    , 361
    , 367
    , 373
    , 379
    , 384
    , 389
    , 395
    , 400
    , 405
    , 410
    , 416
    , 420
    , 425
    , 430
    , 435
    , 440
    , 444
    , 449
    , 453
    , 458
    , 463
    , 467
    , 472
    , 476
    , 480
    , 484
    , 489
    , 493
    , 497
    , 501
    , 505
    , 509
    , 513
    , 517
    , 521
    , 525
    , 529
    , 533
    , 536
    , 540
    , 544
    , 548
    , 552
    , 555
    , 559
    , 562
    , 566
    , 570
    , 573
    , 577
    , 580
    , 584
    , 587
    , 591
    , 594
    , 597
    , 601
    , 604
    , 607
    , 611
    , 614
    , 617
    , 621
    , 624
    , 627
    , 630
    , 633
    , 637
    , 640
    , 643
    , 646
    , 649
    , 652
    , 655
    , 658
    , 661
    , 664
    , 667
    , 670
    , 673
    , 676
    , 679
    , 682
    , 685
    , 688
    , 691
    , 694
    , 697
    , 699
    , 702
    , 705
    , 708
    , 711
    , 713
    , 716
    , 719
    , 722
    , 725
    , 727
    , 730
    , 733
    , 735
    , 738
    , 741
    , 744
    , 746
    , 749
    , 751
    , 754
    , 757
    , 759
    , 762
    , 764
    , 767
    , 769
    , 772
    , 775
    , 777
    , 780
    , 782
    , 785
    , 787
    , 790
    , 792
    , 795
    , 797
    , 799
    , 802
    , 804
    , 807
    , 809
    , 812
    , 814
    , 816
    , 819
    , 821
    , 824
    , 826
    , 828
    , 831
    , 833
    , 835
    , 838
    , 840
    , 842
    , 845
    , 847
    , 849
    , 852
    , 854
    , 856
    , 858
    , 861
    , 863
    , 865
    , 867
    , 870
    , 872
    , 874
    , 876
    , 878
    , 881
    , 883
    , 885
    , 887
    , 889
    , 891
    , 894
    , 896
    , 898
    , 900
    , 902
    , 904
    , 906
    , 909
    , 911
    , 913
    , 915
    , 917
    , 919
    , 921
    , 923
    , 925
    , 927
    , 930
    , 931
    , 934
    , 936
    , 938
    , 940
    , 942
    , 944
    , 946
    , 948
    , 950
    , 952
    , 954
    , 956
    , 958
    , 960
    , 962
    , 964
    , 966
    , 968
    , 970
    , 972
    , 974
    , 976
    , 978
    , 980
    , 981
    , 983
    , 985
    , 987
    , 989
    , 991
    , 993
    , 995
    , 997
    , 999
    , 1001
    , 1002
    , 1004
    , 1006
    , 1008
    , 1010
    , 1012
    , 1014
    , 1016
    , 1017
    , 1019
    , 1021
    , 1023
    , 1025
    , 1027
    , 1028
    , 1030
    , 1032
    , 1034
    , 1036
    , 1038
    , 1039
    , 1041
    , 1043
    , 1045
    , 1046
    , 1048
    , 1050
    , 1052
    , 1054
    , 1056
    , 1057
    , 1059
    , 1061
    , 1063
    , 1064
    , 1066
    , 1068
    , 1070
    , 1071
    , 1073
    , 1075
    , 1077
    , 1078
    , 1080
    , 1082
    , 1083
    , 1085
    , 1087
    , 1089
    , 1090
    , 1092
    , 1094
    , 1095
    , 1097
    , 1099
    , 1100
    , 1102
    , 1104
    , 1105
    , 1107
    , 1109
    , 1111
    , 1112
    , 1114
    , 1115
    , 1117
    , 1119
    , 1120
    , 1122
    , 1124
    , 1125
    , 1127
    , 1129
    , 1130
    , 1132
    , 1134
    , 1135
    , 1137
    , 1138
    , 1140
    , 1142
    , 1143
    , 1145
    , 1147
    , 1148
    , 1150
    , 1151
    , 1153
    , 1155
    , 1156
    , 1158
    , 1159
    , 1161
    , 1162
    , 1164
    , 1166
    , 1167
    , 1169
    , 1170
    , 1172
    , 1173
    , 1175
    , 1177
    , 1178
    , 1180
    , 1181
    , 1183
    , 1184
    , 1186
    , 1187
    , 1189
    , 1190
    , 1192
    , 1193
    , 1195
    , 1197
    , 1198
    , 1200
    , 1201
    , 1203
    , 1204
    , 1206
    , 1207
    , 1209
    , 1210
    , 1212
    , 1213
    , 1215
    , 1216
    , 1218
    , 1219
    , 1220
    , 1222
    , 1223
    , 1225
    , 1226
    , 1228
    , 1229
    , 1231
    , 1232
    , 1234
    , 1235
    , 1237
    , 1238
    , 1240
    , 1241
    , 1242
    , 1244
    , 1245
    , 1247
    , 1248
    , 1250
    , 1251
    , 1253
    , 1254
    , 1256
    , 1257
    , 1258
    , 1260
    , 1261
    , 1263
    , 1264
    , 1265
    , 1267
    , 1268
    , 1270
    , 1271
    , 1272
    , 1274
    , 1275
    , 1277
    , 1278
    , 1279
    , 1281
    , 1282
    , 1284
    , 1285
    , 1287
    , 1288
    , 1289
    , 1291
    , 1292
    , 1293
    , 1295
    , 1296
    , 1297
    , 1299
    , 1300
    , 1302
    , 1303
    , 1304
    , 1306
    , 1307
    , 1308
    , 1310
    , 1311
    , 1313
    , 1314
    , 1315
    , 1316
    , 1318
    , 1319
    , 1321
    , 1322
    , 1323
    , 1325
    , 1326
    , 1327
    , 1329
    , 1330
    , 1331
    , 1333
    , 1334
    , 1335
    , 1336
    , 1338
    , 1339
    , 1340
    , 1342
    , 1343
    , 1344
    , 1346
    , 1347
    , 1348
    , 1350
    , 1351
    , 1352
    , 1353
    , 1355
    , 1356
    , 1357
    , 1359
    , 1360
    , 1361
    , 1363
    , 1364
    , 1365
    , 1366
    , 1368
    , 1369
    , 1370
    , 1371
    , 1373
    , 1374
    , 1375
    , 1377
    , 1378
    , 1379
    , 1380
    , 1382
    , 1383
    , 1384
    , 1385
    , 1387
    , 1388
    , 1389
    , 1390
    , 1392
    , 1393
    , 1394
    , 1395
    , 1397
    , 1398
    , 1399
    , 1400
    , 1402
    , 1403
    , 1404
    , 1405
    , 1407
    , 1408
    , 1409
    , 1410
    , 1411
    , 1413
    , 1414
    , 1415
    , 1416
    , 1418
    , 1419
    , 1420
    , 1421
    , 1422
    , 1424
    , 1425
    , 1426
    , 1427
    , 1428
    , 1430
    , 1431
    , 1432
    , 1433
    , 1434
    , 1436
    , 1437
    , 1438
    , 1439
    , 1441
    , 1442
    , 1443
    , 1444
    , 1445
    , 1446
    , 1448
    , 1449
    , 1450
    , 1451
    , 1452
    , 1454
    , 1455
    , 1456
    , 1457
    , 1458
    , 1459
    , 1461
    , 1462
    , 1463
    , 1464
    , 1465
    , 1466
    , 1468
    , 1469
    , 1470
    , 1471
    , 1472
    , 1473
    , 1474
    , 1476
    , 1477
    , 1478
    , 1479
    , 1480
    , 1481
    , 1483
    , 1484
    , 1485
    , 1486
    , 1487
    , 1488
    , 1489
    , 1491
    , 1492
    , 1493
    , 1494
    , 1495
    , 1496
    , 1497
    , 1498
    , 1500
    , 1501
    , 1502
    , 1503
    , 1504
    , 1505
    , 1506
    , 1507
  };
  int rawFlowSensePressure = analogRead(flowSensePin);
  int flowRate = flowArray[analogRead(flowSensePin)];      // read flow rate from lookup table. Calculated values in excel table

  // pressure sensor reading stuff
  float rawValue = rawFlowSensePressure;
  const float ADC_mV = 4.8828125;       // convesion multiplier from Arduino ADC value to voltage in mV
  const float SensorOffset = 174.0;     // 200 mV taken from datasheet, 174 fonud to be more accurate in testing for MPX5010DP
  const int ADCoffset = 34;           // measured from Arduino when rig is off
  const float sensitivity = 4.413;      // in mV/mmH2O taken from datasheet
  const float mmh2O_cmH2O = 10;         // divide by this figure to convert mmH2O to cmH2O
  const float mmh2O_kpa = 0.00981;      // convesion multiplier from mmH2O to kPa
  const float mmh2O_pa = 9.80665;       // convesion multiplier from mmH2O to Pa
  float pressureValue = ((rawValue * ADC_mV - SensorOffset) / sensitivity * mmh2O_pa);            // result in Pa

  Serial.print(millis());
  Serial.print(" , ");
  Serial.print(rawValue);
  Serial.print(" , ");
  Serial.print(pressureValue);
  Serial.print(" , ");
  Serial.println(flowRate);

  //int filteredFlowRate = getFilteredFlow(flowRate);         // low pass filter on flow readings. Not used
  return flowRate;    // return ml/s
}


int getBreathStatus ()  // inhale or exhale
{
  static int breathStatus = exhale;                           // inhale or exhale status
  static int lastBreathStatus = exhale;

  static int inhaleCounter = millis();
  static int exhaleCounter = millis();
  static long breathStatusTimer = millis();
  static byte breathStatusDebounce = 100;

  if (millis() - breathStatusTimer > breathStatusDebounce)      // debounce breath status
  {
    if (getFlowRate() > 20) breathStatus = inhale;              // sense inhale
    else breathStatus = exhale;
    breathStatusTimer = millis();
  }

  if (breathStatus != lastBreathStatus)                       // if breathStatus changed
  {
    if (breathStatus == inhale)
    {
      if (millis() - exhaleCounter > 0) exhalePeriod = millis() - exhaleCounter;                // store exhale time period
      inhaleCounter = millis();   // count breath time
    }
    else if (breathStatus == exhale)
    {
      if (millis() - inhaleCounter > 0) inhalePeriod = millis() - inhaleCounter;                // store inhale time period
      exhaleCounter = millis();
    }
  }
  lastBreathStatus = breathStatus;                            // reset lastBreathStatus
  return breathStatus;
}



int getVolumePerBreath ()
{
  int newFlowRate = getFlowRate();                            // in ml/s
  static byte integratePeriod = 10;                           // in miliseconds
  static unsigned long integratePeriodCounter = millis();     // to keep track of the current integration time
  static long breathVolume = 0;                               // volume accumulator
  static long breathVolumeOutput = 0;                         // final volume to 'return' from function

  // add to integrated flow rate accumulator every Xms
  if (getBreathStatus() == inhale)        // if during inhale
  {
    // add to integrated flow rate accumulator with volume recorded over the integratePeriod
    if (millis() - integratePeriodCounter >= integratePeriod)
    {
      breathVolume = breathVolume + (newFlowRate * (millis() - integratePeriodCounter)); // convert to volume in ml/ms over integratePeriod
      //Serial.println(millis() - integratePeriodCounter);      // code run time
      integratePeriodCounter = millis();
    }
  }
  else                                    // if exhale
  {
    if (breathVolume > 0)                 // if volume hasn't been recorded already for last breath
    {
      breathVolumeOutput = round(breathVolume * 0.001);   // set output volume
      breathVolume = 0;                   // reset accumulator
    }
    integratePeriodCounter = millis();    // reset
  }
  return breathVolumeOutput;
}


int getBPM ()     // breaths per minute DO THIS: it seems to display a 3 digit number, fix it
{
  float BPM = round(60 / (float)((inhalePeriod + exhalePeriod) * 0.001));   // output integer number for breath per minute
  static int returnBPM = 0;
  if (BPM > 1 && BPM < 100) returnBPM = BPM;
  return int(returnBPM);
}


// WHY IS THIS WRONG?
int getIEratio () // to calculate the 'E' of the Inhale : Exhale ratio
{
  static float IEratio = 0;
  if (inhalePeriod > 0) IEratio = ((float)exhalePeriod / (float)inhalePeriod) - 1;       // DO THIS: display to 1 decimal place & remove the rounding
  //Serial.println(IEratio);
  static float returnIEratio = 0;
  if (IEratio > (float)0.5 && IEratio < 4) returnIEratio = IEratio;                 // remove odd readings
  /*Serial.print("               ");
    Serial.print(inhalePeriod);
    Serial.print(" , ");
    Serial.print(exhalePeriod);
    Serial.print("   IE:");
    Serial.println(returnIEratio);*/
  returnIEratio = round(returnIEratio);
  return returnIEratio;
}


// read the buttons
int read_LCD_buttons()      // debounce these
{
  int adc_key_in = analogRead(buttonsLCDpin);             // read the value from the sensor
  // buttons are centered at these values: 0, 102, 258, 412, 641 (example code vals: 0, 144, 329, 504, 741)
  if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be  the most likely result
  // For V1.1 us this threshold
  if (adc_key_in < 50) return btnRIGHT;
  if (adc_key_in < 175) return btnUP;
  if (adc_key_in < 350) return btnDOWN;
  if (adc_key_in < 525) return btnLEFT;
  if (adc_key_in < 900) return btnSELECT;
  return btnNONE; // when all others fail, return this

  // IMPORTANT: Calibration routine needed to account for resistors out of tollerance
}


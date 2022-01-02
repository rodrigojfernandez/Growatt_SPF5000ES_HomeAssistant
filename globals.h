

char msg[50];
String mqttStat = "";
String message = "";
unsigned long lastTick, uptime, seconds, lastWifiCheck;
int setcounter = 0;
bool ledoff = false;
bool holdingregisters = false;
char newclientid[80];
char buildversion[12]="v1.0.1p2s";
int overflow;

struct modbus_input_registers 
{
    int status;
    float pv1voltage, pv1current, pv1power, outputpower, inputpower, gridfrequency, gridvoltage;
    
    float  totalworktime, opfullpower, opdcvolt;

    float batchargepower,batsoc, batchargecurrent, batdiscargepower, batpower;
    
    float tempinverter, tempdcdc ;

    float outputcurrent, invertercurrent;

    boolean ConstantPowerOKFlag, BatOverCharge;

    int  realoppercent,  faultcode, warningcode;

    int InvFanSpeed, MpptFanSpeed;
    
};

struct modbus_input_registers modbusdata;

struct modbus_holding_registers 
{

  int powerstate, OutputConfig, ChargeConfig, UtiOutStart, UtiOutEnd, UtiChargeStart, UtiChargeEnd, OutputVoltType, OutputFreqType, OverLoadRestart, OverTempRestart, BuzzerEN, MaxChargeCurr, batteryType;
  float BulkChargeVolt, FloatChargeVolt, BatLowToUtiVolt, FloatChargeCurr ;
  char firmware[6], controlfirmware[6];
    
};

struct modbus_holding_registers modbussettings;

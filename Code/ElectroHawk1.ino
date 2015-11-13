

//{ Constant definitions =======================================================

    //Uncommenting this dumps RGB data to the serial port but slows the
    //LED update cycle down a lot.
    //#define SERIAL_ON



    //DUE pin assignments:
/*
    //Red green and blue PWM pins.
    #define RedPWM 6
    #define GreenPWM 7
    #define BluePWM 8

    //Shift register control pins.
    #define ShiftDatIn 46
    #define ShiftReset 48
    #define ShiftClk 50
    #define ShiftDatOut 52

*/

    //Teensy 3.1 pin assignments:

    //Red green and blue PWM pins.
    #define RedPWM 20
    #define GreenPWM 22
    #define BluePWM 21

    //Shift register control pins.
    #define ShiftClk 10
    #define ShiftReset 12
    #define ShiftDatIn 13
    #define ShiftDatOut 5


    //Number of LEDs.
    #define NUM_LEDS 16

    //Colors.
    enum Colors_Enum
    {
        CALC_RED = 0,
        CALC_GREEN,
        CALC_BLUE
    };
    
    //Indeces into the color calculation weights values array.
    enum Weights_Enum
    {
        LEFTRED = 0,
        MIDRED,
        RIGHTRED,   
        LEFTGREEN,
        MIDGREEN,
        RIGHTGREEN,
        LEFTBLUE,
        MIDBLUE,
        RIGHTBLUE
    };
    
    //Weight values for color calculations.
    //--------------------------{    RED    |   GREEN   |   BLUE    }    
    //--------------------------{ L   M   R | L   M   R | L   M   R }        
    const byte WeightValsR[9] = {-2,  4, -2,  2, -5,  2,  2, -5,  2 };
    const byte WeightValsG[9] = { 1, -5,  1, -2,  5, -2,  2, -5,  2 };
    const byte WeightValsB[9] = { 1, -5,  1,  2, -5,  2, -2,  5, -2 };
    
//} ============================================================================

//{ Global variables ===========================================================
  
    //Arrays to hold RGB values.
    byte RedArray1[NUM_LEDS];
    byte GreenArray1[NUM_LEDS];
    byte BlueArray1[NUM_LEDS];

    byte RedArray2[NUM_LEDS];
    byte GreenArray2[NUM_LEDS];
    byte BlueArray2[NUM_LEDS];

    //Pointers to RGB arrays.
    byte* RedVals;
    byte* GreenVals;
    byte* BlueVals;
   
    byte* NewReds;
    byte* NewGreens;
    byte* NewBlues;
   
    //Main loop control variables.
    byte ColorCounter;
    byte LEDCounter;

//} ============================================================================

//{ Clock ======================================================================
//|
//| This function shifts data in the shift register.
//|
//| ----------------------------------------------------------------------------
void
Clock(){

    //Variable to hold state of Q7 in shift register.
    int Q7val;

    //Read value of Q7 and write to data input.
    Q7val = digitalRead(ShiftDatOut);
    digitalWrite(ShiftDatIn, Q7val);

    //Wait a bit for shift register setup.
    delayMicroseconds(10);

    //Toggle shift reg clock.
    digitalWrite(ShiftClk, HIGH);
    delayMicroseconds(10);
    digitalWrite(ShiftClk, LOW);
    
}
//} ============================================================================

//{ InitRGBVals ================================================================
//|
//| This function loads random values into the RGB array to generate an initial
//| state for the automaton.
//|
//| ----------------------------------------------------------------------------   
void 
InitRGBVals(){

    //A loop counter.
    int x;
    
    //Initialize RGB vals to random.
    for(x = 0; x < NUM_LEDS; x++)
    {
        RedVals[x] = (byte)random(0, 256);
        GreenVals[x] = (byte)random(0, 256);
        BlueVals[x] = (byte)random(0, 256);
    }
}
//} ============================================================================

//{ InitShiftReg ===============================================================
//|
//| This function resets the shift regster and clocks in one zero and 7 ones.
//| This will allow one LED to be active at a time.
//|
//| ----------------------------------------------------------------------------
void 
InitShiftReg(){

    //A loop counter.
    int x;

    //Reset shift reg.
    digitalWrite(ShiftReset, LOW);
    delay(10);
    digitalWrite(ShiftReset, HIGH);
    delay(10);


    digitalWrite(ShiftClk, LOW);
    delay(10);

    //Set shift data in pin low and clock.
    digitalWrite(ShiftDatIn, HIGH);
    delay(10);
    digitalWrite(ShiftClk, HIGH);
    delay(10);
    digitalWrite(ShiftClk, LOW);
    delay(10);

    //Set chift data in pin high.
    digitalWrite(ShiftDatIn, LOW);
    delay(10);

    //Clock next 7 bits.
    for(x = 0; x < 7; x++)
    {
        digitalWrite(ShiftClk, HIGH);
        delay(10);
        digitalWrite(ShiftClk, LOW);
        delay(10);
    }
}
//} ============================================================================

//{ CalcNewVals ================================================================
//|
//| This is the function that calculates a new red, green, or blue value for an
//| LED.
//|
//| Function parameters:
//|
//|     LEDNo
//|     Index of the LED the value is being calculated for.
//|
//|     Color
//|     Index of the color being calculated.
//|
//| Returns:
//|
//|     byte
//|     The new red, green, or blue value for the LED. 
//|
//| ----------------------------------------------------------------------------   
byte
CalcNewVals
(
    byte LEDNo,
    byte Color
){

    //Array to hold R, G, and B values of this LED and the LEDs to the left and
    //right.
    int Colors[9];
    
    //Pointer to an array of weight values.
    const byte* Weights;
    
    //Calculation variable.
    int Calc;
    
    //Loop counter.
    byte x;
    
    //The result of this function.
    byte RetVal;
    
    //Get left LED values.
    if(LEDNo <= 0)
    {
        //No '-1' LED so we use random.
        Colors[LEFTRED]     = random(0, 256);
        Colors[LEFTGREEN]   = random(0, 256);
        Colors[LEFTBLUE]    = random(0, 256);
    }
    else
    {
        //Otherwise, use values from LED to the left of the current one.
        Colors[LEFTRED]     = RedVals[(LEDNo - 1)];
        Colors[LEFTGREEN]   = GreenVals[(LEDNo - 1)];
        Colors[LEFTBLUE]    = BlueVals[(LEDNo - 1)];
    }

    //Get right LED values.
    if(LEDNo >= (NUM_LEDS - 1))
    {
        //No LED to the right so we use random.
        Colors[RIGHTRED]    = random(0, 256);
        Colors[RIGHTGREEN]  = random(0, 256);
        Colors[RIGHTBLUE]   = random(0, 256);
    }
    else
    {
        //Get previous values for LED to the right of this one.
        Colors[RIGHTRED]    = RedVals[(LEDNo + 1)];
        Colors[RIGHTGREEN]  = GreenVals[(LEDNo + 1)];
        Colors[RIGHTBLUE]   = BlueVals[(LEDNo + 1)];
    }
    
    //Get previous values for current LED.
    Colors[MIDRED]      = RedVals[LEDNo];
    Colors[MIDGREEN]    = GreenVals[LEDNo];
    Colors[MIDBLUE]     = BlueVals[LEDNo];

    //Set pointer to weights array.
    if(Color == CALC_RED)
        Weights = WeightValsR;
    else if(Color == CALC_GREEN)
        Weights = WeightValsG;
    else if(Color == CALC_BLUE)
        Weights = WeightValsB;
    
    //Initialize calc var.
    Calc = 0;
    
    //Multiply RGB values by weights and sum.
    for(x = 0; x < 9; x++)
    {
        Colors[x] = (Colors[x] * Weights[x]);
        Calc = (Calc + Colors[x]);
    }
 
    //Divide to get a byte value.
    Calc = Calc >> 8;
    
    //Check bounds.
    if(Calc <= 0)
        RetVal = 1;
    else
        RetVal = (byte)Calc;

    //Return the result.
    return(RetVal);

}
//} ============================================================================

//{ setup ======================================================================
//| 
//| Arduino setup function.
//|
//| ----------------------------------------------------------------------------
void
setup(){

    //Set PWM pins to outputs
    pinMode(RedPWM, OUTPUT); 
    pinMode(GreenPWM, OUTPUT); 
    pinMode(BluePWM, OUTPUT); 

    //Set shift reg controls to outputs.
    pinMode(ShiftClk, OUTPUT); 
    pinMode(ShiftReset, OUTPUT); 
    pinMode(ShiftDatIn, OUTPUT);
    
    //Set pin to read shift reg Q7 to input.
    pinMode(ShiftDatOut, INPUT); 
  
    //Initialize RGB array pointers.
    RedVals = RedArray1;
    GreenVals = GreenArray1;
    BlueVals = BlueArray1;
    NewReds = RedArray2;
    NewGreens = GreenArray2;
    NewBlues = BlueArray2;
  
    //Initialize data for cellular automaton.
    InitRGBVals();
    
    delay(100);
    
    //Initialize shift register.
    InitShiftReg();
    
    //If the serial port is being used, then initialize it.
    #ifdef SERIAL_ON
    Serial.begin(115200);
    Serial.print("\r\n");
    Serial.print("\r\n");
    Serial.print("\r\n");
    Serial.print("RGB Start\r\n");
    #endif
    
    //Initialize main loop control vars.
    ColorCounter = 0;
    LEDCounter = 0;
    
}
//} ============================================================================

//{ loop =======================================================================
//|
//| Arduino loop function.
//|
//| ----------------------------------------------------------------------------
void
loop(){

    //Loop counters.
    int x;
    int y;

    //Calculation variables.
    int IncVal;
    int Calc;
    
    //Calculate a new red, green, or blue value for the current LED.
    if(ColorCounter == CALC_RED)
        NewReds[LEDCounter] = CalcNewVals(LEDCounter, CALC_RED);
    else if(ColorCounter == CALC_GREEN)
        NewGreens[LEDCounter] = CalcNewVals(LEDCounter, CALC_GREEN);
    else
        NewBlues[LEDCounter] = CalcNewVals(LEDCounter, CALC_BLUE);
    
    //Print value to serial if that's what we're doing.
    #ifdef SERIAL_ON
    Serial.print(NewReds[LEDCounter]);
    Serial.print("\t");
    Serial.print(NewGreens[LEDCounter]);
    Serial.print("\t");
    Serial.print(NewBlues[LEDCounter]);
    Serial.print("\t");
    #endif

    #ifdef FADE_LEDS
    
    //Get difference between new value and current value.
    if(ColorCounter == CALC_RED)
        IncVal = (NewReds[LEDCounter] - RedVals[LEDCounter]);
    else if(ColorCounter == CALC_GREEN)
        IncVal = (NewGreens[LEDCounter] - GreenVals[LEDCounter]);
    else
        IncVal = (NewBlues[LEDCounter] - BlueVals[LEDCounter]);
    
    //Set calc to sign of difference.
    if(IncVal < 0)
        Calc = -1;
    else
        Calc = 1;
        
    //Outer loop to fade to new color.
    for(y = 0; y < abs(IncVal); y++)
    {
        //Increment or decrement color value.
        if(ColorCounter == CALC_RED)
            RedVals[LEDCounter] += Calc;
        else if(ColorCounter == CALC_GREEN)
            GreenVals[LEDCounter] += Calc;
        else
            BlueVals[LEDCounter] += Calc;
    
        //Cycle through LEDs.
        for(x = 0; x < NUM_LEDS; x++)
        {
            analogWrite(RedPWM, RedVals[x]);
            analogWrite(GreenPWM, GreenVals[x]);
            analogWrite(BluePWM, BlueVals[x]);
            
            delayMicroseconds(100);
        
            Clock();
        }
    }
    
    #else
        
        RedVals[LEDCounter]     = NewReds[LEDCounter];
        GreenVals[LEDCounter]   = NewGreens[LEDCounter];
        BlueVals[LEDCounter]    = NewBlues[LEDCounter];
    
        for(x = 0; x < NUM_LEDS; x++)
        {
            analogWrite(RedPWM, RedVals[x]);
            analogWrite(GreenPWM, GreenVals[x]);
            analogWrite(BluePWM, BlueVals[x]);
        
        
            delay(1);
        
            Clock();
        }
        
    #endif
    
    //Increment next color and LED index.
    ColorCounter++;
    ColorCounter = (ColorCounter % 3);
    LEDCounter++;
    LEDCounter = (LEDCounter % NUM_LEDS);
    
    //If we're back to LED 0 then print a newline to the serial port.
    #ifdef SERIAL_ON
    if(!LEDCounter)
        Serial.print("\r\n");
    #endif

}
//} ============================================================================


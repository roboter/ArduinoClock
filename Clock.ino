#include <TimeLib.h>

#define DS1302_SCLK_PIN   A2    // Arduino pin for the Serial Clock
#define DS1302_IO_PIN     A1    // Arduino pin for the Data I/O
#define DS1302_CE_PIN     A0    // Arduino pin for the Chip Enable

#define RTC_SECONDS_WRITE 0x80
#define RTC_SECONDS_READ 0x81

#define RTC_MINUTES_WRITE 0x82
#define RTC_MINUTES_READ 0x83

#define RTC_HOURS_WRITE 0x84
#define RTC_HOURS_READ 0x85


#define DS1302_ENABLE_READ            0x8F
#define DS1302_ENABLE_WRITE            0x8E
#define DS1302_TRICKLE_READ            0x91
#define DS1302_TRICKLE_WRITE           0x90
//#define DS1302_CLOCK_BURST       0xBE
#define DS1302_CLOCK_BURST_WRITE 0xBE
#define DS1302_CLOCK_BURST_READ  0xBF
#define RAM_READ 0xC1
#define RAM_WRITE 0xC0

#define DS1302_READBIT 0
#define DS1302_RC 6
//CH - Clock Halt 1:stop 0:start
#define CH_STOP B10000000
#define CH_RUN B00000000

// Macros to convert the bcd values of the registers to normal
// integer variables.
// The code uses separate variables for the high byte and the low byte
// of the bcd, so these macros handle both bytes separately.
#define bcd2bin(h,l)    (((h)*10) + (l))
#define bin2bcd_h(x)   ((x)/10)
#define bin2bcd_l(x)    ((x)%10)
// Structure for the first 8 registers.
// These 8 bytes can be read at once with
// the 'clock burst' command.
// Note that this structure contains an anonymous union.
// It might cause a problem on other compilers.
typedef struct ds1302_struct
{
  uint8_t Seconds: 4;     // low decimal digit 0-9
  uint8_t Seconds10: 3;   // high decimal digit 0-5
  uint8_t CH: 1;          // CH = Clock Halt
  uint8_t Minutes: 4;
  uint8_t Minutes10: 3;
  uint8_t reserved1: 1;
  union
  {
    struct
    {
      uint8_t Hour: 4;
      uint8_t Hour10: 2;
      uint8_t reserved2: 1;
      uint8_t hour_12_24: 1; // 0 for 24 hour format
    } h24;
    struct
    {
      uint8_t Hour: 4;
      uint8_t Hour10: 1;
      uint8_t AM_PM: 1;     // 0 for AM, 1 for PM
      uint8_t reserved2: 1;
      uint8_t hour_12_24: 1; // 1 for 12 hour format
    } h12;
  };
  uint8_t Date: 4;          // Day of month, 1 = first day
  uint8_t Date10: 2;
  uint8_t reserved3: 2;
  uint8_t Month: 4;         // Month, 1 = January
  uint8_t Month10: 1;
  uint8_t reserved4: 3;
  uint8_t Day: 3;           // Day of week, 1 = first day (any day)
  uint8_t reserved5: 5;
  uint8_t Year: 4;          // Year, 0 = year 2000
  uint8_t Year10: 4;
  uint8_t reserved6: 7;
  uint8_t WP: 1;            // WP = Write Protect
};
const int numeral[10] = {
  B01111110,// - 0
  B00001100,// - 1
  B10110110,// - 2
  B10011110,// - 3
  B11001100,// - 4
  B11011010,// - 5
  B11111010,// - 6
  B00001110,// - 7
  B11111110,// - 8
  B11011110,// - 9
};

int D1 = 2;
int E = 3;
int D = 4;
int DP = 5;
int A = 6;
int C = 7;
int F = 8;
int G = 9;
int D2 = 10;
int D4 = 11;
int D3 = 12;
int B = 13;

//pins for decimal point and each segment
//dp, G, F, E, D, C, B, A
const int segmentPins[] = { DP, A, B, C, D, E, F, G};

const int numberofDigits = 4;
int value = 9999;

const int digitPins[numberofDigits] = { D1, D2, D3, D4}; //digits 1, 2, 3, 4
// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);
  Serial.println(F("DS1302 Real Time Clock"));
  // initialize the digital pin as an output.
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);

  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(E, OUTPUT);
  pinMode(F, OUTPUT);
  pinMode(G, OUTPUT);

  pinMode(DP, OUTPUT);
  // setTime(8, 17, 00, 23, 10, 2017);

  pinMode(DS1302_SCLK_PIN, OUTPUT);
  pinMode(DS1302_IO_PIN, OUTPUT);
  pinMode(DS1302_CE_PIN, OUTPUT);
  DS1302_WRITE (DS1302_TRICKLE_WRITE, 0x00);
  DS1302_WRITE (RTC_SECONDS_WRITE, CH_RUN);

  // Remove the next define,
  // after the right date and time are set.
  //#define SET_DATE_TIME_JUST_ONCE
#ifdef SET_DATE_TIME_JUST_ONCE

  // Fill these variables with the date and time.
  int seconds, minutes, hours, dayofweek, dayofmonth, month, year;

  // Example for april 15, 2013, 10:08, monday is 2nd day of Week.
  // Set your own time and date in these variables.
  seconds    = 0;
  minutes    = 00;
  hours      = 20;
  dayofweek  = 1;  // Day of week, any day can be first, counts 1...7
  dayofmonth = 23; // Day of month, 1...31
  month      = 10;  // month 1...12
  year       = 2017;

  // Set a time and date
  // This also clears the CH (Clock Halt) bit,
  // to start the clock.
  ds1302_struct rtc;
  // Fill the structure with zeros to make
  // any unused bits zero
  memset ((char *) &rtc, 0, sizeof(rtc));

  rtc.Seconds    = bin2bcd_l( seconds);
  rtc.Seconds10  = bin2bcd_h( seconds);
  rtc.CH         = 0;      // 1 for Clock Halt, 0 to run;
  rtc.Minutes    = bin2bcd_l( minutes);
  rtc.Minutes10  = bin2bcd_h( minutes);
  // To use the 12 hour format,
  // use it like these four lines:
  //    rtc.h12.Hour   = bin2bcd_l( hours);
  //    rtc.h12.Hour10 = bin2bcd_h( hours);
  //    rtc.h12.AM_PM  = 0;     // AM = 0
  //   rtc.h12.hour_12_24 = 1; // 1 for 24 hour format
  rtc.h24.Hour   = bin2bcd_l( hours);
  rtc.h24.Hour10 = bin2bcd_h( hours);
  rtc.h24.hour_12_24 = 0; // 0 for 24 hour format
  rtc.Date       = bin2bcd_l( dayofmonth);
  rtc.Date10     = bin2bcd_h( dayofmonth);
  rtc.Month      = bin2bcd_l( month);
  rtc.Month10    = bin2bcd_h( month);
  rtc.Day        = dayofweek;
  rtc.Year       = bin2bcd_l( year - 2000);
  rtc.Year10     = bin2bcd_h( year - 2000);
  rtc.WP = 0;

  // Write all clock data at once (burst mode).
  DS1302_clock_burst_write( (uint8_t *) &rtc);
#endif
}

// the loop routine runs over and over again forever:
void loop() {
  ds1302_struct rtc;
  char buffer[80];     // the code uses 70 characters.

  // Read all clock data at once (burst mode).
  DS1302_clock_burst_read( (uint8_t *) &rtc);

  sprintf( buffer, "Time = %02d:%02d:%02d, ", \
           bcd2bin( rtc.h24.Hour10, rtc.h24.Hour), \
           bcd2bin( rtc.Minutes10, rtc.Minutes), \
           bcd2bin( rtc.Seconds10, rtc.Seconds));
  Serial.println(buffer);

  showDigit (rtc.h24.Hour10, 0);
  showDigit (rtc.h24.Hour, 1);

  showDigit (rtc.Minutes10, 2);
  showDigit (rtc.Minutes, 3);
}

void showNumber (int number)
{
  if (number == 0)
    showDigit (0, numberofDigits - 1); //display 0 in the rightmost digit
  else
  {
    for (int digit = numberofDigits - 1; digit >= 0; digit--)
    {
      if (number > 0)
      {
        showDigit(number % 10, digit);
        number = number / 10;
      }
    }
  }
}

//Displays given number on a 7-segment display at the given digit position
void showDigit (int number, int digit)
{
  digitalWrite(digitPins[digit], HIGH);
  for (int segment = 0; segment < 8; segment++)
  {
    boolean isBitSet = bitRead(numeral[number], segment);

    digitalWrite(segmentPins[segment], isBitSet);
  }
  delay(4);
  digitalWrite(digitPins[digit], LOW);
}


// --------------------------------------------------------
// DS1302_clock_burst_read
//
// This function reads 8 bytes clock data in burst mode
// from the DS1302.
//
// This function may be called as the first function,
// also the pinMode is set.
//
void DS1302_clock_burst_read( uint8_t *p)
{
  int i;

  _DS1302_start();

  // Instead of the address,
  // the CLOCK_BURST_READ command is issued
  // the I/O-line is released for the data
  _DS1302_togglewrite( DS1302_CLOCK_BURST_READ, true);

  for ( i = 0; i < 8; i++)
  {
    *p++ = _DS1302_toggleread();
  }
  _DS1302_stop();
}


// --------------------------------------------------------
// DS1302_clock_burst_write
//
// This function writes 8 bytes clock data in burst mode
// to the DS1302.
//
// This function may be called as the first function,
// also the pinMode is set.
//
void DS1302_clock_burst_write( uint8_t *p)
{
  int i;

  _DS1302_start();

  // Instead of the address,
  // the CLOCK_BURST_WRITE command is issued.
  // the I/O-line is not released
  _DS1302_togglewrite( DS1302_CLOCK_BURST_WRITE, false);

  for ( i = 0; i < 8; i++)
  {
    // the I/O-line is not released
    _DS1302_togglewrite( *p++, false);
  }
  _DS1302_stop();
}
uint8_t DS1302_READ(int address)
{
  uint8_t data;

  // set lowest bit (read bit) in address
  // bitSet( address, DS1302_READBIT);

  _DS1302_start();
  // the I/O-line is released for the data
  _DS1302_togglewrite( address, true);
  data = _DS1302_toggleread();
  _DS1302_stop();

  return (data);
}
// --------------------------------------------------------
// _DS1302_toggleread
//
// A helper function for reading a byte with bit toggle
//
// This function assumes that the SCLK is still high.
//
uint8_t _DS1302_toggleread( void)
{
  uint8_t i, data;

  data = 0;
  for ( i = 0; i <= 7; i++)
  {
    // Issue a clock pulse for the next databit.
    // If the 'togglewrite' function was used before
    // this function, the SCLK is already high.
    digitalWrite( DS1302_SCLK_PIN, HIGH);
    delayMicroseconds( 1);

    // Clock down, data is ready after some time.
    digitalWrite( DS1302_SCLK_PIN, LOW);
    delayMicroseconds( 1);        // tCL=1000ns, tCDD=800ns

    // read bit, and set it in place in 'data' variable
    bitWrite( data, i, digitalRead( DS1302_IO_PIN));
  }
  return ( data);
}
// --------------------------------------------------------
// DS1302_write
//
// This function writes a byte to the DS1302 (clock or ram).
// This function may be called as the first function,
// also the pinMode is set.
//
void DS1302_WRITE( int address, uint8_t data)
{
  _DS1302_start();
  // don't release the I/O-line
  _DS1302_togglewrite( address, false);
  // don't release the I/O-line
  _DS1302_togglewrite( data, false);
  _DS1302_stop();
}

// --------------------------------------------------------
// _DS1302_start
//
// A helper function to setup the start condition.
//
// An 'init' function is not used.
// But now the pinMode is set every time.
// That's not a big deal, and it's valid.
// At startup, the pins of the Arduino are high impedance.
// Since the DS1302 has pull-down resistors,
// the signals are low (inactive) until the DS1302 is used.
void _DS1302_start( void)
{
  digitalWrite( DS1302_CE_PIN, LOW); // default, not enabled
  pinMode( DS1302_CE_PIN, OUTPUT);

  digitalWrite( DS1302_SCLK_PIN, LOW); // default, clock low
  pinMode( DS1302_SCLK_PIN, OUTPUT);

  pinMode( DS1302_IO_PIN, OUTPUT);

  digitalWrite( DS1302_CE_PIN, HIGH); // start the session
  //delayMicroseconds( 4);           // tCC = 4us
}


// --------------------------------------------------------
// _DS1302_stop
//
// A helper function to finish the communication.
//
void _DS1302_stop(void)
{
  // Set CE low
  digitalWrite( DS1302_CE_PIN, LOW);

  // delayMicroseconds( 4);           // tCWH = 4us
}


// --------------------------------------------------------
// _DS1302_togglewrite
//
// A helper function for writing a byte with bit toggle
//
// The 'release' parameter is for a read after this write.
// It will release the I/O-line and will keep the SCLK high.
//
void _DS1302_togglewrite( uint8_t data, uint8_t release)
{
  int i;

  for ( i = 0; i <= 7; i++)
  {
    // set a bit of the data on the I/O-line
    digitalWrite( DS1302_IO_PIN, bitRead(data, i));
    delayMicroseconds( 1);     // tDC = 200ns

    // clock up, data is read by DS1302
    digitalWrite( DS1302_SCLK_PIN, HIGH);
    delayMicroseconds( 1);     // tCH = 1000ns, tCDH = 800ns

    if ( release && i == 7)
    {
      // If this write is followed by a read,
      // the I/O-line should be released after
      // the last bit, before the clock line is made low.
      // This is according the datasheet.
      // I have seen other programs that don't release
      // the I/O-line at this moment,
      // and that could cause a shortcut spike
      // on the I/O-line.
      pinMode( DS1302_IO_PIN, INPUT);

      // For Arduino 1.0.3, removing the pull-up is no longer needed.
      // Setting the pin as 'INPUT' will already remove the pull-up.
      // digitalWrite (DS1302_IO, LOW); // remove any pull-up
    }
    else
    {
      digitalWrite( DS1302_SCLK_PIN, LOW);
      delayMicroseconds( 1);       // tCL=1000ns, tCDD=800ns
    }
  }
}


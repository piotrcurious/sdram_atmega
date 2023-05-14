// Define pins
#define A0 22
#define A1 23
#define A2 24
#define A3 25
#define A4 26
#define A5 27
#define A6 28
#define A7 29
#define A8 30
#define A9 31
#define A10 32
#define A11 33
#define BA0 34
#define BA1 35
#define DQ0 36
#define DQ1 37
#define DQ2 38
#define DQ3 39
#define DQ4 40
#define DQ5 41
#define DQ6 42
#define DQ7 43
#define DQ8 44
#define DQ9 45
#define CS_ 46 // CS# is a reserved word in C++
#define RAS_ 47 // RAS# is a reserved word in C++
#define CAS_ 48 // CAS# is a reserved word in C++
#define WE_ 49 // WE# is a reserved word in C++
#define OE_ 50 // OE# is a reserved word in C++
#define CLK 9 // PWM pin

// Define commands
#define NOP   B00000000 // No operation
#define PRE   B00000001 // Precharge all banks
#define REF   B00000010 // Auto refresh
#define MRS   B00000011 // Mode register set

// Define mode register bits
// Refer to datasheet for details
// This example sets burst length = full page,
// burst type = sequential,
// CAS latency = 2,
// operating mode = standard,
// write burst mode = single location access

#define BL0   B00000000 
#define BL1   B00000000 
#define BT    B00000000 
#define CL0   B00100000 
#define CL1   B01000000 
#define CL2   B10000000 
#define TM0   B00000000 
#define TM1   B00000000 
#define WBL   B00000000 

// Define SDRAM size and parameters
// Refer to datasheet for details
// This example uses a single SDRAM module with x8 configuration,
// four internal banks, and 12-bit row and column addresses

#define BANKS     4    // Number of internal banks
#define ROWS      4096 // Number of rows per bank
#define COLUMNS   1024 // Number of columns

// Define some helper macros
#define SET(x) digitalWrite(x, HIGH) // Set pin x to high
#define CLR(x) digitalWrite(x, LOW)  // Set pin x to low
#define GET(x) digitalRead(x)        // Get the value of pin x

// Initialize the SDRAM module
void SDRAM_init() {
  // Set pins as inputs or outputs
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  pinMode(A6, OUTPUT);
  pinMode(A7, OUTPUT);
  pinMode(A8, OUTPUT);
  pinMode(A9, OUTPUT);
  pinMode(A10, OUTPUT);
  pinMode(A11, OUTPUT);
  pinMode(BA0, OUTPUT);
  pinMode(BA1, OUTPUT);
  pinMode(DQ0, INPUT); // Data pins are bidirectional
  pinMode(DQ1, INPUT); // Set them as inputs initially
  pinMode(DQ2, INPUT);
  pinMode(DQ3, INPUT);
  pinMode(DQ4, INPUT);
  pinMode(DQ5, INPUT);
  pinMode(DQ6, INPUT);
  pinMode(DQ7, INPUT);
  pinMode(DQ8, INPUT);
  pinMode(DQ9, INPUT);
  pinMode(CS_, OUTPUT);
  pinMode(RAS_, OUTPUT);
  pinMode(CAS_, OUTPUT);
  pinMode(WE_, OUTPUT);
  pinMode(OE_, OUTPUT);

  // Set initial pin states
  SET(CS_); // Chip select is active low
  SET(RAS_); // RAS# is active low
  SET(CAS_); // CAS# is active low
  SET(WE_); // WE# is active low
  SET(OE_); // OE# is active low

  // Configure PWM frequency for CLK pin
  // Refer to https://www.arduino.cc/en/Tutorial/SecretsOfArduinoPWM for details
  // This example sets the frequency to about 133 MHz (7.5 ns period)
  
  TCCR1B = TCCR1B & B11111000 | B00000001; // Set timer 1 prescaler to 1

  analogWrite(CLK, 128); // Set duty cycle to 50%

  // Send initialization commands to SDRAM module
  // Refer to datasheet for details and timing specifications
  // This example follows the recommended initialization sequence
  
  delayMicroseconds(200); // Wait for power and clock to stabilize

  SDRAM_command(PRE); // Precharge all banks
  delayMicroseconds(1); // Wait for minimum precharge time

  for (int i = 0; i < 8; i++) {
    SDRAM_command(REF); // Auto refresh
    delayMicroseconds(1); // Wait for minimum refresh time
  }

  SDRAM_command(MRS); // Mode register set
  delayMicroseconds(1); // Wait for mode register to set
}

// Send a command to the SDRAM module
void SDRAM_command(byte cmd) {
  switch (cmd) {
    case NOP: // No operation
      CLR(CS_); // Enable chip select
      SET(RAS_); // Disable RAS#
      SET(CAS_); // Disable CAS#
      SET(WE_); // Disable WE#
      break;
    case PRE: // Precharge all banks
      CLR(CS_); // Enable chip select
      CLR(RAS_); // Enable RAS#
      SET(CAS_); // Disable CAS#
      CLR(WE_); // Enable WE#
      break;
    case REF: // Auto refresh
      CLR(CS_); // Enable chip select
      CLR(RAS_); // Enable RAS#
      CLR(CAS_); // Enable CAS#
      SET(WE_); // Disable WE#
      break;
    case MRS: // Mode register set
      CLR(CS_); // Enable chip select
      CLR(RAS_); // Enable RAS#
      CLR(CAS_); // Enable CAS#
      CLR(WE_); // Enable WE#
      SDRAM_setAddress(BL0 | BL1 | BT | CL0 | CL1 | CL2 | TM0 | TM1 | WBL, BA0 | BA1); 
      break;
    default: 
      break;
  }
}

// Set the address pins for the SDRAM module
void SDRAM_setAddress(unsigned int col, byte bank) {
  digitalWrite(A0, bitRead(col, 0)); 
  digitalWrite(A1, bitRead(col, 1)); 
  digitalWrite(A2, bitRead(col, 2));
  digitalWrite(A3, bitRead(col, 3)); 
  digitalWrite(A4, bitRead(col, 4)); 
  digitalWrite(A5, bitRead(col, 5)); 
  digitalWrite(A6, bitRead(col, 6)); 
  digitalWrite(A7, bitRead(col, 7)); 
  digitalWrite(A8, bitRead(col, 8)); 
  digitalWrite(A9, bitRead(col, 9)); 
  digitalWrite(A10, bitRead(bank, 0)); 
  digitalWrite(A11, bitRead(bank, 1)); 
}

// Read a byte from the SDRAM module
byte SDRAM_read(unsigned int row, unsigned int col, byte bank) {
  byte data = 0; // Variable to store the data

  // Set the address pins
  SDRAM_setAddress(col, bank);

  // Send the read command
  CLR(CS_); // Enable chip select
  CLR(RAS_); // Enable RAS#
  CLR(CAS_); // Enable CAS#
  SET(WE_); // Disable WE#
  
  // Set the row address
  SDRAM_setAddress(row, bank);

  // Latch the row address
  SET(RAS_); // Disable RAS#

  // Set the column address
  SDRAM_setAddress(col, bank);

  // Latch the column address
  SET(CAS_); // Disable CAS#

  // Enable output
  CLR(OE_); // Enable OE#

  // Read the data pins

  bitWrite(data, 0, GET(DQ0));
  bitWrite(data, 1, GET(DQ1));
  bitWrite(data, 2, GET(DQ2));
  bitWrite(data, 3, GET(DQ3));
  bitWrite(data, 4, GET(DQ4));
  bitWrite(data, 5, GET(DQ5));
  bitWrite(data, 6, GET(DQ6));
  bitWrite(data, 7, GET(DQ7));
  bitWrite(data, 8, GET(DQ8));
  bitWrite(data, 9, GET(DQ9));

  // Disable output
  SET(OE_); // Disable OE#

  // Precharge the bank
  SDRAM_command(PRE);

  // Return the data
  return data;
}

// Write a byte to the SDRAM module
void SDRAM_write(unsigned int row, unsigned int col, byte bank, byte data) {
  
  // Set the address pins
  SDRAM_setAddress(col, bank);

  // Send the write command
  CLR(CS_); // Enable chip select
  CLR(RAS_); // Enable RAS#
  CLR(CAS_); // Enable CAS#
  CLR(WE_); // Enable WE#
  
  // Set the row address
  SDRAM_setAddress(row, bank);

  // Latch the row address

  SET(RAS_); // Disable RAS#

  // Set the column address
  SDRAM_setAddress(col, bank);

  // Latch the column address
  SET(CAS_); // Disable CAS#

  // Set the data pins as outputs
  pinMode(DQ0, OUTPUT);
  pinMode(DQ1, OUTPUT);
  pinMode(DQ2, OUTPUT);
  pinMode(DQ3, OUTPUT);
  pinMode(DQ4, OUTPUT);
  pinMode(DQ5, OUTPUT);
  pinMode(DQ6, OUTPUT);
  pinMode(DQ7, OUTPUT);
  pinMode(DQ8, OUTPUT);
  pinMode(DQ9, OUTPUT);

  // Write the data pins
  digitalWrite(DQ0, bitRead(data, 0));
  digitalWrite(DQ1, bitRead(data, 1));
  digitalWrite(DQ2, bitRead(data, 2));
  digitalWrite(DQ3, bitRead(data, 3));
  digitalWrite(DQ4, bitRead(data, 4));
  digitalWrite(DQ5, bitRead(data, 5));
  digitalWrite(DQ6, bitRead(data, 6));
  digitalWrite(DQ7, bitRead(data, 7));
  digitalWrite(DQ8, bitRead(data, 8));
  digitalWrite(DQ9, bitRead(data, 9));

  // Enable write
  CLR(WE_); // Enable WE#

  // Disable write
  SET(WE_); // Disable WE#

  // Precharge the bank
  SDRAM_command(PRE);

  // Set the data pins as inputs
  pinMode(DQ0, INPUT);
  pinMode(DQ1, INPUT);
  pinMode(DQ2, INPUT);
  pinMode(DQ3, INPUT);
  pinMode(DQ4, INPUT);
  pinMode(DQ5, INPUT);
  pinMode(DQ6, INPUT);
  pinMode(DQ7, INPUT);
  pinMode(DQ8, INPUT);
  pinMode(DQ9, INPUT);
}

// Implement an adjustable auto refresh function running on timer interrupt
// Refer to datasheet for details and timing specifications
// This example uses timer 2 to generate an interrupt every 15.6 ms
// and refreshes all banks in sequence

void SDRAM_autoRefresh() {
  
  // Set up timer 2
  TCCR2A = 0; // Normal mode
  TCCR2B = B00000111; // Prescaler = 1024
  TCNT2 = 0; // Counter value
  OCR2A = 243; // Compare match value
  TIMSK2 = B00000010; // Enable compare match interrupt

  // Enable global interrupts
  sei();
}

// Timer 2 compare match interrupt service routine
ISR(TIMER2_COMPA_vect) {
  
  // Refresh all banks in sequence
  for (byte i = 0; i < BANKS; i++) {
    SDRAM_command(REF); // Auto refresh
    delayMicroseconds(1); // Wait for minimum refresh time
  }
}

// Define some test patterns
#define PATTERN1 B10101010 // Alternating 1s and 0s
#define PATTERN2 B01010101 // Inverted alternating 1s and 0s
#define PATTERN3 B11110000 // Four 1s followed by four 0s
#define PATTERN4 B00001111 // Four 0s followed by four 1s

// Define a variable to store the test result
bool testResult = true; // Assume the test passes initially

// Loop function
void loop() {
  
  // Perform the RAM test for each pattern
  testResult = testResult && SDRAM_test(PATTERN1);
  testResult = testResult && SDRAM_test(PATTERN2);
  testResult = testResult && SDRAM_test(PATTERN3);
  testResult = testResult && SDRAM_test(PATTERN4);

  // Print the test result
  if (testResult) {
    Serial.println("RAM test passed!");
  } else {
    Serial.println("RAM test failed!");
  }

  // Wait for some time before repeating the test
  delay(1000);
}

// Function to perform the RAM test for a given pattern
bool SDRAM_test(byte pattern) {
  
  // Variable to store the read data
  byte data = 0;

  // Variable to store the test result

  bool result = true; // Assume the test passes initially

  // Write the pattern to the entire SDRAM
  for (unsigned int i = 0; i < ROWS; i++) {
    for (unsigned int j = 0; j < COLUMNS; j++) {
      for (byte k = 0; k < BANKS; k++) {
        SDRAM_write(i, j, k, pattern); // Write the pattern to the SDRAM
      }
    }
  }

  // Read back the pattern from the entire SDRAM and compare it
  for (unsigned int i = 0; i < ROWS; i++) {
    for (unsigned int j = 0; j < COLUMNS; j++) {
      for (byte k = 0; k < BANKS; k++) {
        data = SDRAM_read(i, j, k); // Read the data from the SDRAM
        if (data != pattern) { // Compare the data with the pattern
          result = false; // If they don't match, set the result to false
          break; // Break out of the loop
        }
      }
    }
  }

  // Return the test result
  return result;
}

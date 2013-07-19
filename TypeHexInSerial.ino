#include <SoftwareSerial.h>

#define RX  8  // connected to TX Pin
#define TX  9  // connected to RX Pin
#define BAUD 57600 // IMPORTANT NOTE: Factory default is 115200
#define BROADCAST 254

// Let's talk bloody serial!
#define BUFFER_SIZE 30
char pc_send_buf[BUFFER_SIZE];
char pc_recv_buf[BUFFER_SIZE];
byte dev_send_buf[BUFFER_SIZE];
byte dev_recv_buf[BUFFER_SIZE];
// Indices of buffers
int psc = 0; // PC send count
int prc = 0; // PC recv count
int dsc = 0; // Device send count
int drc = 0; // Device recv count

SoftwareSerial DeviceSerial(RX, TX);

byte HexAscii2Byte(char c) {
    switch (c) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case 'A': return 10;
        case 'B': return 11;
        case 'C': return 12;
        case 'D': return 13;
        case 'E': return 14;
        case 'F': return 15;
        case 'a': return 10;
        case 'b': return 11;
        case 'c': return 12;
        case 'd': return 13;
        case 'e': return 14;
        case 'f': return 15;
        default:  return 99;
    }
}

int HexAscii2ByteArray(const char* input_buffer, byte* output_buffer) {
    int i = 0; // input index
    int u = 0; // output index
    byte v = 0; // 4bit value of ASCII character
    byte minor = 0;
    byte major = 0; // Most significant 4 bits
    while (input_buffer[i] != 0) {
        // Get major component
        major = HexAscii2Byte(input_buffer[i]);
        // Skip over spaces, commas, etc.
        if (major > 15) {
            i++; 
            continue;
        }
        // Get minor component
        minor = HexAscii2Byte(input_buffer[i+1]);
        // If the minor character is valid, combine the numbers
        if (minor < 16) {
            v = (major << 4) + minor;
            i = i + 2;
        } else {
            v = major;
            i++;
        }
        // Put final value in output buffer
        output_buffer[u] = v;
        u++;
    }
    // Return the number of chars in the output buffer.
    return u;
}

char Byte2HexAscii(byte b) {
    switch (b) {
        case 0: return '0';
        case 1: return '1';
        case 2: return '2';
        case 3: return '3';
        case 4: return '4';
        case 5: return '5';
        case 6: return '6';
        case 7: return '7';
        case 8: return '8';
        case 9: return '9';
        case 10: return 'A';
        case 11: return 'B';
        case 12: return 'C';
        case 13: return 'D';
        case 14: return 'E';
        case 15: return 'F';
        default: return '?';
    }
}

int ByteArray2HexAscii(const byte* input_buffer, int length, char* output_buffer) {
    byte major = 0;
    byte minor = 0;
    int u = 0;
    for (int i = 0; i<length; i++) {
        // Add spaces between bytes.
        if (i>0) {
            output_buffer[u] = ' ';
            u++;
        }
        // Extract major and minor components of byte
        major = input_buffer[i] >> 4;
        minor = input_buffer[i] & 0x0F;
        // Convert to ASCII and display
        output_buffer[u] = Byte2HexAscii(major);
        u++;
        output_buffer[u] = Byte2HexAscii(minor);
        u++;
    }
    output_buffer[u] = 0; // Null terminate string
    u++;
    return u;
}

void setup()
{
  // Start communicating with PC
  Serial.begin(9600);
  Serial.println("Hello!");
  // Don't forget the buffer size is a limitation!
  Serial.print("Buffer size: ");
  Serial.println(BUFFER_SIZE);

  // Start communicating with device
  DeviceSerial.begin(BAUD);
  // Issue a command to the HerkuleX servos
  Serial.println("Command: STAT");
  dsc = HexAscii2ByteArray("FF FF 07 FE 07 FE 00", dev_send_buf);
  DeviceSerial.write(dev_send_buf,dsc);
}

void loop()
{
  // Read serial buffers from PC (Serial) and device (DeviceSerial)
  drc = 0;
  prc = 0;
  while (DeviceSerial.available()) {
    dev_recv_buf[drc] = DeviceSerial.read();
    drc++;
    delay(5);
  }
  while (Serial.available()) {
    pc_recv_buf[prc] = Serial.read();
    prc++;
    delay(5);
  }
  // Pass messages from the device to the PC
  if (drc > 0) {
    psc = ByteArray2HexAscii(dev_recv_buf,drc,pc_send_buf);
    Serial.println(pc_send_buf);
  }
  // Pass messages from the PC to the device
  if (prc > 0) {
    dsc = HexAscii2ByteArray(pc_recv_buf,dev_send_buf);
    DeviceSerial.write(dev_send_buf,dsc);
  }
}

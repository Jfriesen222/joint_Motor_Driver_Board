/**
 * Characters Strings. 
 *  
 * The character datatype, abbreviated as char, stores letters and 
 * symbols in the Unicode format, a coding system developed to support 
 * a variety of world languages. Characters are distinguished from other
 * symbols by putting them between single quotes ('P').<br />
 * <br />
 * A string is a sequence of characters. A string is noted by surrounding 
 * a group of letters with double quotes ("Processing"). 
 * Chars and strings are most often used with the keyboard methods, 
 * to display text to the screen, and to load images or files.<br />
 * <br />
 * The String datatype must be capitalized because it is a complex datatype.
 * A String is actually a class with its own methods, some of which are
 * featured below. 
 */

char letter;
String words = "Begin...";
String[] lines;
String  inputString;
import processing.serial.*;

Serial myPort;  // Create object from Serial class
int val;        // Data received from the serial port

void setup() {
  size(1920, 2160);
  // Create te font
  frameRate(20);
  textFont(createFont("SourceCodePro-Regular.ttf", 36));
  String portName = Serial.list()[0];
  myPort = new Serial(this,"COM6", 115200);
}

void draw() {
  background(0); // Set background to black

  // Draw the letter to the center of the screen
  textSize(14);
  lines = loadStrings("../restlengths.dat");
   myPort.write(lines[0] + "\r" + "\n");
  text(lines[0], 50, 50);
  text("Current key: " + letter, 50, 70);
  if(inputString != null){
        text(inputString, 50, 90);
   }
  
  textSize(36);
  text(words, 50, 120, 540, 300);
}

void keyTyped() {
  // The variable "key" always contains the value 
  // of the most recent key pressed.
  if ((key >= 'A' && key <= 'z') || key == ' ') {
    letter = key;
    words = words + key;
    // Write the letter to the console
    println(key);
  }
}

void serialEvent(Serial p) {
  if (p.available() >= 90) {
    inputString = p.readStringUntil('\n');
    if(inputString != null){
    print(inputString);
    }
  }
}
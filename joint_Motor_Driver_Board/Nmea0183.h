/* 
 * File:   Nmea0183.h
 * Author: Owner
 *
 * Created on September 3, 2014, 4:20 PM
 */


#ifndef NMEA0183_H
#define NMEA0183_H

void buildAndCheckSentence(unsigned char characterIn, char *sentence, unsigned char *sentenceIndex, unsigned char *sentenceState, unsigned char *checksum, void (*processResult)(const char *));

// a return value of 1 means the string is done. No more tokens
// This function is stateful, call it once with the String and then with NULL
// similar to strtok but this will support succesive tokens like
// "find,,,the,,commas"
unsigned char myTokenizer(const char *stringToTokenize, char token, char *returnToken);

// GPS checksum code based on
// http://www.codeproject.com/KB/mobile/WritingGPSApplications2.aspx
// original code in C# written by Jon Person, author of "GPS.NET" (www.gpsdotnet.com)
unsigned char getChecksum(char *sentence, unsigned char size);

#endif // NMEA0183_H



#ifndef EDITOR_H
#define EDITOR_H
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <ncurses.h>
//#include "FileContainer.h"


using namespace std;

class FileContainer
{
   public:
	 unsigned int curLinePos ; // This indicates the Line in the vector from which we have to  
                            // start redrawing the screen in case of vertical scrolling

	 unsigned int curColPos ;  // This indicates the Column in the vector from which we have to 
                            // start redrawing the screen in case of horizontal scrolling

	 int cursorRow ; // This is used to track the row position of the cursor on the screeen
	 unsigned int cursorCol ; // This is used to track the col position of the cursor on the screeen

	unsigned int dispLineCtr ; // This is used to display the line# in the status line at the bottom
	unsigned int dispColCtr ;  // This is used to display the col# in the status line at the bottom

	unsigned int filetotlines; // Total lines in the file

	std::string fileName;
	std::vector <std::string> fileContent ; // This vector holds the file contents. The element 
                                                // at the 0th position is the first ine in the file
                                               
	FileContainer() ;
};



void printMsg(string msgStr) ;
void chkArgs(int argc, char *argv[]);
int maininit() ;
void mainexit() ;
int showScreen() ;
void refreshwins() ;
void handlekeys();
void initWin() ;
void insert_char(char ch) ;
void delete_char() ;
void backspace() ;
void FileBeg();

int fileMenu() ;
void joinLine(int line2index) ;
void splitLine(int line2index) ;
void lineBegin() ;
void lineEnd() ;

void leftArrow() ;
void rightArrow() ;
void upArrow() ;
void downArrow() ;
void arrowCommon() ;

void getFileName(string& fldStr ) ;

int fileSave() ;

int fileOpenRead() ;
void varInit() ;
int dispError(string fldStr1, string fldStr2) ;
void savePrompt(string str1, string str2) ;


#endif

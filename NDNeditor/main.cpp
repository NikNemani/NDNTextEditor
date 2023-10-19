#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <ncurses.h>
#include "editorN.h"


using namespace std;


int main(int argc, char *argv[]) {

//    	vector<string> list;

	maininit();

	 chkArgs(argc, argv);

	showScreen();

        handlekeys();

	mainexit() ;

    return 0;
}



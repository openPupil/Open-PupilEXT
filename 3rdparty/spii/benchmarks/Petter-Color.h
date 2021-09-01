/*
*
*   Color
*
*   Allows you to use colors with cout in win32
*   Copyright (c) 2002,2006
*
*   Made by Petter Strandmark 
*
*   Compiles in both Windows and Unix without
*   any changes
*
*/

/*
*  USAGE:
*
*  cout << RED << "This is red." << BLUE << "\nThis is blue.";
*
*
*/

#ifndef COLOR_PETTER_H
#define COLOR_PETTER_H

//Retain ANSI/ISO Compability
#ifdef _WIN32 //Includes 64-bit
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
    #include <windows.h>
	#undef NOMINMAX
#endif

#include <iostream>
#include <iomanip>
#include <ctime>
#include <string>

namespace Petter
{
    
    namespace
    {
        class Color
        {
            friend std::ostream& operator<<(std::ostream& stream,const Color& c);

        public:
        #ifdef _WIN32
            Color(unsigned short c): color(c) {}      
            unsigned short color;    
        #else
            Color(const char* s): str(s) {}
            const char* str;
        #endif   
        
        };

        std::ostream& operator<<(std::ostream& stream,const Color& c)
        {
            stream.flush();
            #ifdef WIN32
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),c.color);
            #else
                stream << "\033[0m" << c.str;
            #endif
            stream.flush();

            return stream;
        }

        #ifdef _WIN32
            const Color NORMAL  = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE;
            const Color WHITE   = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY;
            const Color RED     = FOREGROUND_RED|FOREGROUND_INTENSITY;
            const Color DKRED     = FOREGROUND_RED;
            const Color BLUE    = FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_INTENSITY;
            const Color DKBLUE    = FOREGROUND_BLUE|FOREGROUND_GREEN;
            const Color GREEN   = FOREGROUND_GREEN|FOREGROUND_INTENSITY;
            const Color DKGREEN   = FOREGROUND_GREEN;
            const Color YELLOW  = FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY;
            const Color BROWN   = FOREGROUND_RED|FOREGROUND_GREEN;
        #else
            const Color NORMAL  = "";
            const Color WHITE   = "\033[37;1m";
            const Color RED     = "\033[31;1m";
            const Color DKRED   = "\033[31m";
            const Color BLUE    = "\033[34;1m";
            const Color DKBLUE  = "\033[34m";
            const Color GREEN   = "\033[32;1m";
            const Color DKGREEN = "\033[32m";
            const Color YELLOW  = "\033[33;1m";
            const Color BROWN   = "\033[33m";
        #endif
		
		std::clock_t start_time;
		bool status_is_active = false;

		void statusTry(const std::string& str)
		{
			std::cerr << std::left << std::setw(40) << str << " [ " << YELLOW << "WAIT" << NORMAL << " ] ";
			status_is_active = true;
			start_time = clock();
		}
		double statusOK()
		{
			double seconds = double(clock() - start_time) / double(CLOCKS_PER_SEC);
			if (status_is_active) {
				std::cerr << "\b\b\b\b\b\b\b\b" << GREEN << "  OK  " << NORMAL << "]   ";
				std::cerr << /*std::fixed << std::setprecision(1) <<*/ seconds << " s." << std::endl;
				status_is_active = false;
			}
			else {
				seconds = -1;
			}
			return seconds;
		}
		void statusFailed()
		{
			if (status_is_active) {
				std::cerr << "\b\b\b\b\b\b\b\b" << RED << "FAILED" << NORMAL << "]" << std::endl;
				status_is_active = false;
			}
		}
    }

}



#endif //ifndef 


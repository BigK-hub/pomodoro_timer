
#define OLC_PGE_APPLICATION
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Winmm.lib")
#include <sstream>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <vcruntime.h>
#include "olcPixelGameEngine.h"
#include <iostream>
#include <vector>
#include <chrono>
#include "Windows.h"
#include "WinUser.h"
#include <iomanip>
#include "conio.h"
#include <thread>
#include <cmath>

using steady_clock = std::chrono::steady_clock;
using chronoHours = std::chrono::hours;
using chronoMinutes = std::chrono::minutes;
using chronoSeconds = std::chrono::seconds;
using chronoMilliseconds = std::chrono::milliseconds;
using chronoMicroseconds = std::chrono::microseconds;
constexpr size_t SCREEN_WIDTH = 1000;
constexpr size_t SCREEN_HEIGHT = 640;

//TODO:
/*
- [x] get input from user
- [ ] implement break time
*/
class Button
{
public:
	size_t width{0};
	size_t height{0};
	olc::vi2d pos{0,0};
	bool isPressed{false};
	std::string text{""};
	olc::vi2d textDim{0,0};
	olc::Pixel buttonColor{olc::BLACK};
	const double textSize{2};
	bool isTimerOver = false;
	olc::PixelGameEngine* pge = nullptr;
public:
	Button(){}
	Button(olc::PixelGameEngine& pge,const olc::vi2d& pos,const size_t& width, const size_t& height, const bool& isPressed = false, const std::string& text = "")
	{
		this->width = width;
		this->height = height;
		this->pos = pos;
		this->isPressed = isPressed;
		this->text = text;
		this->textDim = {pge.GetTextSize(text)};
		this->pge = &pge;
	}
	void draw(const olc::Pixel& textColor = olc::WHITE)
	{
		pge->FillRect(pos.x, pos.y, width, height, buttonColor);
		const uint32_t pos_x = pos.x + width/2.0 - textDim.x ;
		const uint32_t pos_y = pos.y + height/2.0 - textDim.y; 
		pge->DrawString(pos_x, pos_y, text, textColor, textSize);
	}
	bool is_mouse_enter()
	{
		const olc::vi2d mouse_pos = {pge->GetMousePos()};
		const bool isOnX = mouse_pos.x < pos.x + width && mouse_pos.x > pos.x;
		const bool isOnY = mouse_pos.y < pos.y + height && mouse_pos.y > pos.y;

		return (isOnX && isOnY);
	}	
	void button_collision()
	{
		if(is_mouse_enter())
		{
			buttonColor = (olc::DARK_GREY);
			if(pge->GetMouse(olc::Mouse::LEFT).bPressed)
			{
				isPressed = true;
			}
		}
		else
		{
			buttonColor = (olc::BLACK);
		}
	}
};

class Time_Input
{
public:
	olc::vi2d pos = olc::vi2d(0,0);
	bool isActive = false;
	olc::PixelGameEngine* pge = nullptr;
	olc::Key activationKey = olc::A;

	int hours = 0;
	int minutes = 0;
	int seconds = 0;

	std::vector<int> time = {hours, minutes, seconds};
	int currentIndex = 2;

	Time_Input(){}
	Time_Input(olc::PixelGameEngine& pge, olc::Key activationkey,olc::vi2d pos, bool isActive = false, std::vector<int> defaultTimes = {0,0,0})
	{
		this->pos = pos;
		this->isActive = isActive;
		this->pge = &pge;
		this->activationKey = activationkey;
		this->time = defaultTimes;
	}
	void draw(const olc::Pixel& textColor = olc::WHITE)
	{
		std::stringstream ss;
		ss << std::setw(2) << std::setfill('0') << time[0] << ":";
		olc::vi2d hoursDim = pge->GetTextSize(ss.str());
		pge->DrawString(pos,ss.str(),(currentIndex == 0 && isActive)? olc::RED : textColor, 3);
		ss.str(std::string());

		ss << std::setw(2) <<std::setfill('0')<< time[1] << ":";
		olc::vi2d minutesDim = pge->GetTextSize(ss.str());
		pge->DrawString(pos.x + hoursDim.x*3,pos.y,ss.str(),(currentIndex == 1 && isActive)? olc::RED : textColor,3);
		ss.str(std::string());

		ss << std::setw(2) <<std::setfill('0')<< time[2];
		olc::vi2d secondsDim = pge->GetTextSize(ss.str());
		pge->DrawString(pos.x + hoursDim.x*3 + minutesDim.x*3,pos.y,ss.str(),(currentIndex == 2 && isActive)? olc::RED : textColor,3);
		ss.str(std::string());
	}
	bool keyInput()
	{
		if(pge->GetKey(activationKey).bPressed)
		{
			isActive = !isActive;
		}
		if (isActive)
		{
			if(pge->GetKey(olc::LEFT).bPressed)
			{
				currentIndex = std::max(currentIndex - 1, 0);
			}
			if(pge->GetKey(olc::RIGHT).bPressed)
			{
				currentIndex = std::min(currentIndex + 1 , 2 );
			}
			if(pge->GetKey(olc::UP).bPressed ||pge->GetKey(olc::UP).bHeld)
			{
				time[currentIndex] = std::min(time[currentIndex]+1,(currentIndex == 0)? 24: 59);
				std::this_thread::sleep_for(chronoMilliseconds(100));
			}
			if(pge->GetKey(olc::DOWN).bPressed||pge->GetKey(olc::DOWN).bHeld)
			{
				time[currentIndex] = std::max(time[currentIndex]-1, 0);
				std::this_thread::sleep_for(chronoMilliseconds(100));
			}
			
		}
		return isActive;
	}
	std::vector<int> getTime()
	{
		return time;
	}
};
class Example : public olc::PixelGameEngine
{
public:
	using timePoint = steady_clock::time_point;

	std::vector<Button> buttons{
		Button(*this, olc::vi2d(SCREEN_WIDTH * 0.1, SCREEN_HEIGHT * 0.1), 200, 60, false, "Start"),
		Button(*this, olc::vi2d(SCREEN_WIDTH * 0.1, SCREEN_HEIGHT * 0.2), 200, 60, false, "Pause"),
		Button(*this, olc::vi2d(SCREEN_WIDTH * 0.1, SCREEN_HEIGHT * 0.3), 200, 60, false, "Reset"),
		Button(*this, olc::vi2d(SCREEN_WIDTH * 0.1, SCREEN_HEIGHT * 0.4), 200, 60, false, "Productive"),
		Button(*this, olc::vi2d(SCREEN_WIDTH * 0.1, SCREEN_HEIGHT * 0.5), 200, 60, false, "Break")

		};

	std::vector<Time_Input> timeInputs{
		Time_Input(*this,olc::P, olc::vi2d(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.1),false,{0,25,0}),
		Time_Input(*this,olc::B, olc::vi2d(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.3),false,{0,10,0})
		};

	timePoint timerStart = steady_clock::now();
	chronoSeconds currentTimerState = chronoSeconds(0);

	chronoHours hoursPassed = chronoHours(0);
	chronoMinutes minutesPassed = chronoMinutes(0);
 	chronoSeconds secondsPassed = chronoSeconds(0);

	chronoSeconds timeLeft = chronoSeconds(0);
	chronoHours hoursLeft = chronoHours(0);
	chronoMinutes minutesLeft = chronoMinutes(0);
 	chronoSeconds secondsLeft = chronoSeconds(0);

	long long elapsedTime = 0;
	bool isTimerRunning = false;
    bool isTimerOver = false;
    bool isPaused = false;
	bool isBreakTime = false;

	chronoSeconds productiveTime = std::chrono::duration_cast<chronoSeconds>(chronoMinutes(25));
	chronoSeconds breakTime = std::chrono::duration_cast<chronoSeconds>(chronoMinutes(10));

	Example()
	{
		sAppName = "Pomodoro";
	}

public:
	
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
        FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,olc::Pixel(204, 255, 204));
		
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override
	{
        FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,olc::Pixel(204, 255, 204));
		
		buttons[0].draw();
		buttons[0].button_collision();

		buttons[1].draw();
		buttons[1].button_collision();
		
		buttons[2].draw();
		buttons[2].button_collision();

		buttons[3].draw();
		buttons[3].button_collision();

        buttons[4].draw();
		buttons[4].button_collision();

		timeInputs[0].draw(olc::BLACK);
		timeInputs[1].draw(olc::BLACK);

		if(timeInputs[1].keyInput())
		{
			timeInputs[0].isActive = false;
		}
		if(timeInputs[0].keyInput())
		{
			timeInputs[1].isActive = false;
		}
		
		if(buttons[0].isPressed)
		{
			if(!isTimerRunning && !isPaused)
			{
				std::vector<int> time = (!isBreakTime)? timeInputs[0].getTime() : timeInputs[1].getTime();
				((!isBreakTime)? productiveTime : breakTime) = std::chrono::duration_cast<chronoSeconds>(chronoHours(time[0]) + chronoMinutes(time[1]) + chronoSeconds(time[2]));
				timerStart = steady_clock::now();
			}
			else if (isPaused)
			{
				timerStart = steady_clock::now();
				isPaused = false;
			}
			isTimerRunning = true;
			buttons[0].isPressed = false;
			PlaySound("sounds/explosion.wav",NULL,SND_SYNC);
		}
		if(buttons[1].isPressed)
		{
			if(isTimerRunning && !isPaused)
			{
				isTimerRunning = false;
				isPaused = true;
				elapsedTime += std::chrono::duration_cast<chronoSeconds>(steady_clock::now() - timerStart).count();
				PlaySound("sounds/pause2.wav",NULL,SND_SYNC);
			}
			buttons[1].isPressed = false;
		}
		if(buttons[2].isPressed)
		{
			std::vector<int> time = (!isBreakTime)? timeInputs[0].getTime() : timeInputs[1].getTime();
			((!isBreakTime)? productiveTime : breakTime) = std::chrono::duration_cast<chronoSeconds>(chronoHours(time[0]) + chronoMinutes(time[1]) + chronoSeconds(time[2]));
			timerStart = steady_clock::now();
			currentTimerState = chronoSeconds(0);

			hoursPassed = chronoHours(0);
			minutesPassed = chronoMinutes(0);
			secondsPassed = chronoSeconds(0);

			timeLeft = chronoSeconds(0);
			hoursLeft = chronoHours(0);
			minutesLeft = chronoMinutes(0);
			secondsLeft = chronoSeconds(0);
			
			elapsedTime = 0;
			isTimerRunning = false;
			isTimerOver = false;
			isPaused = false;
			
			buttons[2].isPressed = false;
		}
		if(buttons[3].isPressed)
		{
			if(isBreakTime)
			{
				timerStart = steady_clock::now();
				currentTimerState = chronoSeconds(0);

				hoursPassed = chronoHours(0);
				minutesPassed = chronoMinutes(0);
				secondsPassed = chronoSeconds(0);

				timeLeft = chronoSeconds(0);
				hoursLeft = chronoHours(0);
				minutesLeft = chronoMinutes(0);
				secondsLeft = chronoSeconds(0);
			}
			isBreakTime = false;
			isTimerRunning = false;
			isTimerOver = false;
			isPaused = false;
			
			buttons[3].isPressed = false;

		}
		if(buttons[4].isPressed)
		{
			if(!isBreakTime)
			{
				timerStart = steady_clock::now();
				currentTimerState = chronoSeconds(0);

				hoursPassed = chronoHours(0);
				minutesPassed = chronoMinutes(0);
				secondsPassed = chronoSeconds(0);

				timeLeft = chronoSeconds(0);
				hoursLeft = chronoHours(0);
				minutesLeft = chronoMinutes(0);
				secondsLeft = chronoSeconds(0);
				
			}
			isBreakTime = true;
			isTimerRunning = false;
			isTimerOver = false;
			isPaused = false;
			buttons[4].isPressed = false;
		}
		if(isTimerRunning && isTimerOver == false && !isPaused)
		{
			currentTimerState = chronoSeconds(elapsedTime) + std::chrono::duration_cast<chronoSeconds>((steady_clock::now() - timerStart));

           	hoursPassed = std::chrono::duration_cast<chronoHours>(currentTimerState);
			minutesPassed = std::chrono::duration_cast<chronoMinutes>(currentTimerState) - std::chrono::duration_cast<chronoMinutes>(hoursPassed);
            secondsPassed = currentTimerState - std::chrono::duration_cast<chronoSeconds>(minutesPassed) - chronoSeconds(hoursLeft);
            isTimerOver = (currentTimerState.count() >= ((!isBreakTime) ? productiveTime.count() : breakTime.count()));
		}
		else if(isTimerOver == true)
		{
			isTimerRunning = false;
			isTimerOver = false;
			elapsedTime = 0;
    		PlaySound("sounds/finish.wav",NULL,SND_SYNC);
		}

		timeLeft = ((!isBreakTime)? productiveTime : breakTime) - currentTimerState;
		
		hoursLeft = std::chrono::duration_cast<chronoHours>(timeLeft);
		minutesLeft = std::chrono::duration_cast<chronoMinutes>(timeLeft) - chronoMinutes(hoursLeft);
		secondsLeft = timeLeft - chronoSeconds(minutesLeft) - chronoSeconds(hoursLeft);
		
		std::stringstream ss;
		ss << ((!isBreakTime)? "Productive time:\n": "Break time: \n")
		   << std::setw(2)<< std::setfill('0')<< hoursLeft.count() <<":" 
		   << std::setw(2)<< std::setfill('0')<< minutesLeft.count() <<":" 
		   << std::setw(2)<< std::setfill('0')<< secondsLeft.count();

		DrawStringDecal(olc::vf2d(SCREEN_HEIGHT *0.5 ,SCREEN_HEIGHT *0.8),ss.str(), olc::BLACK, {2.0,2.0});
		
		return true;
	}
};

int main()
{
    Example example;
    if(example.Construct(SCREEN_WIDTH, SCREEN_HEIGHT, 1, 1))
    {
        example.Start();
    }

}
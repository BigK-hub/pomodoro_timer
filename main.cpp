#include <sstream>
#include <stdint.h>
#include <string>
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Winmm.lib")
#define OLC_PGE_APPLICATION
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

using steady_clock = std::chrono::steady_clock;
using hours = std::chrono::hours;
using minutes = std::chrono::minutes;
using microseconds = std::chrono::microseconds;
using seconds = std::chrono::seconds;

constexpr size_t SCREEN_WIDTH = 1000;
constexpr size_t SCREEN_HEIGHT = 640;

//TODO:
/*
- [ ] get input from user
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
	int minutes = 25;
	int seconds = 0;

	std::vector<int> time = {hours, minutes, seconds};
	uint32_t currentIndex = 2;

	Time_Input(){}
	Time_Input(olc::PixelGameEngine& pge, olc::Key activationkey,olc::vi2d pos, bool isActive = false)
	{
		this->pos = pos;
		this->isActive = isActive;
		this->pge = &pge;
		this->activationKey = activationkey;
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
	void keyInput()
	{
		if(pge->GetKey(activationKey).bPressed)
		{
			isActive = true;
		}
		if (isActive)
		{
			if(pge->GetKey(olc::LEFT).bPressed)
			{
				currentIndex = std::max(currentIndex-1,uint32_t(0));
			}
			if(pge->GetKey(olc::RIGHT).bPressed)
			{
				currentIndex = std::min(currentIndex+1,uint32_t(2));
			}
			if(pge->GetKey(olc::UP).bPressed ||pge->GetKey(olc::UP).bHeld)
			{
				time[currentIndex] = std::min(time[currentIndex]+1,(currentIndex == 0)? 24: 59);
				Sleep(80);
			}
			if(pge->GetKey(olc::DOWN).bPressed||pge->GetKey(olc::DOWN).bHeld)
			{
				time[currentIndex] = std::max(time[currentIndex]-1, 0);
				Sleep(80);
			}
			
		}
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
		Button(*this, olc::vi2d(SCREEN_WIDTH * 0.1, SCREEN_HEIGHT * 0.3), 200, 60, false, "Reset")
		};
	Time_Input timeInput = Time_Input(*this,olc::P, olc::vi2d(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.1),false);
	timePoint timerStart = steady_clock::now();
	microseconds currentTimerState = microseconds(0);

	hours hoursPassed = hours(0);
	minutes minutesPassed = minutes(0);
 	seconds secondsPassed = seconds(0);

	microseconds timeLeft = microseconds(0);
	hours hoursLeft = hours(0);
	minutes minutesLeft = minutes(0);
 	seconds secondsLeft = seconds(0);

	long long elapsedTime = 0;
	bool isTimerRunning = false;
    bool isTimerOver = false;
    bool isPaused = false;
	
	microseconds productiveTime = std::chrono::duration_cast<microseconds>(minutes(25));

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

		timeInput.draw(olc::BLACK);
		timeInput.keyInput();

		timeLeft = productiveTime - currentTimerState;

		hoursLeft = std::chrono::duration_cast<hours>(timeLeft) ;
		minutesLeft = std::chrono::duration_cast<minutes>(timeLeft) - hoursLeft;
		secondsLeft = std::chrono::duration_cast<seconds>(timeLeft) - minutesLeft;
		
		std::stringstream ss;
		
		ss << std::setw(2)<< std::setfill('0')<<hoursLeft.count() <<":" 
		   << std::setw(2)<<std::setfill('0')<< minutesLeft.count() <<":" 
		   << std::setw(2)<<std::setfill('0')<< secondsLeft.count();

		DrawStringDecal(olc::vf2d(SCREEN_HEIGHT *0.5 ,SCREEN_HEIGHT *0.8),ss.str(), olc::BLACK, {2.0,2.0});
		
		if(isTimerRunning && isTimerOver == false && !isPaused)
		{
			currentTimerState = microseconds(elapsedTime) + std::chrono::duration_cast<microseconds>((steady_clock::now() - timerStart));
           	hoursPassed = std::chrono::duration_cast<hours>(currentTimerState); //- std::chrono::duration_cast<seconds>(minutesAmount);
			minutesPassed = std::chrono::duration_cast<minutes>(currentTimerState) - std::chrono::duration_cast<minutes>(hoursPassed);
            secondsPassed = std::chrono::duration_cast<seconds>(currentTimerState)- std::chrono::duration_cast<seconds>(minutesPassed);

            isTimerOver = (currentTimerState.count() >= productiveTime.count());
			
		}
		else if(isTimerOver == true)
		{
			std::cout <<"time elapsed";
			isTimerRunning = false;
			isTimerOver = false;
			elapsedTime = 0;
    		PlaySound("sounds/finish.wav",NULL,SND_SYNC);
		}


		if(buttons[0].isPressed)
		{
			//timer
			if(!isTimerRunning && !isPaused)
			{
				std::vector<int> time = timeInput.getTime();
				productiveTime = std::chrono::duration_cast<microseconds>(hours(time[0]) + minutes(time[1]) + seconds(time[2]));
				timerStart = steady_clock::now();
			}
			else if (isPaused)
			{
				timerStart = steady_clock::now();
				isPaused = false;
			}
			isTimerRunning = true;
			//PlaySound("sounds/start1.wav",NULL,SND_SYNC);
            //runTimer(std::chrono::duration_cast<microseconds>(PROD_TIME).count());
			buttons[0].isPressed = false;
			PlaySound("sounds/explosion.wav",NULL,SND_SYNC);
		}
		if(buttons[1].isPressed)
		{
			if(isTimerRunning && !isPaused)
			{
				isTimerRunning = false;
				isPaused = true;
				elapsedTime += std::chrono::duration_cast<microseconds>(steady_clock::now() - timerStart).count();
				PlaySound("sounds/pause2.wav",NULL,SND_SYNC);
			}
			buttons[1].isPressed = false;
		}
		if(buttons[2].isPressed)
		{
			timerStart = steady_clock::now();
			currentTimerState = microseconds(0);

			hoursPassed = hours(0);
			minutesPassed = minutes(0);
			secondsPassed = seconds(0);

			timeLeft = microseconds(0);
			hoursLeft = hours(0);
			minutesLeft = minutes(0);
			secondsLeft = seconds(0);
			
			elapsedTime = 0;
			isTimerRunning = false;
			isTimerOver = false;
			isPaused = false;
			
			buttons[2].isPressed = false;
		}
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
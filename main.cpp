#include <sstream>
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

using steady_clock = std::chrono::steady_clock;
using hours = std::chrono::hours;
using minutes = std::chrono::minutes;
using microseconds = std::chrono::microseconds;
using seconds = std::chrono::seconds;

constexpr size_t SCREEN_WIDTH = 1000;
constexpr size_t SCREEN_HEIGHT = 640;
constexpr auto PROD_TIME = std::chrono::duration_cast<microseconds>(seconds(60));


class Button
{
public:
	size_t mWidth{0};
	size_t mHeight{0};
	olc::vi2d mPos{0,0};
	bool mIsPressed{false};
	std::string mText{""};
	olc::vi2d mTextDim{0,0};
	olc::Pixel mButtonColor{olc::BLACK};
	const double mTextSize{2};
	bool isTimerOver = false;
public:
	Button(){}
	Button(olc::PixelGameEngine& pge,const olc::vi2d& pos,const size_t& width, const size_t& height, const bool& isPressed = false, const std::string& text = "")
	{
		mWidth = width;
		mHeight = height;
		mPos = pos;
		mIsPressed = isPressed;
		mText = text;
		mTextDim = {pge.GetTextSize(text)};
	}
	void draw(olc::PixelGameEngine& pge,const olc::Pixel& textColor = olc::WHITE)
	{
		pge.FillRect(mPos.x, mPos.y, mWidth, mHeight, mButtonColor);
		const uint32_t pos_x = mPos.x + mWidth/2.0 - mTextDim.x ;
		const uint32_t pos_y = mPos.y + mHeight/2.0 - mTextDim.y; 
		pge.DrawString(pos_x, pos_y, mText, textColor, mTextSize);
	}

	bool is_mouse_enter(olc::PixelGameEngine& pge)
	{
		const olc::vi2d mouse_pos = {pge.GetMousePos()};
		const bool isOnX = mouse_pos.x < mPos.x + mWidth && mouse_pos.x > mPos.x;
		const bool isOnY = mouse_pos.y < mPos.y + mHeight && mouse_pos.y > mPos.y;

		return (isOnX && isOnY);
	}	
	void button_collision(olc::PixelGameEngine& pge)
	{
		if(is_mouse_enter(pge))
		{
			mButtonColor = (olc::DARK_GREY);
			if(pge.GetMouse(olc::Mouse::LEFT).bPressed)
			{
				mIsPressed = true;
			}
		}
		else
		{
			mButtonColor = (olc::BLACK);
		}
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
		
		buttons[0].draw(*this);
		buttons[0].button_collision(*this);

		buttons[1].draw(*this);
		buttons[1].button_collision(*this);
		
		buttons[2].draw(*this);
		buttons[2].button_collision(*this);
		
		timeLeft = PROD_TIME - currentTimerState;

		hoursLeft = std::chrono::duration_cast<hours>(timeLeft) ;
		minutesLeft = std::chrono::duration_cast<minutes>(timeLeft) - hoursLeft;
		secondsLeft = std::chrono::duration_cast<seconds>(timeLeft) - minutesLeft;
		
		std::stringstream ss;
		ss << hoursLeft.count() <<" : " 
		   << minutesLeft.count() <<" : " 
		   << secondsLeft.count();

		DrawStringDecal(olc::vf2d(SCREEN_HEIGHT *0.5 ,SCREEN_HEIGHT *0.8),ss.str(), olc::BLACK, {2.0,2.0});
		
		if(isTimerRunning && isTimerOver == false && !isPaused)
		{
			currentTimerState = microseconds(elapsedTime) + std::chrono::duration_cast<microseconds>((steady_clock::now() - timerStart));
           	hoursPassed = std::chrono::duration_cast<hours>(currentTimerState); //- std::chrono::duration_cast<seconds>(minutesAmount);
			minutesPassed = std::chrono::duration_cast<minutes>(currentTimerState) - std::chrono::duration_cast<minutes>(hoursPassed);
            secondsPassed = std::chrono::duration_cast<seconds>(currentTimerState)- std::chrono::duration_cast<seconds>(minutesPassed);

            isTimerOver = (currentTimerState.count() >= PROD_TIME.count());
			
		}
		else if(isTimerOver == true)
		{
			std::cout <<"time elapsed";
			isTimerRunning = false;
			isTimerOver = false;
			elapsedTime = 0;
    		PlaySound("sounds/finish.wav",NULL,SND_SYNC);
		}
		else if(buttons[0].mIsPressed)
		{
			//timer
			if(isPaused)
			{
				timerStart = steady_clock::now();
				isPaused = false;
			}
			else if (!isTimerRunning && !isPaused)
			{
				timerStart = steady_clock::now();
			}
			isTimerRunning = true;
			//PlaySound("sounds/start1.wav",NULL,SND_SYNC);
            //runTimer(std::chrono::duration_cast<microseconds>(PROD_TIME).count());
			buttons[0].mIsPressed = false;
			PlaySound("sounds/explosion.wav",NULL,SND_SYNC);
		}
		if(buttons[1].mIsPressed)
		{
			if(isTimerRunning && !isPaused)
			{
				isTimerRunning = false;
				isPaused = true;
				elapsedTime += std::chrono::duration_cast<microseconds>(steady_clock::now() - timerStart).count();
				PlaySound("sounds/pause2.wav",NULL,SND_SYNC);
			}
			buttons[1].mIsPressed = false;
		}
		if(buttons[2].mIsPressed)
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
			
			buttons[2].mIsPressed = false;
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
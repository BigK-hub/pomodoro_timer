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
	size_t m_width{0};
	size_t m_height{0};
	olc::vi2d m_pos{0,0};
	bool m_is_pressed{false};
	std::string m_text{""};
	olc::vi2d m_text_dim{0,0};
	olc::Pixel m_button_color{olc::BLACK};
	const double m_text_size{2};
	bool isTimerOver = false;
public:
	Button(){}
	Button(olc::PixelGameEngine& pge,const olc::vi2d& pos,const size_t& width, const size_t& height, const bool& is_pressed = false, const std::string& text = "")
	{
		m_width = width;
		m_height = height;
		m_pos = pos;
		m_is_pressed = is_pressed;
		m_text = text;
		m_text_dim = {pge.GetTextSize(text)};
	}
	void draw(olc::PixelGameEngine& pge,const olc::Pixel& text_color = olc::WHITE)
	{
		pge.FillRect(m_pos.x, m_pos.y, m_width, m_height, m_button_color);
		const uint32_t pos_x = m_pos.x + m_width/2.0 - m_text_dim.x ;
		const uint32_t pos_y = m_pos.y + m_height/2.0 - m_text_dim.y; 
		pge.DrawString(pos_x, pos_y, m_text, text_color, m_text_size);
	}

	bool is_mouse_enter(olc::PixelGameEngine& pge)
	{
		const olc::vi2d mouse_pos = {pge.GetMousePos()};
		const bool isOnX = mouse_pos.x < m_pos.x + m_width && mouse_pos.x > m_pos.x;
		const bool isOnY = mouse_pos.y < m_pos.y + m_height && mouse_pos.y > m_pos.y;

		return (isOnX && isOnY);
	}	
	void button_collision(olc::PixelGameEngine& pge)
	{
		if(is_mouse_enter(pge))
		{
			m_button_color = (olc::DARK_GREY);
			if(pge.GetMouse(olc::Mouse::LEFT).bPressed)
			{
				m_is_pressed = true;
			}
		}
		else
		{
			m_button_color = (olc::BLACK);
		}
	}
};

class Example : public olc::PixelGameEngine
{
public:
	using tp = steady_clock::time_point;

	std::vector<Button> buttons{Button(*this,olc::vi2d(100,100),200, 60, false, "Start"), Button( *this,olc::vi2d(400,100),200,60,false,"Pause")};

	
	tp timerStart = steady_clock::now();
	microseconds offset = microseconds(0);
	tp pauseStart = steady_clock::now();
	tp pauseEnd = steady_clock::now() ;
	
	microseconds currentTimerState = microseconds(0);
	hours hoursAmount = hours(0);
	minutes minutesAmount = minutes(0);
 	seconds secondsAmount = seconds(0);

	long long elapsedTime = 0;
	bool isTimerRunning = false;
    bool isTimerOver = false;
    bool isPaused = false;
	COORD coord = COORD();
  	
	Example()
	{
		sAppName = "Pomodoro";
	}

public:
	
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
        FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,olc::Pixel(204, 255, 204));
		coord.X = 0;
  		coord.Y = 0;
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override
	{
        FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,olc::Pixel(204, 255, 204));
		
		buttons[0].draw(*this);
		buttons[0].button_collision(*this);

		buttons[1].draw(*this);
		buttons[1].button_collision(*this);
		
		const auto timeLeft = PROD_TIME - currentTimerState;

		const auto hoursProd = std::chrono::duration_cast<hours>(timeLeft) ;
		const auto minutesProd = std::chrono::duration_cast<minutes>(timeLeft) - hoursProd;
		const auto secondsProd = std::chrono::duration_cast<seconds>(timeLeft) - minutesProd;

		std::stringstream ss;
		ss << hoursProd.count() <<" : " 
		   << minutesProd.count() <<" : " 
		   << secondsProd.count();

		DrawStringDecal(olc::vf2d(SCREEN_HEIGHT *0.5 ,SCREEN_HEIGHT *0.8),ss.str(), olc::BLACK, {2.0,2.0});
		
		if(isTimerRunning && isTimerOver == false && !isPaused)
		{
			currentTimerState = microseconds(elapsedTime) + std::chrono::duration_cast<microseconds>((steady_clock::now() - timerStart));
           	hoursAmount = std::chrono::duration_cast<hours>(currentTimerState); //- std::chrono::duration_cast<seconds>(minutesAmount);
			minutesAmount = std::chrono::duration_cast<minutes>(currentTimerState) - std::chrono::duration_cast<minutes>(hoursAmount);
            secondsAmount = std::chrono::duration_cast<seconds>(currentTimerState)- std::chrono::duration_cast<seconds>(minutesAmount);

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
		else if(buttons[0].m_is_pressed)
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
			buttons[0].m_is_pressed = false;
			PlaySound("sounds/explosion.wav",NULL,SND_SYNC);
		}
		if(buttons[1].m_is_pressed)
		{
			if(isTimerRunning && !isPaused)
			{
				isTimerRunning = false;
				isPaused = true;
				elapsedTime += std::chrono::duration_cast<microseconds>(steady_clock::now() - timerStart).count();
				PlaySound("sounds/pause2.wav",NULL,SND_SYNC);
			}
			buttons[1].m_is_pressed = false;

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
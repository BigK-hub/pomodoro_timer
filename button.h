#ifndef BUTTON_H
#define BUTTON_H



class Button
	{
		size_t m_width{0};
		size_t m_height{0};
		olc::vi2d m_pos{0,0};
		bool m_is_pressed{false};
	public:
		Button(const size_t& width,const size_t& height, const olc::vi2d& pos, const bool& is_pressed = false )
		{
			
		}
	};
#endif
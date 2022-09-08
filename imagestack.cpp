/*
 * Copyright (C) 2018 Microchip Technology Inc.  All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <iostream>
#include <egt/ui>
#include <memory>
#include <string>
#include <vector>

#include <unistd.h>

#define delay_time		5
#define LED_DEVICE	"/sys/class/leds/red/brightness"

class MainWindow : public egt::TopWindow
{
public:
	int Start = 0;

	class Led
	{
		private: 
			int ledFd;

		public:
			Led()
			{
				ledFd = -1;
				ledFd = open(LED_DEVICE, O_WRONLY);
			}

			int on(void)
			{
				int ret;
				if( ledFd < 0 )
				{
					return -1;
				}

				ret = write(ledFd, "1", 1);
				return ret;	
			}

			int off(void)
			{
				int ret;
				if( ledFd < 0 )
				{
					return -1;
				}
		
				ret = write(ledFd, "0", 1);
				return ret;
			}
			~Led()
			{
				close(ledFd);
			}
	};

    class LauncherItem : public egt::Frame
    {
    public:
        explicit LauncherItem(const egt::Image& image) : m_background(image)
        {
            fill_flags().clear();
            no_layout(true);

            m_background.align(egt::AlignFlag::expand);
            m_background.image_align(egt::AlignFlag::expand);
            add(m_background);
        }

    private:
        egt::ImageLabel m_background;
    };

    MainWindow()
    {
        background(egt::Image("file:images/background.png"));

        m_animation.starting(0);
        m_animation.duration(std::chrono::seconds(2));
        m_animation.easing_func(egt::easing_cubic_easeout);
        m_animation.on_change(std::bind(&MainWindow::move_boxes, this, std::placeholders::_1));
        m_seq.add(m_delay);
        m_seq.add(m_animation);

        load();
    }

    void handle(egt::Event& event) override
    {
        egt::TopWindow::handle(event);

        switch (event.id())
        {
		case egt::EventId::pointer_click:
			if( Start == 0 )
			{
				mLed.on();
				Start = 1;
			} else
			{
				mLed.off();
				Start = 0;
			}
			break;
        case egt::EventId::pointer_drag_start:
            break;
        case egt::EventId::pointer_drag:
            break;
        case egt::EventId::pointer_drag_stop:
            break;
        default:
            break;
        }
    }

    void move_boxes(int diff = 0)
    {
        if (m_boxes.empty())
            return;

        auto x = m_start+diff;
        for (auto& box : m_boxes)
        {
            if (x + box->width() > width())
                x = width() - box->width();
			box->x(x);
            x += box->width();
        }

		m_start += diff;

		const auto& last = m_boxes.back();
		if( last->x() < 0 )
		{
			for (auto& box : m_boxes)
		    {
				box->x(0);
				m_start = 0;
		    }
		}
    }

private:

    void load()
    {
        for (auto x = 0; x < 10; x++)
        {
            auto image = egt::Image("file:images/image" + std::to_string(x) + ".png");
            auto box = std::make_shared<LauncherItem>(image);
            box->resize(egt::Size(width(), height() ));
            m_boxes.push_back(box);
        }

        for (auto& box : egt::detail::reverse_iterate(m_boxes))
        {
			box->y(0);
            box->x(0);
            add(box);
        }

        if (!m_boxes.empty())
        {
            m_start = m_boxes.front()->x();
            move_boxes();
        }
    }

    std::vector<std::shared_ptr<LauncherItem>> m_boxes;
    int m_start{0};
    egt::AnimationSequence m_seq;
    egt::AnimationDelay m_delay{std::chrono::seconds(2)};
    egt::PropertyAnimator m_animation;

	Led mLed;
};

int main(int argc, char** argv)
{
    egt::Application app(argc, argv);
#ifdef EXAMPLEDATA
    egt::add_search_path(EXAMPLEDATA);
#endif

	
	
    MainWindow window;
    window.show();

	egt::PeriodicTimer cputimer(std::chrono::seconds(delay_time));
	cputimer.on_timeout([&window]()
	{
		if( window.Start == 1 ) {
			window.move_boxes(-800);
			//std::cout<<"Timeout 1"<<std::endl;
		} else {
			//std::cout<<"Timeout 0"<<std::endl;
		}
	});
	cputimer.start();

    return app.run();
}

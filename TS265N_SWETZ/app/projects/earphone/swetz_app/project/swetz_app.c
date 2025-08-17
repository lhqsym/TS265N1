#include "../swetz_app_config.h"


//Power on initial
void user_power_on_init(void)
{

}


//10ms timer
void user_event_handler(void)
{
    bool event_timer_cnt_enable = true;
    user_event_e event_id = user_event_get(event_timer_cnt_enable);

    //APP_DBG("[swetz_jiu] 10ms event handler!\n");
    while(event_id)
    {
        switch(event_id)
        {
        }
        event_timer_cnt_enable = false;
        event_id = user_event_get(event_timer_cnt_enable);
    }
}


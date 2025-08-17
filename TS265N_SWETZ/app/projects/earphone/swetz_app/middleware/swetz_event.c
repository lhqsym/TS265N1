#include "../swetz_app_config.h"

user_event_stru user_event_tab[USER_EVENT_MAX];

//Clear all user event in queue
void clear_user_event_list(void)
{
    swetz_uint8 i;
    for(i = 0;i < USER_EVENT_MAX;i++)
    {
        user_event_tab[i].user_event = USER_EVENT_NONE;
        user_event_tab[i].user_timer_set_cnt = 0;
        user_event_tab[i].user_timer_current_cnt = 0;
    }
}

/******************************************
Description: Check user event is in queue and return queue number
Parameters[in]: 
    user_event_check: User event which need to check
******************************************/
swetz_uint8 get_user_event_num(user_event_e user_event_check)
{
    swetz_uint8 i;
    swetz_uint8 user_event_num;

    if(user_event_check == USER_EVENT_NONE)
    {
        user_event_num = USER_EVENT_ERROR_NUM;
    }
    else
    {
        for(i = 0;i < USER_EVENT_MAX;i++)
        {
            if(user_event_tab[i].user_event == USER_EVENT_NONE)
            {
                user_event_num = USER_EVENT_ERROR_NUM;
                break;
            }
            else if(user_event_tab[i].user_event == user_event_check)
            {
                user_event_num = i;
                break;
            }
        }
    }
    return user_event_num;
}

/******************************************
Description: Remove user event in queue
Parameters[in]: 
    user_event_remove: User event which need to remove
******************************************/
void remove_user_event_in_list(user_event_e user_event_remove)
{
    swetz_uint8 i;
    swetz_uint8 user_event_remove_num = get_user_event_num(user_event_remove);

    if(user_event_remove_num != USER_EVENT_ERROR_NUM)
    {
        for(i = user_event_remove_num;i < USER_EVENT_MAX - 1;i++)
        {
            if(user_event_tab[i].user_event == USER_EVENT_NONE)
            {
                break;
            }
            else
            {
                user_event_tab[i] = user_event_tab[i + 1];
            }
        }
    }
}

/******************************************
Description: Put user event into queue
Parameters[in]: 
    event: Event define by user
******************************************/
void user_event_set(user_event_stru event)
{
    swetz_uint8 i;
    for(i = 0;i < USER_EVENT_MAX;i++)
    {
        if(user_event_tab[i].user_event == USER_EVENT_NONE)
        {
            break;
        }
    }
    user_event_tab[i] = event;
}

/*******************************************
Description: Get event from queue
Parameters[in]: 
    timer_cnt_enable: Flag to decide is timer need to count,usually every 10ms enable once.
Return:
    user_event_e: Event which need to handler.
********************************************/
user_event_e user_event_get(bool timer_cnt_enable)
{
    swetz_uint8 i;
    user_event_e  user_event_r = USER_EVENT_NONE;

    /*Every 10ms enable once, to count time which event has set timer.*/
    if(timer_cnt_enable)    
    {
        for(i = 0;i < USER_EVENT_MAX;i++)
        {
            if(user_event_tab[i].user_event == USER_EVENT_NONE)
            {
                break;
            }
            else
            {
                if(user_event_tab[i].user_timer_set_cnt != 0)
                {
                    user_event_tab[i].user_timer_current_cnt++;
                }
            }
        }
    }
    if(user_event_tab[0].user_event != USER_EVENT_NONE)
    {
        i = 0;

        for(i = 0;i < USER_EVENT_MAX;i++)
        {
            if(user_event_tab[i].user_event == USER_EVENT_NONE)
            {
                break;
            }
            else
            {
                /*If event haven't set timer or timer timeout, return event to handler*/
                if(user_event_tab[i].user_timer_set_cnt == 0 || user_event_tab[i].user_timer_set_cnt == user_event_tab[i].user_timer_current_cnt)
                {
                    swetz_uint8 j;
                    user_event_r = user_event_tab[i].user_event;
                    /*Delete the event which is handler*/
                    for(j = i;j < USER_EVENT_MAX - 1;j++)
                    {
                        if(user_event_tab[j].user_event == USER_EVENT_NONE)
                        {
                            break;
                        }
                        else
                        {
                            user_event_tab[j] = user_event_tab[j + 1];
                        }
                    }
                    break;
                }
            }
        }
    }
    else
    {
        user_event_r = USER_EVENT_NONE;
    }
    return user_event_r;
}




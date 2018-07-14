/*
 *  This file is part of Permafrost Engine. 
 *  Copyright (C) 2018 Eduard Permyakov 
 *
 *  Permafrost Engine is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Permafrost Engine is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "timer_events.h"
#include "../event.h"

#include <math.h>
#include <assert.h>
#include <SDL.h>

#define TIMER_INTERVAL  (1000.0f/60.0f)

/*****************************************************************************/
/* STATIC VARIABLES                                                          */
/*****************************************************************************/

static unsigned long long s_num_60hz_ticks;
static SDL_TimerID        s_60hz_timer;

/*****************************************************************************/
/* STATIC FUNCTIONS                                                          */
/*****************************************************************************/

/* Timer callback gets called from another thread. In the callback, push 
 * a user event (threadsafe) with the code '0' and handle the 'EVENT_60HZ_TICK' 
 * event in the main thread.
 */
static uint32_t timer_callback(uint32_t interval, void *param)
{
    static double s_error = 0.0f;
    s_error += (TIMER_INTERVAL - interval);

    double intpart, fracpart;
    fracpart = modf(s_error, &intpart);

    s_error -= intpart;
    interval = ((int)TIMER_INTERVAL) + intpart;

    SDL_Event event = (SDL_Event) {
        .type = SDL_USEREVENT,
        .user = (SDL_UserEvent) {
            .type = SDL_USEREVENT,
            .code = 0,
            .data1 = NULL,
            .data2 = NULL,
        },
    };

    SDL_PushEvent(&event);
    return interval;
}

static void timer_60hz_handler(void *unused1, void *unused2)
{
    s_num_60hz_ticks++;

    if(s_num_60hz_ticks % 2 == 0)
        E_Global_Notify(EVENT_30HZ_TICK, NULL, ES_ENGINE);

    if(s_num_60hz_ticks % 6 == 0)
        E_Global_Notify(EVENT_10HZ_TICK, NULL, ES_ENGINE);

    if(s_num_60hz_ticks % 60 == 0)
        E_Global_Notify(EVENT_1HZ_TICK, NULL, ES_ENGINE);
}

/*****************************************************************************/
/* EXTERN FUNCTIONS                                                          */
/*****************************************************************************/

bool G_Timer_Init(void)
{
    s_60hz_timer = SDL_AddTimer(TIMER_INTERVAL, timer_callback, NULL);
    if(0 == s_60hz_timer)
        return false;

    E_Global_Register(EVENT_60HZ_TICK, timer_60hz_handler, NULL);
    return true;
}

void G_Timer_Shutdown(void)
{
    E_Global_Unregister(EVENT_60HZ_TICK, timer_60hz_handler);
    SDL_RemoveTimer(s_60hz_timer);
}

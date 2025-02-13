/***************************************************************************
*                                  _   _ ____  _
*  Project                     ___| | | |  _ \| |
*                             / __| | | | |_) | |
*                            | (__| |_| |  _ <| |___
*                             \___|\___/|_| \_\_____|
*
* Copyright (C) 1998 - 2017, Daniel Stenberg, <daniel@haxx.se>, et al.
*
* This software is licensed as described in the file COPYING, which
* you should have received as part of this distribution. The terms
* are also available at https://curl.haxx.se/docs/copyright.html.
*
* You may opt to use, copy, modify, merge, publish, distribute and/or sell
* copies of the Software, and permit persons to whom the Software is
* furnished to do so, under the terms of the COPYING file.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
*
***************************************************************************/

#include "curl_setup.h"
#pragma hdrstop

void Curl_speedinit(struct Curl_easy * data)
{
	memzero(&data->state.keeps_speed, sizeof(struct timeval));
}
/*
 * @unittest: 1606
 */
CURLcode Curl_speedcheck(struct Curl_easy * data, struct timeval now)
{
	if((data->progress.current_speed >= 0) && data->set.low_speed_time) {
		if(data->progress.current_speed < data->set.low_speed_limit) {
			if(!data->state.keeps_speed.tv_sec)
				data->state.keeps_speed = now; // under the limit at this very moment 
			else {
				// how long has it been under the limit 
				time_t howlong = Curl_tvdiff(now, data->state.keeps_speed);
				if(howlong >= data->set.low_speed_time * 1000) {
					// too long 
					failf(data, "Operation too slow. Less than %ld bytes/sec transferred the last %ld seconds",
					    data->set.low_speed_limit, data->set.low_speed_time);
					return CURLE_OPERATION_TIMEDOUT;
				}
			}
		}
		else
			data->state.keeps_speed.tv_sec = 0; // faster right now 
	}
	if(data->set.low_speed_limit) // if low speed limit is enabled, set the expire timer to make this connection's speed get checked again in a second 
		Curl_expire_latest(data, 1000);
	return CURLE_OK;
}


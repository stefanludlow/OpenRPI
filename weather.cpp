/*------------------------------------------------------------------------\
|  weather.c :  Weather Module                        www.middle-earth.us |
|  Copyright (C) 2004, Shadows of Isildur: Traithe                        |
|  Subsequently ravaged by Kithrater in 2010 for AtonementRPI             |
|  Derived under license from DIKU GAMMA (0.0).                           |
\------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "protos.h"
#include "utils.h"
#include "utility.h"

Weather weather_info[100];

int global_moon_light = 0;
int moon_light[100];
int sun_light = 0;
int desc_weather[100];
AFFECTED_TYPE *world_affects = NULL;

const int sunrise[] = { 7, 6, 6, 6, 6, 5, 4, 4, 5, 5, 6, 6 };
const int sunset[] = { 18, 18, 19, 19, 20, 21, 22, 22, 21, 20, 19, 18 };

const int seasonal_temp[7][12] = {
  {40, 47, 55, 63, 71, 79, 85, 79, 71, 63, 55, 47},	/* Temperate baseline */

  {22, 29, 37, 45, 53, 61, 67, 61, 53, 45, 37, 29},	/* Cool */
  {15, 22, 28, 38, 46, 50, 55, 50, 46, 38, 28, 22},	/* Cold */
  {7, 14, 18, 22, 25, 27, 30, 27, 25, 22, 18, 14},	/* Arctic */

  {55, 57, 60, 65, 73, 81, 89, 81, 73, 65, 60, 53},	/* Hot */
  {60, 62, 65, 70, 78, 86, 94, 86, 78, 70, 65, 58},	/* Hot */
  {75, 77, 80, 85, 93, 101, 109, 101, 93, 85, 80, 73}	/* Desert */
};



void
initialize_weather_zones (void)
{
  // Minas Tirith is chillier due to the mountain range
  zone_table[1].weather_type = WEATHER_COOL;
  zone_table[3].weather_type = WEATHER_COOL;

  // Osgiliath and the Ithilien are generally warm
  zone_table[2].weather_type = WEATHER_TEMPERATE;
  zone_table[4].weather_type = WEATHER_WARM;

  // Minas Morgul is warmer due to Mordor proximity
  zone_table[5].weather_type = WEATHER_HOT;
  zone_table[6].weather_type = WEATHER_HOT;
  
  // New Weather zones for Tur Edendor, at Zapata's request
  zone_table[10].weather_type = WEATHER_TEMPERATE;
  zone_table[66].weather_type = WEATHER_COOL;
  zone_table[67].weather_type = WEATHER_WARM;

  // Pelargir, 100 miles south of Gondor, is hot
  zone_table[13].weather_type = WEATHER_HOT;
}

bool Weather::weather_unification (int zone)
{
  bool zone_updated = false;

  // Begin Japheth's "Weather zone unification" changes
  if ( zone == 3 || zone == 11 || zone == 8)
    {
      weather_info[zone] = weather_info[1];
      zone_updated = true;
    }
  else if ( zone == 15)
    {
      weather_info[zone] = weather_info[2];
      zone_updated = true;
    }
  else if ( zone == 65 || zone == 66)
    {
      weather_info[zone] = weather_info[64];
      zone_updated = true;
    }
  else if ( zone >= 70 && zone <= 79)
    {
      weather_info[zone] = weather_info[10];
      zone_updated = true;
    }
  // End Japheth's changes

  return zone_updated;
}

void
weather (int moon_setting, int moon_rise, int moon_set)
{
  int last_temp = 0, last_clouds = 0, last_state = 0;
  int roll = 0, chance_of_rain = 0, last_fog = 0, i = 0;
  char buf[MAX_STRING_LENGTH];
  char storm[MAX_STRING_LENGTH];

  for (i = 0; i <= 99; i++)
    {
      if (Weather::weather_unification (i))
	continue;
      chance_of_rain = 0;

      last_temp = weather_info[i].temperature;
      last_clouds = weather_info[i].clouds;
      last_state = weather_info[i].state;
      last_fog = weather_info[i].fog;

      weather_info[i].temperature =
	seasonal_temp[zone_table[i].weather_type][time_info.month];

      if (time_info.hour == sunrise[time_info.month])
	weather_info[i].trend =
	  (weather_info[i].trend * 2 + number (0, 15)) / 3;

      weather_info[i].temperature += weather_info[i].trend;

      if ((weather_info[i].wind_speed < STORMY)
	  && (weather_info[i].wind_speed > CALM))
	{
	  if (weather_info[i].wind_dir == WEST_WIND)
	    {
	      if (!number (0, 47))
		{
		  send_outside_zone
		    ("The westerly winds wane and colder air descends out of the north.\n\r",
		     i);
		  weather_info[i].wind_dir = NORTH_WIND;
		}
	    }
	  else
	    {
	      if (!number (0, 23))
		{
		  send_outside_zone
		    ("The warmer prevailing winds rise up out of the west, driving off the chill northerlies.\n\r",
		     i);
		  weather_info[i].wind_dir = WEST_WIND;
		}
	    }
	}

      if (!number (0, 15) && weather_info[i].wind_speed)
	{
	  roll = number (0, 99);
	  roll += weather_info[i].temperature / 3;

	  if ((time_info.season == SPRING) || (time_info.season == AUTUMN))
	    {
	      if (roll < 20)
		{
		  if (last_clouds == OVERCAST)
		    weather_info[i].clouds = HEAVY_CLOUDS;
		  else
		    weather_info[i].clouds = CLEAR_SKY;
		}
	      else if (roll < 45)
		weather_info[i].clouds = LIGHT_CLOUDS;
	      else if (roll < 80)
		weather_info[i].clouds = HEAVY_CLOUDS;
	      else
		{
		  if (last_clouds == CLEAR_SKY)
		    weather_info[i].clouds = LIGHT_CLOUDS;
		  else
		    weather_info[i].clouds = OVERCAST;
		}
	    }
	  else if (time_info.season == SUMMER)
	    {
	      if (roll < 50)
		{
		  if (last_clouds == OVERCAST)
		    weather_info[i].clouds = HEAVY_CLOUDS;
		  else
		    weather_info[i].clouds = CLEAR_SKY;
		}
	      else if (roll < 80)
		weather_info[i].clouds = LIGHT_CLOUDS;
	      else if (roll < 90)
		weather_info[i].clouds = HEAVY_CLOUDS;
	      else
		{
		  if (last_clouds == CLEAR_SKY)
		    weather_info[i].clouds = LIGHT_CLOUDS;
		  else
		    weather_info[i].clouds = OVERCAST;
		}
	    }
	  else
	    {
	      if (roll < 10)
		{
		  if (last_clouds == OVERCAST)
		    weather_info[i].clouds = HEAVY_CLOUDS;
		  else
		    weather_info[i].clouds = CLEAR_SKY;
		}
	      else if (roll < 25)
		weather_info[i].clouds = LIGHT_CLOUDS;
	      else if (roll < 75)
		weather_info[i].clouds = HEAVY_CLOUDS;
	      else
		{
		  if (last_clouds == CLEAR_SKY)
		    weather_info[i].clouds = LIGHT_CLOUDS;
		  else
		    weather_info[i].clouds = OVERCAST;
		}
	    }
	}

      if (weather_info[i].fog < THICK_FOG)
	{
	  if ((weather_info[i].clouds == CLEAR_SKY)
	      && (weather_info[i].clouds != last_clouds))
	    {
	      if (weather_info[i].wind_dir == WEST_WIND)
		send_outside_zone
		  ("The clouds are born away upon the prevailing winds and clear skies open up above.\n",
		   i);
	      if (weather_info[i].wind_dir == NORTH_WIND)
		send_outside_zone
		  ("The northern winds carry away the cloud cover, leaving the sky clear.\n\r",
		   i);
	    }

	  if ((weather_info[i].clouds == LIGHT_CLOUDS)
	      && (weather_info[i].clouds != last_clouds))
	    {
	      if (weather_info[i].wind_dir == WEST_WIND)
		{
		  if (last_clouds > weather_info[i].clouds)
		    send_outside_zone
		      ("The cloud cover begins to clear, carried eastward upon the prevailing winds.\n\r",
		       i);
		  if (last_clouds < weather_info[i].clouds)
		    send_outside_zone
		      ("Wisplike clouds drift out of the west upon the prevailing winds.\n\r",
		       i);
		}

	      if (weather_info[i].wind_dir == NORTH_WIND)
		{
		  if (last_clouds > weather_info[i].clouds)
		    send_outside_zone
		      ("The cloud cover begins to clear, carried southward upon the chill northern winds.\n\r",
		       i);
		  if (last_clouds < weather_info[i].clouds)
		    send_outside_zone
		      ("Threadlike clouds begin to drift overhead upon the chill northern winds.\n\r",
		       i);
		}
	    }

	  if ((weather_info[i].clouds == HEAVY_CLOUDS)
	      && (weather_info[i].clouds != last_clouds))
	    {
	      if (weather_info[i].wind_dir == WEST_WIND)
		{
		  if (last_clouds < weather_info[i].clouds)
		    send_outside_zone
		      ("A host of clouds marches out of the west upon the prevailing winds.\n\r",
		       i);
		  if (last_clouds > weather_info[i].clouds)
		    send_outside_zone
		      ("Small patches of sky open up as the storm clouds drift eastward.\n\r",
		       i);
		}

	      if (weather_info[i].wind_dir == NORTH_WIND)
		{
		  if (last_clouds > weather_info[i].clouds)
		    send_outside_zone
		      ("Small patches of sky peek through the cloud cover as the storm clouds move southward.\n\r",
		       i);
		  if (last_clouds < weather_info[i].clouds)
		    send_outside_zone
		      ("The chill northerly winds bring heavy clouds in their wake.\n\r",
		       i);
		}
	    }

	  if ((weather_info[i].clouds == OVERCAST)
	      && (weather_info[i].clouds != last_clouds))
	    {
	      if (weather_info[i].wind_dir == WEST_WIND)
		{
		  if (sun_light == 1)
		    send_outside_zone
		      ("The prevailing winds bring a blanket of thick storm clouds to obscure Anor.\n\r",
		       i);
		  else
		    send_outside_zone
		      ("The prevailing winds bring a blanket of thick storm clouds into the sky.\n\r",
		       i);
		}

	      if (weather_info[i].wind_dir == NORTH_WIND)
		{
		  if (sun_light == 1)
		    send_outside_zone
		      ("A thick veil of storm clouds sweeps out of the north, plunging the land into twilight.\n\r",
		       i);
		  else
		    send_outside_zone
		      ("A thick veil of storm clouds sweeps out of the north.\n\r",
		       i);
		}
	    }
	}

      if (weather_info[i].clouds != last_clouds)
	{			/*   Is the new front a rain front?   */

	  if (time_info.season == SPRING)	/*   Spring rains   */
	    chance_of_rain = 20;

	  if (weather_info[i].clouds == CLEAR_SKY)	/*   More clouds = Higher chance of rain   */
	    chance_of_rain = 0;
	  else
	    chance_of_rain += (weather_info[i].clouds * 15);

	  if (number (0, 99) < chance_of_rain)
	    {
	      weather_info[i].state = CHANCE_RAIN;
	    }
	  else if ((last_state > CHANCE_RAIN) && (last_state < LIGHT_SNOW))
	    {
	      weather_info[i].state = NO_RAIN;
	      send_outside_zone ("The rain passes.\n\r", i);
	    }
	  else if (last_state > HEAVY_RAIN)
	    {
	      weather_info[i].state = NO_RAIN;
	      send_outside ("It stops snowing.\n\r");
	    }
	}

      /*   Lightning is more common the closer you get to midsummer. I wanted it to be more common   */
      /*   with higher temperatures, but we haven't determined temp and we need to know now.   */

      if ((weather_info[i].clouds > LIGHT_CLOUDS)
	  && (weather_info[i].state > NO_RAIN))
	{
	  if (number (35, 350) <
	      seasonal_temp[zone_table[i].weather_type][time_info.month])
	    {
	      if (number (1, 10) && number (1, 3) < weather_info[i].clouds)
		{
		  weather_info[i].lightning = 1;
		  send_outside_zone
		    ("Lightning flashes across the heavens.\n\r", i);
		}
	      else
		weather_info[i].lightning = 0;
	    }
	}

      if (!number (0, 4))
	{
	  roll = number (-1, 1);
	  switch (roll)
	    {
	    case -1:
	      if (weather_info[i].wind_speed == CALM)
		break;
	      if (weather_info[i].wind_speed == BREEZE && number (0, 1))
		break;
	      weather_info[i].wind_speed -= 1;
	      if (weather_info[i].wind_speed == CALM)
		send_outside_zone ("The winds die and the air stills.\n\r",
				   i);
	      if (weather_info[i].wind_speed == BREEZE)
		send_outside_zone ("The wind dies down to a mild breeze.\n\r",
				   i);
	      if (weather_info[i].wind_speed == WINDY)
		send_outside_zone
		  ("The gale winds die down to a steady current.\n\r", i);
	      if (weather_info[i].wind_speed == GALE)
		send_outside_zone
		  ("The stormy winds slow to a steady gale.\n\r", i);
	      break;

	    case 1:
	      sprintf (storm, "wind storm");
	      if (weather_info[i].state > CHANCE_RAIN)
		sprintf (storm, "rain storm");
	      if (weather_info[i].lightning)
		sprintf (storm, "thunder storm");
	      if (weather_info[i].state > HEAVY_RAIN)
		sprintf (storm, "blizzard");
	      if (weather_info[i].wind_speed == STORMY)
		{
		  send_outside_zone
		    ("The storm winds slow, leaving a steady gale in their wake.\n\r",
		     i);
		  weather_info[i].wind_speed -= 1;
		  break;
		}
	      if (weather_info[i].wind_speed == CALM)
		send_outside_zone ("A capricious breeze picks up.\n\r", i);
	      if (weather_info[i].wind_speed == BREEZE)
		{
		  if (number (0, 1))
		    break;
		  send_outside_zone
		    ("The breeze strengthens into a steady wind.\n\r", i);
		}
	      if (weather_info[i].wind_speed == WINDY)
		{
		  if (!number (0, 3))
		    break;
		  if (weather_info[i].state < LIGHT_RAIN)
		    send_outside_zone
		      ("The winds grow fierce, building into a strong gale.\n\r",
		       i);
		  else
		    {
		      if (weather_info[i].state > HEAVY_RAIN)
			sprintf (storm, "snow storm");
		      sprintf (buf,
			       "The winds grow fierce, building into a mild %s.\n\r",
			       storm);
		      send_outside_zone (buf, i);
		    }
		}
	      if (weather_info[i].wind_speed == GALE)
		{
		  if (!number (0, 5))
		    break;
		  sprintf (buf,
			   "The winds begin to rage, and a fierce %s is born.\n\r",
			   storm);
		  send_outside_zone (buf, i);
		}
	      weather_info[i].wind_speed += 1;
	      break;
	    }
	}

      /*   Wind Chill - This is FAR from scientific, but I didnt want winds to totally take over temperatures. - Koldryn  */
      if (weather_info[i].wind_dir == NORTH_WIND)
	{
	  weather_info[i].temperature -= weather_info[i].wind_speed * 2;
	  roll = 0 - weather_info[i].wind_speed * 2;
	}

      if (weather_info[i].wind_dir == WEST_WIND)
	{
	  weather_info[i].temperature += (5 - weather_info[i].wind_speed * 2);
	  roll = 0 + (5 - weather_info[i].wind_speed * 2);
	}

      /*   Angle of Sunlight   */
      if (sun_light)
	{
	  roll = ((sunrise[time_info.month] + sunset[time_info.month]) / 2);

	  if (time_info.hour > roll)
	    roll =
	      (sunset[time_info.month] -
	       time_info.hour) * 100 / (sunset[time_info.month] -
					roll) * 15 / 100;
	  else if (time_info.hour == roll)
	    roll = 15;
	  else if (time_info.hour < roll)
	    roll =
	      (time_info.hour - sunrise[time_info.month]) * 100 / (roll -
								   sunrise
								   [time_info.
								    month]) *
	      15 / 100;

	  weather_info[i].temperature += roll;
	}

      /*   Cloud Chill, which applies only in the daytime - This is not scientific.   */
      if (sun_light)
	{
	  if (weather_info[i].clouds == LIGHT_CLOUDS)
	    weather_info[i].temperature -= ((roll * 3) / 10);
	  if (weather_info[i].clouds == HEAVY_CLOUDS)
	    weather_info[i].temperature -= ((roll * 6) / 10);
	  if (weather_info[i].clouds == OVERCAST)
	    weather_info[i].temperature -= ((roll * 9) / 10);
	  weather_info[i].temperature = ((weather_info[i].temperature + last_temp * 2) / 3);	/*   Limits Drastic Immediate Changes   */
	}
      else
	{
	  if ((time_info.season == SPRING) || (time_info.season == AUTUMN))	/*   Gradual Nighttime Cooling   */
	    weather_info[i].temperature -= 10;
	  else if (time_info.season == SUMMER)
	    weather_info[i].temperature -= 15;
	  else
	    weather_info[i].temperature -= 5;
	  roll = 0;
	  if (time_info.hour != sunset[time_info.month])
	    roll = 5;
	  weather_info[i].temperature =
	    ((weather_info[i].temperature + (last_temp + roll) * 4) / 5);
	  weather_info[i].temperature -= 5;	/*   Immediate Nighttime Chill   */
	}

      /*   Will it rain?   */
      if (weather_info[i].state == CHANCE_RAIN)
	{
	  chance_of_rain = (weather_info[i].clouds * 15);
	  if (time_info.season == SUMMER)
	    chance_of_rain -= 20;
	  if (time_info.season == AUTUMN)
	    chance_of_rain -= 10;
	  if (time_info.season == SPRING)
	    chance_of_rain += 10;
	  chance_of_rain = MAX (1, chance_of_rain);
	  if (weather_info[i].lightning && number (0, 39))	/*   Its very rare for lightning not to cause rain   */
	    chance_of_rain = 100;

	  if (number (0, 99) < chance_of_rain)
	    weather_info[i].state = weather_info[i].clouds + 1;
	}

      /*   If its going to rain, how hard?   */
      if (weather_info[i].state > CHANCE_RAIN)
	{
	  if (weather_info[i].state > HEAVY_RAIN)
	    weather_info[i].state -= 3;
	  roll = number (0, 99);
	  if (weather_info[i].clouds == LIGHT_CLOUDS)
	    {
	      if (roll < 40)
		weather_info[i].state = CHANCE_RAIN;
	      else if (roll < 85)
		weather_info[i].state = LIGHT_RAIN;
	      else
		weather_info[i].state = STEADY_RAIN;
	    }
	  if (weather_info[i].clouds == HEAVY_CLOUDS)
	    {
	      if (roll < 30)
		weather_info[i].state = CHANCE_RAIN;
	      else if (roll < 60)
		weather_info[i].state = LIGHT_RAIN;
	      else if (roll < 90)
		weather_info[i].state = STEADY_RAIN;
	      else
		weather_info[i].state = HEAVY_RAIN;
	    }
	  if (weather_info[i].clouds == OVERCAST)
	    {
	      if (roll < 20)
		weather_info[i].state = CHANCE_RAIN;
	      else if (roll < 50)
		weather_info[i].state = LIGHT_RAIN;
	      else if (roll < 80)
		weather_info[i].state = STEADY_RAIN;
	      else
		weather_info[i].state = HEAVY_RAIN;
	    }

	  /*   Is it rain or snow?   */
	  if ((weather_info[i].temperature < 32) 
	      && (weather_info[i].state > CHANCE_RAIN))	
	    {
	      if ((weather_info[i].state += 3) == HEAVY_SNOW
		  && (weather_info[i].temperature < 20))
		weather_info[i].state--;
	    }

	  /*   Lightning should never allow existing rain to stop   */
	  if (weather_info[i].lightning 
	      && (weather_info[i].state == CHANCE_RAIN))
	    weather_info[i].state = weather_info[i].clouds + 1;
	}

      /*   If the rain has changed, display a message.   */
      if ((weather_info[i].state != last_state)
	  && (weather_info[i].state != NO_RAIN))
	{
	  if ((weather_info[i].state == CHANCE_RAIN)
	      && (last_state > CHANCE_RAIN) && (last_state < LIGHT_SNOW))
	    send_outside_zone ("The rain fades and stops.\n\r", i);
	  if ((weather_info[i].state == CHANCE_RAIN)
	      && (last_state > HEAVY_RAIN))
	    send_outside_zone ("It stops snowing.\n\r", i);
	  if (weather_info[i].state == LIGHT_RAIN)
	    send_outside_zone
	      ("A light sprinkling of rain falls from the sky.\n\r", i);
	  if (weather_info[i].state == STEADY_RAIN)
	    send_outside_zone ("A steady rain falls from the sky.\n\r", i);
	  if (weather_info[i].state == HEAVY_RAIN)
	    send_outside_zone ("Pouring rain showers down upon the land.\n\r",
			       i);
	  if (weather_info[i].state == LIGHT_SNOW)
	    send_outside_zone ("A light snow falls lazily from the sky.\n\r",
			       i);
	  if (weather_info[i].state == STEADY_SNOW)
	    send_outside_zone ("Snow falls steadily from the sky.\n\r", i);
	  if (weather_info[i].state == HEAVY_SNOW)
	    send_outside_zone ("Blinding snows fall from the sky.\n\r", i);
	}

      if (weather_info[i].fog)
	{
	  if (weather_info[i].wind_speed == WINDY)
	    weather_info[i].fog -= 1;
	  if (weather_info[i].wind_speed > WINDY)
	    {
	      weather_info[i].fog = NO_FOG;
	      send_outside_zone
		("The fog churns in the wind and is swept away.\n\r", i);
	    }
	  if (time_info.hour > sunrise[time_info.month]
	      && weather_info[i].clouds == CLEAR_SKY)
	    weather_info[i].fog -= 1;
	  if (time_info.hour == 9)
	    weather_info[i].fog -= 1;
	  if (time_info.hour == 12)
	    weather_info[i].fog -= 1;
	  if (time_info.hour > sunrise[time_info.month]
	      && weather_info[i].clouds > CLEAR_SKY)
	    {
	      roll = number (1, 4);
	      if (roll > weather_info[i].clouds)
		weather_info[i].fog -= 1;
	    }
	  if (weather_info[i].fog < NO_FOG)
	    weather_info[i].fog = NO_FOG;
	  if (weather_info[i].fog == THIN_FOG && last_fog == THICK_FOG)
	    send_outside_zone ("The fog thins a little.\n\r", i);
	  if (weather_info[i].fog == NO_FOG)
	    send_outside_zone ("The fog lifts.\n\r", i);
	}

      /*  If its after midnight, before dawn, within 3 hours of dawn, there is no fog, and there is no artificial sunlight....   */
      if ((sunrise[time_info.month] < (time_info.hour + 4))
	  && (sun_light == 0) && (time_info.hour < sunrise[time_info.month])
	  && (weather_info[i].fog == NO_FOG))
	{
	  switch (time_info.season)
	    {
	    case WINTER:
	      chance_of_rain = 25;
	      break;		/*   Chance of Fog   */
	    case AUTUMN:
	      chance_of_rain = 15;
	      break;
	    case SPRING:
	      chance_of_rain = 8;
	      break;
	    case SUMMER:
	      chance_of_rain = 5;
	      break;
	    }
	  roll = number (1, 100);
	  if (weather_info[i].wind_speed == BREEZE && roll < chance_of_rain)
	    {
	      weather_info[i].fog = THIN_FOG;
	      send_outside_zone ("A thin fog wafts in on the breeze.\n\r", i);
	    }
	  if (weather_info[i].wind_speed == CALM && roll < chance_of_rain)
	    {
	      weather_info[i].fog = THICK_FOG;
	      send_outside_zone
		("A thick fog begins to condense in the still air.\n\r", i);
	    }
	}

      /*
         sprintf (buf,"%d:00 %d degrees F\n\r",time_info.hour,weather_info.temperature);
         send_to_gods (buf);
       */

      desc_weather[i] = WR_NORMAL;

      if (weather_info[i].clouds > CLEAR_SKY)
	desc_weather[i] = WR_CLOUDY;

      if (weather_info[i].fog)
	desc_weather[i] = WR_FOGGY;

      if (weather_info[i].state > CHANCE_RAIN)
	{
	  desc_weather[i] = WR_RAINY;
	  if (weather_info[i].wind_speed == STORMY)
	    desc_weather[i] = WR_STORMY;
	}

      if (weather_info[i].state > HEAVY_RAIN)
	{
	  desc_weather[i] = WR_SNOWY;
	  if (weather_info[i].wind_speed == STORMY)
	    desc_weather[i] = WR_BLIZARD;
	}

      if (time_info.hour < sunrise[time_info.month]
	  && time_info.hour > sunset[time_info.month])
	{
	  switch (desc_weather[i])
	    {
	    case WR_NORMAL:
	      desc_weather[i] = WR_NIGHT;
	      break;
	    case WR_FOGGY:
	      desc_weather[i] = WR_NIGHT_FOGGY;
	      break;
	    case WR_CLOUDY:
	      desc_weather[i] = WR_NIGHT;
	      break;
	    case WR_RAINY:
	      desc_weather[i] = WR_NIGHT_RAINY;
	      break;
	    case WR_SNOWY:
	      desc_weather[i] = WR_NIGHT_SNOWY;
	      break;
	    case WR_BLIZARD:
	      desc_weather[i] = WR_NIGHT_BLIZARD;
	      break;
	    case WR_STORMY:
	      desc_weather[i] = WR_NIGHT_STORMY;
	      break;
	    }
	}

      if (weather_info[i].clouds < HEAVY_CLOUDS
	  && weather_info[i].fog < THICK_FOG
	  && weather_info[i].state < HEAVY_SNOW)
	{
	  if (moon_setting)
	    send_outside_zone ("Ithil hangs low in the sky.\n\r", i);
	  if (moon_set)
	    send_outside_zone
	      ("Ithil slowly sinks from the sky, guided by Tilion to its rest.\n\r",
	       i);
	  if (moon_rise)
	    send_outside_zone
	      ("Ithil rises with stately grace into the sky.\n\r", i);
	  moon_light[i] = global_moon_light;
	}
      else
	{
	  moon_light[i] = 0;
	}
    }
}

int
is_leap_year (int year)
{
  if (year % 4 == 0)
    {
      if (year % 100 == 0)
	{
	  if (year % 400 == 0)
	    return 1;
	  else
	    return 0;
	}
      return 1;
    }

  return 0;
}

void
weather_and_time (int mode)
{
  int moon_rising = 0;
  int moon_setting = 0;
  int moon_set = 0;
  int moon_q;
  int d_day;
  int i;
  bool new_day = false;

  next_hour_update += 900;	/* This is a mud hour; 60*60/4 */

  sun_light = 0;
  /*global_moon_light = 0; */

  time_info.hour++;

  if (time_info.hour >= 24)
    {
      time_info.day++;
      time_info.hour = 0;
      new_day = true;
    }

  if (time_info.day >= 30 && new_day)
    {
      if (!time_info.holiday)
	time_info.month++;
      if (time_info.month >= 12)
	{
	  time_info.month = 0;
	}
      if (time_info.month == 0 || time_info.month == 1
	  || time_info.month == 11)
	time_info.season = WINTER;
      else if (time_info.month >= 2 && time_info.month <= 4)
	time_info.season = SPRING;
      else if (time_info.month > 4 && time_info.month <= 7)
	time_info.season = SUMMER;
      else if (time_info.month > 7 && time_info.month <= 10)
	time_info.season = AUTUMN;

      if (time_info.holiday == HOLIDAY_METTARE)
	{
	  time_info.holiday = HOLIDAY_YESTARE;
	  time_info.year++;
	}
      else if (time_info.holiday == HOLIDAY_LOENDE
	       && is_leap_year (time_info.year))
	time_info.holiday = HOLIDAY_ENDERI;
      else if (time_info.holiday == HOLIDAY_ENDERI)
	{
	  time_info.holiday = 0;
	  time_info.day = 0;
	}
      else if (time_info.month == 0)
	{
	  if (time_info.holiday != HOLIDAY_YESTARE)
	    time_info.holiday = HOLIDAY_METTARE;
	  else
	    {
	      time_info.holiday = 0;
	      time_info.day = 0;
	    }
	}
      else if (time_info.month == 3)
	{
	  if (time_info.holiday != HOLIDAY_TUILERE)
	    time_info.holiday = HOLIDAY_TUILERE;
	  else
	    {
	      time_info.holiday = 0;
	      time_info.day = 0;
	    }
	}
      else if (time_info.month == 6)
	{
	  if (time_info.holiday != HOLIDAY_LOENDE)
	    time_info.holiday = HOLIDAY_LOENDE;
	  else
	    {
	      time_info.holiday = 0;
	      time_info.day = 0;
	    }
	}
      else if (time_info.month == 9)
	{
	  if (time_info.holiday != HOLIDAY_YAVIERE)
	    time_info.holiday = HOLIDAY_YAVIERE;
	  else
	    {
	      time_info.holiday = 0;
	      time_info.day = 0;
	    }
	}
      else
	time_info.day = 0;
    }

  if (time_info.month >= 12)
    {
      time_info.year++;
      time_info.month = 0;
    }

  if (sunrise[time_info.month] <= time_info.hour &&
      sunset[time_info.month] > time_info.hour)
    sun_light = 1;

  if (sunrise[time_info.month] == time_info.hour + 1)
    send_outside ("A glow illuminates the eastern horizon.\n\r");

  if (sunrise[time_info.month] == time_info.hour)
    send_outside
      ("Anor's fiery exterior slowly lifts itself up over the eastern horizon beneath Arien's unwavering guidance.\n\r");

  if (sunset[time_info.month] == time_info.hour + 1)
    send_outside
      ("Anor begins dipping below the western horizon, guided to its respite by Arien.\n\r");

  if (sunset[time_info.month] == time_info.hour)
    send_outside
      ("Anor sets in a fiery cascade of brilliant color upon the western horizon.\n\r");

  d_day = (time_info.day + 15) % 30;
  moon_q = d_day * 24 / 30;

  for (i = -7; i <= 6; i++)
    {
      if (moon_q == (24 + time_info.hour + i) % 24)
	{
	  if (i == -7)
	    {
	      moon_set = 1;
	      global_moon_light = 0;
	    }
	  /*else
	     global_moon_light = 1; */

	  if (i == -6)
	    {
	      moon_setting = 1;
	    }
	  if (i == 6)
	    {
	      moon_rising = 1;
	      global_moon_light = (time_info.hour > 16
				   && time_info.hour < 20) ? 1 : 0;
	    }
	}
    }

  if (is_room_affected (world_affects, MAGIC_WORLD_SOLAR_FLARE))
    sun_light = 1;

  if (is_room_affected (world_affects, MAGIC_WORLD_CLOUDS))
    sun_light = 0;

  if (is_room_affected (world_affects, MAGIC_WORLD_MOON))
    global_moon_light = 1;

  weather (moon_setting, moon_rising, moon_set);
  }
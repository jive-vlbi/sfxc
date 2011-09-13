#!/usr/bin/env python
import datetime
months = [31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365]
months2 = [31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366]

def get_time(vextime):
  s = vextime.partition("y")
  year = int(s[0])
  s = s[2].partition("d")
  yday = int(s[0])
  s = s[2].partition("h")
  hour = int(s[0])
  s = s[2].partition("m")
  min = int(s[0])
  s = s[2].partition("s")
  sec = int(s[0])
  
  if (year % 4) == 0:
    month_table = months2
  else:
    month_table = months
  
  month = (i for i,m in enumerate(month_table) if m>=yday).next() + 1
  if(yday > 31):
    day = yday - month_table[month - 2]
  else:
    day = yday
  return datetime.datetime(year, month, day, hour, min, sec)

def get_date_string(year, day, seconds):
  hour = seconds / (60*60)
  min = (seconds % (60*60))/60
  sec = seconds % 60
  return `year`+'y'+`day`+'d'+`hour`+'h'+`min`+'m'+`sec`+'s'

/* 
 * Proview   $Id: MhData.java,v 1.5 2006-06-16 05:09:38 claes Exp $
 * Copyright (C) 2005 SSAB Oxel�sund AB.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with the program, if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

package jpwr.rt;
import java.util.Vector;
import java.util.ListIterator;
/**
 *  Description of the Class
 *
 *@author     Jonas Nylund
 *@created    November 25, 2002
 */
public class MhData
{

  public int maxNrOfAlarms;
  public int maxNrOfEvents;
  public Vector alarmVec;
  public Vector eventVec;


  public MhData(int maxNrOfAlarms, int maxNrOfEvents)
  {
    this.maxNrOfAlarms = maxNrOfAlarms;
    this.maxNrOfEvents = maxNrOfEvents;
    this.alarmVec = new Vector(maxNrOfAlarms);
    this.eventVec = new Vector(maxNrOfEvents);
  }
  public int getNrOfAlarms()
  {
    return alarmVec.size();
  }
  public int getNrOfEvents()
  {
    return eventVec.size();
  }
  public MhrEvent getAlarm(int i)
  {
    return (MhrEvent)alarmVec.get(i);
  }
  public MhrEvent getEvent(int i)
  {
    return (MhrEvent)eventVec.get(i);
  }
  public void addMessToVectorInSortedOrder(Vector v, MhrEvent ev)
  {
    ListIterator iter = v.listIterator();
    MhrEvent vEv;
    while(iter.hasNext())
    {
      vEv = (MhrEvent)iter.next();
      if(ev.eventTime.compareTo(vEv.eventTime) >= 0)
      {
        iter.previous();
	iter.add(ev);
	return;
      }
    }
    try
    {
      iter.add(ev);
    }
    catch(UnsupportedOperationException e)
    {
      System.out.println(e.toString());
    }
    catch(Exception e)
    {
      System.out.println(e.toString());
    }
  }

  /**
   *  Description of the Method
   *
   *@param  ev  Description of the Parameter
   */
  public void insertNewMess(MhrEvent ev)
  {
    switch (ev.eventType)
    {
      case Mh.mh_eEvent_Alarm:
        // Add to alarm list
        this.addMessToVectorInSortedOrder(alarmVec, ev);
        // Add copy to event list
        this.addMessToVectorInSortedOrder(eventVec, ev.getCopy());
        break;
      case Mh.mh_eEvent_Return:
        //leta reda p� objektet i larmlistan och vidta l�mplig �tg�rd
        for(int i = 0; i < alarmVec.size(); i++)
        {
          MhrEvent alEv = (MhrEvent)alarmVec.get(i);
          if((ev.targetId.nix == alEv.eventId.nix) && (ev.targetId.idx == alEv.eventId.idx))
          {
            // The alarm is acknowledged and can be removed
            if((alEv.eventStatus & Mh.mh_mEventStatus_NotAck) == 0)
            {
              alarmVec.removeElementAt(i);
            }
            // Set the return bit and display it
            else
            {
              alEv.eventStatus &= ~Mh.mh_mEventStatus_NotRet;
            }
            break;
          }
        }
        if((ev.eventFlags & Mh.mh_mEventFlags_Return) != 0)
        {
          // Add copy to event list
	  this.addMessToVectorInSortedOrder(eventVec, ev.getCopy());
        }
        break;
      case Mh.mh_eEvent_Ack:
        for(int i = 0; i < alarmVec.size(); i++)
        {
          MhrEvent alEv = (MhrEvent)alarmVec.get(i);
          if((ev.targetId.nix == alEv.eventId.nix) && (ev.targetId.idx == alEv.eventId.idx))
          {
            if((alEv.eventStatus & Mh.mh_mEventStatus_NotRet) == 0)
            {
              alarmVec.removeElementAt(i);
            }
            else
            {
              alEv.eventStatus &= ~Mh.mh_mEventStatus_NotAck;
            }
            break;
          }
        }
        if((ev.eventFlags & Mh.mh_mEventFlags_Ack) != 0)
        {
          // Add copy to event list
          this.addMessToVectorInSortedOrder(eventVec, ev.getCopy());
        }
        break;
      case Mh.mh_eEvent_Info:
        // Add to alarm list
        if((ev.eventFlags & Mh.mh_mEventFlags_InfoWindow) != 0)
        {
	  this.addMessToVectorInSortedOrder(alarmVec, ev);
	}
        // Add copy to event list
        eventVec.add(0, ev.getCopy());
        break;
      case Mh.EventType_ClearAlarmList:
	 alarmVec.clear();
	 System.out.println("Clearing alarmlist");
      break;
      
      default:
        break;
    }
    if(alarmVec.size() > maxNrOfAlarms)
    {
      alarmVec.removeElementAt(alarmVec.size() - 1);
    }
    if(eventVec.size() > maxNrOfEvents)
    {
      eventVec.removeElementAt(eventVec.size() - 1);
    }
  }
}



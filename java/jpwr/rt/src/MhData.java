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
    //System.out.println(ev.eventTime + " " + ev.eventText + " sts " + ev.eventStatus + " type " + ev.eventType);

    switch (ev.eventType)
    {
      case Mh.mh_eEvent_Alarm:
        //addera till larm-listan
        this.addMessToVectorInSortedOrder(alarmVec, ev);
	//***alarmVec.add(0, ev);
        //addera kopia till h�ndelse-listan
        this.addMessToVectorInSortedOrder(eventVec, ev.getCopy());
	//***eventVec.add(0, ev.getCopy());
        break;
      case Mh.mh_eEvent_Return:
        //leta reda p� objektet i larmlistan och vidta l�mplig �tg�rd
        for(int i = 0; i < alarmVec.size(); i++)
        {
          MhrEvent alEv = (MhrEvent)alarmVec.get(i);
          //System.out.println("loopar " + ev.eventId.nix + " " + alEv.eventId.nix + " " + ev.eventId.idx + " " + alEv.eventId.idx);
          if((ev.object.oix == alEv.object.oix) && (ev.object.oix == alEv.object.oix))
          {
            //larmet �r kvitterat och kan tas bort
            if((alEv.eventStatus & Mh.mh_mEventStatus_NotAck) == 0)
            {
              alarmVec.removeElementAt(i);
            }
            //annars s�tter vi returned biten och visar detta
            else
            {
              alEv.eventStatus &= ~Mh.mh_mEventStatus_NotRet;
            }
            break;
          }
        }
        //skall det l�ggas till i h�ndelselistan
        if((ev.eventFlags & Mh.mh_mEventFlags_Return) != 0)
        {
          //addera kopia till h�ndelse-listan
	  this.addMessToVectorInSortedOrder(eventVec, ev.getCopy());
          //**eventVec.add(0, ev.getCopy());
        }
        break;
      case Mh.mh_eEvent_Ack:
        for(int i = 0; i < alarmVec.size(); i++)
        {
          MhrEvent alEv = (MhrEvent)alarmVec.get(i);
          if((ev.object.oix == alEv.object.oix) && (ev.object.oix == alEv.object.oix))
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
        //System.out.println("AckMess eventFlags " + ev.eventFlags);
        //skall det l�ggas till i h�ndelselistan
        if((ev.eventFlags & Mh.mh_mEventFlags_Ack) != 0)
        {
          //addera kopia till h�ndelse-listan
          this.addMessToVectorInSortedOrder(eventVec, ev.getCopy());
	  //**eventVec.add(0, ev.getCopy());
        }
        break;
      case Mh.mh_eEvent_Info:
        //System.out.println("InfoMeddelande: " + ev.eventPrio + " " + ev.eventTime + " " + ev.eventText + " sts " + ev.eventStatus + " type " + ev.eventType);
        //addera till larm-listan
        if((ev.eventFlags & Mh.mh_mEventFlags_InfoWindow) != 0)
        {
	  this.addMessToVectorInSortedOrder(alarmVec, ev);
	  //**alarmVec.add(0, ev);
	}
        //addera kopia till h�ndelse-listan
        eventVec.add(0, ev.getCopy());
        break;
      case Mh.EventType_ClearAlarmList:
	 alarmVec.clear();
	 System.out.println("Rensar larmlistan");
        //addera kopia till h�ndelse-listan
        //eventVec.add(0, ev.getCopy());
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
    //System.out.println("eventtyp " + ev.eventType + " evestst "+ ev.eventStatus + " NotAck " + Mh.mh_mEventStatus_NotAck + " uttryck " + (ev.eventStatus &
    //Mh.mh_mEventStatus_NotAck));

  }
}



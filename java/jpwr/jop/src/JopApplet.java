
/**
 * Title:        <p>
 * Description:  Ut�ka befintliga JButton till PwrButton.<p>
 * Copyright:    Copyright (c) <p>
 * Company:      <p>
 * @author
 * @version 1.0
 */

package jpwr.jop;
import javax.swing.*;
import java.awt.event.*;
import java.awt.*;
import java.applet.*;
import jpwr.rt.*;

public class JopApplet extends JApplet
{
  public JopSession session;
  public JopEngine engine;

  public JopApplet()
  {
  }

  public void init() {
    String instance = this.getParameter("instance");

    engine = new JopEngine(1000, (Object)this);
    session = new JopSession( engine, (Object)this);
    if ( instance != null && instance.length() != 0) {
      // Substitutes for ��� because of mozilla...
      instance = instance.replace( '\\', '�');
      instance = instance.replace( '/', '�');
      instance = instance.replace( '@', '�');

      System.out.println( "Parameter instance: " + instance);
      engine.gdh.logString("Parameter instance: " + instance);
      engine.setInstance( instance);
    }
  }

  public void stop() {
//      System.exit(0);
  }
  public void destroy() {
    engine.close();
  }
  public String getAppletInfo(){
    return "Applet Information";
  }
  public String[][] getParameterInfo() {
    return null;
  }
}

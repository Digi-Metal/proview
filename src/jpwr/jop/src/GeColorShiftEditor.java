
package jpwr.jop;
import java.beans.*;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2000
 * Company:
 * @author 	 CS
 * @version 1.0
 */

public class GeColorShiftEditor extends PropertyEditorSupport {

  public GeColorShiftEditor() {
  }
  private static String[] tagStrings = {
	"Next",
	"Previous",
        "Reset"
	};
  public String[] getTags() {
    return tagStrings;
  }
  public String getJavaInitializationString() {
    return java.lang.String.valueOf(((Number)getValue()).intValue());
  }
  public void setAsText(String s) throws IllegalArgumentException {
    int value = ((Number)getValue()).intValue();
    if (s.equals("Next")) {
      setValue( new Integer(++value));
    }
    else if (s.equals("Previous")) {
     setValue( new Integer(--value));
    }
    else if (s.equals("Reset")) {
     setValue( new Integer(0));
    }
    else throw new IllegalArgumentException(s);
  }
}

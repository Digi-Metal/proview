package jpwr.pwrxtt;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URL;
import java.util.Date;
import java.util.StringTokenizer;
import java.util.Timer;
import java.util.TimerTask;
import java.util.Vector;
import java.util.concurrent.Semaphore;

import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.text.Editable;
import android.text.InputType;
import android.util.DisplayMetrics;
import android.util.FloatMath;
import android.util.Log;
import android.util.TypedValue;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.view.*;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.graphics.*;
import jpwr.rt.*;
import jpwr.app.*;
import jpwr.jopg.*;

public class MainActivity extends Activity implements PlowAppl, GraphApplIfc, GdhApplIfc, OnClickListener, 
						      AEvAppl, CcmApplIfc {
	static final int MODE_NO = 0;
	static final int MODE_SCROLL = 1;
	static final int MODE_ZOOM = 2;

        String pwrp_exe = "pwrp_exe/";
        String pwrp_load = "pwrp_load/";
        String pwr_exe = "pwr_exe/";
	Timer timer = new Timer();
        MyTask timerTask;
	MainView view;
	Gdh gdh = null;
	Handler handler = new Handler();
	MainActivity appl;
	float lastTouchX;
	float lastTouchY;
	float downTouchX;
	float downTouchY;
        int lastTouchId;
        double lastXSpeed;
        double lastYSpeed;
	MotionEvent lastMotionEvent = null;
        int eventCnt = 0;
	Timer eventTimer = new Timer();
	private EditText editInput;
	private EditText editValueInput;
	private EditText editUsername;
	private EditText editPassword;
	
	PlowDraw gdraw;
	PlowCmnIfc currentCmn;
	PlowRect r1;
	PlowAnnot a1;
	PlowAnnot a11;
	PlowAnnotPixmap p1;
	PlowNode zoomIn;
	PlowNode zoomOut;
	String inputFullName;
	Graph graph = null;
	Vector<String> graphObject = new Vector<String>();
	int inputType;
        AlertDialog valueInputDialog = null;
        Object valueInputDyn;
        Object valueInputObject;
        AlertDialog confirmDialog = null;
        Object confirmDyn;
        Object confirmObject;
        String confirmText;
        String messageText;
        AlertDialog inputDialog = null;
        AlertDialog loginDialog = null;
        Context context;
        boolean initDone = false;
        String currentUser = null;
        boolean newCurrentUser = false;
        String pwrHost = null;
	AEv aev = null;
	OpwinCmn opwinCmn = null;
	Menu menu = null;
	int viewOffsetY = 0;
	int eventMode;
	float eventDistance;
	Vector<AGraphInfo> graphList = new Vector<AGraphInfo>();	
	Vector<PlowCmnIfc> cmnList = new Vector<PlowCmnIfc>();
        Canvas currentCanvas = null;
        float density;
        final Semaphore semDraw = new Semaphore( 1, true);
        String[] urlSymbols = null;
        int lang = 45;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		//setContentView(R.layout.activity_main);
		appl = this;
		view = new MainView(this);
		setContentView(view);
		Log.i("PwrXtt", "Starting");
		view.invalidate();
		setScanTime(0.5);

		DisplayMetrics metrics = getResources().getDisplayMetrics();
		density = metrics.densityDpi / 150;
		
		PlowCmn cmn = new PlowCmn(this, density * 15);		
		gdraw = new PlowDraw(this);
		cmn.setGDraw(gdraw);
		currentCmn = cmn;
		cmnList.add(cmn);
		
		editInput = new EditText(this);

		// Get stored host
		String host = "10.0.2.2";		
		try {
			SharedPreferences hostPrefs = getSharedPreferences("prefs", MODE_PRIVATE);
			if ( hostPrefs != null)
			  host = hostPrefs.getString("host", "10.0.2.2");
		}
		catch ( ClassCastException e) {
		}
	       
		editInput.setText( host, TextView.BufferType.EDITABLE);

		editInput.setSingleLine();
		new AlertDialog.Builder(this)
			.setTitle(R.string.app_name)
			.setMessage("Enter target node")
			.setView(editInput)
			.setPositiveButton("Ok", this)
			.setNegativeButton("Cancel", null)
			.show();
	}

        void setScanTime( double time) {
	    if ( timerTask != null)
		timerTask.cancel();
	    timerTask = new MyTask();
	    timer.schedule( timerTask, 0, (int)(1000 * time));
	}

	public void onClick(DialogInterface dialog, int position) {
		if ( dialog == inputDialog) {
		    Editable value = editInput.getText();
		    System.out.println( "Change Value: " + value.toString());
		    new GdhTask().execute(new GdhTaskArg(GdhTask.CHANGE_VALUE, value.toString()));
		}
		else if ( dialog == valueInputDialog) {
		    Editable value = editValueInput.getText();
		    System.out.println( "Change Value: " + value.toString());

		    new GdhTask().execute(new GdhTaskArg(GdhTask.VALUEINPUT_ACTION, value.toString()));
		}
		else if ( dialog == loginDialog) {
		    Editable usernameValue = editUsername.getText();
		    Editable passwordValue = editPassword.getText();

		    String[] item = new String[2];
		    item[0] = usernameValue.toString();
		    item[1] = GlowCrypt.crypt( "aa", passwordValue.toString());
			
		    new GdhTask().execute(new GdhTaskArg(GdhTask.LOGIN, item));
		}
		else if ( dialog == confirmDialog) {
		    System.out.println( "Confirm");

		    new GdhTask().execute(new GdhTaskArg(GdhTask.CONFIRM_ACTION, null));
		}
		else {
		    Editable value = editInput.getText();
		    pwrHost = value.toString();
		    System.out.println( "Input: " + value.toString());
		    //MainActivity callingActivity = (MainActivity) this;
		    //callingActivity.onUserSelectValue(value);
		    dialog.dismiss();
			setTitle("PwrXtt on " + pwrHost);
		    new GdhTask().execute(new GdhTaskArg(GdhTask.ROOTLIST,(AXttItemBase)null));
			new GdhTask().execute(new GdhTaskArg(GdhTask.OPWIN, null));

			// Store entered host for next session
			SharedPreferences settings = getSharedPreferences("prefs", MODE_PRIVATE);
			if ( settings != null) {
			  SharedPreferences.Editor editor = settings.edit();
			  editor.putString("host", pwrHost);
			  editor.commit();
			}
		}
	}

        public void onBackPressed() {
	    if ( (opwinCmn != null && currentCmn == opwinCmn) ||
		 (opwinCmn == null && currentCmn == cmnList.get(0))) {
		super.onBackPressed();
		return;
	    }

	    // Back to opwind
	    if ( opwinCmn != null) {
		while ( currentCmn != opwinCmn)
		    pushCmn();	       
	    }
	    else {
		while ( currentCmn != cmnList.get(0))
		    pushCmn();	       
	    }
        }

	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
		
		view.invalidate();
	}

	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		this.menu = menu;
		configMenu();
		return true;
	}
		
	public void configMenu() {
		if ( menu == null)
			return;
		
		MenuItem item_close = menu.findItem(R.id.close_option);
		MenuItem item_zoomin = menu.findItem(R.id.zoomin_option);
		MenuItem item_zoomout = menu.findItem(R.id.zoomout_option);
		MenuItem item_pageup = menu.findItem(R.id.pageup_option);
		MenuItem item_pagedown = menu.findItem(R.id.pagedown_option);
		MenuItem item_openobject = menu.findItem(R.id.openobject_option);
		MenuItem item_opengraph = menu.findItem(R.id.opengraph_option);
		MenuItem item_openclassgraph = menu.findItem(R.id.openclassgraph_option);
		MenuItem item_openplc = menu.findItem(R.id.openplc_option);
		MenuItem item_opencrr = menu.findItem(R.id.opencrr_option);
		MenuItem item_alarmack = menu.findItem(R.id.alarmack_option);
		MenuItem item_navigator = menu.findItem(R.id.navigator_option);
		MenuItem item_helpclass = menu.findItem(R.id.helpclass_option);
		MenuItem item_changevalue = menu.findItem(R.id.changevalue_option);
		if ( aev != null && currentCmn == aev.getCmnAla()) {
			// Alarm list
			item_close.setVisible(true);
			item_zoomin.setVisible(false);
			item_zoomout.setVisible(false);
			item_pageup.setVisible(false);
			item_pagedown.setVisible(false);
			item_openobject.setVisible(true);
			item_opengraph.setVisible(true);
			item_openclassgraph.setVisible(true);
			item_openplc.setVisible(true);
			item_opencrr.setVisible(true);
			item_alarmack.setVisible(true);
			item_navigator.setVisible(true);
			item_helpclass.setVisible(true);
			item_changevalue.setVisible(false);
		}
		else if ( aev != null && currentCmn == aev.getCmnEve()) {
			// Event list
			item_close.setVisible(true);
			item_zoomin.setVisible(false);
			item_zoomout.setVisible(false);
			item_pageup.setVisible(false);
			item_pagedown.setVisible(false);
			item_openobject.setVisible(true);
			item_opengraph.setVisible(true);
			item_openclassgraph.setVisible(true);
			item_openplc.setVisible(true);
			item_opencrr.setVisible(true);
			item_alarmack.setVisible(false);
			item_navigator.setVisible(true);
			item_helpclass.setVisible(true);
			item_changevalue.setVisible(false);
		}
		else if ( currentCmn == cmnList.get(0)) {
			// Navigator
			item_close.setVisible(true);
			item_zoomin.setVisible(false);
			item_zoomout.setVisible(false);
			item_pageup.setVisible(false);
			item_pagedown.setVisible(false);
			item_openobject.setVisible(true);
			item_opengraph.setVisible(true);
			item_openclassgraph.setVisible(true);
			item_openplc.setVisible(true);
			item_opencrr.setVisible(true);
			item_alarmack.setVisible(false);
			item_navigator.setVisible(true);
			item_helpclass.setVisible(true);
			item_changevalue.setVisible(true);
		}
		else if ( currentCmn.type() == PlowCmnIfc.TYPE_FLOW) {
			// Plc
			item_close.setVisible(true);
			item_zoomin.setVisible(false);
			item_zoomout.setVisible(false);
			item_pageup.setVisible(false);
			item_pagedown.setVisible(false);
			item_openobject.setVisible(true);
			item_opengraph.setVisible(true);
			item_openclassgraph.setVisible(true);
			item_openplc.setVisible(true);
			item_opencrr.setVisible(true);
			item_alarmack.setVisible(false);
			item_navigator.setVisible(true);
			item_helpclass.setVisible(true);
			item_changevalue.setVisible(false);
		}
		else if ( currentCmn.type() == PlowCmnIfc.TYPE_GRAPH) {
			// Graph
			item_close.setVisible(true);
			item_zoomin.setVisible(false);
			item_zoomout.setVisible(false);
			item_pageup.setVisible(false);
			item_pagedown.setVisible(false);
			item_openobject.setVisible(false);
			item_opengraph.setVisible(false);
			item_openclassgraph.setVisible(false);
			item_openplc.setVisible(false);
			item_opencrr.setVisible(false);
			item_alarmack.setVisible(false);
			item_navigator.setVisible(true);
			item_helpclass.setVisible(false);
			item_changevalue.setVisible(false);
		}
		else if ( currentCmn.type() == PlowCmnIfc.TYPE_OPWIN) {
			// Operator window
			item_close.setVisible(true);
			item_zoomin.setVisible(false);
			item_zoomout.setVisible(false);
			item_pageup.setVisible(false);
			item_pagedown.setVisible(false);
			item_openobject.setVisible(false);
			item_opengraph.setVisible(false);
			item_openclassgraph.setVisible(false);
			item_openplc.setVisible(false);
			item_opencrr.setVisible(false);
			item_alarmack.setVisible(false);
			item_navigator.setVisible(true);
			item_helpclass.setVisible(false);
			item_changevalue.setVisible(false);
		}
		else {
			// Default
			item_pageup.setVisible(false);
			item_zoomin.setVisible(false);
			item_zoomout.setVisible(false);
			item_pagedown.setVisible(false);
			item_openobject.setVisible(true);
			item_opengraph.setVisible(true);
			item_openclassgraph.setVisible(true);
			item_openplc.setVisible(true);
			item_opencrr.setVisible(true);
			item_alarmack.setVisible(false);
			item_navigator.setVisible(true);
			item_helpclass.setVisible(false);
			item_changevalue.setVisible(false);
		}
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch( item.getItemId()) {
		case R.id.zoomin_option:
			currentCmn.zoom(1.25D);
			view.invalidate();
			System.out.println("Zoom in");
			break;			
		case R.id.zoomout_option:
			currentCmn.zoom(0.8D);
			view.invalidate();
			System.out.println("Zoom in");
			break;			
		case R.id.pageup_option:
			currentCmn.pageUp();
			view.invalidate();
			System.out.println("Page up");
			break;			
		case R.id.pagedown_option:
			currentCmn.pageDown();
			view.invalidate();
			System.out.println("Page down");
			break;			
		case R.id.close_option:
		        pushCmn();
			break;
		case R.id.openobject_option: {
 			System.out.println("Open object");
			if ( !gdh.isAuthorized(Pwr.mAccess_AllRt)) {
			    openMessageDialog("Not authorized");
			    break;
			}
			switch (currentCmn.type()) {
			case PlowCmnIfc.TYPE_PLOW: {
				PlowNode selectedNode = (PlowNode)currentCmn.getSelect();
				if ( selectedNode != null) {
					AXttItemBase[] items = new AXttItemBase[1];
					AXttItemBase baseItem = (AXttItemBase)selectedNode.getUserData();
					new GdhTask().execute(new GdhTaskArg(GdhTask.OPEN_ATTRIBUTES, baseItem));
				}
				break;
			}
			case PlowCmnIfc.TYPE_FLOW: {
				FlowNode selectedNode = (FlowNode)currentCmn.getSelect();
				new GdhTask().execute(new GdhTaskArg(GdhTask.OPEN_FLOWNODE, selectedNode));
				break;
			}
			}
			break;			
		}
		case R.id.opencrr_option: {
			System.out.println("Open cross");
			if ( !gdh.isAuthorized(Pwr.mAccess_AllRt)) {
			    openMessageDialog("Not authorized");
			    break;
			}
			switch (currentCmn.type()) {
			case PlowCmnIfc.TYPE_PLOW: {
				PlowNode selectedNode = (PlowNode)currentCmn.getSelect();
				if ( selectedNode != null) {
					AXttItemBase[] items = new AXttItemBase[1];
					AXttItemBase baseItem = (AXttItemBase)selectedNode.getUserData();
					new GdhTask().execute(new GdhTaskArg(GdhTask.CROSSREFERENCES, baseItem));
				}
				break;
			}
			case PlowCmnIfc.TYPE_FLOW: {
				FlowNode selectedNode = (FlowNode)currentCmn.getSelect();
				if ( selectedNode != null) {
					new GdhTask().execute(new GdhTaskArg(GdhTask.FLOW_CROSSREFERENCE, selectedNode));
				}
				break;
			}
			}
			break;			
		}
		case R.id.openplc_option: {
			System.out.println("Open plc");					
			if ( !gdh.isAuthorized(Pwr.mAccess_RtPlc | Pwr.mAccess_System)) {
			    openMessageDialog("Not authorized");
			    break;
			}
			switch (currentCmn.type()) {
			case PlowCmnIfc.TYPE_PLOW: {
				PlowNode selectedNode = (PlowNode)currentCmn.getSelect();
				if ( selectedNode != null) {
					AXttItemBase baseItem = (AXttItemBase)selectedNode.getUserData();
					if ( baseItem instanceof AXttItemObject) {
						if (((AXttItemObject)baseItem).getClassid() == Pwrb.cClass_plc) {
							PwrtObjid woid = ((AXttItemObject)baseItem).getWindowObjid();
							new ReaderTask().execute(new ReaderTaskArg(ReaderTask.FLOW_READ, woid, null));
						}
						else {
							PwrtObjid oid = ((AXttItemObject)baseItem).getObjid();
							new GdhTask().execute(new GdhTaskArg(GdhTask.OPEN_FLOW, oid));
						}
					}
					else if ( baseItem instanceof AXttItemCrr) {
						new GdhTask().execute(new GdhTaskArg(GdhTask.OPEN_FLOWCROSS, baseItem));
					}
				}
				break;
			}
			case PlowCmnIfc.TYPE_FLOW: {
				FlowNode selectedNode = (FlowNode)currentCmn.getSelect();
				if ( selectedNode != null) {
					new GdhTask().execute(new GdhTaskArg(GdhTask.OPEN_FLOWSUBWINDOW, selectedNode));
					System.out.println("Open object");					
				}
				break;
			}
			}
			break;
		}
		case R.id.opengraph_option: {
			System.out.println("Open graph");					
			if ( !gdh.isAuthorized(Pwr.mAccess_AllRt)) {
			    openMessageDialog("Not authorized");
			    break;
			}
			switch (currentCmn.type()) {
			case PlowCmnIfc.TYPE_PLOW: {
				PlowNode selectedNode = (PlowNode)currentCmn.getSelect();
				if ( selectedNode != null) {
					AXttItemBase baseItem = (AXttItemBase)selectedNode.getUserData();
					if ( baseItem instanceof AXttItemObject) {
						PwrtObjid oid = ((AXttItemObject)baseItem).getObjid();
						new GdhTask().execute(new GdhTaskArg(GdhTask.OPEN_DEFGRAPH, oid));
					}
				}
				break;
			}
			case PlowCmnIfc.TYPE_FLOW: {
				break;
			}
			}
			break;
		}
		case R.id.openclassgraph_option: {
			System.out.println("Open Object graph " + currentCmn.type());								
			if ( !gdh.isAuthorized(Pwr.mAccess_AllRt)) {
			    openMessageDialog("Not authorized");
			    break;
			}
			switch (currentCmn.type()) {
			case PlowCmnIfc.TYPE_PLOW: {
				PlowNode selectedNode = (PlowNode)currentCmn.getSelect();
				if ( selectedNode != null) {
					AXttItemBase baseItem = (AXttItemBase)selectedNode.getUserData();
					if ( baseItem instanceof AXttItemObject) {
						PwrtAttrRef aref = new PwrtAttrRef(((AXttItemObject)baseItem).getObjid());
						new GdhTask().execute(new GdhTaskArg(GdhTask.OPEN_CLASSGRAPH, aref));
					}
					else if ( baseItem instanceof AXttItemAttrObject) {
						PwrtAttrRef aref = ((AXttItemAttrObject)baseItem).getAttrRef();
						new GdhTask().execute(new GdhTaskArg(GdhTask.OPEN_CLASSGRAPH, aref));
					}
				}
				break;
			}
			case PlowCmnIfc.TYPE_FLOW: {
				break;
			}
			}
			break;
		}
		case R.id.changevalue_option: {
			switch (currentCmn.type()) {
			case PlowCmnIfc.TYPE_PLOW: {
				if ( !gdh.isAuthorized(Pwr.mPrv_RtWrite | Pwr.mPrv_System)) {
				    openMessageDialog("Not authorized to change value");
				    break;
				}
				else {					
					PlowNode selectedNode = (PlowNode)currentCmn.getSelect();
					if ( selectedNode != null) {
						AXttItemBase baseItem = (AXttItemBase)selectedNode.getUserData();
						if ( baseItem instanceof AXttItemAttr) {
							inputFullName = ((AXttItemAttr)baseItem).getFullName();
							inputType = ((AXttItemAttr)baseItem).getType();
							editInput = new EditText(this);
							editInput.setSingleLine();
							inputDialog = new AlertDialog.Builder(this)
							.setTitle(R.string.app_name)
							.setMessage("Change Value")
							.setView(editInput)
							.setPositiveButton("Ok", this)
							.setNegativeButton("Cancel", null)
							.show();
						}
					}
				}
				break;
			}
			}
			break;
		}
		case R.id.login_option: {
			System.out.println("Login");					

			editUsername = new EditText(this);
			editUsername.setHint("Username");
			editUsername.setSingleLine();
			editPassword = new EditText(this);
			editPassword.setHint("Password");
			editPassword.setSingleLine();
			editPassword.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
			LinearLayout layout = new LinearLayout(context);
			layout.setOrientation(LinearLayout.VERTICAL);
			layout.addView(editUsername);
			layout.addView(editPassword);
			
			loginDialog = new AlertDialog.Builder(this)
							.setTitle(R.string.app_name)
							.setMessage("Login")
							.setView(layout)
							.setPositiveButton("Ok", this)
							.setNegativeButton("Cancel", null)
							.show();
			break;
		}
		case R.id.logout_option: {
			gdh.logout();
			appl.setTitle("PwrXtt on " + pwrHost);
			break;
		}
		case R.id.navigator_option: {
			System.out.println("Open Navigator");
			if ( !gdh.isAuthorized(Pwr.mAccess_RtNavigator | Pwr.mAccess_System)) {
			    openMessageDialog("Not authorized");
			    break;
			}

			int xttCmnIdx = 0;
				
			currentCmn = cmnList.get(xttCmnIdx);
			for ( int i = cmnList.size()-1; i > xttCmnIdx; i--)
				cmnList.removeElementAt(i);
			break;			
		}
		case R.id.alarmlist_option: {
			System.out.println("Open Alarmlist");					
			if ( !gdh.isAuthorized(Pwr.mAccess_AllRt)) {
			    openMessageDialog("Not authorized");
			    break;
			}

			if ( aev == null) {
				PlowCmnEv cmnAla = new PlowCmnEv(appl, density * 15);		
				cmnAla.setGDraw(gdraw);
				PlowCmnEv cmnEve = new PlowCmnEv(appl, density * 15);		
				cmnEve.setGDraw(gdraw);
				setScanTime( 1);
				currentCmn = cmnAla;
				aev = new AEv(cmnAla, cmnEve, appl);
				cmnAla.setUserData(aev);
				cmnEve.setUserData(aev);
				view.invalidate();
			}
			else {
			        setScanTime( 1);
				currentCmn = aev.getCmnAla();
				view.invalidate();
			}
			break;
		}
		case R.id.eventlist_option: {
			System.out.println("Open Eventlist");					
			if ( !gdh.isAuthorized(Pwr.mAccess_AllRt)) {
			    openMessageDialog("Not authorized");
			    break;
			}

			if ( aev == null) {
				PlowCmnEv cmnAla = new PlowCmnEv(appl, density * 15);		
				cmnAla.setGDraw(gdraw);
				PlowCmnEv cmnEve = new PlowCmnEv(appl, density * 15);		
				cmnEve.setGDraw(gdraw);
				setScanTime( 1);
				currentCmn = cmnEve;
				aev = new AEv(cmnAla, cmnEve, appl);
				cmnAla.setUserData(aev);
				cmnEve.setUserData(aev);
				view.invalidate();
			}
			else {
			        setScanTime( 1);
				currentCmn = aev.getCmnEve();
				view.invalidate();
			}
			break;
		}
		case R.id.alarmack_option: {
			if ( !gdh.isAuthorized(Pwr.mAccess_RtEventsAck | Pwr.mAccess_System)) {
			    openMessageDialog("Not authorized");
			    break;
			}

			if ( aev != null && currentCmn == aev.getCmnAla()) {
				aev.acknowledge();
				view.invalidate();
			}
			else {
			        setScanTime( 1);
				currentCmn = aev.getCmnEve();
				view.invalidate();
			}
			break;
		}
		case R.id.helpclass_option: {
			System.out.println("Help Class");					
			if ( !gdh.isAuthorized(Pwr.mAccess_AllRt)) {
			    openMessageDialog("Not authorized");
			    break;
			}
			switch (currentCmn.type()) {
			case PlowCmnIfc.TYPE_PLOW: {
				PlowNode selectedNode = (PlowNode)currentCmn.getSelect();
				if ( selectedNode != null) {
				    AXttItemBase baseItem = (AXttItemBase)selectedNode.getUserData();
				    new GdhTask().execute(new GdhTaskArg(GdhTask.CLASS_HELP, baseItem));
				}
				break;
			}
			case PlowCmnIfc.TYPE_FLOW: {
			    // TODO
				FlowNode selectedNode = (FlowNode)currentCmn.getSelect();
				new GdhTask().execute(new GdhTaskArg(GdhTask.FLOW_CLASSHELP, selectedNode));
				break;
			}
			}
			break;			
		}
		}
		return true;
	}

	public void eventHandler(PlowEvent e) {
		switch (e.type) {
		case PlowEvent.TYPE_CLICK:
			if ( e.object == null)
				return;
			
			switch(currentCmn.type()) {
			case PlowCmnIfc.TYPE_PLOW:
				if ( e.inIcon) {
					// Open children
					System.out.println("Click event");
					AXttItemBase[] items = new AXttItemBase[1];
					AXttItemBase item = (AXttItemBase)((PlowNode)e.object).getUserData();
					new GdhTask().execute(new GdhTaskArg(GdhTask.OPEN_CHILDREN, item));
				}
				else {
					// Select
					if ( currentCmn.getSelect() == e.object) {
						currentCmn.select(null);
						((PlowNode)e.object).setInvert(false);
						//view.invalidate();
					}
					else {
						if ( currentCmn.getSelect() != null)
							((PlowNode)currentCmn.getSelect()).setInvert(false);
						currentCmn.select((PlowNode)e.object);
						((PlowNode)e.object).setInvert(true);					
						//view.invalidate();
					}
				}	
				break;
			case PlowCmnIfc.TYPE_EV:
				// Select
				if ( currentCmn.getSelect() == e.object) {
					currentCmn.select(null);
					((PlowNode)e.object).setInvert(false);
					//view.invalidate();
				}
				else {
					if ( currentCmn.getSelect() != null)
						((PlowNode)currentCmn.getSelect()).setInvert(false);
					currentCmn.select((PlowNode)e.object);
					((PlowNode)e.object).setInvert(true);					
					//view.invalidate();
				}
				break;
			case PlowCmnIfc.TYPE_FLOW:
				if (e.object instanceof FlowNode) {
					Object s = currentCmn.getSelect();
					if ( s == e.object) {
						((FlowNode)e.object).setSelect(false);
						currentCmn.selectClear();
						view.invalidate();
					}
					else if ( s != null) {
						((FlowNode)s).setSelect(false);						
						((FlowNode)e.object).setSelect(true);
						currentCmn.select(e.object);
						view.invalidate();
					}
					else {
						((FlowNode)e.object).setSelect(true);						
						currentCmn.select(e.object);
						view.invalidate();
					}
				}
				break;
			case PlowCmnIfc.TYPE_OPWIN:
				currentCmn.eventHandler(e.type, e.x, e.y);
				break;
			}
			break;
		case PlowEvent.TYPE_LONG_CLICK:
			if ( e.object == null)
				return;
			
			switch(currentCmn.type()) {
			case PlowCmnIfc.TYPE_PLOW:
				if ( e.inIcon) {
					// Open object
					System.out.println("Long click event");
					AXttItemBase item = (AXttItemBase)((PlowNode)e.object).getUserData();
					new GdhTask().execute(new GdhTaskArg(GdhTask.OPEN_ATTRIBUTES, item));
				}
				else {
					// Open plc
					System.out.println("Long click event");
					AXttItemBase item = (AXttItemBase)((PlowNode)e.object).getUserData();
					if ( item instanceof AXttItemObject) {
					    switch (((AXttItemObject)item).getClassId()) {
					    case Pwrb.cClass_plc:
					    case Pwrb.cClass_windowplc:
					    case Pwrb.cClass_windowcond:
					    case Pwrb.cClass_windoworderact:
 					    case Pwrb.cClass_windowsubstep: {
						PwrtObjid oid = ((AXttItemObject)item).getObjid();
						new GdhTask().execute(new GdhTaskArg(GdhTask.OPEN_FLOW, oid));
						break;
					    }
 					    case Pwrb.cClass_AppGraph:
 					    case Pwrb.cClass_WebGraph:
 					    case Pwrb.cClass_XttGraph: {
						PwrtObjid oid = ((AXttItemObject)item).getObjid();
						new GdhTask().execute(new GdhTaskArg(GdhTask.OPEN_DEFGRAPH, oid));
						break;
					    }
					    }
					}
					else if ( item instanceof AXttItemAttr) {
					    switch ( ((AXttItemAttr)item).getType()) {
					    case Pwr.eType_String: {
						System.out.println("URL found");
						break;
					    }
					    }
					}
				}	
				break;
			case PlowCmnIfc.TYPE_EV:
				break;
			case PlowCmnIfc.TYPE_FLOW:
				break;
			case PlowCmnIfc.TYPE_OPWIN:
				break;
			}
			break;
		case PlowEvent.TYPE_OBJECT_DELETED: {
			AXtt axtt = (AXtt)currentCmn.getUserData();
			AXttItemBase item = (AXttItemBase)((PlowNode)e.object).getUserData();
			if ( item instanceof AXttItemAttr) {
				((AXttItemAttr)item).close(axtt);
			}
			else if ( item instanceof AXttItemAttrArrayElem) {
				((AXttItemAttrArrayElem)item).close(axtt);
			}
			break;
		}
		}
	}

	private class GdhTaskArg {
		int action;
		Object item;

		public GdhTaskArg(int action, Object item) {
			this.action = action;
			this.item = item;
		}
	}
	private class GdhEventArg {
			int action;
			float x;
			float y;
			public GdhEventArg(int action, float x, float y) {
				this.action = action;
				this.x = x;
				this.y = y;
			}
	}
	
	private class GdhTask extends AsyncTask<GdhTaskArg, Void, Void> {
		public static final int ROOTLIST = 1;
		public static final int SCAN = 2;
		public static final int OPEN_CHILDREN = 3;
		public static final int OPEN_ATTRIBUTES = 4;
		public static final int DYNAMIC_OPEN = 5;
		public static final int DYNAMIC_CLOSE = 6;
		public static final int DYNAMIC_UPDATE = 7;
		public static final int OPEN_FLOWNODE = 8;
		public static final int CROSSREFERENCES = 9;
		public static final int OPEN_FLOWSUBWINDOW = 10;
		public static final int OPEN_FLOW = 11;
		public static final int OPEN_FLOWCROSS = 12;
		public static final int FLOW_CROSSREFERENCE = 13;
		public static final int CHANGE_VALUE = 14;
		public static final int OPEN_DEFGRAPH = 15;
		public static final int EVENTHANDLER = 16;
		public static final int OPEN_CLASSGRAPH = 17;
		public static final int OPEN_CLASSGRAPH_NAME = 18;
		public static final int VALUEINPUT_ACTION = 19;
		public static final int CONFIRM_ACTION = 20;
		public static final int OPWIN = 21;
		public static final int LOGIN = 22;
    	        public static final int CLASS_HELP = 23;
    	        public static final int FLOW_CLASSHELP = 24;
    	        public static final int OPEN_GRAPH_OBJECT = 25;
		
		@Override
		protected Void doInBackground(GdhTaskArg... arg) {
			
			if ( gdh == null) {
				gdh = new Gdh(appl);		
				AXtt axtt = new AXtt((PlowCmn)currentCmn, gdh);
				currentCmn.setUserData(axtt);
				gdh.refObjectInfoList();
			}

			switch ( arg[0].action) {
			case ROOTLIST: {
				AXtt axtt = (AXtt)currentCmn.getUserData();

				System.out.println("Open rootlist");
				AXttItemObject.openRoot(axtt);
				currentCmn.configure();
				loadUrlSymbols();
				initDone = true;
				break;
			}
			case SCAN: {
				if ( currentCmn.type() != PlowCmnIfc.TYPE_PLOW)
					break;

				PlowCmn cmn = (PlowCmn)currentCmn;
				AXtt axtt = (AXtt)currentCmn.getUserData();
				
				gdh.getObjectRefInfoAll();
				for ( int i = 0; i < cmn.size(); i++) {
					AXttItemBase bitem = (AXttItemBase)((PlowNode)cmn.get(i)).getUserData();
					if ( bitem instanceof AXttItemAttr) {
						((AXttItemAttr)bitem).scan(axtt);
					}
					else if ( bitem instanceof AXttItemAttrArrayElem) {
						((AXttItemAttrArrayElem)bitem).scan(axtt);
					}
				}
				break;
			}
			case OPEN_CHILDREN: {
				AXtt axtt = (AXtt)currentCmn.getUserData();

				if (((AXttItemBase)arg[0].item).hasChildren())
					((AXttItemBase)arg[0].item).openChildren(axtt);
				else
					((AXttItemBase)arg[0].item).openAttributes(axtt);
				currentCmn.configure();
				break;
			}
			case OPEN_ATTRIBUTES: {
				AXtt axtt = (AXtt)currentCmn.getUserData();

				((AXttItemBase)arg[0].item).openAttributes(axtt);
				currentCmn.configure();
				break;
			}
			case CROSSREFERENCES: {
				AXtt axtt = (AXtt)currentCmn.getUserData();

				((AXttItemBase)arg[0].item).openCrossreferences(axtt);
				currentCmn.configure();
				break;
			}
			case DYNAMIC_OPEN:
				((FlowCmn)currentCmn).dynamicOpen();
				break;
			case DYNAMIC_CLOSE:
				((PlowCmnIfc)arg[0].item).dynamicClose();
				break;
			case DYNAMIC_UPDATE:
				gdh.getObjectRefInfoAll();
				try {
				    semDraw.acquire();
				}
				catch ( InterruptedException e) {
				    System.out.println("InterruptedException");
				}
				currentCmn.dynamicUpdate();
				semDraw.release();
				break;
			case OPEN_FLOWNODE: {
				FlowNode node = (FlowNode)arg[0].item;
				String oName = node.getTraceObject();
				CdhrObjid oret = gdh.nameToObjid(oName);

				PlowCmn cmn = new PlowCmn(appl, density * 15);		
				cmn.setGDraw(gdraw);
				setScanTime( 1);
				currentCmn = cmn;
				cmnList.add(cmn);
				AXtt axtt = new AXtt((PlowCmn)currentCmn, gdh);
				cmn.setUserData(axtt);
				
				AXttItemObject o = new AXttItemObject(axtt, oret.objid, oName, false, null, Plow.DEST_INTOLAST);
				o.openAttributes(axtt);
				break;
			}
			case OPEN_FLOWSUBWINDOW: {
				// Open flow file for first child
				FlowNode node = (FlowNode)arg[0].item;
				PwrtObjid nodeObjid = node.getCmn().getObjid();
				CdhrString rName = gdh.objidToName(nodeObjid, Cdh.mName_pathStrict);
				if (rName.evenSts())
					break;
				String name = rName.str + "-" + node.getName();
				System.out.println( "Open flow: " + name);
				CdhrObjid oret = gdh.nameToObjid(name);
				if ( oret.evenSts()) break;
				oret = gdh.getChild(oret.objid);
				if ( oret.evenSts()) break;
				new ReaderTask().execute(new ReaderTaskArg(ReaderTask.FLOW_READ, oret.objid, null));
				break;
			}
			case OPEN_FLOW: {
				// If plc window open flow for window, else for first child
				PwrtObjid oid = (PwrtObjid)arg[0].item;
				CdhrClassId retCid = gdh.getObjectClass(oid);
				if ( retCid.evenSts())
					break;
				if ( !(retCid.classId == Pwrb.cClass_windowplc ||
						retCid.classId == Pwrb.cClass_windoworderact ||
						retCid.classId == Pwrb.cClass_windowcond ||
						retCid.classId == Pwrb.cClass_windowsubstep)) {
					CdhrObjid oret = gdh.getChild(oid);
					if (oret.evenSts()) break;

					oid = oret.objid;
				}
				new ReaderTask().execute(new ReaderTaskArg(ReaderTask.FLOW_READ, oid, null));
				break;
			}
			case OPEN_FLOWCROSS: {
				// If plc window open flow for window, else for first child
				AXttItemCrr crr = (AXttItemCrr)arg[0].item;
				String text = crr.getText();

				int idx = text.indexOf(' ');
				if ( idx < 0) break;
				text = text.substring(0, idx);
				idx = text.lastIndexOf('-');
				if ( idx < 0) break;

				String wname = text.substring(0, idx);
				String oname = text.substring(idx+1);
				System.out.println( "wname: " + wname + " oname: " + oname);
				
				CdhrObjid oret = gdh.nameToObjid(wname);
				if ( oret.evenSts()) break;

				new ReaderTask().execute(new ReaderTaskArg(ReaderTask.FLOW_READ, oret.objid, oname));
				break;
			}
			case FLOW_CROSSREFERENCE: {
				FlowNode node = (FlowNode)arg[0].item;
				String oName = node.getTraceObject();
				CdhrObjid oret = gdh.nameToObjid(oName);

				PlowCmn cmn = new PlowCmn(appl, density * 15);		
				cmn.setGDraw(gdraw);
				setScanTime( 1);
				currentCmn = cmn;
				cmnList.add(cmn);
				AXtt axtt = new AXtt((PlowCmn)currentCmn, gdh);
				cmn.setUserData(axtt);
				
				AXttItemObject o = new AXttItemObject(axtt, oret.objid, oName, false, null, Plow.DEST_INTOLAST);
				o.openCrossreferences(axtt);
				currentCmn.configure();
				break;
			}
			case CHANGE_VALUE: {
				switch ( inputType) {
				case Pwr.eType_Boolean: {
					boolean bval;
					if ( ((String)arg[0].item).equals("1"))
						bval = true;
					else if ( ((String)arg[0].item).equals("0"))
						bval = false;
					else
						break;
					PwrtStatus sts = gdh.setObjectInfo(inputFullName, bval);
					if ( sts.evenSts())
						System.out.println( "setObjectInfo " + sts.getSts() + " " + inputFullName);
					break;
				}
				case Pwr.eType_Float64:
				case Pwr.eType_Float32: {
					float fval;
					try {
						fval = Float.parseFloat((String)arg[0].item);
					}
					catch ( NumberFormatException e) {
						break;
					}
					PwrtStatus sts = gdh.setObjectInfo(inputFullName, fval);
					if ( sts.evenSts())
						System.out.println( "setObjectInfo " + sts.getSts() + " " + inputFullName);
					break;
				}
				case Pwr.eType_Int64:
				case Pwr.eType_Int32:
				case Pwr.eType_Int16:
				case Pwr.eType_Int8:
				case Pwr.eType_UInt64:
				case Pwr.eType_UInt32:
				case Pwr.eType_UInt16:
				case Pwr.eType_UInt8: {
					int ival;
					try {
						ival = Integer.parseInt((String)arg[0].item);
					}
					catch ( NumberFormatException e) {
						break;
					}
					PwrtStatus sts = gdh.setObjectInfo(inputFullName, ival);
					if ( sts.evenSts())
						System.out.println( "setObjectInfo " + sts.getSts() + " " + inputFullName);
					break;
				}				
				default: {
					PwrtStatus sts = gdh.setObjectInfo(inputFullName, (String)arg[0].item);
					if ( sts.evenSts())
						System.out.println( "setObjectInfo " + sts.getSts() + " " + inputFullName);
				}
				}	
				break;
			}
			case OPEN_DEFGRAPH: {
				PwrtObjid oid = (PwrtObjid)arg[0].item;
				String action = null;
				String instance = null;

				CdhrClassId cret = gdh.getObjectClass(oid);
				if (cret.evenSts())
				    break;
				
				switch ( cret.getClassId()) {
				case Pwrb.cClass_XttGraph: {
				  CdhrString rName = gdh.objidToName(oid, Cdh.mName_pathStrict);
				  if (rName.evenSts())
					break;
				  String oname = rName.str;
				  String name = oname + ".Action";
				  rName = gdh.getObjectInfoString(name);
				  if (rName.evenSts())
					break;				
				  action = rName.str;
				
				  name = oname + ".Object";
				  rName = gdh.getObjectInfoString(name);
				  if (rName.evenSts())
					break;
				  instance = rName.str;
				  if ( instance.equals(""))
					instance = null;
				  break;
				}
				case Pwrb.cClass_AppGraph:
				case Pwrb.cClass_WebGraph: {
				  CdhrString rName = gdh.objidToName(oid, Cdh.mName_pathStrict);
				  if (rName.evenSts())
					break;
				  String oname = rName.str;
				  String name = oname + ".Name";
				  rName = gdh.getObjectInfoString(name);
				  if (rName.evenSts())
					break;
				  action = rName.str;
				  if ( !action.contains(".pwg"))
				       action = action + ".pwg";
				  instance = null;
				  break;
				}
				default: {
				  CdhrString rName = gdh.objidToName(oid, Cdh.mName_pathStrict);
				  if (rName.evenSts())
					break;
				  String name = rName.str + ".DefGraph";
				  rName = gdh.getObjectInfoString(name);
				  if (rName.evenSts())
					break;
				  String oname = rName.str;
				
				  name = oname + ".Action";
				  rName = gdh.getObjectInfoString(name);
				  if (rName.evenSts())
					break;				
				  action = rName.str;
				
				  name = oname + ".Object";
				  rName = gdh.getObjectInfoString(name);
				  if (rName.evenSts())
					break;
				  instance = rName.str;
				  if ( instance.equals(""))
					instance = null;
				}
				}				
				if ( action == null)
				    break;

				System.out.println("Open " + action);
				new ReaderTask().execute(new ReaderTaskArg(ReaderTask.GROW_READ, action, instance));
				break;
			}
			case OPEN_CLASSGRAPH_NAME: {
				CdhrAttrRef aret = gdh.nameToAttrRef((String)arg[0].item);
				System.out.println("Classname " + (String)arg[0].item + " sts " + aret.getSts());
				if ( aret.evenSts())
					break;
				arg[0].item = new PwrtAttrRef(aret.aref.objid, aret.aref.body, aret.aref.offset, aret.aref.size, aret.aref.flags);

				// No break, continue with OPEN_CLASSGRAPH
			}
			case OPEN_CLASSGRAPH: {
				PwrtAttrRef aref = (PwrtAttrRef)arg[0].item;

				CdhrTypeId cret = gdh.getAttrRefTid(aref);
				System.out.println("Classgraph  type " + cret.getSts());
				if (cret.evenSts()) break; 

				CdhrObjid coidret = gdh.classIdToObjid(cret.typeId);
				if ( coidret.evenSts()) break;
				
				CdhrString cnameret = gdh.objidToName(coidret.objid, Cdh.mName_object);
				if (cnameret.evenSts()) break;
				
				String classname = cnameret.str.toLowerCase();
				if ( classname.startsWith("$")) 
					classname = classname.substring(1);
				
				// Get GraphConfiguration if any
				CdhrString nret = gdh.attrRefToName(aref, Cdh.mName_volumeStrict);
				if ( nret.evenSts()) break;
				
				String suffix = "";
				CdhrInt gcret = gdh.getObjectInfoInt(nret.str + ".GraphConfiguration");
				if (gcret.oddSts()) {
					if ( gcret.value > 0)
						suffix = Integer.toString(gcret.value);
				}
								
				String fname;
				if ( coidret.objid.vid < Cdh.cUserClassVolMin || 
						(coidret.objid.vid >= Cdh.cManufactClassVolMin && 
						coidret.objid.vid <= Cdh.cManufactClassVolMax)) {
					// Add pwr_c_ to filename
					fname = "$pwr_exe/pwr_c_" + classname + suffix + ".pwg";
				}
				else
					fname = classname + suffix + ".pwg";

				CdhrString rName = gdh.attrRefToName(aref, Cdh.mName_pathStrict);
				if (rName.evenSts())
					break;
									
				System.out.println("Open " + fname);
				new ReaderTask().execute(new ReaderTaskArg(ReaderTask.GROW_READ, fname, rName.str));
				break;
			}
			case OPEN_GRAPH_OBJECT: {
				String objectStr = (String)arg[0].item;
				String objectName;

				// Replace * by node object
				if ( objectStr.charAt(0) == '*') {
				    CdhrObjid cdhr_node = gdh.getNodeObject(0);
				    if ( cdhr_node.evenSts())
					break;
				    CdhrString cdhr_nodestr = gdh.objidToName( cdhr_node.objid, Cdh.mName_volumeStrict);
				    objectName = cdhr_nodestr.str + objectStr.substring(1);
				}
				else
				    objectName = objectStr;

				String attrName = objectName + ".Action";
				CdhrString cdhr = gdh.getObjectInfoString( attrName);
				if ( cdhr.evenSts()) {
				    System.out.println("Object name error");
				    break;
				}
				int idx = cdhr.str.lastIndexOf( ".pwg");
				if ( idx == -1)
				    break;

				String fname = cdhr.str;

				attrName = objectName + ".Object";
				cdhr = gdh.getObjectInfoString( attrName);
				String instance = null;
				if ( cdhr.oddSts() && !cdhr.str.equals(""))
				    instance = cdhr.str;

				new ReaderTask().execute(new ReaderTaskArg(ReaderTask.GROW_READ, fname, instance));
				break;
			}
			case EVENTHANDLER: {
				GdhEventArg event = (GdhEventArg)arg[0].item;
				try {
				  semDraw.acquire();
				}
				catch ( InterruptedException e) {
				    System.out.println("InterruptedException");
				}
				currentCmn.eventHandler(event.action, event.x, event.y);
				semDraw.release();
				break;
			}
			case VALUEINPUT_ACTION: {
			        String value = (String)arg[0].item;
				int sts = ((Dyn)valueInputDyn).valueInputAction( valueInputObject, value);
				switch ( sts) {
				case Dyn.eValueInput_Success:
				    // valueInputDia.dispose();
				    break;
				case Dyn.eValueInput_Error:
				    openMessageDialog("Unable to set value");
				    break;
				case Dyn.eValueInput_SyntaxError:
				    openMessageDialog("Syntax error");
				    break;
				case Dyn.eValueInput_MinValueExceeded:
				    openMessageDialog("Minimum value exceeded");
				    break;
				case Dyn.eValueInput_MaxValueExceeded:
				    openMessageDialog("Maximum value exceeded");
				    break;
				}				
				break;
			}
			case CONFIRM_ACTION: {
				((Dyn)confirmDyn).confirmedAction( Glow.eEvent_MB1Click, confirmObject);
				break;
			}
			case CLASS_HELP: {
			        int classid = 0;
			        AXttItemBase item = (AXttItemBase)arg[0].item;
				if ( item instanceof AXttItemObject)
				    classid = ((AXttItemObject)item).getClassId();
				else if ( item instanceof AXttItemAttrObject)
				    classid = ((AXttItemAttrObject)item).getClassId();
				if ( classid != 0) {
				    CdhrObjid oret = gdh.classIdToObjid( classid);
				    if ( oret.evenSts()) break;

				    CdhrString sret = gdh.objidToName( oret.objid, Cdh.mName_volumeStrict); 
				    if ( sret.evenSts()) break;
				    
				    int idx = sret.str.lastIndexOf(":");
				    if ( idx == -1) break;
				    
				    String volume = sret.str.substring( 0, idx);
				    idx = sret.str.lastIndexOf("-");
				    if ( idx == -1) break;

				    String className = sret.str.substring(idx + 1);

				    String url;

				    if ( isBaseClassVolume( volume))
					url = "$pwr_doc/" + getLang() + "/orm/" + volume.toLowerCase() + "_" + 
					    className.toLowerCase() + ".html";
				    else
					url = "http://" + pwrHost + "/pwrp_web/" + volume.toLowerCase() + "_" + 
					    className.toLowerCase() + ".html";

				    openURL( url, null);
				}
				break;
			}
			case FLOW_CLASSHELP: {
			        int classid = 0;
				FlowNode node = (FlowNode)arg[0].item;

				PwrtObjid nodeObjid = node.getCmn().getObjid();
				CdhrString rName = gdh.objidToName(nodeObjid, Cdh.mName_pathStrict);
				if (rName.evenSts())
					break;
				String name = rName.str + "-" + node.getName();
				CdhrObjid oret = gdh.nameToObjid(name);
				if ( oret.evenSts()) {
				    String oName = node.getTraceObject();

				    oret = gdh.nameToObjid(oName);
				    if ( oret.evenSts()) break;
				}

				CdhrClassId cret = gdh.getObjectClass(oret.objid);
				if (cret.evenSts())
				    break;

				switch ( cret.getClassId()) {
				case Pwrb.cClass_stodv:
				case Pwrb.cClass_stodi:
				case Pwrb.cClass_stodo:
				case Pwrb.cClass_setdv:
				case Pwrb.cClass_setdi:
				case Pwrb.cClass_setdo:
				case Pwrb.cClass_resdv:
				case Pwrb.cClass_resdi:
				case Pwrb.cClass_resdo:
				case Pwrb.cClass_stoav:
				case Pwrb.cClass_stoai:
				case Pwrb.cClass_stoao:
				case Pwrb.cClass_stodp:
				case Pwrb.cClass_setdp:
				case Pwrb.cClass_resdp:
				case Pwrb.cClass_stoap:
				case Pwrb.cClass_stoip:
				case Pwrb.cClass_cstoav:
				case Pwrb.cClass_cstoai:
				case Pwrb.cClass_cstoao:
				case Pwrb.cClass_cstoap:
				case Pwrb.cClass_cstoip:
				case Pwrb.cClass_GetData:
				    String oName = node.getTraceObject();

				    oret = gdh.nameToObjid(oName);
				    break;
				default: ;
				}

				if ( oret.evenSts()) break;

				cret = gdh.getObjectClass(oret.objid);
				if (cret.evenSts())
				    break;

				oret = gdh.classIdToObjid( cret.getClassId());
				if ( oret.evenSts()) break;

				CdhrString sret = gdh.objidToName( oret.objid, Cdh.mName_volumeStrict); 
				if ( sret.evenSts()) break;
				    
				int idx = sret.str.lastIndexOf(":");
				if ( idx == -1) break;
				    
				String volume = sret.str.substring( 0, idx);
				idx = sret.str.lastIndexOf("-");
				if ( idx == -1) break;

				String className = sret.str.substring(idx + 1);
				String url;

				if ( isBaseClassVolume( volume))				    
				    url = "$pwr_doc/" + getLang() + "/orm/" + volume.toLowerCase() + "_" + 
					className.toLowerCase() + ".html";
				else
				    url = "http://" + pwrHost + "/pwrp_web/" + volume.toLowerCase() + "_" + 
					className.toLowerCase() + ".html";

				openURL( url, null);
				break;
			}
			case LOGIN: {
				String[] item = (String[])arg[0].item;
				int sts = gdh.login( item[0], item[1]);
				if ( sts % 2 == 0) {
					openMessageDialog("Authorization error");
				}
				else {
					currentUser = item[0];
					newCurrentUser = true;
				}
				break;
			}
			case OPWIN: {
				// Find AppGraph object under WebHandler, search on two hierarchy levels
				CdhrObjid oret = gdh.getClassList( Pwrb.cClass_WebHandler);
				if (oret.oddSts()) {
					
				        CdhrString sret = gdh.objidToName(oret.objid, Cdh.mName_volumeStrict);
					if ( sret.evenSts()) break;
				        String appUseWebDir = sret.str + ".AppUseWebDir";

				        CdhrBoolean bret = gdh.getObjectInfoBoolean(appUseWebDir);
				        if ( bret.oddSts() && bret.value == true) {
					    pwr_exe="pwrp_web/";
					    pwrp_exe="pwrp_web/";
					    pwrp_load="pwrp_web/";
				        }

					for ( oret = gdh.getChild(oret.objid); oret.oddSts(); oret = gdh.getNextSibling(oret.objid)) {
						CdhrClassId cret = gdh.getObjectClass(oret.objid);
						if (cret.evenSts())
						    break;

						switch ( cret.getClassId()) {
						case Pwrb.cClass_AppGraph:
						case Pwrb.cClass_AppLink:
						    addGraphList(oret.objid, cret.getClassId());
						    break;
						case Pwrs.cClass_PlantHier:
						case Pwrs.cClass_NodeHier:
						    for ( CdhrObjid oret2 = gdh.getChild(oret.objid); 
							  oret2.oddSts(); 
							  oret2 = gdh.getNextSibling(oret2.objid)) {
							CdhrClassId cret2 = gdh.getObjectClass(oret2.objid);
							if (cret2.oddSts() && 
							    (cret2.getClassId() == Pwrb.cClass_AppGraph ||
							     cret2.getClassId() == Pwrb.cClass_AppLink))
							    addGraphList(oret2.objid, cret2.getClassId());
						    }
						    break;
						default: ;
						}
						    
					}					
					if ( graphList.size() > 0) {
					        opwinCmn = new OpwinCmn(appl, graphList, density);
						setScanTime( 1);
						currentCmn = opwinCmn;
						cmnList.add(opwinCmn);
					}
				}

				break;
			}
			}
			return null;
		}

		@Override
		protected void onPostExecute(Void params) {
			super.onPostExecute(params);
			if ( newCurrentUser) {
				newCurrentUser = false;
				if ( currentUser != null)
					appl.setTitle("PwrXtt " + currentUser + " on " + pwrHost);
				else
					appl.setTitle("PwrXtt on " + pwrHost);
			}
			// if ( currentCmn.type() != PlowCmnIfc.TYPE_GRAPH )
			  view.invalidate();
		}
	}

    private void addGraphList( PwrtObjid objid, int cid) {
	CdhrString sret = gdh.objidToName(objid, Cdh.mName_volumeStrict);
	
	String name = sret.str;
	AGraphInfo info = new AGraphInfo();
	String aName = name + ".Image";
	sret = gdh.getObjectInfoString(aName);
	if ( sret.evenSts())
	    return;

	String image = sret.str;
	if ( image.equals("")) {
	    info.bpm = BitmapFactory.decodeResource(getResources(), R.drawable.graph_icon);
	    info.bpmInverted = invertBitmap(info.bpm);
	}
	else {
	    URL url = null;
	    try {
		if ( image.startsWith("$pwr_exe/")) {
		    // url = new URL("http://10.0.2.2/data0/x4-8-6/rls/os_linux/hw_x86/exp/exe/" + filename.substring(9));
		    url = new URL("http://" + pwrHost + "/" + pwr_exe + image.substring(9));
		}
		else {
		    // url = new URL("http://10.0.2.2/data0/pwrp/opg7/bld/x86_linux/exe/" + filename);
		    url = new URL("http://" + pwrHost + "/" + pwrp_exe + image);
		}
		info.bpm = BitmapFactory.decodeStream(url.openConnection().getInputStream());
		info.bpmInverted = invertBitmap(info.bpm);
	    } catch ( IOException e) {
		info.bpm = BitmapFactory.decodeResource(getResources(), R.drawable.graph_icon);
		info.bpmInverted = invertBitmap(info.bpm);
		System.out.println( "Unable to open file " + image  + " " + url);
	    }
	}
	if ( cid == Pwrb.cClass_AppGraph) {
	    info.type = AGraphInfo.TYPE_GRAPH;
	    aName = name + ".Name";
	    sret = gdh.getObjectInfoString(aName);
	    if ( sret.evenSts())
		return;
	    info.graph = sret.str;
	}
	else if ( cid== Pwrb.cClass_AppLink) {
	    info.type = AGraphInfo.TYPE_LINK; 
	    aName = name + ".URL";
	    sret = gdh.getObjectInfoString(aName);
	    if ( sret.evenSts())
		return;
	    info.graph = sret.str;
	}
	
	aName = name + ".Text";
	sret = gdh.getObjectInfoString(aName);
	if ( sret.evenSts())
	    return;
	
	info.text = sret.str;
							
	graphList.add(info);
    }

	private class ReaderTaskArg {
		int action;
		Object data1;
		Object data2;
		
		public ReaderTaskArg(int action, Object data1, Object data2) {
			this.action = action;
			this.data1 = data1;
			this.data2 = data2;
		}
	}
	
	private class ReaderTask extends AsyncTask<ReaderTaskArg, Void, Void> {
		public static final int FLOW_READ = 1;
		public static final int CRR_READ = 2;
		public static final int GROW_READ = 3;
				
		@Override
		protected Void doInBackground(ReaderTaskArg... arg) {

			switch ( arg[0].action) {
			case FLOW_READ:
				readFlow((PwrtObjid)arg[0].data1, (String)arg[0].data2);
				break;
			case GROW_READ:
				readGrow((String)arg[0].data1, (String)arg[0].data2);
				break;
			}
			return null;
		}

		@Override
		protected void onPostExecute(Void params) {
			super.onPostExecute(params);
			//cmn.draw();
			view.invalidate();
		}
	}
	public Resources getApplResources() {
		return getResources();
	}
	
	@Override
	public boolean onTouchEvent(MotionEvent me) {
	    if ( me == null)
		return true;

	    int action = me.getActionMasked();
	    float x = me.getX();
	    float y = me.getY();
	    boolean isSustained = false;		
	    boolean sliderActive = false;

	    if ( lastMotionEvent == me && eventCnt > 0) {
		isSustained = true;
		x = me.getX();
		y = me.getY();
		System.out.println("Sustain motion event: " + eventCnt + " " + lastXSpeed + " " + lastYSpeed);
		System.out.println("x,y " + lastMotionEvent.getX() + " " + lastMotionEvent.getY());
		eventCnt--;
		if ( eventCnt == 0) {
		    lastMotionEvent = null;
		    lastXSpeed = 0;
		    lastYSpeed = 0;
		    eventMode = MODE_NO;
		}
		else {
		    lastXSpeed = 0.8 * lastXSpeed;
		    lastYSpeed = 0.8 * lastYSpeed;
		    lastMotionEvent = MotionEvent.obtain( lastMotionEvent.getDownTime(), 
							  lastMotionEvent.getEventTime(),
							  lastMotionEvent.getAction(),
							  (float)(lastMotionEvent.getX() + lastXSpeed * 50),
							  (float)(lastMotionEvent.getY() + lastYSpeed * 50),
							  lastMotionEvent.getMetaState());
		    
		    System.out.println("x,y " + lastMotionEvent.getX() + " " + lastMotionEvent.getY());
		    eventMode = MODE_SCROLL;
		    eventTimer.schedule( new EventTimerTask(), new Date(System.currentTimeMillis() + 50));
		}
	    }
	    
	    if ( viewOffsetY == 0) {
		if ( context != null) {
		    Window window= getWindow();
		    viewOffsetY = window.findViewById(Window.ID_ANDROID_CONTENT).getTop();
		
		}
		else
		    viewOffsetY = 56;
	    }

	    switch (action) {
	    case MotionEvent.ACTION_MOVE:

		if ( isSustained)
		    System.out.println("Sustained scroll");
		if ( eventMode == MODE_SCROLL) {
		    if ( currentCmn.type() == PlowCmnIfc.TYPE_GRAPH &&
			 ((GraphCmn)currentCmn).graph.getClickActive() == 1) {
			//eventMode = MODE_NO;
		    }

		    if ( lastTouchId != me.getPointerId(0)) {
			lastTouchX = x;
			lastTouchY = y;
			lastTouchId = me.getPointerId(0);
			break;
		    }
		    boolean scroll = true;
		    if ( currentCmn.type() == PlowCmnIfc.TYPE_GRAPH ) {
			if ( ((GraphCmn)currentCmn).getAppMotion() == Glow.eAppMotion_Slider || 
			     (((GraphCmn)currentCmn).getAppMotion() == Glow.eAppMotion_Both &&
			      ((GraphCmn)currentCmn).getSliderActive())) {
			    new GdhTask().execute(new GdhTaskArg(GdhTask.EVENTHANDLER, 
								 new GdhEventArg(GraphCmn.ACTION_MOVE, x, y-viewOffsetY)));
			    scroll = false;
			    sliderActive = true;
			}
		    }
		    if ( scroll && !sliderActive) {
			System.out.println("Scroll " + x + " " + y);
			if ( (int)(lastTouchY - y) != 0) {
			    currentCmn.scroll((int)(lastTouchX -x), (int)(lastTouchY - y));
			    view.invalidate();
			}
			lastTouchX = x;
			lastTouchY = y;
			lastTouchId = me.getPointerId(0);
		    }
		    if ( !isSustained && !sliderActive) {
			if ( lastMotionEvent != null) {
			    lastXSpeed = 0.5 * lastXSpeed + 0.5 * (x - lastMotionEvent.getX()) / (me.getEventTime() - lastMotionEvent.getEventTime());
			    lastYSpeed = 0.5 * lastYSpeed + 0.5 * (y - lastMotionEvent.getY()) / (me.getEventTime() - lastMotionEvent.getEventTime());			  			
			}
			lastMotionEvent = MotionEvent.obtain( me.getDownTime(), me.getEventTime(),
							      me.getAction(), me.getX(), me.getY(),
							      me.getMetaState());				    
		    }
		    
		}
		else if ( eventMode == MODE_ZOOM) {
		    float distance = eventDistance(me);

		    if ( currentCmn.type() == PlowCmnIfc.TYPE_GRAPH &&
			 ((GraphCmn)currentCmn).getAppMotion() == Glow.eAppMotion_Slider)
			currentCmn.zoom(distance/eventDistance);
		    else
			currentCmn.zoom(distance/eventDistance,
					(me.getX(0) + me.getX(1))/2, 
					(me.getY(0) + me.getY(1))/2 + viewOffsetY);
		    eventDistance = distance;
		    view.invalidate();
		}
		
		break;
	    case MotionEvent.ACTION_UP:
		if ( currentCmn == null)
		    break;
		
		if ( currentCmn.type() == PlowCmnIfc.TYPE_GRAPH){
		    System.out.println("Event Up   " + action + " (" + x + "," + y + ") cmn " + currentCmn.type());
		    if ( Math.abs(x - downTouchX) < 10 && Math.abs(y - downTouchY) < 10 && 
			 me.getEventTime() - me.getDownTime() < 700) {
			System.out.println("Event Click   " + action + " (" + x + "," + y + ") cmn " + currentCmn.type());
			new GdhTask().execute(new GdhTaskArg(GdhTask.EVENTHANDLER, 
							     new GdhEventArg(GraphCmn.ACTION_CLICK, x, y-viewOffsetY)));					
		    }
		    else {
			new GdhTask().execute(new GdhTaskArg(GdhTask.EVENTHANDLER, 
							     new GdhEventArg(GraphCmn.ACTION_UP, x, y-viewOffsetY)));
		    }
		}
		else if ( currentCmn.type() == PlowCmnIfc.TYPE_OPWIN){
		    currentCmn.eventHandler(PlowCmnIfc.ACTION_UP, x, y-viewOffsetY);
		}
		else {
		    if ( Math.abs(x - downTouchX) < 10 && Math.abs(y - downTouchY) < 10 && currentCmn != null &&
			 me.getEventTime() - me.getDownTime() > 500) {
			currentCmn.eventHandler(PlowCmnIfc.ACTION_LONG_CLICK, x, y-viewOffsetY);
		    }
		    else if ( Math.abs(x - downTouchX) < 10 && Math.abs(y - downTouchY) < 10 && currentCmn != null) {
			currentCmn.eventHandler(PlowCmnIfc.ACTION_CLICK, x, y-viewOffsetY);
		    }
		}
		view.invalidate();
		eventMode = MODE_NO;
		sliderActive = false;
		
		if ( lastMotionEvent != null) {
		    System.out.println("ACTION UP, sustained motion event scheduled");
		    //lastXSpeed = (downTouchX - x) / (me.getDownTime() - me.getEventTime());
		    //lastYSpeed = (downTouchY - y) / (me.getDownTime() - me.getEventTime());			  			
		    eventCnt = (int)(10 * FloatMath.sqrt((float)(lastXSpeed * lastXSpeed + lastYSpeed * lastYSpeed)));
		    long dt = me.getEventTime() - lastMotionEvent.getEventTime();
		    lastMotionEvent = MotionEvent.obtain( lastMotionEvent.getDownTime(), 
							  lastMotionEvent.getEventTime(),
							  lastMotionEvent.getAction(),
							  (float)(lastMotionEvent.getX() + lastXSpeed * dt),
							  (float)(lastMotionEvent.getY() + lastYSpeed * dt),
							  lastMotionEvent.getMetaState());
		    
		    eventTimer.schedule( new EventTimerTask(), new Date(System.currentTimeMillis() + 50));
		    eventMode = MODE_SCROLL;
		}
		break;
	    case MotionEvent.ACTION_DOWN:
	    System.out.println("offset : " + viewOffsetY + " (x,y) (" + x + "," + y + ")");
		lastMotionEvent = null;
		lastXSpeed = 0;
		lastYSpeed = 0;
		if ( currentCmn.type() == PlowCmnIfc.TYPE_GRAPH) {
		    new GdhTask().execute(new GdhTaskArg(GdhTask.EVENTHANDLER, 
							 new GdhEventArg(GraphCmn.ACTION_DOWN, x, y-viewOffsetY)));					
		}
		else if ( currentCmn.type() == PlowCmnIfc.TYPE_OPWIN) {
		    currentCmn.eventHandler(PlowCmnIfc.ACTION_DOWN, x, y-viewOffsetY);
		    view.invalidate();
		}
		lastTouchId = me.getPointerId(0);
		lastTouchX = x;
		lastTouchY = y;
		downTouchX = x;
		downTouchY = y;
		eventMode = MODE_SCROLL;
		break;
	    case MotionEvent.ACTION_POINTER_DOWN:
		System.out.println("Event Action Pointer Down");
		lastMotionEvent = null;
		lastXSpeed = 0;
		lastYSpeed = 0;
		
		eventDistance = eventDistance(me);
		if ( eventDistance > 10)
		    eventMode = MODE_ZOOM;
		break;
	    case MotionEvent.ACTION_POINTER_UP:
		System.out.println("Event Action Pointer Up");
		eventMode = MODE_SCROLL;
		int idx = me.getActionIndex();
		if ( idx == 0) {
		    lastTouchX = me.getX(1);
		    lastTouchY = me.getY(1);
		}
		else {
		    lastTouchX = x;
		    lastTouchY = y;
		}
		break;
	    }
	    return true;
	}
	private float eventDistance(MotionEvent me) {
	        float x = me.getX(0) - me.getX(1);
	        float y = me.getY(0) - me.getY(1);
		
		return FloatMath.sqrt(x * x + y * y);
	}
	
	class EventTimerTask extends TimerTask {
		@Override
		public void run() {
		runOnUiThread( new Runnable() {
		public void run() {
		    if ( lastMotionEvent != null)
			onTouchEvent( lastMotionEvent);
		}});
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}

	public void forceError() {
		if (true) {
			throw new Error("Whooops");			
		}
	}
	private class MainView extends SurfaceView {
		Bitmap bitmap = null;
	        Canvas drawCanvas, currentCanvas;
		
		public MainView(Context context) {
			super(context);
			appl.context = context;
			setWillNotDraw(false);
		}

	    @Override
		protected void onDraw( Canvas canvas) {
	    	if ( currentCmn != null) {

		    if ( currentCmn.type() == PlowCmnIfc.TYPE_GRAPH) {
	    			if ( bitmap == null) {
				    bitmap = Bitmap.createBitmap(canvas.getWidth(), canvas.getHeight(), Bitmap.Config.ARGB_8888);
				    drawCanvas = new Canvas(bitmap);
				    drawCanvas.drawColor(graph.cmn.gdraw.getColor(graph.cmn.background_color));
	    			}	    		
				try {
				  semDraw.acquire();
				}
				catch ( InterruptedException e) {
				    System.out.println("InterruptedException");
				}
	    			currentCmn.setCanvas(drawCanvas);
				graph.cmn.gdraw.set_clip();
				drawCanvas.drawColor(graph.cmn.gdraw.getColor(graph.cmn.background_color));
	    			currentCmn.draw();
				currentCanvas = getHolder().lockCanvas();
	    			currentCanvas.drawBitmap(bitmap,  0F, 0F, null);
				getHolder().unlockCanvasAndPost(currentCanvas);
				semDraw.release();
	    		}
	    		else {

			    Canvas currentCanvas = getHolder().lockCanvas();
			    //if ( currentCanvas == null)
			    //	currentCanvas = getHolder().lockCanvas();

			    if ( currentCanvas != null) {
				    
				if ( currentCmn.type() == PlowCmnIfc.TYPE_GRAPH)
				    currentCanvas.drawColor(graph.cmn.gdraw.getColor(graph.cmn.background_color));
				else
				    currentCanvas.drawColor(Color.WHITE);
				
				if ( currentCmn.type() == PlowCmnIfc.TYPE_FLOW)
				    gdraw.setDensity(1, density);
				else
				    gdraw.setDensity(density * density, density);
				currentCmn.setCanvas(currentCanvas);
				currentCmn.draw();
	    	
				/* Test */
				   Paint paint = new Paint();
				   currentCanvas.drawText( "Density " + density, 10, 100, paint);
				   /* */

				getHolder().unlockCanvasAndPost(currentCanvas);
				// currentCanvas = getHolder().lockCanvas(); 
			    }
	    		}
	    	}	

	    	}
	}
	
	
	class MyTask extends TimerTask {
		@Override
		public void run() {
		runOnUiThread( new Runnable() {
		public void run() {
			if ( !initDone)
				return;
			switch(currentCmn.type()) {
			case PlowCmnIfc.TYPE_PLOW:
				new GdhTask().execute(new GdhTaskArg(GdhTask.SCAN, 
						(AXttItemBase)null));
				view.invalidate();
				break;
			case PlowCmnIfc.TYPE_FLOW:
			case PlowCmnIfc.TYPE_GRAPH:
				new GdhTask().execute(new GdhTaskArg(GdhTask.DYNAMIC_UPDATE,(AXttItemBase)null));
				view.invalidate();
				break;
			case PlowCmnIfc.TYPE_EV:
				view.invalidate();
				break;
			}
		}});
		}
	}

	public void readFlow( PwrtObjid windowObjid, String  object) {
/*
		AssetManager am = getAssets();
		InputStream inputStream;
		try {
		  inputStream = am.open(filename);
		}
		catch ( IOException e ) {
			System.out.println( "Unable to open file " + filename);
			return;
		}
		BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));
*/
		String filename = Flow.getFileName(windowObjid);
		BufferedReader reader;
		URL url = null;
		try {
			// url = new URL("http://10.0.2.2/data0/pwrp/opg7/bld/common/load/" + filename);
			url = new URL("http://" + pwrHost + "/" + pwrp_load + filename);
			reader = new BufferedReader(new InputStreamReader(url.openStream(), "ISO-8859-1"));
		} catch ( IOException e) {
			System.out.println( "Unable to open file " + filename  + " " + url);
			return;			
		}
		FlowCmn fcmn = new FlowCmn(this, gdh, windowObjid);
		setScanTime( 1);
		currentCmn = fcmn;
		cmnList.add(fcmn);
		fcmn.setGDraw(gdraw);
		FlowCtx ctx = new FlowCtx(fcmn);
		ctx.open(reader);

		if ( object != null) {
			FlowNode node = (FlowNode)ctx.getObject(object);
			if ( node != null) {
			    fcmn.select(node);
			    node.setSelect(true);
			    ctx.centerObject(node);			
			}
		}		
		
		new GdhTask().execute(new GdhTaskArg(GdhTask.DYNAMIC_OPEN, null));
	}

	public void readGrow( String file, String  instance) {
/*
		AssetManager am = getAssets();
		InputStream inputStream;
		try {
		  inputStream = am.open(filename);
		}
		catch ( IOException e ) {
			System.out.println( "Unable to open file " + filename);
			return;
		}
		BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));
*/
		String filename = file;
		BufferedReader reader;
		URL url = null;
		try {
			if ( filename.startsWith("$pwr_exe/")) {
				// url = new URL("http://10.0.2.2/data0/x4-8-6/rls/os_linux/hw_x86/exp/exe/" + filename.substring(9));
				url = new URL("http://" + pwrHost + "/" + pwr_exe + filename.substring(9));
			}
			else {
				// url = new URL("http://10.0.2.2/data0/pwrp/opg7/bld/x86_linux/exe/" + filename);
				url = new URL("http://" + pwrHost + "/" + pwrp_exe + filename);
			}
			reader = new BufferedReader(new InputStreamReader(url.openStream(), "ISO-8859-1"));
		} catch ( IOException e) {
			System.out.println( "Unable to open file " + filename  + " " + url);
			return;			
		}
		graph = new Graph(this, gdh);
		graphObject.add(instance);
		graph.setOwner(instance);
		graph.open(reader);
	    graph.gdraw.setActivity(this);
	    GraphCmn cmn = new GraphCmn(graph);
	    currentCmn = cmn;
	    setScanTime( Math.max(graph.getAnimationScanTime(), 0.25));
		cmnList.add(cmn);

		// new GdhTask().execute(new GdhTaskArg(GdhTask.DYNAMIC_OPEN, null));
	}

    public void openMessageDialog(String text) {
	messageText = text;

	runOnUiThread( new Runnable() {
		public void run() {
		    new AlertDialog.Builder(appl)
			.setTitle(R.string.app_name)
			.setMessage(messageText)
			.setPositiveButton("Ok", null)
			.show();		    
		}});		
    }

    // GraphApplIfc interface
    public void openConfirmDialog( Object dyn, String text, Object object) {
 		confirmDyn = dyn;
		confirmObject = object;
		confirmText = text;

		runOnUiThread( new Runnable() {
		public void run() {
		    confirmDialog = new AlertDialog.Builder(appl)
			.setTitle(R.string.app_name)
			.setMessage(confirmText)
			.setPositiveButton("Yes", appl)
			.setNegativeButton("No", null)
			.show();
		    
		}});		
    }
    public void openValueInputDialog( Object dyn, String text, Object object) {	        
 		valueInputDyn = dyn;
		valueInputObject = object;

		runOnUiThread( new Runnable() {
		public void run() {
		    editValueInput = new EditText(appl);
		    editValueInput.setSingleLine();
		    valueInputDialog = new AlertDialog.Builder(appl)
			.setTitle(R.string.app_name)
			.setMessage("Change Value")
			.setView(editValueInput)
			.setPositiveButton("Ok", appl)
			.setNegativeButton("Cancel", null)
			.show();
		    
		}});		
    }

    public int getWidth() {
    	return 500;
    }
    public int getHeight() {
    	return 500;
    }
    public String getObject() {
    	if ( graphObject.size() > 0)
    		return graphObject.get(graphObject.size()-1);
    	else
    		return null;
    }
    
    public Object loadGrowCtx( String fname) {
	String filename = fname;
	BufferedReader reader;
	URL url = null;
	try {
	    if ( filename.startsWith("$pwr_exe/")) {
		// url = new URL("http://10.0.2.2/data0/x4-8-6/rls/os_linux/hw_x86/exp/exe/" + filename.substring(9));
		url = new URL("http://" + pwrHost + "/" + pwr_exe + filename.substring(9));
	    }
	    else {
		// url = new URL("http://10.0.2.2/data0/pwrp/opg7/bld/x86_linux/exe/" + filename);
		url = new URL("http://" + pwrHost + "/" + pwrp_exe + filename);
	    }
	    reader = new BufferedReader(new InputStreamReader(url.openStream(), "ISO-8859-1"));
	} catch ( IOException e) {
	    System.out.println( "Unable to open file " + filename  + " " + url);
	    return null;			
	}
	return graph.loadGrowCtx(reader);
    }

    public int loadSubgraph( String fname) {
	String filename = fname;
	BufferedReader reader;
	URL url = null;
	try {
	    if ( filename.startsWith("pwr_")) {
		// url = new URL("http://10.0.2.2/data0/x4-8-6/rls/os_linux/hw_x86/exp/exe/" + filename.substring(9));
		url = new URL("http://" + pwrHost + "/" + pwr_exe + filename);
	    }
	    else {
		// url = new URL("http://10.0.2.2/data0/pwrp/opg7/bld/x86_linux/exe/" + filename);
		url = new URL("http://" + pwrHost + "/" + pwrp_exe + filename);
	    }
	    reader = new BufferedReader(new InputStreamReader(url.openStream(), "ISO-8859-1"));
	} catch ( IOException e) {
	    System.out.println( "Unable to open file " + filename  + " " + url);
	    return 0;			
	}
	return graph.loadSubgraph(reader);
    }

    public int externCmd( String cmd) {
	return command( cmd);
    }
    public String defFilename( String filename) {
	String fname = "http://" + pwrHost + "/" + pwrp_exe + filename;
	if ( !fname.endsWith(".rtt_com"))
	    fname += ".rtt_com";
	return fname;
    }
    public void errorMessage( String msg, int severity) {
    }
    public int confirmDialog( String title, String text) {
	return 1;
    }
    public int getRootType() {
	return Ccm.ROOT_AAPP;
    }
    public int script(String script) {
      new JopgCcm( this, appl.gdh, null, script);
      return 1;
    }

    static CliTable[] cliTable = new CliTable[] { 
        new CliTable( "OPEN", new String[] {"cli_arg1", "cli_arg2", "/NAME", 
    	"/FILE", "/SCROLLBAR", "/WIDTH", "/HEIGHT", "/MENU", "/NAVIGATOR", 
    	"/CENTER", "/OBJECT", "/INSTANCE", "/NEW", "/CLASSGRAPH", "/ACCESS", "/PARENT"}),
        new CliTable( "EXIT", null),
        new CliTable( "HELP", new String[] {"cli_arg1", "cli_arg2", "cli_arg3",
    	"cli_arg4", "/HELPFILE", "/POPNAVIGATOR", "/BOOKMARK", "/INDEX",
            "/BASE", "/RETURNCOMMAND", "/WIDTH", "/HEIGHT", "/VERSION"}),
        new CliTable( "SET", new String[] {"cli_arg1", "cli_arg2",
        	"/NAME", "/VALUE", "/BYPASS", "/SOURCE", "/OBJECT"}),
        new CliTable( "EXAMPLE", new String[] {"/NAME", "/HIERARCHY"}),
        new CliTable( "CHECK", new String[] {"cli_arg1", "/METHOD", "/OBJECT"}),
        new CliTable( "CALL", new String[] {"cli_arg1", "/METHOD", "/OBJECT"}),
        new CliTable( "SET", new String[] {"cli_arg1", "dcli_arg2"}) 
      };

    public int command(String cmd) {
    	System.out.println("Command: " + cmd);
        boolean local_cmd = false;

        if ( !appl.gdh.isAuthorized(Pwr.mAccess_AllRt)) {
	    openMessageDialog("Not authorized");
	    return 0;
	}

	if ( cmd.charAt(0) == '@') {
	    // Execute a script
	    new JopgCcm( this, gdh, cmd.substring(1),  null);
	    return 1;
	}

        Cli cli = new Cli( cliTable);
        String command = cli.parse( cmd);
        if (cli.oddSts()) {
        	System.out.println("JopSpider1 : " + command);
        	if ( command.equals("OPEN")) {
        		if ( cli.qualifierFound("cli_arg1")) {

        			String jgraph = "JGRAPH";
        			String graph = "GRAPH";
        			String url = "URL";
        			String trend = "TREND";
        			String fast = "FAST";
        			String cli_arg1 = cli.getQualValue("cli_arg1").toUpperCase();
        			if ( jgraph.length() >= cli_arg1.length() &&
        					jgraph.substring(0,cli_arg1.length()).equals(cli_arg1)) {
        				// Command is "OPEN JGRAPH"
        				System.out.println("Command is not implemented, \"OPEN JGRAPH\"");
        			}
        			else if ( graph.length() >= cli_arg1.length() &&
        					graph.substring(0,cli_arg1.length()).equals(cli_arg1)) {
        				// Command is "OPEN GRAPH"
        				String graphName = null;
        				String instanceValue = null;
        				boolean classGraph = false;
					String objectValue = null;
					
        				if ( cli.qualifierFound("/OBJECT")) {
        					objectValue = cli.getQualValue("/OBJECT");
    						new GdhTask().execute(new GdhTaskArg(GdhTask.OPEN_GRAPH_OBJECT, objectValue));
						return 1;
					}
        				if ( cli.qualifierFound("/INSTANCE")) {
        					instanceValue = cli.getQualValue("/INSTANCE");
        					classGraph = cli.qualifierFound("/CLASSGRAPH");		      
        					boolean parent = cli.qualifierFound("/PARENT");
        					if ( parent) {
        						int idx = instanceValue.lastIndexOf( '.');
        						if ( idx != -1 && idx != 0)
        							instanceValue = instanceValue.substring( 0, idx);
        						System.out.println( "open graph /parent: " + instanceValue);
        					}
        				}
        				if ( classGraph) {
    						new GdhTask().execute(new GdhTaskArg(GdhTask.OPEN_CLASSGRAPH_NAME, instanceValue));
        				}
        				else {
        					if ( ! cli.qualifierFound("cli_arg2")) {
        						System.out.println("Syntax error");
        						return 0;
        					}
        					// Back to basic cmn
        					int cmnIdx = 0;
        					if ( opwinCmn != null)
        						cmnIdx = 1;
        					
						setScanTime( 1);
        					currentCmn = cmnList.get(cmnIdx);
        					for ( int i = cmnList.size()-1; i > cmnIdx; i--)
        						cmnList.removeElementAt(i);
        					for ( int i = graphObject.size()-1; i >= 0; i--)
        						graphObject.removeElementAt(i);
        					
        					graphName = cli.getQualValue("cli_arg2").toLowerCase();
        					if ( !graphName.contains(".pwg"))
        						graphName = graphName + ".pwg";

        					new ReaderTask().execute(new ReaderTaskArg(ReaderTask.GROW_READ, graphName, instanceValue));
        				}
        			}
        			else if ( url.length() >= cli_arg1.length() &&
        					url.substring(0,cli_arg1.length()).equals(cli_arg1)) {
        				// Command is "OPEN URL"
        				Boolean newFrame = true;
        				String frameName = null;
        				String urlValue = cli.getQualValue("cli_arg2");

					if ( urlValue.startsWith("pwrb_") ||
					     urlValue.startsWith("pwrs_") ||
					     urlValue.startsWith("nmps_") ||
					     urlValue.startsWith("profibus_") ||
					     urlValue.startsWith("otherio_") ||
					     urlValue.startsWith("opc_") ||
					     urlValue.startsWith("basecomponent_") ||
					     urlValue.startsWith("abb_") ||
					     urlValue.startsWith("siemens_") ||
					     urlValue.startsWith("ssabox_"))
					    // Object reference manual
					    urlValue = "$pwr_doc/" + getLang() + "/orm/" + urlValue;

        				System.out.println("open url " + urlValue);
					// Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(urlValue));
					//startActivity(browserIntent);
					openURL( urlValue, null); 
				}
        			else if ( trend.length() >= cli_arg1.length() &&
        					trend.substring(0,cli_arg1.length()).equals(cli_arg1)) {
        				// Command is "OPEN TREND"
        			}
        			else if ( fast.length() >= cli_arg1.length() &&
        					fast.substring(0,cli_arg1.length()).equals(cli_arg1)) {
        				// Command is "OPEN FAST"
        			}
        			else if ( command.equals("HELP")) {
        			}
        		}
        	}
        	else if ( command.equals("SET")) {
        		if ( cli.qualifierFound("cli_arg1")) {
			    String cli_arg1 = cli.getQualValue("cli_arg1").toUpperCase();
			    String subwindow = "SUBWINDOW";
			    if ( subwindow.length() >= cli_arg1.length() &&
				 subwindow.substring(0,cli_arg1.length()).equals(cli_arg1)) {
				// Command is "SET SUBWINDOW"
				if ( currentCmn.type() != PlowCmnIfc.TYPE_GRAPH)
				    return 0;

				String name;
				String graphstr;
				String source;
				String object;
				PwrtStatus sts;

				System.out.println("Command: set subwindow");
				local_cmd = true;
				if ( cli.qualifierFound("/NAME"))
				    name = cli.getQualValue("/NAME");
				else {
				    System.out.println( "Cmd: name is missing\n");
				    return 0;
				}
				if ( cli.qualifierFound("/SOURCE"))
				    source = cli.getQualValue("/SOURCE");
				else {
				    System.out.println( "Cmd: source is missing\n");
				    return 0;
				}
				if ( cli.qualifierFound("/OBJECT"))
				    object = cli.getQualValue("/OBJECT");
				else
				    object = null;
				if ( cli.qualifierFound("cli_arg2"))
				    graphstr = cli.getQualValue("cli_arg2").toLowerCase();
				else {
				    System.out.println("Syntax error");
				    return 0;
				}
				
				if ( source.indexOf('.') == -1)
				    source = source + ".pwg";

				((GraphCmn)currentCmn).graph.setSubwindowSource( name, source, object);
			    }
			}
        	}
		else if ( command.equals("HELP")) {
		    String fileName = "xtt_help_";
		    String bookmarkValue = null;

		    if ( cli.qualifierFound("/VERSION")) {
			fileName = pwrHost + "/pwr_doc/xtt_version_help_version.html";
			openURL( fileName, null);
		    }
		    else {
			if ( cli.qualifierFound("/BASE"))
			    // Not language dependent !! TODO
			    fileName = pwrHost + "/pwr_doc/help/xtt_help_";
			else
			    fileName = pwrHost + "/pwrp_web/xtt_help_";
			
			if ( cli.qualifierFound("cli_arg1"))
			    fileName += cli.getQualValue("cli_arg1").toLowerCase();
			if ( cli.qualifierFound("cli_arg2"))
			    fileName += "_" + cli.getQualValue("cli_arg2").toLowerCase();
			if ( cli.qualifierFound("cli_arg3"))
			    fileName += "_" + cli.getQualValue("cli_arg3").toLowerCase();
			if ( cli.qualifierFound("cli_arg4"))
			    fileName += "_" + cli.getQualValue("cli_arg4").toLowerCase();
			
			if ( fileName.startsWith("pwrb_") ||
			     fileName.startsWith("pwrs_") ||
			     fileName.startsWith("nmps_") ||
			     fileName.startsWith("profibus_") ||
			     fileName.startsWith("otherio_") ||
			     fileName.startsWith("opc_") ||
			     fileName.startsWith("basecomponent_") ||
			     fileName.startsWith("abb_") ||
			     fileName.startsWith("siemens_") ||
			     fileName.startsWith("ssabox_"))
			    // Object reference manual
			    fileName = "$pwr_doc/orm/" + fileName;
			
			if ( cli.qualifierFound("/BOOKMARK"))
			    bookmarkValue = cli.getQualValue("/BOOKMARK");
			
			System.out.println( "Loading helpfile \"" + fileName + "\"");
			openURL( fileName, bookmarkValue);
		    }
		    local_cmd = true;
		}
        	else if ( command.equals("CHECK")) {
        		if ( cli.qualifierFound("cli_arg1")) {

        			String methodstr = "METHOD";
        			String isattributestr = "ISATTRIBUTE";
        			String cli_arg1 = cli.getQualValue("cli_arg1").toUpperCase();
        			if ( methodstr.length() >= cli_arg1.length() &&
        					methodstr.substring(0,cli_arg1.length()).equals(cli_arg1)) {
        				// Command is "CHECK METHOD"
        				String method;
        				String object;

        				if ( cli.qualifierFound("/METHOD"))
        					method = cli.getQualValue("/METHOD");
        				else {
        					System.out.println( "Cmd: Method is missing\n");
        					return 0;
        				}

        				if ( cli.qualifierFound("/OBJECT"))
        					object = cli.getQualValue("/OBJECT");
        				else {
        					System.out.println( "Cmd: Object is missing\n");
        					return 0;
        				}
        			}	
        			else if ( isattributestr.length() >= cli_arg1.length() &&
        					isattributestr.substring(0,cli_arg1.length()).equals(cli_arg1)) {
        				// Command is "CHECK ISATTRIBUTE"
        				String method;
        				String object;

        				if ( cli.qualifierFound("/OBJECT"))
        					object = cli.getQualValue("/OBJECT");
        				else {
        					System.out.println( "Cmd: Object is missing\n");
        					return 0;
        				}
        			}
        		}
        		else if ( command.equals("CALL")) {
        			if ( cli.qualifierFound("cli_arg1")) {

        				String parameter = "METHOD";
        				String cli_arg1 = cli.getQualValue("cli_arg1").toUpperCase();
        				if ( parameter.length() >= cli_arg1.length() &&
        						parameter.substring(0,cli_arg1.length()).equals(cli_arg1)) {
        					// Command is "CHECK METHOD"
        					String method;
        					String object;

        					if ( cli.qualifierFound("/METHOD"))
        						method = cli.getQualValue("/METHOD");
        					else {
        						System.out.println( "Cmd: Method is missing\n");
        						return 0;
        					}

        					if ( cli.qualifierFound("/OBJECT"))
        						object = cli.getQualValue("/OBJECT");
        					else {
        						System.out.println( "Cmd: Object is missing\n");
        						return 0;
        					}
        				}

        			}
        		}
        		else if ( command.equals("SET")) {
        			return 1;
        		}
        	}
        	else {
        		System.out.println( "JopSpider: Parse error " + cli.getStsString());
        		return 0;
        	}
        }
        return 1;
    }

    public void closeGraph() {
	setScanTime( 1);
	new GdhTask().execute(new GdhTaskArg(GdhTask.DYNAMIC_CLOSE, currentCmn));
	if (graphObject.size() > 0)
	    graphObject.removeElementAt(graphObject.size()-1);
	currentCmn = cmnList.get(cmnList.size()-2);
	cmnList.removeElementAt(cmnList.size()-1);
	//view.invalidate();
    }

    public boolean isAuthorized(int access) {
	return gdh.isAuthorized(access);
    }

    public void openPopupMenu( String object, double x, double y) {
    }

    public void invalidateView() {
    	view.invalidate();
    }
    
    // GdhApplIfc functions
    @Override
    public String getPwrHost() {
    	return pwrHost;		    	
    }
    
    public PlowCmnIfc getCurrentCmn() {
    	return currentCmn;
    }

    public Bitmap invertBitmap(Bitmap src) {
	Bitmap output = Bitmap.createBitmap(src.getWidth(), src.getHeight(), src.getConfig());
	int A, R, G, B;
	int pixelColor;
	int height = src.getHeight();
	int width = src.getWidth();

	for (int y = 0; y < height; y++) {
	    for (int x = 0; x < width; x++) {
		pixelColor = src.getPixel(x, y);
		A = Color.alpha(pixelColor);
                
		R = Math.max( Color.red(pixelColor) - 50, 0);
		G = Math.max( Color.green(pixelColor) - 50, 0);
		B = Math.max( Color.blue(pixelColor) - 50, 0);
                
		output.setPixel(x, y, Color.argb(A, R, G, B));
	    }
	}
	
	return output;
    }  

    public void pushCmn() {
	switch(currentCmn.type()) {
	case PlowCmnIfc.TYPE_PLOW: {
	    if ( cmnList.size() == 1) {
		// Open opwin
		if ( opwinCmn != null) {
		    setScanTime( 1);
		    currentCmn = opwinCmn;
		    cmnList.add(opwinCmn);
		    view.invalidate();
		}
		break;
	    }

	    PlowCmn cmn = (PlowCmn)currentCmn;
	    AXtt axtt = (AXtt)currentCmn.getUserData();
	    PlowNode o = (PlowNode)cmn.get(0);
	    AXttItemObject itemo = (AXttItemObject)o.getUserData();
	    itemo.close(axtt);
	    setScanTime( 1);
	    currentCmn = cmnList.get(cmnList.size()-2);
	    cmnList.removeElementAt(cmnList.size()-1);
	    System.out.println( "cmnList.size() " + cmnList.size());
	    view.invalidate();				
	    break;
	}
	case PlowCmnIfc.TYPE_FLOW:
	    new GdhTask().execute(new GdhTaskArg(GdhTask.DYNAMIC_CLOSE, currentCmn));
	    System.out.println( "cmnList.size() " + cmnList.size());
	    setScanTime( 1);
	    currentCmn = cmnList.get(cmnList.size()-2);
	    cmnList.removeElementAt(cmnList.size()-1);
	    System.out.println( "cmnList.size() " + cmnList.size());
	    view.invalidate();
	    System.out.println("Close flow");
	    break;
	case PlowCmnIfc.TYPE_EV:
	    System.out.println( "cmnList.size() " + cmnList.size());				
	    setScanTime( 1);
	    currentCmn = cmnList.get(cmnList.size()-1);
	    aev = null;
	    view.invalidate();
	    System.out.println("Close ev");
	    break;
	case PlowCmnIfc.TYPE_GRAPH:
	    new GdhTask().execute(new GdhTaskArg(GdhTask.DYNAMIC_CLOSE, currentCmn));
	    System.out.println( "cmnList.size() " + cmnList.size());
	    if (graphObject.size() > 0)
		graphObject.removeElementAt(graphObject.size()-1);
	    setScanTime( 1);
	    currentCmn = cmnList.get(cmnList.size()-2);
	    cmnList.removeElementAt(cmnList.size()-1);
	    System.out.println( "cmnList.size() " + cmnList.size());
	    view.invalidate();
	    System.out.println("Close graph");
	    break;
	case PlowCmnIfc.TYPE_OPWIN:
	    // Close app ?
	    break;
	}
    }

    void loadUrlSymbols() {

	// Get language from webhandler object
	CdhrObjid webHandler = gdh.getClassList( Pwrb.cClass_WebHandler);
	if ( webHandler.evenSts()) return;

	CdhrString webHandlerName = gdh.objidToName( webHandler.objid, Cdh.mName_volumeStrict);
	if ( webHandlerName.evenSts()) return;

	String attr = webHandlerName.str + ".Language";

	CdhrInt iAttrValue = gdh.getObjectInfoInt( attr);
	if ( iAttrValue.evenSts()) return;

	lang = iAttrValue.value;

	// Get URL Symbols
	CdhrObjid webConfig = gdh.getClassList( Pwrb.cClass_WebBrowserConfig);
	if ( webConfig.evenSts()) return;

	CdhrString webName = gdh.objidToName( webConfig.objid, Cdh.mName_volumeStrict);
	if ( webConfig.evenSts()) return;

	urlSymbols = new String[10];
	for ( int i = 0; i < 10; i++) {
	    attr = webName.str + ".URL_Symbols[" + i + "]";
	    CdhrString attrValue = gdh.getObjectInfoString( attr);
	    if ( attrValue.evenSts()) return;
	    
	    urlSymbols[i] = attrValue.str;
	}
    }
	    
    String replaceUrlSymbol( String url) {
	if ( urlSymbols == null)
	    return url;

	for ( int i = 0; i < 10; i++) {
	    if ( urlSymbols[i].equals(""))
		continue;
	    
	    StringTokenizer token = new StringTokenizer( urlSymbols[i]);
	    String symbol = "$" + token.nextToken();
	    if ( !token.hasMoreTokens()) 
		continue;
  
	    String value = token.nextToken();

	    int idx = url.lastIndexOf( symbol);
	    while ( idx != -1) {
		url = url.substring( 0, idx) + value + url.substring( idx + symbol.length());
		idx = url.lastIndexOf( symbol);
	    }
	}
	return url;
    }

    void openURL( String name, String bookmark) {
	System.out.println("openURL " + name);

	// Replace any URL symbol
	name = replaceUrlSymbol( name);

	String url_str = null;
	if ( name.startsWith("http:") || name.startsWith("https:")) {
	    url_str = name;
	    if ( url_str.lastIndexOf(".html") == -1 &&
		 url_str.lastIndexOf(".shtml") == -1 &&
		 url_str.lastIndexOf(".htm") == -1 &&
		 url_str.lastIndexOf(".gif") == -1 &&
		 url_str.lastIndexOf(".jpg") == -1 &&
		 url_str.lastIndexOf(".png") == -1 &&
		 url_str.lastIndexOf(".pdf") == -1)
		url_str = url_str + ".html";
	}
	else if ( name.startsWith("$pwr_doc/")) {
	    url_str = "http://" + pwrHost + "/pwr_doc/" + name.substring(9);	      
	    if ( url_str.lastIndexOf(".html") == -1 &&
		 url_str.lastIndexOf(".shtml") == -1 &&
		 url_str.lastIndexOf(".htm") == -1 &&
		 url_str.lastIndexOf(".pdf") == -1)
		url_str = url_str + ".html";
	}
	else {
	    String path = "http://";
	    
	    if ( name.lastIndexOf(".html") == -1 &&
		 name.lastIndexOf(".shtml") == -1 &&
		 name.lastIndexOf(".htm") == -1 &&
		 name.lastIndexOf(".gif") == -1 &&
		 name.lastIndexOf(".jpg") == -1 &&
		 name.lastIndexOf(".png") == -1 &&
		 name.lastIndexOf(".pdf") == -1)
		url_str = new String( path + name + ".html");
	    else
		url_str = new String( path + name);
	    if ( bookmark != null)
		url_str += "#" + bookmark;
	}
	System.out.println( "Opening URL: " + url_str);
	
	Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(url_str));
	startActivity(browserIntent);
    }
    
    boolean isBaseClassVolume(String vname) {
	String v = vname.toLowerCase();
	if ( v.equals("pwrb") ||
	     v.equals("pwrs") ||
	     v.equals("nmps") ||
	     v.equals("profibus") ||
	     v.equals("otherio") ||
	     v.equals("opc") ||
	     v.equals("basecomponent") ||
	     v.equals("abb") ||
	     v.equals("siemens") ||
	     v.equals("miscellaneous") ||
	     v.equals("ssabox"))
	    return true;
	return false;
    }
		
    String getLang() {
	final int LANGUAGE_en_US = 45;
        final int LANGUAGE_de_DE = 31;
	final int LANGUAGE_fr_FR = 76;
        final int LANGUAGE_sv_SE = 119;

	switch ( lang) {
	case LANGUAGE_en_US:
	    return new String("en_us");
	case LANGUAGE_de_DE:
	    return new String("de_de");
	case LANGUAGE_fr_FR:
	    return new String("fr_fr");
	case LANGUAGE_sv_SE:
	    return new String("sv_se");
	default:
	    return new String("en_us");
	}
    }
}

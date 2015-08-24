package il.ac.technion.nssl.btproxy.dummyapp;

import java.io.IOException;
import java.io.OutputStream;
import java.util.UUID;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.widget.TextView;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;

public class MainActivity extends Activity {
	TextView out;
	private static final int REQUEST_ENABLE_BT = 1;
	private BluetoothAdapter btAdapter = null;
	private BluetoothSocket btSocket = null;
	private OutputStream outStream = null;
	   
	// Well known SPP UUID
	private static final UUID MY_UUID =
	UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
	//UUID.fromString("00000000-0000-0000-0000-000000000000");
	
	// Insert your server's MAC address
	// private static String address = "00:11:67:8A:6D:49"; // Lab computer
	// private static String address = "C0:F8:DA:C6:41:AB"; // Itay's personal laptop
	//private static String address = "80:86:F2:25:22:80"; // Itay's work laptop
	//private static String address = "E4:D5:3D:CA:0B:11"; // Avishay's work laptop
	private static String address = "00:11:67:55:90:57"; // CSR Dongle
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		out = (TextView) findViewById(R.id.out);
		out.append("\n...In onCreate()...");
		
		btAdapter = BluetoothAdapter.getDefaultAdapter();
		CheckBTState();
	}

	
	@Override
	protected void onStart() {
		super.onStart();
		out.append("\n...In onStart()...");
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		out.append("\n...In onResume...\n...Attempting client connect...");
		
		// Set up a pointer to the remote node using it's address.
	    BluetoothDevice device = btAdapter.getRemoteDevice(address);
	    
	    // Two things are needed to make a connection:
	    //   A MAC address, which we got above.
	    //   A Service ID or UUID.  In this case we are using the
	    //     UUID for SPP.
	    try {
	      btSocket = device.createRfcommSocketToServiceRecord(MY_UUID);
	    } catch (IOException e) {
	      AlertBox("Fatal error", "In onResume() and socket create failed: " + e.getMessage() + ".");
	    }
	    out.append("\n...Created socket...");
	    // Discovery is resource intensive.  Make sure it isn't going on
	    // when you attempt to connect and pass your message.
	    btAdapter.cancelDiscovery();
	    out.append("\n...Canceled discovery...");
	    // Establish the connection.  This will block until it connects.
	    try {
	      btSocket.connect();
	      out.append("\n...Connection established and data link opened...");
	    } catch (IOException e) {
	      try {
	        btSocket.close();
	      } catch (IOException e2) {
	        AlertBox("Fatal error", "In onResume() and unable to close socket during connection failure" + e2.getMessage() + ".");
	      }
	    }
	    
	    // Create a data stream so we can talk to server.
	    out.append("\n...Sending message to server...");
	 
	    try {
	      outStream = btSocket.getOutputStream();
	    } catch (IOException e) {
	      AlertBox("Fatal error", "In onResume() and output stream creation failed:" + e.getMessage() + ".");
	    }
	 
	    String message = "Hello from Android.\n";
	    byte[] msgBuffer = message.getBytes();
	    
	    try {
	    	out.append("\n...Trying write...");
	    	outStream.write(msgBuffer);
	    } catch (IOException e) {
	    	String msg = "In onResume() and an exception occurred during write: " + e.getMessage();
	    	msg = msg +  ".\n\nCheck that the SPP UUID: " + MY_UUID.toString() + " exists on server.\n\n";
    		AlertBox("Fatal Error", "write failed - " + msg);       
	    }	    
	}
	
	@Override
	protected void onPause() {
		super.onPause();
		out.append("\n...In onPause()...");
		 
	    if (outStream != null) {
	      try {
	        outStream.flush();
	      } catch (IOException e) {
	        AlertBox("Fatal error", "In onPause() and failed to flush output stream: " + e.getMessage() + ".");
	      }
	    }
	 
	    try     {
	      btSocket.close();
	    } catch (IOException e2) {
	      AlertBox("Fatal error", "In onPause() and failed to close socket." + e2.getMessage() + ".");
	    }
	}
	
	
	@Override
	protected void onStop() {
		super.onStop();
		out.append("\n...In onStop()...");
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		out.append("\n...In onDestroy()...");
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
	
	private void CheckBTState() {
	    // Check for Bluetooth support and then check to make sure it is turned on
	 
	    // Emulator doesn't support Bluetooth and will return null
	    if(btAdapter==null) { 
	      AlertBox("Fatal error", "Bluetooth Not supported. Aborting.");
	    } else {
	      if (btAdapter.isEnabled()) {
	        out.append("\n...Bluetooth is enabled...");
	      } else {
	        //Prompt user to turn on Bluetooth
	        Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
	        startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
	      }
	    }
	  }
	   
	  public void AlertBox( String title, String message ){
	    new AlertDialog.Builder(this)
	    .setTitle( title )
	    .setMessage( message + " Press OK to exit." )
	    .setPositiveButton("OK", new OnClickListener() {
	        public void onClick(DialogInterface arg0, int arg1) {
	          finish();
	        }
	    }).show();
	  }
	
}
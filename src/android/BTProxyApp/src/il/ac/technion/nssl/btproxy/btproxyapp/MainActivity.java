package il.ac.technion.nssl.btproxy.btproxyapp;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.util.Enumeration;
import java.util.UUID;

import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;



// =============================================================================
// MainActivity class
// =============================================================================
public class MainActivity extends Activity {
	private Button btnRestart;
	final Context context = this;
	// -------------------------------------------------------------------------
	// TCP variables
	// -------------------------------------------------------------------------
	private TextView tvServerIP,tvServerPort;
	private ServerSocket TCPServerSocket;
	private Socket TCPClientSocket = null;
	private final int SERVER_PORT = 4020; //Define the server port
	private boolean receivedTCPmsg = false;
	private String  TCPmessage = null;
	private Runnable TCPinitiator = new Runnable() {
		@Override
		public void run() {
			try {
				android.util.Log.e("TrackingFlow", "entered TCPinitiator");
				//Create a server socket object and bind it to a port
				TCPServerSocket = new ServerSocket(SERVER_PORT);
				TCPClientSocket = TCPServerSocket.accept();
				//Get the data input stream coming from the client 
				BufferedReader in = new BufferedReader(new InputStreamReader(TCPClientSocket.getInputStream()));
				//Read the contents of the data buffer
				android.util.Log.e("TrackingFlow", "before read");
				
				final StringBuilder sb = new StringBuilder();
				String result = in.readLine() + System.getProperty("line.separator");
				android.util.Log.e("TrackingFlow", "after read");
				sb.append(result);
				TCPmessage = result;
				receivedTCPmsg = true;
				// close tcp connections?
				in.close();
				TCPClientSocket.close();
				//Show message on UIThread
				runOnUiThread(new Runnable() {	
					@Override
					public void run() {
						AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(context);
						alertDialogBuilder.setTitle("Message from TCP client");
						alertDialogBuilder.setMessage(sb.toString());
						alertDialogBuilder.setCancelable(true);
						alertDialogBuilder.setNeutralButton("Close", new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog,int id) {
								// if this button is clicked, just close
								// the dialog box and do nothing
								dialog.cancel();
							}
						});
						// create alert dialog
						AlertDialog alertDialog = alertDialogBuilder.create();
						// show it
						alertDialog.show();
					}
				});
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	};
	// -------------------------------------------------------------------------
	// BT variables
	// -------------------------------------------------------------------------
	BluetoothAdapter adapter = null;
	private TextView tvFoundBT;
	private static String address = "D0:C1:B1:4B:EB:23";
	private BluetoothSocket BTSocket;
	private OutputStreamWriter os;
	private BluetoothDevice remoteDevice;
	private BroadcastReceiver discoveryResult = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			android.util.Log.e("TrackingFlow", "Entered onReceive");
			unregisterReceiver(this);
			//remoteDevice = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
			remoteDevice = adapter.getRemoteDevice(address);
			new Thread(BTinitiator).start();
			android.util.Log.e("TrackingFlow", "Started initiator");
		}
	};
	private Runnable BTinitiator = new Runnable() {
		@Override
		public void run() {
			try {
				android.util.Log.e("TrackingFlow", "Found: " + remoteDevice.getName());
				UUID uuid = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
				BTSocket = remoteDevice.createRfcommSocketToServiceRecord(uuid);
				runOnUiThread(new Runnable() {	
					@Override
					public void run() {
						tvFoundBT.append("\n" + "Name: " + remoteDevice.getName() + "\nAddress : " + remoteDevice.getAddress());
					}
				});
				BTSocket.connect();
				android.util.Log.e("TrackingFlow", "Connected...");
				os = new OutputStreamWriter(BTSocket.getOutputStream());
				android.util.Log.e("TrackingFlow", "waiting for TCP message...");
				while (!receivedTCPmsg);
				android.util.Log.e("TrackingFlow", "sending the following : " + TCPmessage);
				os.write(TCPmessage);
				os.flush();
				android.util.Log.e("TrackingFlow", "finished sending");
				BTSocket.close();
				android.util.Log.e("TrackingFlow", "Closed BT socket");
			}
			catch (IOException e) {e.printStackTrace();}
		}
	};


	
	// -------------------------------------------------------------------------
	// OnCreate
	// -------------------------------------------------------------------------
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		tvServerIP = (TextView) findViewById(R.id.textViewServerIP);
		tvServerPort = (TextView) findViewById(R.id.textViewServerPort);
		tvServerPort.append(" " + Integer.toString(SERVER_PORT));
		tvFoundBT = (TextView) findViewById(R.id.textViewFoundBT);
		btnRestart = (Button) findViewById(R.id.btnRestart);
		final String initBTfoundText = (String) tvFoundBT.getText(); 
		
		btnRestart.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				android.util.Log.e("TrackingFlow", "Restarting TCP server...");
				tvFoundBT.setText(initBTfoundText);
				try{
					if(BTSocket != null){
							os.close();
							BTSocket.close();
					}
					if (TCPClientSocket != null) {
						TCPClientSocket.close();
						if (TCPServerSocket != null)
							TCPServerSocket.close();
					}
				} catch(Exception e) {
					android.util.Log.e("TrackingFlow", "onClick exception when closing sockets: " + e.getMessage());
				}
				receivedTCPmsg = false;
				new Thread(TCPinitiator).start();
				
				// BT discovery
				registerReceiver(discoveryResult, new IntentFilter(BluetoothDevice.ACTION_FOUND));
				
				adapter = BluetoothAdapter.getDefaultAdapter();
				if(adapter != null && adapter.isDiscovering()) {
					adapter.cancelDiscovery();
				}
				adapter.startDiscovery();
			}
		});

		//Find IP
		getDeviceIpAddress();
		
		//New thread to listen to incoming connections
		new Thread(TCPinitiator).start();
		
		// BT discovery
		registerReceiver(discoveryResult, new IntentFilter(BluetoothDevice.ACTION_FOUND));
		
		adapter = BluetoothAdapter.getDefaultAdapter();
		if(adapter != null && adapter.isDiscovering()){
			adapter.cancelDiscovery();
		}
		adapter.startDiscovery();
		
	}
	// -------------------------------------------------------------------------
	// OnDestory
	// -------------------------------------------------------------------------
	@Override
	protected void onDestroy() {
		super.onDestroy();
		/*try {
			unregisterReceiver(discoveryResult);
		} catch(Exception e) {
			android.util.Log.e("TrackingFlow", "onDestroy exception in unregisterReciever : " + e.getMessage());
		}*/
		try{
			if(BTSocket != null){
					os.close();
					BTSocket.close();
			}
			if (TCPClientSocket != null) {
				TCPClientSocket.close();
				if (TCPServerSocket != null)
					TCPServerSocket.close();
			}
		} catch(Exception e) {
			android.util.Log.e("TrackingFlow", "onDestroy exception when closing sockets: " + e.getMessage());
		}
	}

	// -------------------------------------------------------------------------
	// getDeviceIpAddress
	// -------------------------------------------------------------------------
	public void getDeviceIpAddress() {
		try {
			//Loop through all the network interface devices
			for (Enumeration<NetworkInterface> enumeration = NetworkInterface
					.getNetworkInterfaces(); enumeration.hasMoreElements();) {
				NetworkInterface networkInterface = enumeration.nextElement();
				//Loop through all the ip addresses of the network interface devices
				for (Enumeration<InetAddress> enumerationIpAddr = networkInterface.getInetAddresses(); enumerationIpAddr.hasMoreElements();) {
					InetAddress inetAddress = enumerationIpAddr.nextElement();
					//Filter out loopback address and other irrelevant ip addresses 
					if (!inetAddress.isLoopbackAddress() && inetAddress.getAddress().length == 4) {
						//Print the device ip address in to the text view 
						tvServerIP.append(" " + inetAddress.getHostAddress());
					}
				}
			}
		} catch (SocketException e) {
			Log.e("ERROR:", e.toString());
		}
	}
	// -------------------------------------------------------------------------
	// onCreateOptionsMenu
	// -------------------------------------------------------------------------
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	// -------------------------------------------------------------------------
	// onOptionsItemSelected
	// -------------------------------------------------------------------------	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
}



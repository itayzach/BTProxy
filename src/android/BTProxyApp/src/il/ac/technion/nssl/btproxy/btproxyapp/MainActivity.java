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
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncTask;
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
	// -------------------------------------------------------------------------
	// TCP variables
	// -------------------------------------------------------------------------
	private TextView tvClientMsg,tvServerIP,tvServerPort;
	private final int SERVER_PORT = 4020; //Define the server port
	private boolean receivedTCPmsg = false;
	private String  TCPmessage = null;
	
	// -------------------------------------------------------------------------
	// BT variables
	// -------------------------------------------------------------------------
	BluetoothAdapter adapter = null;
	private static String address = "D0:C1:B1:4B:EB:23";
	private boolean CONTINUE_READ_WRITE = true;
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
	// -------------------------------------------------------------------------
	// initiator
	// -------------------------------------------------------------------------	
	private Runnable BTinitiator = new Runnable() {

		@Override
		public void run() {
			try {
				
				android.util.Log.e("TrackingFlow", "Found: " + remoteDevice.getName());
				UUID uuid = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
				BTSocket = remoteDevice.createRfcommSocketToServiceRecord(uuid);
				BTSocket.connect();
				android.util.Log.e("TrackingFlow", "Connected...");
				os = new OutputStreamWriter(BTSocket.getOutputStream());
				new Thread(writter).start();
				android.util.Log.e("TrackingFlow", "Started new writter thread");
				
			}
			catch (IOException e) {e.printStackTrace();}
		}
	};
	
	// -------------------------------------------------------------------------
	// writter
	// -------------------------------------------------------------------------
	private Runnable writter = new Runnable() {

		@Override
		public void run() {
			// wait for TCP message
			android.util.Log.e("TrackingFlow", "waiting for message");
			while (!receivedTCPmsg);
			android.util.Log.e("TrackingFlow", "got message");
			while (CONTINUE_READ_WRITE) {
				try {
					os.write(TCPmessage);
					os.flush();
					Thread.sleep(2000);
					//CONTINUE_READ_WRITE = false;
					
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
			try {
				BTSocket.close();
				android.util.Log.e("TrackingFlow", "Closed BT socket");
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	};

	private Runnable initiator = new Runnable() {
		@Override
		public void run() {
			try {
				//Create a server socket object and bind it to a port
				ServerSocket socServer = new ServerSocket(SERVER_PORT);
				//Create server side client socket reference
				Socket socClient = null;
				//Infinite loop will listen for client requests to connect
				while (true) {
					//Accept the client connection and hand over communication to server side client socket
					socClient = socServer.accept();
					//For each client new instance of AsyncTask will be created
					ServerAsyncTask serverAsyncTask = new ServerAsyncTask();
					//Start the AsyncTask execution 
					//Accepted client socket object will pass as the parameter
					serverAsyncTask.execute(new Socket[] {socClient});
					
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	};
	// -------------------------------------------------------------------------
	// ServerAsyncTask class
	// handles the TCP communication with clients
	// -------------------------------------------------------------------------
	class ServerAsyncTask extends AsyncTask<Socket, Void, String> {
		//Background task which serve for the client
		@Override
		protected String doInBackground(Socket... params) {
			android.util.Log.e("TrackingFlow", "entered doInBackground");
			String result = null;
			//Get the accepted socket object 
			Socket TCPSocket = params[0];
			try {
				//Get the data input stream coming from the client 
				InputStream is = TCPSocket.getInputStream();
				//Buffer the data input stream
				BufferedReader br = new BufferedReader(
						new InputStreamReader(is));
				//Read the contents of the data buffer
				android.util.Log.e("TrackingFlow", "before read");
				result = br.readLine();
				TCPmessage = result;
				receivedTCPmsg = true;
				android.util.Log.e("TrackingFlow", "after read");
				
				//Close the client connection
				TCPSocket.close();
				CONTINUE_READ_WRITE = false;
			} catch (IOException e) {
				e.printStackTrace();
			}
			return result;
		}

		@Override
		protected void onPostExecute(String s) {
			//After finishing the execution of background task data will be write the text view
			tvClientMsg.append(s);
			
		}
	}
	
	// -------------------------------------------------------------------------
	// OnCreate
	// -------------------------------------------------------------------------
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		tvClientMsg = (TextView) findViewById(R.id.textViewClientMessage);
		tvServerIP = (TextView) findViewById(R.id.textViewServerIP);
		tvServerPort = (TextView) findViewById(R.id.textViewServerPort);
		tvServerPort.append(Integer.toString(SERVER_PORT));
		btnRestart = (Button) findViewById(R.id.btnRestart);
		
		btnRestart.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				android.util.Log.e("TrackingFlow", "Restarting TCP server...");
				if(BTSocket != null){
					try{
						os.close();
						BTSocket.close();
						CONTINUE_READ_WRITE = false;
					}catch(Exception e){}
				}
				finish();
				Intent intent = new Intent(MainActivity.this, MainActivity.class);
				startActivity(intent);
				
			}
		});

		
		//Find IP
		getDeviceIpAddress();
		//New thread to listen to incoming connections
		new Thread(initiator).start();
		
		
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
		try{unregisterReceiver(discoveryResult);}catch(Exception e){e.printStackTrace();}
		if(BTSocket != null){
			try{
				os.close();
				BTSocket.close();
				CONTINUE_READ_WRITE = false;
			}catch(Exception e){}
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
						tvServerIP.append(inetAddress.getHostAddress());
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



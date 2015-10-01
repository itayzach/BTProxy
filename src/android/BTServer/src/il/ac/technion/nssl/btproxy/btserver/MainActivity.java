package il.ac.technion.nssl.btproxy.btserver;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.UUID;

import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends Activity {
	
	private static final int DISCOVERABLE_REQUEST_CODE = 0x1;
	final Context context = this;
	private Button btnRestartServer;
	private TextView tvBTServerName;
	private TextView tvBTServerAddr;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		tvBTServerName = (TextView) findViewById(R.id.textViewBTname);
		tvBTServerAddr = (TextView) findViewById(R.id.textViewBTaddr);		
		btnRestartServer = (Button) findViewById(R.id.btnRestart);
		final String initBTServerNameString = (String) tvBTServerName.getText();
		final String initBTServerAddrString = (String) tvBTServerAddr.getText();
		btnRestartServer.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				android.util.Log.e("TrackingFlow", "Restarting BT server...");
				try{
					if(socket != null){
						socket.close();
					}
					if (serverSocket != null) {
						serverSocket.close();
					}
				} catch(Exception e) {
					android.util.Log.e("TrackingFlow", "onClick exception when closing sockets: " + e.getMessage());
				}
				tvBTServerName.setText(initBTServerNameString);
				tvBTServerAddr.setText(initBTServerAddrString);
				Intent discoverableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_DISCOVERABLE);
				startActivityForResult(discoverableIntent, DISCOVERABLE_REQUEST_CODE);
			}
		});
		//Always make sure that Bluetooth server is discoverable during listening...
		Intent discoverableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_DISCOVERABLE);
		startActivityForResult(discoverableIntent, DISCOVERABLE_REQUEST_CODE);
	}
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		android.util.Log.e("TrackingFlow", "Creating thread to start listening...");
		new Thread(reader).start();
	}
	
	@Override
	protected void onDestroy() {
		super.onDestroy();
		try{
			if(socket != null){
				socket.close();
			}
			if (serverSocket != null) {
				serverSocket.close();
			}
		} catch(Exception e) {
			android.util.Log.e("TrackingFlow", "onDestroy exception when closing sockets: " + e.getMessage());
		}
	}
	
	private BluetoothSocket socket;
	private BluetoothServerSocket serverSocket;
	private Runnable reader = new Runnable() {
		public void run() {
			final BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
			UUID uuid = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
			runOnUiThread(new Runnable() {
				@Override
				public void run() {
					tvBTServerName.append("\n" + adapter.getName());
					tvBTServerAddr.append("\n" + adapter.getAddress());
				}
			});
			try {
				serverSocket = adapter.listenUsingRfcommWithServiceRecord("BLTServer", uuid);
				android.util.Log.e("TrackingFlow", "Listening...");
				socket = serverSocket.accept();
				android.util.Log.e("TrackingFlow", "Socket accepted...");				
				BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
				final StringBuilder sb = new StringBuilder();
				String result = in.readLine() + System.getProperty("line.separator");
				android.util.Log.e("TrackingFlow", "after read");
				sb.append(result);
				
				//Show message on UIThread
				runOnUiThread(new Runnable() {	
					@Override
					public void run() {
						AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(context);
						alertDialogBuilder.setTitle("Message from BT client");
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
				
				in.close();
				socket.close();
				android.util.Log.e("TrackingFlow", "closed socket");
			} catch (IOException e) {
				android.util.Log.e("TrackingFlow", "got IOException : " + e.getMessage());
			}
		}
	};
	
	
}

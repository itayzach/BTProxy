package il.ac.technion.nssl.btproxy.btserver;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.util.UUID;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends Activity {
	
	private static final int DISCOVERABLE_REQUEST_CODE = 0x1;
	private boolean CONTINUE_READ_WRITE = true;
	private TextView tvClientMsg;
	private Button btnRestartServer;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		tvClientMsg = (TextView) findViewById(R.id.textViewClientMsg);
		btnRestartServer = (Button) findViewById(R.id.btnRestart);
		btnRestartServer.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				android.util.Log.e("TrackingFlow", "Restarting BT server...");
				if(socket != null){
					try{
						is.close();
						os.close();
						socket.close();
					}catch(Exception e){}
				}
				finish();
				Intent intent = new Intent(MainActivity.this, MainActivity.class);
		        startActivity(intent);
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
		if(socket != null){
			try{
				is.close();
				os.close();
				socket.close();
			}catch(Exception e){}
			CONTINUE_READ_WRITE = false;
		}
	}
	
	private BluetoothSocket socket;
	private InputStream is;
	private OutputStreamWriter os;
	private Runnable reader = new Runnable() {
		public void run() {
			BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
			//UUID uuid = UUID.fromString("4e5d48e0-75df-11e3-981f-0800200c9a66");
			UUID uuid = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
			try {
				BluetoothServerSocket serverSocket = adapter.listenUsingRfcommWithServiceRecord("BLTServer", uuid);
				android.util.Log.e("TrackingFlow", "Listening...");
				socket = serverSocket.accept();
				android.util.Log.e("TrackingFlow", "Socket accepted...");
				is = socket.getInputStream();
				//os = new OutputStreamWriter(socket.getOutputStream());
				//new Thread(writter).start();
				
				int bufferSize = 1024;
				int bytesRead = -1;
				byte[] buffer = new byte[bufferSize];
				//Keep reading the messages while connection is open...
				while(CONTINUE_READ_WRITE){
					android.util.Log.e("TrackingFlow", "CONTINUE_READ_WRITE");
					final StringBuilder sb = new StringBuilder();
					android.util.Log.e("TrackingFlow", "before first read");
					bytesRead = is.read(buffer);
					android.util.Log.e("TrackingFlow", "after first read");
					if (bytesRead != -1) {
						String result = "";
						while ((bytesRead == bufferSize) && (buffer[bufferSize-1] != 0)){
							result = result + new String(buffer, 0, bytesRead - 1);
							bytesRead = is.read(buffer);
							android.util.Log.e("TrackingFlow", "reading: " + sb.toString());
						}
						result = result + new String(buffer, 0, bytesRead - 1);
						sb.append(result);
						android.util.Log.e("TrackingFlow", "result: " + sb.toString());
					}
					android.util.Log.e("TrackingFlow", "Read: " + sb.toString());
					
					//Show message on UIThread
					runOnUiThread(new Runnable() {	
						@Override
						public void run() {
							tvClientMsg.append(sb.toString());
						}
					});
					//CONTINUE_READ_WRITE = false;
				}
			} catch (IOException e) {
				android.util.Log.e("TrackingFlow", "got IOException : " + e.getMessage());
			}
		}
	};
	
	
}

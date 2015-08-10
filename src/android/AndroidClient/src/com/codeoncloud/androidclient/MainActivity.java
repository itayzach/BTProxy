package com.codeoncloud.androidclient;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.net.UnknownHostException;
import android.support.v7.app.ActionBarActivity;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;

public class MainActivity extends ActionBarActivity {
	private TextView tvServerMessage;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		tvServerMessage = (TextView) findViewById(R.id.textViewServerMessage);
		//Create an instance of AsyncTask
		ClientAsyncTask clientAST = new ClientAsyncTask();
		//Pass the server ip, port and client message to the AsyncTask
		clientAST.execute(new String[] { "132.68.60.117", "4020","Hello from client" });
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
/**
 * AsyncTask which handles the communication with the server 
 */
	class ClientAsyncTask extends AsyncTask<String, Void, String> {
		@Override
		protected String doInBackground(String... params) {
			String result = null;
			try {
				//Create a client socket and define internet address and the port of the server
				Socket socket = new Socket(params[0],
						Integer.parseInt(params[1]));
				//Get the input stream of the client socket
				InputStream is = socket.getInputStream();
				//Get the output stream of the client socket
				PrintWriter out = new PrintWriter(socket.getOutputStream(),true);
				//Write data to the output stream of the client socket
				out.println(params[2]);	
				//Buffer the data coming from the input stream
				BufferedReader br = new BufferedReader(
						new InputStreamReader(is));
				//Read data in the input buffer
				result = br.readLine();
				//Close the client socket
				socket.close();
			} catch (NumberFormatException e) {
				e.printStackTrace();
			} catch (UnknownHostException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
			return result;
		}
		@Override
		protected void onPostExecute(String s) {
			//Write server message to the text view
			tvServerMessage.setText(s);
		}
	}
}

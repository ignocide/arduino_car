package com.example.carmanager;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.StringTokenizer;
import java.util.UUID;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.graphics.Matrix;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageView;
import android.widget.ImageView.ScaleType;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity {

	private static final int REQUEST_CONNECT_DEVICE = 1;
	private static final int REQUEST_ENABLE_BT = 2;

	private byte AttinyOut;
	private boolean connectStat = false;
	private TextView xAccel;
	protected static final int MOVE_TIME = 80;
	private long lastWrite = 0;
	private AlertDialog aboutAlert;
	private View aboutView;
	private View controlView;
	OnClickListener myClickListener;
	ProgressDialog myProgressDialog;
	private Toast failToast;
	private Handler mHandler;
	private Handler mHandler2;

	// Sensor object used to handle accelerometer
	// private SensorManager mySensorManager;
	// private List<Sensor> sensors;
	// private Sensor accSensor;

	// Bluetooth Stuff
	private BluetoothAdapter mBluetoothAdapter = null;
	private BluetoothSocket btSocket = null;
	private OutputStream outStream = null;
	private InputStream inStream = null;
	private ConnectThread mConnectThread = null;
//	 private ReadThread mReadThead = new ReadThread();
	private String deviceAddress = null;
	// Well known SPP UUID (will *probably* map to RFCOMM channel 1 (default) if
	// not in use);
	private static final UUID SPP_UUID = UUID
			.fromString("00001101-0000-1000-8000-00805F9B34FB");

	// public boolean state;
	TextView txtX, txtY;
	TextView total,val1,val2,val3,val4;
	TextView val_x,val_y,val_z;
	JoystickView joystick;
	ImageView xg,yg,zg;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
//		total = (TextView) findViewById(R.id.total);
		val1 = (TextView) findViewById(R.id.val1);
		val2 = (TextView) findViewById(R.id.val2);
		val3 = (TextView) findViewById(R.id.val3);
		val4 = (TextView) findViewById(R.id.val4);
		txtX = (TextView) findViewById(R.id.TextViewX);
		txtY = (TextView) findViewById(R.id.TextViewY);
		joystick = (JoystickView) findViewById(R.id.joystickView);
		joystick.setOnJostickMovedListener(joy_listen);
		xg = (ImageView) findViewById(R.id.xg);
		yg = (ImageView) findViewById(R.id.yg);
		val_x = (TextView) findViewById(R.id.val_x);
		val_y = (TextView) findViewById(R.id.val_y);

		
//		total.setText("11");
		myProgressDialog = new ProgressDialog(this);
		failToast = Toast.makeText(this, R.string.failedToConnect,
				Toast.LENGTH_SHORT);
        int count = 0;

		mHandler = new Handler() {
			@Override
			public void handleMessage(Message msg) {
				if (myProgressDialog.isShowing()) {
					myProgressDialog.dismiss();
				}

				// Check if bluetooth connection was made to selected device
				if (msg.what == 1) {
					// Set button to display current status
					connectStat = true;
					// connect_button.setText(R.string.connected);
					// Reset the BluCar
					AttinyOut = 0;
					// ledStat = false;
					write(AttinyOut);
				} 
				if(msg.what ==2) {
//					if(msg.obj.toString().length()!=0){
//	                byte[] readBuf = (byte[]) msg.obj;
	                // construct a string from the valid bytes in the buffer
//	                String readMessage = new String(readBuf, 0, msg.arg1);
					String[] values = msg.obj.toString().split("\n");
					String[] HT = values[2].split(" ");
					String[] accel = values[4].split(" ");
					val2.setText(HT[3]);
					val1.setText(HT[7]);
					val4.setText(values[0].trim());
					val3.setText(values[12]+"cm");

					String xa = accel[2];
					xa = xa.substring(0, xa.length()-1);
					float xa_i = Float.parseFloat(xa);
					String ya = accel[3];
					ya = ya.substring(0, ya.length()-1);
					float ya_i = Float.parseFloat(ya);
					
					String za = accel[4];
					float za_i = Float.parseFloat(za);

					
					float PI = 3.141592654f;
//					float pitch = (float) (Math.atan2(ya_i, za_i));
//					float roll = (float) (Math.atan2(xa_i, za_i));
					float pitch = (float) (Math.atan2(ya_i, za_i) * 180/PI);
					float roll = (float) (Math.atan2(xa_i, za_i) * 180/PI);

//					val_x.setText(String.valueOf(xa_i));
//					xg.setRotation(xa_i-45);
//					val_y.setText(String.valueOf(ya_i));
//					yg.setRotation(ya_i-45);					
//					val_z.setText(String.valueOf(za_i));
//					zg.setRotation(za_i-45);

					
					val_x.setText(String.valueOf(roll));
					xg.setRotation(roll+45);
					val_y.setText(String.valueOf(pitch));
					yg.setRotation(pitch+45);					
//					val_z.setText(String.valueOf(yaw));
//					zg.setRotation(yaw-45);

//					st4.nextToken();//
//					st4.nextToken();//
//					st4.nextToken();//
//					st4.nextToken();
//					st4.nextToken();
//					st4.nextToken();
//					total.setText(msg.obj.toString());
//					Log.i("!!!2", readMessage);
//					}
				}

				else {
					// Connection failed
					failToast.show();
				}
			}

		};
		
		mHandler2 = new Handler() {
			@Override
			public void handleMessage(Message msg) {
//				String str = msg.obj.toString();
//				if (msg.obj!=null) {
//					total.setText(str);
//			}
				
//				byte[] readBuf = (byte[]) msg.obj;
				// construct a string from the valid bytes in the buffer
//				String readMessage = new String(readBuf, 0, msg.arg1);
				total.setText(msg.obj.toString());

			}
		};

		mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
		if (mBluetoothAdapter == null) {
			Toast.makeText(this, R.string.no_bt_device, Toast.LENGTH_LONG)
					.show();
			finish();
			return;
		}

		// If BT is not on, request that it be enabled.
		if (!mBluetoothAdapter.isEnabled()) {
			Intent enableIntent = new Intent(
					BluetoothAdapter.ACTION_REQUEST_ENABLE);
			startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
		}

		/**********************************************************************
		 * Buttons for controlling BluCar
		 */

	}

	private JoystickMovedListener joy_listen = new JoystickMovedListener() {

		@Override
		public void OnMoved(int pan, int tilt) {
			AttinyOut = (byte) 0;
			if (pan >= 5)
				AttinyOut = (byte) (AttinyOut | 8);
			else if (pan <= -5)
				AttinyOut = (byte) (AttinyOut | 4);
			if (tilt <= -5)
				AttinyOut = (byte) (AttinyOut | 16);
			else if (tilt >= 5)
				AttinyOut = (byte) (AttinyOut | 32);
			write(AttinyOut);
			// txtX.setText(Integer.toString(pan));
			// txtY.setText(Integer.toString(tilt));
		}

		@Override
		public void OnReleased() {
			// txtX.setText("released");
			// txtY.setText("released");
			AttinyOut = (byte) 0;
			write(AttinyOut);
		}

		public void OnReturnedToCenter() {
			// txtX.setText("stopped");
			// txtY.setText("stopped");
			AttinyOut = (byte) 0;
			write(AttinyOut);
		};
	};

	public class ReadThread extends Thread {

		public void run() {
			byte[] buffer = new byte[1024];
			int bytes;
			
			try {
				while (true) {
					bytes = inStream.read(buffer);
                    mHandler.obtainMessage(2, bytes, -1, buffer)
                    .sendToTarget();					
                    total.setText(String.valueOf(buffer));
					Log.i("!!!", String.valueOf(buffer));
				}
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}


	public class ConnectThread extends Thread {
		private String address;
		private boolean connectionStatus;

		ConnectThread(String MACaddress) {
			address = MACaddress;
			connectionStatus = true;
		}

		public void run() {
			// When this returns, it will 'know' about the server,
			// via it's MAC address.
			try {
				BluetoothDevice device = mBluetoothAdapter
						.getRemoteDevice(address);

				try {
					btSocket = device
							.createRfcommSocketToServiceRecord(SPP_UUID);
				} catch (IOException e) {
					connectionStatus = false;
				}
			} catch (IllegalArgumentException e) {
				connectionStatus = false;
			}

			mBluetoothAdapter.cancelDiscovery();

			try {
				btSocket.connect();
			} catch (IOException e1) {
				try {
					btSocket.close();
				} catch (IOException e2) {
				}
			}

			// Create a data stream so we can talk to server.
			try {
				outStream = btSocket.getOutputStream();
				inStream = btSocket.getInputStream();
			} catch (IOException e2) {
				connectionStatus = false;
			}

			

			// Send final result
			if (connectionStatus) {
				mHandler.sendEmptyMessage(1);
			} else {
				mHandler.sendEmptyMessage(0);
			}
			
			byte[] buffer = new byte[1024];
			int bytes;
			
			try {
				while (true) {
					String str = "";
					for(int i = 0; i<50;i++)
					{
						bytes = inStream.read(buffer);
						byte[] tmp = buffer;
						String tmp_str= new String(buffer,0,bytes);
						str += tmp_str;
					}
					String[] values = str.split("cm ");
					
					Log.i("!!!", str);
					Log.i("!!!2",values[1]);
//					msg.obj = mHandler.obtainMessage(2, bytes, -1, buffer).obj.toString();
					mHandler.obtainMessage(2, (Object)values[1]).sendToTarget();;
//					Log.i("!!!2", msg.obj.toString());
//					mHandler2.sendMessage(msg);
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	public void onActivityResult(int requestCode, int resultCode, Intent data) {
		switch (requestCode) {
		case REQUEST_CONNECT_DEVICE:
			// When DeviceListActivity returns with a device to connect
			if (resultCode == Activity.RESULT_OK) {
				// Show please wait dialog
				myProgressDialog = ProgressDialog.show(this, getResources()
						.getString(R.string.pleaseWait), getResources()
						.getString(R.string.makingConnectionString), true);

				// Get the device MAC address
				deviceAddress = data.getExtras().getString(
						DeviceListActivity.EXTRA_DEVICE_ADDRESS);
				// Connect to device with specified MAC address
				mConnectThread = new ConnectThread(deviceAddress);
				// mReadThead = new ReadThread(deviceAddress);
				mConnectThread.start();
//				mReadThead.start();
			} else {
				// Failure retrieving MAC address
				Toast.makeText(this, R.string.macFailed, Toast.LENGTH_SHORT)
						.show();
			}
			break;
		case REQUEST_ENABLE_BT:
			// When the request to enable Bluetooth returns
			if (resultCode == Activity.RESULT_OK) {
				// Bluetooth is now enabled
			} else {
				// User did not enable Bluetooth or an error occured
				Toast.makeText(this, R.string.bt_not_enabled_leaving,
						Toast.LENGTH_SHORT).show();
				finish();
			}
		}
	}

	public void write(byte data) {
		if (outStream != null) {
			try {
				outStream.write(data);
			} catch (IOException e) {
			}
		}
	}

	public void emptyOutStream() {
		if (outStream != null) {
			try {
				outStream.flush();
			} catch (IOException e) {
			}
		}
	}

	public void connect() {
		// Launch the DeviceListActivity to see devices and do scan
		Intent serverIntent = new Intent(this, DeviceListActivity.class);
		startActivityForResult(serverIntent, REQUEST_CONNECT_DEVICE);
	}

	public void disconnect() {
		if (outStream != null) {
			try {
				outStream.close();
				connectStat = false;
				// connect_button.setText(R.string.disconnected);
			} catch (IOException e) {
			}
		}
	}

	// @Override
	// public boolean onCreateOptionsMenu(Menu menu) {
	// // Inflate the menu; this adds items to the action bar if it is present.
	// getMenuInflater().inflate(R.menu.main, menu);
	// return true;
	// }
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		menu.add(0, 1, 0, "Connect");
		menu.add(0, 2, 0, "Disconnect");
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case 1:
			connect();
			return true;
		case 2:
			disconnect();
			return true;
		}
		return false;
	}
}

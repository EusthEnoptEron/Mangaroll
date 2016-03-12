/************************************************************************************

Filename    :   MainActivity.java
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.


*************************************************************************************/
package oculus;

import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.content.Intent;
import com.oculus.vrappframework.VrActivity;

import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.regex.Pattern;

public class MainActivity extends VrActivity {
	public static final String TAG = "Mangaroll";

	/** Load jni .so on initialization */
	static {
		Log.d(TAG, "LoadLibrary");
		try {
			Thread.sleep(2000);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		System.loadLibrary("mangaroll");
	}

    public static native long nativeSetAppInterface( VrActivity act, String fromPackageNameString, String commandString, String uriString );

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

		Intent intent = getIntent();
		String commandString = VrActivity.getCommandStringFromIntent( intent );
		String fromPackageNameString = VrActivity.getPackageStringFromIntent( intent );
		String uriString = VrActivity.getUriStringFromIntent( intent );

		setAppPtr( nativeSetAppInterface( this, fromPackageNameString, commandString, uriString ) );
    }

	public static String InsertParam( String source, String paramName, String value) {
        return source.replaceAll(Pattern.quote(paramName), Uri.encode(value));
	}

	public static byte[] LoadHttpUrl( String str ) {
		int totalLen = 0;
		byte[] returnBuffer = new byte[totalLen];

		Log.d(TAG, "LoadHttpUrl " + str );
		try {
			URL aURL = new URL( str );
			HttpURLConnection conn = (HttpURLConnection)aURL.openConnection();
			conn.connect();
			InputStream is = conn.getInputStream();

			byte[]	readbuffer = new byte[0x100000];

			while( true ) {
				int count = is.read(readbuffer);
				if ( count < 0 ) {
					break;
				}
				byte[] tempBuffer = new byte[totalLen + count];
				System.arraycopy(returnBuffer,  0,  tempBuffer,  0,  totalLen );
				System.arraycopy(readbuffer,  0, tempBuffer,  totalLen,  count );
				totalLen += count;
				returnBuffer = tempBuffer;
			}

			is.close();
		} catch (Exception e) {
			Log.v(TAG, "LoadHttpUrl", e);
		}

		Log.d(TAG, "totalLen " + totalLen);

		return returnBuffer;
	}
}
